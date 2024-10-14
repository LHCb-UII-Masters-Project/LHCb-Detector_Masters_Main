/***************************************************************************** \
 * (c) Copyright 2000-2018 CERN for the benefit of the LHCb Collaboration      *
 *                                                                             *
 * This software is distributed under the terms of the GNU General Public      *
 * Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
 *                                                                             *
 * In applying this licence, CERN does not waive the privileges and immunities *
 * granted to it by virtue of its status as an Intergovernmental Organization  *
 * or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#include "Detector/Muon/MuonChamberGrid.h"

#include "DD4hep/Printout.h"

#include "Detector/Muon/FrontEndID.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <numeric>
#include <string_view>
#include <vector>

//-----------------------------------------------------------------------------
// Implementation file for class : MuonChamberGrid
//
// 2004-01-07 : Alessia(o) Sat(r)ta(i)
//-----------------------------------------------------------------------------

//=============================================================================
using namespace LHCb::Detector::Muon;

detail::MuonChamberGrid::MuonChamberGrid( const nlohmann::json& grid_info ) {

  m_x_pad_rdout1 = grid_info["xrd1"].get<std::vector<double>>();
  m_y_pad_rdout1 = grid_info["yrd1"].get<std::vector<double>>();
  m_x_pad_rdout2 = grid_info["xrd2"].get<std::vector<double>>();
  m_y_pad_rdout2 = grid_info["yrd2"].get<std::vector<double>>();
  m_readoutType  = grid_info["grrd"].get<std::vector<unsigned int>>();
  m_mapType      = grid_info["mapT"].get<std::vector<unsigned int>>();
}

detail::MuonChamberGrid::MuonChamberGrid( const detail::MuonChamberGrid& old ) {

  m_x_pad_rdout1 = old.m_x_pad_rdout1;
  m_y_pad_rdout1 = old.m_y_pad_rdout1;
  m_x_pad_rdout2 = old.m_x_pad_rdout2;
  m_y_pad_rdout2 = old.m_y_pad_rdout2;
  m_readoutType  = old.m_readoutType;
  m_mapType      = old.m_mapType;
}

std::vector<std::pair<FrontEndID, std::array<float, 4>>>
detail::MuonChamberGrid::listOfPhysChannels( double x_enter, double y_enter, double x_exit, double y_exit ) const {

  std::vector<std::pair<FrontEndID, std::array<float, 4>>> keepTemporary;
  //
  double ParallelCutOff = 0.0001;

  bool debug        = false;
  bool parallelFlag = false;

  double slopeY( 0 ), intercept( 0 );
  if ( fabs( x_exit - x_enter ) > ParallelCutOff ) {
    slopeY    = ( y_exit - y_enter ) / ( x_exit - x_enter );
    intercept = y_exit - slopeY * x_exit;
  } else {
    parallelFlag = true;
  }

  // The choice on the readout numbers is taken
  // looking for dummy grids
  int TstRead = m_x_pad_rdout2.size();
  int RdN     = 1;
  if ( TstRead > 1 ) RdN = 2;

  /* if ( debug )
  //std::cout << "Returning List of Phys Channels for grid G" << m_number_of_grid
  std::cout << "Returning List of Phys Channels for grid G -> Vectors rd1x: " << m_x_pad_rdout1
  << " rd1y: " << m_y_pad_rdout1
  << " rd2x: " << m_x_pad_rdout2 << " rd2y: " << m_y_pad_rdout2 << std::endl;
  */

  // Questa parte sui readout va vista bene
  for ( int iRd = 0; iRd < RdN; iRd++ ) {

    // Readout choice
    const std::vector<double>& x_rdout = ( iRd ? m_x_pad_rdout2 : m_x_pad_rdout1 );
    const std::vector<double>& y_rdout = ( iRd ? m_y_pad_rdout2 : m_y_pad_rdout1 );

    // Physical channels settings related to the grid/readout
    int PhNx = x_rdout.size();
    int PhNy = y_rdout.size();

    unsigned int nxChaEntry, nxChaExit;
    int          tmpNxChaEntry( 0 ), tmpNxChaExit( 20 ), inxLo, inyLo;

    dd4hep::printout( dd4hep::DEBUG, "MuonChamberGrid",
                      "Looping on readout n. %i with vct dim nX: %i  nY: %i. Entry/Exit of hit. (%f,%f), (%f,%f)", iRd,
                      PhNx, PhNy, x_enter, y_enter, x_exit, y_exit );

    double tmpXLength( 0.0 );
    for ( inxLo = 0; inxLo < PhNx; inxLo++ ) {
      if ( x_enter - tmpXLength > ParallelCutOff ) {
        tmpXLength += x_rdout[inxLo];
        tmpNxChaEntry = inxLo;
        //	tmpNxChaEntry =  inxLo+1;
      } else {
        break;
      }
    }
    tmpXLength = 0.0;
    for ( inxLo = 0; inxLo < PhNx; inxLo++ ) {
      if ( x_exit - tmpXLength > ParallelCutOff ) {
        tmpXLength += x_rdout[inxLo];
        tmpNxChaExit = inxLo;
        //	tmpNxChaExit =  inxLo+1;
      } else {
        break;
      }
    }
    double xstart, xstop;

    dd4hep::printout( dd4hep::DEBUG, "MuonChamberGrid",
                      "Returning List of Phys Channels. TmpCha_entry: %i | TmpCha_exit: %i", tmpNxChaEntry,
                      tmpNxChaExit );

    if ( tmpNxChaEntry <= tmpNxChaExit ) {
      // normal order
      if ( tmpNxChaEntry < 0 ) tmpNxChaEntry = 0;
      if ( tmpNxChaExit >= (int)PhNx ) tmpNxChaExit = (int)PhNx - 1;

      xstart     = x_enter;
      xstop      = x_exit;
      nxChaEntry = (unsigned int)tmpNxChaEntry;
      nxChaExit  = (unsigned int)tmpNxChaExit;

    } else {
      // inverse order
      if ( tmpNxChaExit < 0 ) tmpNxChaExit = 0;
      if ( tmpNxChaEntry >= (int)PhNx ) tmpNxChaEntry = (int)PhNx - 1;
      xstop      = x_enter;
      xstart     = x_exit;
      nxChaEntry = (unsigned int)tmpNxChaExit;
      nxChaExit  = (unsigned int)tmpNxChaEntry;
    }

    unsigned int nyBegin;
    unsigned int nyEnd;
    double       xBegin( 0. ), xEnd( 0. );
    double       yBegin( 0. ), yEnd( 0. );

    dd4hep::printout( dd4hep::DEBUG, "MuonChamberGrid", "Returning List of Phys Channels. Cha_entry: %i | Cha_exit: %i",
                      nxChaEntry, nxChaExit );

    for ( unsigned int Xloop = nxChaEntry; Xloop <= nxChaExit; Xloop++ ) {
      xBegin = ( Xloop == nxChaEntry ? xstart : retLength( Xloop, x_rdout ) );
      xEnd   = ( Xloop == nxChaExit ? xstop : retLength( Xloop + 1, x_rdout ) );

      if ( debug )
        std::cout << "Linear Parameters bef p flag. Xbeg " << xBegin << "; xEnd: " << xEnd << " Ybeg " << yBegin
                  << "; yEnd: " << yEnd << " int: " << intercept << " slope: " << slopeY << std::endl;

      if ( parallelFlag ) {
        yBegin = y_enter;
        yEnd   = y_exit;
      } else {
        yBegin = intercept + slopeY * xBegin;
        yEnd   = intercept + slopeY * xEnd;
      }

      dd4hep::printout( dd4hep::DEBUG, "MuonChamberGrid",
                        "Computing Linear Parameters :: Begin (%f,%f)| End (%f,%f)| intercept: %f| slope: %f ", xBegin,
                        xEnd, yBegin, yEnd, intercept, slopeY );

      double xinit, yinit, xend, yend;
      int    tmpYBegin( 0 ), tmpYEnd( 0 );
      double tmpYLength( 0 );

      for ( inyLo = 0; inyLo < PhNy; inyLo++ ) {
        if ( yBegin - tmpYLength > ParallelCutOff ) {
          tmpYLength += y_rdout[inyLo];
          tmpYBegin = inyLo;
        } else {
          break;
        }
      }
      tmpYLength = 0;
      for ( inyLo = 0; inyLo < PhNy; inyLo++ ) {
        if ( yEnd - tmpYLength > ParallelCutOff ) {
          tmpYLength += y_rdout[inyLo];
          tmpYEnd = inyLo;
        } else {
          break;
        }
      }

      dd4hep::printout( dd4hep::DEBUG, "MuonChamberGrid", " DEBUG :: Y Begin : %f | Y End :%f", tmpYBegin, tmpYEnd );

      if ( tmpYBegin <= tmpYEnd ) {
        if ( tmpYBegin < 0 ) tmpYBegin = 0;
        if ( tmpYEnd >= (int)PhNy ) tmpYEnd = (int)PhNy - 1;

        nyBegin = tmpYBegin;
        nyEnd   = tmpYEnd;

        xinit = xBegin;
        yinit = yBegin;
        xend  = xEnd;
        yend  = yEnd;

      } else {

        if ( tmpYEnd < 0 ) tmpYEnd = 0;
        if ( tmpYBegin >= (int)PhNy ) tmpYBegin = (int)PhNy - 1;

        nyBegin = tmpYEnd;
        nyEnd   = tmpYBegin;

        xinit = xEnd;
        yinit = yEnd;
        xend  = xBegin;
        yend  = yBegin;
      }

      dd4hep::printout( dd4hep::DEBUG, "MuonChamberGrid", " DEBUG :: NY Begin : %f | NY End :%f", nyBegin, nyEnd );

      for ( unsigned int Yloop = nyBegin; Yloop <= nyEnd; Yloop++ ) {
        // Compute distance from the boundaries
        // of the physical channel
        double myX( 0 ), myY( 0 );
        if ( nyBegin == nyEnd ) {
          myX = ( xinit + xend ) / 2;
          myY = ( yinit + yend ) / 2;
        } else if ( Yloop == nyBegin && Yloop != nyEnd ) {
          if ( parallelFlag ) {
            myX = xinit;
          } else {
            myX = ( xinit + ( retLength( Yloop + 1, y_rdout ) - intercept ) / slopeY ) / 2;
          }
          myY = ( yinit + retLength( Yloop + 1, y_rdout ) ) / 2;
        } else if ( Yloop != nyBegin && Yloop == nyEnd ) {
          if ( parallelFlag ) {
            myX = xend;
          } else {
            myX = ( ( ( retLength( Yloop, y_rdout ) - intercept ) / slopeY ) + xend ) / 2;
          }
          myY = ( retLength( Yloop, y_rdout ) + yend ) / 2;
        } else {
          if ( parallelFlag ) {
            myX = ( xend + xinit ) / 2;
          } else {
            myX = ( ( retLength( Yloop, y_rdout ) - intercept ) / slopeY +
                    ( retLength( Yloop + 1, y_rdout ) - intercept ) / slopeY ) /
                  2;
          }
          myY = ( retLength( Yloop, y_rdout ) + retLength( Yloop + 1, y_rdout ) ) / 2;
        }

        FrontEndID input;

        input.setFEGridX( PhNx );
        input.setFEGridY( PhNy );
        input.setFEIDX( Xloop );
        input.setFEIDY( Yloop );
        input.setReadout( m_readoutType[iRd] );

        keepTemporary.emplace_back( input, std::array{float( myX - retLength( Xloop, x_rdout ) ),
                                                      float( myY - retLength( Yloop, y_rdout ) ),
                                                      float( retLength( Xloop + 1, x_rdout ) - myX ),
                                                      float( retLength( Yloop + 1, y_rdout ) - myY )} );
        const auto& myBoundary = keepTemporary.back().second;

        dd4hep::printout( dd4hep::DEBUG, "MuonChamberGrid", "FE adding :: %i %i %i %i %i", PhNx, PhNy, Xloop, Yloop,
                          m_readoutType[iRd] );
        int nn = keepTemporary.size();
        dd4hep::printout( dd4hep::DEBUG, "MuonChamberGrid", "FE Grid :: %i %i %i %i",
                          keepTemporary.back().first.getFEGridX(), keepTemporary[nn - 1].first.getFEGridX(),
                          input.getFEGridX(), input.getReadout() );
        dd4hep::printout( dd4hep::DEBUG, "MuonChamberGrid",
                          "Hit processing.  RT :: %i | Xl :: %i | Bd1 = %f, Bd2 = %f, Bd3 = %f, Bd4 = %f | Yl = %i",
                          m_readoutType[iRd], Xloop, myBoundary[0], myBoundary[1], myBoundary[2], myBoundary[3],
                          Yloop );
      }
    }
  }

  return keepTemporary;
}

