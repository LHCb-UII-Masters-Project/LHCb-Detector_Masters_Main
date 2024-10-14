/*****************************************************************************\
* (c) Copyright 2020 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once

#include <DD4hep/Conditions.h>
#include <functional>
#include <string_view>
#include <yaml-cpp/yaml.h>

namespace LHCb::YAMLConverters {
  /// Type for functions to create Condition object from a name and a YAML::Node.
  using Converter = std::function<dd4hep::Condition( std::string_view, const YAML::Node& )>;

  /// Create a Condition object delegating to the spcific converter, based on the YAML::Node::Tag.
  dd4hep::Condition make_condition( std::string_view name, const YAML::Node& cond_data );

  /// Register a Converter for the given tag.
  void set_converter( std::string_view tag, Converter converter );

} // namespace LHCb::YAMLConverters
