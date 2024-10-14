/*****************************************************************************\
* (c) Copyright 2022 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#include <Core/Utils.h>

#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

// -----------------------------------------------------------------------------
// backtrace utilities
// -----------------------------------------------------------------------------
#ifdef __linux
#  include <cxxabi.h>
#  include <dlfcn.h>
#  include <execinfo.h>
#endif

int LHCb::Detector::utils::backTrace( [[maybe_unused]] void** addresses, [[maybe_unused]] const int depth ) {

#ifdef __linux

  int count = backtrace( addresses, depth );
  return count > 0 ? count : 0;

#else // windows and osx parts not implemented
  return 0;
#endif
}

bool LHCb::Detector::utils::backTrace( std::string& btrace, const int depth, const int offset ) {
  try {
    // Always hide the first two levels of the stack trace (that's us)
    const int totalOffset = offset + 2;
    const int totalDepth  = depth + totalOffset;

    std::string fnc, lib;

    std::vector<void*> addresses( totalDepth, nullptr );
    int                count = backTrace( addresses.data(), totalDepth );
    for ( int i = totalOffset; i < count; ++i ) {
      void* addr = nullptr;

      if ( getStackLevel( addresses[i], addr, fnc, lib ) ) {
        std::ostringstream ost;
        ost << "#" << std::setw( 3 ) << std::setiosflags( std::ios::left ) << i - totalOffset + 1;
        ost << std::hex << addr << std::dec << " " << fnc << "  [" << lib << "]" << std::endl;
        btrace += ost.str();
      }
    }
    return true;
  } catch ( const std::bad_alloc& e ) { return false; }
}

bool LHCb::Detector::utils::getStackLevel( [[maybe_unused]] void* addresses, [[maybe_unused]] void*& addr,
                                           [[maybe_unused]] std::string& fnc, [[maybe_unused]] std::string& lib ) {

#ifdef __linux

  Dl_info info;

  if ( dladdr( addresses, &info ) && info.dli_fname && info.dli_fname[0] != '\0' ) {
    const char* symbol = info.dli_sname && info.dli_sname[0] != '\0' ? info.dli_sname : nullptr;

    lib  = info.dli_fname;
    addr = info.dli_saddr;

    if ( symbol ) {
      int  stat = -1;
      auto dmg =
          std::unique_ptr<char, decltype( free )*>( abi::__cxa_demangle( symbol, nullptr, nullptr, &stat ), std::free );
      fnc = ( stat == 0 ) ? dmg.get() : symbol;
    } else {
      fnc = "local";
    }
    return true;
  } else {
    return false;
  }

#else // not implemented for windows and osx
  return false;
#endif
}

nlohmann::json LHCb::Detector::utils::yaml2json( const YAML::Node& y ) {
  nlohmann::json j;
  switch ( y.Type() ) {
  case YAML::NodeType::Sequence: {
    j = nlohmann::json::array();
    for ( const auto& element : y ) { j.push_back( yaml2json( element ) ); }
  } break;
  case YAML::NodeType::Map: {
    j = nlohmann::json::object();
    for ( const auto& entry : y ) { j[entry.first.as<std::string>()] = yaml2json( entry.second ); }
  } break;
  case YAML::NodeType::Scalar: {
    auto tmp = y.as<std::string>();
    j        = tmp; // implicit default: fall back on a string
    if ( tmp.empty() ) {
      // just an empty string is OK
    } else if ( tmp == "true" ) {
      j = true;
    } else if ( tmp == "false" ) {
      j = false;
    } else { // if it's not a boolean literal, we try with numbers
      std::int64_t tmp_int = 0;
      double       tmp_d   = 0.;
      if ( YAML::convert<std::int64_t>::decode( y, tmp_int ) ) {
        j = tmp_int;
      } else if ( YAML::convert<double>::decode( y, tmp_d ) ) {
        j = tmp_d;
        // not an int, not a float, we can only fall back on string
      }
    }
  } break;
  case YAML::NodeType::Undefined:
  case YAML::NodeType::Null:
  default: {
    // nothing to do in these cases
  }
  }
  return j;
}

YAML::Node LHCb::Detector::utils::json2yaml( const nlohmann::json& j ) { return YAML::Load( j.dump() ); }
