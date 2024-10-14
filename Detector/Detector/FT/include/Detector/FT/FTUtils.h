/*****************************************************************************\
 * (c) Copyright 2023 CERN for the benefit of the LHCb Collaboration           *
 *                                                                             *
 * This software is distributed under the terms of the GNU General Public      *
 * Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
 *                                                                             *
 * In applying this licence, CERN does not waive the privileges and immunities *
 * granted to it by virtue of its status as an Intergovernmental Organization  *
 * or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once
#include "FTChannelID.h"
#include "FTConstants.h"
#include <array>
#include <cstdint>

namespace FTConstants = LHCb::Detector::FT;

namespace LHCb::Detector {
  namespace FTUtils {

    /// From global module-ID, retrieve "local" coordinates in a string format useful for monitoring
    std::string globalModuleToLocalString( const unsigned int globalModuleIdx ) {
      unsigned int station, layer, quarter, module, temp_id, temp_station, nModules;

      station = globalModuleIdx / ( 5 * FTConstants::nQuarters * FTConstants::nLayers );

      /// There are 5 modules per quarter in T1 and T2, while there are 6 modules per quarter in T3
      if ( globalModuleIdx < 160 ) {
        nModules     = 5;
        temp_id      = globalModuleIdx;
        temp_station = station;
      } else {
        nModules     = 6;
        temp_id      = globalModuleIdx - 160;
        temp_station = 0;
        station      = 2;
      }

      /// Retrieve the "local" coordinates
      layer   = temp_id / ( nModules * FTConstants::nQuarters ) - temp_station * FTConstants::nLayers;
      quarter = temp_id / nModules - FTConstants::nQuarters * ( layer + temp_station * FTConstants::nLayers );
      module =
          temp_id - nModules * ( quarter + FTConstants::nQuarters * ( layer + temp_station * FTConstants::nLayers ) );
      station = station + 1;

      /// Return a string
      return fmt::format( "T{}L{}Q{}M{}", station, layer, quarter, module );
    }

    /// From global quarter-ID, retrieve "local" coordinates in a string format useful for monitoring
    std::string globalQuarterToLocalString( const unsigned int globalQuarterIdx ) {
      unsigned int station, layer, quarter;

      /// Retrieve the local coordinates
      station = globalQuarterIdx / ( FTConstants::nQuarters * FTConstants::nLayers );
      layer   = globalQuarterIdx / FTConstants::nQuarters - FTConstants::nLayers * station;
      quarter = globalQuarterIdx - FTConstants::nQuarters * ( FTConstants::nLayers * station + layer );
      station++;

      /// Convert from layer number to name
      std::map<unsigned int, std::string> fromLayerNumberToName = {{0, "X1"}, {1, "U"}, {2, "V"}, {3, "X2"}};

      /// Return a string
      return fmt::format( "T{}L{}Q{}", station, fromLayerNumberToName.at( layer ), quarter );
    }

    /// From "local" coordinates in a string format, retrieve global module ID.
    unsigned int fromLocalStringToGlobalModule( const std::string local_coords ) {

      int          count       = 0;
      unsigned int temp_arr[4] = {0};

      for ( auto str : local_coords ) {
        if ( count % 2 == 1 ) temp_arr[count / 2] = str - '0';
        count++;
      }

      LHCb::Detector::FTChannelID chan = LHCb::Detector::FTChannelID(
          LHCb::Detector::FTChannelID::StationID{temp_arr[0]}, LHCb::Detector::FTChannelID::LayerID{temp_arr[1]},
          LHCb::Detector::FTChannelID::QuarterID{temp_arr[2]}, LHCb::Detector::FTChannelID::ModuleID{temp_arr[3]}, 0 );

      return chan.globalModuleIdx();
    }

  } // namespace FTUtils
} // End namespace LHCb::Detector
