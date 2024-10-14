/*****************************************************************************\
* (c) Copyright 2024 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once
#include <nlohmann/json.hpp>

namespace LHCb::Detector {
  struct SMOGInfo {

    SMOGInfo() = default;

    enum struct Mode { UNKNOWN = -1, NONE, SMOG, SMOG2 };
    enum struct Gas { NONE = 0, HYDROGEN, DEUTERIUM, HELIUM, NITROGEN, OXYGEN, NEON, ARGON, KRYPTON, XENON };

    Mode injection_mode{Mode::NONE};
    Gas  injected_gas{Gas::NONE};
    bool stable_injection{false};
  };

  void to_json( nlohmann::json& j, const SMOGInfo& p );
  void from_json( const nlohmann::json& j, SMOGInfo& p );
}; // namespace LHCb::Detector
