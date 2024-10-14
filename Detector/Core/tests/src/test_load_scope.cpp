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
#include "DD4hep/ConditionsPrinter.h"
#include "DD4hep/ConditionsProcessor.h"
#include "DD4hep/Detector.h"
#include "DD4hep/Factories.h"
#include "DD4hep/Printout.h"

#include "Core/DetectorDataService.h"
#include "Core/Keys.h"

#include <exception>
#include <string>
#include <vector>

static long test_load_scope( dd4hep::Detector& description, int argc, char** argv ) {

  bool        help = false;
  std::string conditions;
  for ( int i = 0; i < argc && argv[i]; ++i ) {
    if ( argv[i][0] == '-' || argv[i][0] == '/' ) {
      if ( 0 == ::strncmp( "-help", argv[i], 4 ) )
        help = true;
      else if ( 0 == ::strncmp( "-conditions", argv[i], 11 ) )
        conditions = argv[++i];
      else
        help = true;
    }
  }
  if ( help || conditions.empty() ) {
    /// Help printout describing the basic command line interface
    std::cout << "Usage: -plugin <name> -arg [-arg]                                   \n"
                 "     name:   factory name     LHCb_TEST_load_scope                      \n"
                 "     -detector   <name>       Name of the sub-detector to analyze.  \n"
                 "     -conditions <directory>  Top-directory with conditions files.  \n"
                 "     -help                    Show this help.                       \n"
                 "\tArguments given: "
              << dd4hep::arguments( argc, argv ) << '\n';
    ::exit( EINVAL );
  }

  // Loading the detector and retrieving the condition with the file names for the Magnetic field service
  std::vector<std::string>            detector_list{"/world", "scope"};
  LHCb::Detector::DetectorDataService dds( description, detector_list );
  dds.initialize( nlohmann::json{{"repository", conditions}} );

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
          auto              node = cond.get<YAML::Node>();
          std::stringstream s;
          s << '\n' << node;
          auto st = s.str();
          dd4hep::printout( dd4hep::INFO, "Condition", "++ YAML Content: %s", st.c_str() );
        }
      }
    }
  }
  dds.finalize();
  return 0;
}
DECLARE_APPLY( LHCb_TEST_load_scope, test_load_scope )
