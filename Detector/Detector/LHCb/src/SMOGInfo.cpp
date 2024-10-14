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
#include <Detector/LHCb/SMOGInfo.h>

#include <nlohmann/json.hpp>

namespace LHCb::Detector {
  NLOHMANN_JSON_SERIALIZE_ENUM( SMOGInfo::Mode, {{SMOGInfo::Mode::NONE, "NONE"},
                                                 {SMOGInfo::Mode::SMOG, "SMOG"},
                                                 {SMOGInfo::Mode::SMOG2, "SMOG2"},
                                                 {SMOGInfo::Mode::UNKNOWN, "UNKNOWN"}} )

  NLOHMANN_JSON_SERIALIZE_ENUM( SMOGInfo::Gas, {{SMOGInfo::Gas::NONE, "NONE"},
                                                {SMOGInfo::Gas::HYDROGEN, "HYDROGEN"},
                                                {SMOGInfo::Gas::DEUTERIUM, "DEUTERIUM"},
                                                {SMOGInfo::Gas::HELIUM, "HELIUM"},
                                                {SMOGInfo::Gas::NITROGEN, "NITROGEN"},
                                                {SMOGInfo::Gas::OXYGEN, "OXYGEN"},
                                                {SMOGInfo::Gas::NEON, "NEON"},
                                                {SMOGInfo::Gas::ARGON, "ARGON"},
                                                {SMOGInfo::Gas::KRYPTON, "KRYPTON"},
                                                {SMOGInfo::Gas::XENON, "XENON"}} )

  void to_json( nlohmann::json& j, const SMOGInfo& p ) {
    j = nlohmann::json{{"Mode", p.injection_mode}, {"Gas", p.injected_gas}, {"Stable", int( p.stable_injection )}};
  }

  void from_json( const nlohmann::json& j, SMOGInfo& p ) {
    if ( j.contains( "Mode" ) ) j.at( "Mode" ).get_to( p.injection_mode );
    if ( j.contains( "Gas" ) ) j.at( "Gas" ).get_to( p.injected_gas );
    if ( j.contains( "Stable" ) ) {
      const auto& stable = j.at( "Stable" );
      p.stable_injection = stable.is_boolean() ? stable.get<bool>() : bool( stable.get<int>() );
    }
  }
} // namespace LHCb::Detector
