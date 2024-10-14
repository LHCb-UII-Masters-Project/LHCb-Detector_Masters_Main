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
#pragma once

#include "Core/DeIOV.h"

#include <nlohmann/json.hpp>
#include <vector>

// Include files
#include "Detector/Muon/FrontEndID.h"

namespace LHCb::Detector::Muon {

  namespace detail {

    struct MuonChamberGrid {
      MuonChamberGrid() = default;
      MuonChamberGrid( const nlohmann::json& grid_info );
      MuonChamberGrid( const MuonChamberGrid& old );

      std::vector<unsigned int> m_readoutType;
      std::vector<unsigned int> m_mapType;
      std::vector<double>       m_x_pad_rdout1;
      std::vector<double>       m_y_pad_rdout1;
      std::vector<double>       m_x_pad_rdout2;
      std::vector<double>       m_y_pad_rdout2;

      // Returns distances relative to chamber dimension
      //
      std::vector<std::pair<FrontEndID, std::array<float, 4>>> listOfPhysChannels( double x_enter, double y_enter,
                                                                                   double x_exit, double y_exit ) const;

      bool getPCCenter( FrontEndID fe, double& xcenter, double& ycenter ) const;

      double retLength( int nLx, const std::vector<double>& my_list ) const;

      const std::vector<unsigned int>& getReadoutGrid() { return m_readoutType; }

      const std::vector<unsigned int>& getMapGrid() { return m_mapType; }

      inline unsigned int getGrid1SizeX() { return m_x_pad_rdout1.size(); }

      inline unsigned int getGrid1SizeY() { return m_y_pad_rdout1.size(); }

      inline unsigned int getGrid2SizeX() { return m_x_pad_rdout2.size(); }

      inline unsigned int getGrid2SizeY() { return m_y_pad_rdout2.size(); }
    };

  } // namespace detail
} // namespace LHCb::Detector::Muon
