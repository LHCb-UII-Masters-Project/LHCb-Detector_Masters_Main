/*****************************************************************************\
* (c) Copyright 2022 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once
#include <fmt/core.h>
#include <nlohmann/json.hpp>
#include <string>
#include <yaml-cpp/yaml.h>

#if __cplusplus > 201703L && __has_include( <source_location> ) && ! defined( __CLING__ )
#  include <source_location>
namespace LHCb::Detector {
  using std::source_location;
}
#elif __cplusplus >= 201402L
#  include <experimental/source_location>
namespace LHCb::Detector {
  using std::experimental::source_location;
}
#endif

namespace LHCb::Detector::utils {

  /**
   * @brief DUmps the stack trace at the point when the method is called. Copy
   * of the backTrace implementation from Gaudi but not useable from Detector
   * as it does not depend on Gaudi.
   *
   * @param std::string where the backtrace should be written
   * @param depth Max number of calls in the call stack to dump
   * @param offset for indentation
   * @return whether reading the  stacktrace was successful
   */
  bool backTrace( std::string& btrace, const int depth, const int offset );

  /**
   * @brief  used by backTrace( std::string& btrace, const int depth, const int offset )
   */
  int backTrace( [[maybe_unused]] void** addresses, [[maybe_unused]] const int depth );

  /**
   * @brief  used by backTrace( std::string& btrace, const int depth, const int offset )
   */
  bool getStackLevel( [[maybe_unused]] void* addresses, [[maybe_unused]] void*& addr, [[maybe_unused]] std::string& fnc,
                      [[maybe_unused]] std::string& lib );

  /// Convert a YAML::Node instance to a nlohmann::json object.
  ///
  /// Since yaml-cpp does not make any assumption about [YAML untagged
  /// nodes](https://yaml.org/spec/1.2.2/#resolved-tags) leaving it to the application to decide how to interprete the
  /// values, this conversion function tries to map the value to bool, int64_t or double on a best effort basis, falling
  /// back on std::string in case of failure.
  nlohmann::json yaml2json( const YAML::Node& y );

  /// Convert a nlohmann::json object to a YAML::Node instance.
  ///
  /// This function relies on YAML syntax being a superset of JSON, so we serialize
  /// and deserialize.
  YAML::Node json2yaml( const nlohmann::json& j );
} // End namespace LHCb::Detector::utils

namespace LHCb::Detector {

  /// To be dropped once everything is implemented
  struct NotImplemented : std::exception {
    NotImplemented( const std::string& missing = "", source_location src_loc = source_location::current() )
        : std::exception() {
      std::string stack;
      LHCb::Detector::utils::backTrace( stack, 100, 1 );
      std::string msg = "NotImplemented";
      if ( !missing.empty() ) { msg = missing; }
      m_message = fmt::format( "{} at {}:{}\n{}", msg, src_loc.file_name(), src_loc.line(), stack );
    }
    const char* what() const noexcept override { return m_message.c_str(); };

    std::string m_message;
  };
} // End namespace LHCb::Detector

namespace LHCb::Detector::detail {

  // For compatibility with implementation of the Rich in LHCb
  using RichNotImplemented = LHCb::Detector::NotImplemented;

} // End namespace LHCb::Detector::detail
