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

#include <nlohmann/json.hpp>

namespace LHCb::Detector {
  /// Helper to pass some configuration options to the geometry constructors.
  ///
  /// This is perfectly opaque to DD4hep, but it can be used to fine tune the behavior
  /// of various plugins, for example to enable/disable support for specific conditions.
  ///
  /// @note it is a bit silly to specialize nlohmann::json, but DD4hep extension mechanism uses the type as access key
  struct Options : nlohmann::basic_json<> {
    using basic_json::basic_json;
  };
} // namespace LHCb::Detector
