/*****************************************************************************\
* (c) Copyright 2022-2023 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#include <catch2/catch.hpp>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "DD4hep/ConditionsPrinter.h"
#include "DD4hep/ConditionsProcessor.h"
#include "DD4hep/Detector.h"
#include "DD4hep/Factories.h"
#include "DD4hep/Printout.h"

#include "Core/DetectorDataService.h"
#include "Core/Keys.h"

TEST_CASE( "scope_test" ) {

  const char* compact_path    = "tests/testscope/scope.xml";
  const char* conditions_path = "file:tests/testscope/";

  dd4hep::Detector& description = dd4hep::Detector::getInstance();
  description.fromXML( compact_path );
  std::vector<std::string>            detector_list{"/world", "scope"};
  LHCb::Detector::DetectorDataService dds( description, detector_list );
  dds.initialize( nlohmann::json{{"repository", conditions_path}} );

  auto                        slice = dds.get_slice( 100 );
  dd4hep::DetElement          det   = description.detector( "/world/scope/side1/sensor0" );
  const LHCb::Detector::DeIOV scope = slice->get( det, LHCb::Detector::Keys::deKey );

  // Creating the DD4hep condition printer
  dd4hep::cond::ConditionsPrinter printer( slice.get(), "",
                                           dd4hep::Condition::WITH_IOV | dd4hep::Condition::WITH_ADDRESS |
                                               dd4hep::Condition::WITH_TYPE | dd4hep::Condition::WITH_COMMENT |
                                               dd4hep::Condition::WITH_DATATYPE );
  std::vector<dd4hep::Condition>  condition_vec;
  dd4hep::cond::conditionsCollector( *slice.get(), condition_vec )( det, 0 );
  dd4hep::printout( dd4hep::INFO, "Condition", "++ %-3ld Conditions for DetElement %s", condition_vec.size(),
                    det.path().c_str() );
  for ( auto cond : condition_vec ) {
    printer( cond );

    // Custom print for YAML nodes
    if ( cond.isValid() ) {

      if ( !cond.is_bound() ) {
        dd4hep::printout( dd4hep::INFO, "Condition", "++ <Unbound-Condition>" );
      } else {
        const std::type_info& type = cond.typeInfo();
        if ( type == typeid( YAML::Node ) ) {
          auto node = cond.get<YAML::Node>();
          std::cout << node["value"] << '\n';
          std::stringstream s;
          s << '\n' << node;
          auto st = s.str();
          dd4hep::printout( dd4hep::INFO, "Condition", "++ YAML Content: %s", st.c_str() );
          int value = node["value"].as<int>();
          CHECK( value == 40 );
        }
      }
    }
  }
  dds.finalize();
}
