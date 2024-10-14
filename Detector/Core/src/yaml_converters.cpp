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
#include <Core/Utils.h>
#include <Core/yaml_converters.h>

#include <DD4hep/AlignmentData.h>
#include <DD4hep/GrammarUnparsed.h>
#include <DD4hep/Objects.h>
#include <DD4hep/Printout.h>
#include <DD4hep/detail/ConditionsInterna.h>
#include <Evaluator/Evaluator.h>

#include <exception>
#include <fmt/core.h>
#include <functional>
#include <nlohmann/json.hpp>
#include <regex>
#include <string>
#include <string_view>
#include <unordered_map>

namespace {
  auto eval( const YAML::Node& node, const double default_unit = 1.0 ) {
    // Note: this regex is meant to match any valid floating point number,
    //       including special cases like `+1`, `.3`, `- 7`.
    //       Technically `+1` and `- 7` are not a valid representations in JSON
    //       (in YAML`+1` is understood and `- 7` is impossible to represent),
    //       but CLHEP evaluator understands them, so I have to trap them here
    //       to avoid that they are processed without the default unit.
    static std::regex valid_floating_point( "^ *[+-]? *([0-9]+(\\.[0-9]*)?|\\.[0-9]+)([eE][+-]?[0-9]+)? *$" );
    std::smatch       match;
    std::string       expression = node.as<std::string>();
    if ( std::regex_match( expression, match, valid_floating_point ) ) {
      // it is just a plain number (no explicit unit), so we use the provided unit
      return node.as<double>() * default_unit;
    }
    return dd4hep::_toDouble( expression );
  }

  static std::unordered_map<std::string, LHCb::YAMLConverters::Converter> s_converters{
      // Converter for default YAML Tag (i.e. tag not specified).
      {"?",
       []( std::string_view name, const YAML::Node& cond_data ) {
         dd4hep::Condition c{std::string{name}, ""};

         auto& payload = c.bind<nlohmann::json>();

         payload = LHCb::Detector::utils::yaml2json( cond_data );
         return c;
       }},
      // Converter for null value (i.e. the YAML `~` or `null` or ``)
      {"",
       []( std::string_view name, const YAML::Node& ) {
         // in this case we can ignore the data and directly store null
         dd4hep::Condition c{std::string{name}, ""};

         auto& payload = c.bind<nlohmann::json>();

         payload = nullptr; // nullptr maps to JSON null
         return c;
       }},
      {"!alignment", []( std::string_view name, const YAML::Node& cond_data ) {
         using Delta = dd4hep::Delta;
         using dd4hep::mm;
         dd4hep::Condition c{std::string{name}, "alignment"};
         auto&             delta = c.bind<Delta>();
         for ( const auto& param : cond_data ) {
           const auto param_name = param.first.as<std::string>();
           if ( !param.second.IsSequence() || param.second.size() != 3 ) {
             throw std::invalid_argument(
                 fmt::format( "{}: invalid argument for {}: it must be a sequence of size 3", name, param_name ) );
           }
           const auto& x = param.second[0];
           const auto& y = param.second[1];
           const auto& z = param.second[2];
           if ( param_name == "position" || param_name == "dPosXYZ" ) {
             delta.translation = dd4hep::Position{eval( x, mm ), eval( y, mm ), eval( z, mm )};
             delta.flags |= Delta::HAVE_TRANSLATION;
           } else if ( param_name == "rotation" || param_name == "dRotXYZ" ) {
             delta.rotation = dd4hep::RotationZYX{eval( z ), eval( y ), eval( x )};
             delta.flags |= Delta::HAVE_ROTATION;
           } else if ( param_name == "pivot" || param_name == "pivotXYZ" ) {
             delta.pivot = Delta::Pivot{eval( x, mm ), eval( y, mm ), eval( z, mm )};
             delta.flags |= Delta::HAVE_PIVOT;
           } else {
             dd4hep::printout( dd4hep::ERROR, "Delta", "++ Unknown alignment conditions field: %s",
                               param_name.c_str() );
           }
         }
         if ( ( delta.flags & Delta::HAVE_PIVOT ) && !( delta.flags & Delta::HAVE_ROTATION ) ) {
           // we cannot have only the pivot
           throw std::invalid_argument( fmt::format( "condition {} has a pivot, but no rotation", name ) );
         }
         c->setFlag( dd4hep::Condition::ALIGNMENT_DELTA );
         return c;
       }}};
} // namespace

dd4hep::Condition LHCb::YAMLConverters::make_condition( std::string_view name, const YAML::Node& cond_data ) {
  const auto& converter = s_converters.find( cond_data.Tag() );
  if ( converter == end( s_converters ) )
    throw std::runtime_error( fmt::format( "no converter for '{}'", cond_data.Tag() ) );
  return converter->second( name, cond_data );
}

void LHCb::YAMLConverters::set_converter( std::string_view tag, Converter converter ) {
  s_converters[std::string{tag}] = converter;
}
