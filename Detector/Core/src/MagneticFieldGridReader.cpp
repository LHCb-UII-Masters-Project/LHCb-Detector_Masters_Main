/*****************************************************************************\
* (c) Copyright 2000-2018 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/

#include "Core/MagneticFieldGridReader.h"
#include "Core/FloatComparison.h"
#include "Core/MagneticFieldGrid.h"
#include "DD4hep/DD4hepUnits.h"
#include "DD4hep/Printout.h"
#include "Math/Vector3D.h"
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <string>

namespace {
  // Conversion factors needed to load the MagneticFieldGridData
  static inline constexpr double cm    = 10;
  static inline constexpr double gauss = 1e-7;
} // namespace

//=============================================================================
// Read the field map files and scale by scaleFactor
//=============================================================================

bool LHCb::Magnet::MagneticFieldGridReader::readFiles( const std::vector<std::string>&  filenames,
                                                       LHCb::Magnet::MagneticFieldGrid& grid ) const {

  GridQuadrants quadrants;
  assert( filenames.size() == quadrants.size() );

  bool sc = true;
  for ( std::size_t iquad = 0; iquad < quadrants.size() && sc; ++iquad ) {
    sc = readQuadrant( filenames[iquad], quadrants[iquad] );
  }

  if ( sc ) {
    // check that the quadrants are consistent
    for ( std::size_t iquad = 1; iquad < quadrants.size(); ++iquad ) {
      assert( essentiallyEqual( quadrants[0].zOffset, quadrants[iquad].zOffset ) );
      for ( std::size_t icoord = 0; icoord < quadrants[0].Dxyz.size(); ++icoord ) {
        assert( essentiallyEqual( quadrants[0].Dxyz[icoord], quadrants[iquad].Dxyz[icoord] ) );
        assert( quadrants[0].Nxyz[icoord] == quadrants[iquad].Nxyz[icoord] );
      }
    }

    // now fill the grid
    fillGridFromQuadrants( quadrants, grid );
  }

  return sc;
}

bool LHCb::Magnet::MagneticFieldGridReader::readDC06File( const std::string&               filename,
                                                          LHCb::Magnet::MagneticFieldGrid& grid ) const {
  GridQuadrants quadrants;

  // read the first quadrant
  const bool sc = readQuadrant( filename, quadrants[0] );
  if ( sc ) {
    // clone first quadrant to remaining 3
    for ( std::size_t iquad = 1; iquad < quadrants.size(); ++iquad ) { quadrants[iquad] = quadrants[0]; }
    // now fill the grid
    fillGridFromQuadrants( quadrants, grid );
  }

  return sc;
}

void LHCb::Magnet::MagneticFieldGridReader::fillConstantField( const ROOT::Math::XYZVector& /* field */,
                                                               LHCb::Magnet::MagneticFieldGrid& grid ) const {
  // make a grid that spans the entire world
  auto Dxyz      = std::array<float, 3>{2 * dd4hep::km, 2 * dd4hep::km, 2 * dd4hep::km};
  grid.m_invDxyz = {1.0f / Dxyz[0], 1.0f / Dxyz[1], 1.0f / Dxyz[2], 0.0f};
  grid.m_min     = {-dd4hep::km, -dd4hep::km, -dd4hep::km, 0.0f};
  grid.m_Nxyz    = {2, 2, 2};
  grid.m_B.clear();
  grid.m_B.resize( grid.m_Nxyz[0] * grid.m_Nxyz[1] * grid.m_Nxyz[2], {0, 0, 0, 0} );
}

