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
#pragma once

#include <DD4hep/Detector.h>
#include <DD4hep/Printout.h>
#include <TError.h> // gErrorIgnoreLevel
#include <cstdlib>
#include <string_view>

namespace Detector::Test {

  size_t test_printer( void* fixture, dd4hep::PrintLevel, const char* src, const char* text );

  struct Fixture {
    // Instantiate the global dd4hep instance and silence printouts
    Fixture( bool capture_printout = false ) : originalLevels{gErrorIgnoreLevel, dd4hep::printLevel()} {

      auto keep_output = std::getenv( "KEEP_OUTPUT" );

      if ( capture_printout ) {
        dd4hep::setPrinter( this, &test_printer );
      } else if ( !keep_output || std::string_view( keep_output ) == "" ) {
        // Set the error level to ignore all messages
        // - from DD4hep
        dd4hep::setPrintLevel( static_cast<dd4hep::PrintLevel>( dd4hep::ALWAYS + 1 ) );
        // - from ROOT
        gErrorIgnoreLevel = kError + 1;
      }
      descriptionPtr = &dd4hep::Detector::getInstance();
    }
    // Destroy the global dd4hep instance and restore message levels
    ~Fixture() {
      // Restore the error level
      gErrorIgnoreLevel = originalLevels.gErrorIgnoreLevel;
      dd4hep::setPrintLevel( originalLevels.printLevel );
      // Restore the default printer
      dd4hep::setPrinter( nullptr, nullptr );
      // destroy the global dd4hep instance
      dd4hep::Detector::destroyInstance();
    }

    struct {
      Int_t              gErrorIgnoreLevel;
      dd4hep::PrintLevel printLevel;
    } originalLevels;

    std::mutex        output_mutex;
    std::stringstream printout;

    dd4hep::Detector* descriptionPtr{nullptr};

    // helpers to access the detector description
    dd4hep::Detector& description() { return *descriptionPtr; }
                      operator dd4hep::Detector&() { return *descriptionPtr; }
    dd4hep::Detector* operator->() { return descriptionPtr; }
  };
} // namespace Detector::Test
