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
#include <Detector/Test/Fixture.h>
#include <fmt/format.h>

namespace {

  // Copied from DD4hep, because it's not available outside it...
  const char* print_level( dd4hep::PrintLevel lvl ) {
    switch ( lvl ) {
    case dd4hep::NOLOG:
      return "NOLOG";
    case dd4hep::VERBOSE:
      return "VERB ";
    case dd4hep::DEBUG:
      return "DEBUG";
    case dd4hep::INFO:
      return "INFO ";
    case dd4hep::WARNING:
      return "WARN ";
    case dd4hep::ERROR:
      return "ERROR";
    case dd4hep::FATAL:
      return "FATAL";
    case dd4hep::ALWAYS:
      return "     ";
    default:
      if ( lvl > dd4hep::ALWAYS ) return print_level( dd4hep::ALWAYS );
      return print_level( dd4hep::NOLOG );
    }
  }
} // namespace

size_t Detector::Test::test_printer( void* fixture, dd4hep::PrintLevel lvl, const char* src, const char* text ) {
  Fixture*                    f = reinterpret_cast<Fixture*>( fixture );
  std::lock_guard<std::mutex> lock( f->output_mutex );
  // std::string s = std::string{src} + " " + text;
  std::string s = fmt::format( "{:-<16} {} {}", src, print_level( lvl ), text );
  f->printout << s << "\n";
  std::cout << s << "\n";
  return s.size() + 1;
}