////////////////////////////////////////////////////////////////////////////////////////
// routine to fill the grid from the 4 quadrants
////////////////////////////////////////////////////////////////////////////////////////
void LHCb::Magnet::MagneticFieldGridReader::fillGridFromQuadrants( GridQuadrants&                   quadrants,
                                                                   LHCb::Magnet::MagneticFieldGrid& grid ) const {
  // flip the signs of the field such that they are all in the same frame
  for ( std::size_t iquad = 1; iquad < quadrants.size(); ++iquad ) {
    int signx( 1 ), signz( 1 );
    switch ( iquad ) {
    case 1:
      signx = -1;
      break;
    case 2:
      signx = -1;
      signz = -1;
      break;
    case 3:
      signz = -1;
    }
    for ( auto& Q : quadrants[iquad].Q ) Q = {signx * Q.x(), Q.y(), signz * Q.z()};
  }

  // now we remap: put the 4 quadrants in a single grid
  const auto Nxquad = quadrants[0].Nxyz[0];
  const auto Nyquad = quadrants[0].Nxyz[1];
  const auto Nzquad = quadrants[0].Nxyz[2];

  // new number of bins. take into account that they overlap at z axis
  auto Dxyz      = std::array{(float)quadrants[0].Dxyz[0], (float)quadrants[0].Dxyz[1], (float)quadrants[0].Dxyz[2]};
  using N        = LHCb::Magnet::MagneticFieldGrid::IndexType;
  grid.m_invDxyz = {1.0f / Dxyz[0], 1.0f / Dxyz[1], 1.0f / Dxyz[2], 0.0f};
  grid.m_Nxyz    = {N( 2 * Nxquad - 1 ), N( 2 * Nyquad - 1 ), N( Nzquad ), 0};
  grid.m_B.resize( ( std::size_t )( grid.m_Nxyz[0] * grid.m_Nxyz[1] * grid.m_Nxyz[2] ), {0, 0, 0, 0} );
  for ( std::size_t iz = 0; iz < Nzquad; ++iz ) {
    for ( std::size_t iy = 0; iy < Nyquad; ++iy ) {
      for ( std::size_t ix = 0; ix < Nxquad; ++ix ) {
        // 4th quadrant (negative x, negative y)
        const auto& Q4 = quadrants[3].Q[Nxquad * ( Nyquad * iz + iy ) + ix];
        grid.m_B[grid.m_Nxyz[0] * ( grid.m_Nxyz[1] * iz + ( Nyquad - iy - 1 ) ) + ( Nxquad - ix - 1 )] = {
            (float)Q4.x(), (float)Q4.y(), (float)Q4.z(), 0.0f};
        // 2nd quadrant (negative x, positive y)
        const auto& Q2 = quadrants[1].Q[Nxquad * ( Nyquad * iz + iy ) + ix];
        grid.m_B[grid.m_Nxyz[0] * ( grid.m_Nxyz[1] * iz + ( Nyquad + iy - 1 ) ) + ( Nxquad - ix - 1 )] = {
            (float)Q2.x(), (float)Q2.y(), (float)Q2.z(), 0.0f};
        // 3rd quadrant (postive x, negative y)
        const auto& Q3 = quadrants[2].Q[Nxquad * ( Nyquad * iz + iy ) + ix];
        grid.m_B[grid.m_Nxyz[0] * ( grid.m_Nxyz[1] * iz + ( Nyquad - iy - 1 ) ) + ( Nxquad + ix - 1 )] = {
            (float)Q3.x(), (float)Q3.y(), (float)Q3.z(), 0.0f};
        // 1st quadrant (positive x, positive y)
        const auto& Q1 = quadrants[0].Q[Nxquad * ( Nyquad * iz + iy ) + ix];
        grid.m_B[grid.m_Nxyz[0] * ( grid.m_Nxyz[1] * iz + ( Nyquad + iy - 1 ) ) + ( Nxquad + ix - 1 )] = {
            (float)Q1.x(), (float)Q1.y(), (float)Q1.z(), 0.0f};
      }
    }
  }
  grid.m_min = {-( ( Nxquad - 1 ) * Dxyz[0] ), -( ( Nyquad - 1 ) * Dxyz[1] ), (float)quadrants[0].zOffset};

  dd4hep::printout( dd4hep::DEBUG, "MagneticFieldGridReader", "Field grid , nbins x,y,z  : (%d, %d, %d)",
                    grid.m_Nxyz[0], grid.m_Nxyz[1], grid.m_Nxyz[2] );
  dd4hep::printout( dd4hep::DEBUG, "MagneticFieldGridReader", "dx, xmin, xmax: (%f, %f, %f)", Dxyz[0], grid.m_min[0],
                    grid.m_min[0] + ( grid.m_Nxyz[0] - 1 ) * Dxyz[0] );
  dd4hep::printout( dd4hep::DEBUG, "MagneticFieldGridReader", "dy, ymin, ymax: (%f, %f, %f)", Dxyz[0], grid.m_min[1],
                    grid.m_min[1] + ( grid.m_Nxyz[1] - 1 ) * Dxyz[1] );
  dd4hep::printout( dd4hep::DEBUG, "MagneticFieldGridReader", "dz, zmin, zmax: (%f, %f, %f)", Dxyz[0], grid.m_min[2],
                    grid.m_min[2] + ( grid.m_Nxyz[2] - 1 ) * Dxyz[2] );
}

