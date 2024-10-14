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

#pragma once

#include "Core/MagneticFieldGrid.h"
#include "Math/Vector3D.h"
#include <array>
#include <string>
#include <utility>
#include <vector>

namespace LHCb::Magnet {

  class MagneticFieldGridReader {
  public:
    MagneticFieldGridReader( std::string field_map_path ) : m_mapFilePath{std::move( field_map_path )} {}

    bool readFiles( const std::vector<std::string>& filenames, MagneticFieldGrid& grid ) const;

    bool readDC06File( const std::string& filename, MagneticFieldGrid& grid ) const;

    void fillConstantField( const ROOT::Math::XYZVector& field, MagneticFieldGrid& grid ) const;

  private:
    // structure that holds the grid parameters for one quadrant (a la DC06)
    struct GridQuadrant {
      std::vector<ROOT::Math::XYZVector> Q;
      double                             zOffset{0};
      std::array<double, 3>              Dxyz{{}};
      std::array<std::size_t, 3>         Nxyz{{}};
    };
    using GridQuadrants = std::array<GridQuadrant, 4>;

  private:
    void fillGridFromQuadrants( GridQuadrants& quadrants, MagneticFieldGrid& grid ) const;
    bool readQuadrant( const std::string& filename, GridQuadrant& quad ) const;

  private:
    std::string m_mapFilePath = "";
  };

} // namespace LHCb::Magnet