double detail::MuonChamberGrid::retLength( int nLx, const std::vector<double>& my_list ) const {

  int VctSize = my_list.size();
  if ( nLx > VctSize ) {
    nLx = VctSize;
    dd4hep::printout( dd4hep::INFO, "MuonChamberGrid", "MuonChamberGrid:: Vector index Out Of Range." );
  }
  return std::accumulate( begin( my_list ), std::next( begin( my_list ), nLx ), double{0} );
}
//
bool detail::MuonChamberGrid::getPCCenter( FrontEndID fe, double& xcenter, double& ycenter ) const {

  bool                sc = false;
  std::vector<double> x_rdout;
  std::vector<double> y_rdout;
  unsigned int        readout = fe.getReadout();

  int TstRead = m_x_pad_rdout2.size();
  int RdN     = 1;
  if ( TstRead > 1 ) RdN = 2;

  // get the correct grid
  for ( int iRd = 0; iRd < RdN; iRd++ ) {
    if ( m_readoutType[iRd] == readout ) {
      if ( iRd ) {
        x_rdout = m_x_pad_rdout2;
        y_rdout = m_y_pad_rdout2;
      } else {
        x_rdout = m_x_pad_rdout1;
        y_rdout = m_y_pad_rdout1;
      }
      break;
    }
  }

  unsigned int chx = fe.getFEIDX();
  unsigned int chy = fe.getFEIDY();
  xcenter          = ( chx > 0 ? x_rdout[chx] + x_rdout[chx - 1] : x_rdout[chx] ) / 2;
  ycenter          = ( chy > 0 ? y_rdout[chy] + y_rdout[chy - 1] : y_rdout[chy] ) / 2;

  sc = true;
  return sc;
}