////////////////////////////////////////////////////////////////////////////////////////
// read the data for a single quadrant from a file
////////////////////////////////////////////////////////////////////////////////////////
bool LHCb::Magnet::MagneticFieldGridReader::readQuadrant( const std::string& filename, GridQuadrant& quad ) const {

  // Fix this !
  const auto full_filename = m_mapFilePath + "/" + filename;
  dd4hep::printout( dd4hep::DEBUG, "MagneticFieldGridReader", "FieldMapFile:  %s", full_filename.c_str() );
  // open the file
  std::ifstream infile( full_filename.c_str() );

  if ( infile.is_open() ) {
    dd4hep::printout( dd4hep::INFO, "MagneticFieldGridReader", "Opened magnetic field file:  %s",
                      full_filename.c_str() );

    // Skip the header till PARAMETER
    constexpr std::size_t line_size = 255;
    char                  line[line_size];
    char*                 saveptr = nullptr;
    std::size_t           iline{0};
    auto                  cdfreadline = [&]() {
      infile.getline( line, line_size );
      ++iline;
    };

    try {

      do { cdfreadline(); } while ( line[0] != 'P' );

      // Get the PARAMETER
      std::array<std::string, 2> sPar;
      char*                      token = strtok_r( line, " ", &saveptr );
      int                        ip    = 0;
      do {
        if ( token ) {
          sPar[ip] = token;
          token    = strtok_r( nullptr, " ", &saveptr );
        } else {
          continue;
        }
        ++ip;
      } while ( token != nullptr );
      const long int npar = std::stoi( sPar[1] );
      if ( npar <= 7 ) { throw std::runtime_error( "Too few parameters read | npar=" + std::to_string( npar ) ); }

      // Skip the header till GEOMETRY
      do { cdfreadline(); } while ( line[0] != 'G' );

      // Skip any comment before GEOMETRY
      do { cdfreadline(); } while ( line[0] != '#' );

      // Get the GEOMETRY
      cdfreadline();
      std::array<std::string, 7> sGeom;
      token  = strtok_r( line, " ", &saveptr );
      int ig = 0;
      do {
        if ( token ) {
          sGeom.at( ig ) = token;
          token          = strtok_r( nullptr, " ", &saveptr );
        } else {
          continue;
        }
        ++ig;
      } while ( token != nullptr );

      // Grid dimensions are given in cm in CDF file. Convert to CLHEP units

      quad.Dxyz[0] = std::stof( sGeom[0] ) * cm;
      quad.Dxyz[1] = std::stof( sGeom[1] ) * cm;
      quad.Dxyz[2] = std::stof( sGeom[2] ) * cm;
      quad.Nxyz[0] = std::stoi( sGeom[3] );
      quad.Nxyz[1] = std::stoi( sGeom[4] );
      quad.Nxyz[2] = std::stoi( sGeom[5] );
      quad.zOffset = std::stof( sGeom[6] ) * cm;

      // Number of lines with data to be read
      const std::size_t nlines = ( npar - 7 ) / 3;
      quad.Q.clear();
      quad.Q.reserve( nlines );

      // Skip comments and fill a vector of magnetic components for the
      // x, y and z positions given in GEOMETRY
      while ( infile ) {

        // parse each line of the file,
        // comment lines begin with '#' in the cdf file
        cdfreadline();
        if ( line[0] == '#' ) { continue; }

        // read field (x,y,z) values
        // Field values are given in gauss in CDF file, so convert to CLHEP units.
        double      fx{0}, fy{0}, fz{0};
        const char* token = strtok_r( line, " ", &saveptr );
        if ( token ) {
          fx    = gauss * std::atof( token );
          token = strtok_r( nullptr, " ", &saveptr );
        } else {
          continue;
        }
        if ( token ) {
          fy    = gauss * std::atof( token );
          token = strtok_r( nullptr, " ", &saveptr );
        } else {
          continue;
        }
        if ( token ) {
          fz    = gauss * std::atof( token );
          token = strtok_r( nullptr, " ", &saveptr );
        } else {
          continue;
        }
        if ( token != nullptr ) { continue; }

        // Add the magnetic field components of each point
        quad.Q.emplace_back( fx, fy, fz );
      }

      // close the file
      infile.close();

      // sanity check
      if ( nlines != quad.Q.size() ) {
        dd4hep::printout( dd4hep::ERROR, "MagneticFieldGridReader",
                          "Number of points in field map file (%i) does not match cached quadrant size (%i)", nlines,
                          quad.Q.size() );
        dd4hep::printout( dd4hep::ERROR, "MagneticFieldGridReader", " -> Read %i lines from %s", iline,
                          full_filename.c_str() );
        dd4hep::printout( dd4hep::ERROR, "MagneticFieldGridReader", " -> Last read line '%s'", line );
        return false;
      }

    } catch ( const std::exception& excpt ) {
      dd4hep::printout( dd4hep::ERROR, "MagneticFieldGridReader", "Exception reading CDF file %s '%s'",
                        full_filename.c_str(), excpt.what() );
      dd4hep::printout( dd4hep::ERROR, "MagneticFieldGridReader", "Last line read '%s'", line );
      return false;
    }

  } else {
    dd4hep::printout( dd4hep::ERROR, "MagneticFieldGridReader", "Unable to open magnetic field file: %s",
                      full_filename.c_str() );
    return false;
  }
  return true;
}
