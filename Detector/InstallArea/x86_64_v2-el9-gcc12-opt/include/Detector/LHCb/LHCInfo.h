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

#include <nlohmann/json_fwd.hpp>

namespace LHCb::Detector {
  struct LHCInfo {

    LHCInfo() = default;
    LHCInfo( const nlohmann::json& obj );

    unsigned int fillnumber{};
    std::string  lhcstate{};
    int          lhcenergy{};
    float        lhcbclockphase{};
    float        xangleh{};
    float        xanglev{};
    std::string  beamtype{};
  };
} // namespace LHCb::Detector
