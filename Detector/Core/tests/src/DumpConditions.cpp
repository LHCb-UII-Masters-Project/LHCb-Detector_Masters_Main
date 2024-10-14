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
#include "DD4hep/ConditionsData.h"
#include "DD4hep/ConditionsPrinter.h"
#include "DD4hep/ConditionsProcessor.h"

#include "DD4hep/Detector.h"
#include "DD4hep/DetectorProcessor.h"
#include "DD4hep/Factories.h"
#include "DD4hep/Printout.h"

#include "Core/DetectorDataService.h"
#include "Core/Keys.h"
#include "Core/MagneticFieldExtension.h"

#include "Detector/Magnet/DeMagnet.h"
#include "Detector/VP/DeVP.h"

#include <cstring>
#include <exception>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

/**
 * DD4hep plugin that dumps the conditions of a specific detector, e.g. by running, from the Detector directory
 *
 * ./build.x86_64_v2-centos7-gcc11-dbg/bin/run geoPluginRun
 *           -input Detector/compact/trunk/LHCb.xml -plugin LHCb_DumpConditions -conditions
 * file:/`pwd`/tests/ConditionsIOV -iov 300 -path VP
 *
 */
static long dump_conditions( dd4hep::Detector& description, int argc, char** argv ) {

  bool        help = false;
  std::string conditions;
  int         iov = 100;
  std::string path( "" );
  for ( int i = 0; i < argc && argv[i]; ++i ) {
    if ( argv[i][0] == '-' || argv[i][0] == '/' ) {
      if ( 0 == ::strncmp( "-help", argv[i], 4 ) )
        help = true;
      else if ( 0 == ::strncmp( "-conditions", argv[i], 4 ) )
        conditions = argv[++i];
      else if ( 0 == ::strncmp( "-iov", argv[i], 4 ) )
        iov = atoi( argv[++i] );
      else if ( 0 == ::strncmp( "-path", argv[i], 5 ) )
        path = argv[++i];
      else
        help = true;
    }
  }
  if ( help || conditions.empty() ) {
    /// Help printout describing the basic command line interface
    std::cout << "Usage: -plugin <name> -arg [-arg]                                   \n"
                 "     name:   factory name     LHCb_DumpConditions                   \n"
                 "     -conditions <directory>  Top-directory with conditions files.  \n"
                 "                              Fully qualified: <protocol>://<path>  \n"
                 "     -iov <iov>               IOV for which to get the conditions   \n"
                 "     -path <path>             Detector path for which to9 retrieve conditions \n"
                 "     -help                    Show this help.                       \n"
                 "\tArguments given: "
              << dd4hep::arguments( argc, argv ) << std::endl;
    ::exit( EINVAL );
  }

  // Attaching the MagneticFieldExtension to the Magnet DetElement
  LHCb::Magnet::setup_magnetic_field_extension( description, "/cvmfs/lhcb.cern.ch/lib/lhcb/DBASE/FieldMap/v5r7/cdf" );

  // Now creating the DetectorDataService that will load the conditions
  std::vector<std::string> detector_list{"/world", "Magnet"};

  // Adding the requested detector if needed
  auto it = std::find( detector_list.begin(), detector_list.end(), path );
  if ( it == detector_list.end() ) { detector_list.emplace_back( path ); }

  LHCb::Detector::DetectorDataService dds( description, detector_list );
  dds.initialize( nlohmann::json{{"repository", conditions}} );
  auto slice = dds.get_slice( iov );

  // Creating the DD4hep condition printer
  dd4hep::cond::ConditionsPrinter printer( slice.get(), "",
                                           dd4hep::Condition::WITH_IOV | dd4hep::Condition::WITH_ADDRESS |
                                               dd4hep::Condition::WITH_TYPE | dd4hep::Condition::WITH_COMMENT |
                                               dd4hep::Condition::WITH_DATATYPE );
  // We could iterate like this but DD4hep doesn't handle YAML::Node
  // printer( description.detector( path ), 0 );

  // Gathering the conditions in a vector
  dd4hep::DetElement             de = description.detector( path );
  std::vector<dd4hep::Condition> condition_vec;
  dd4hep::cond::conditionsCollector( *slice.get(), condition_vec )( de, 0 );

  dd4hep::printout( dd4hep::INFO, "Condition", "++ %-3ld Conditions for DetElement %s", condition_vec.size(),
                    de.path().c_str() );
  for ( auto cond : condition_vec ) {
    printer( cond );

    // Custom print for JSON/YAML nodes
    if ( cond.isValid() ) {

      if ( !cond.is_bound() ) {
        dd4hep::printout( dd4hep::INFO, "Condition", "++ <Unbound-Condition>" );
      } else {
        const std::type_info& type = cond.typeInfo();
        if ( type == typeid( nlohmann::json ) ) {
          const auto& node = cond.get<nlohmann::json>();
          dd4hep::printout( dd4hep::INFO, "Condition", "++ JSON Content: %s", node.dump().c_str() );
        } else if ( type == typeid( YAML::Node ) ) {
          const auto&       node = cond.get<YAML::Node>();
          std::stringstream s;
          s << '\n' << node;
          auto st = s.str();
          dd4hep::printout( dd4hep::INFO, "Condition", "++ YAML Content: %s", st.c_str() );
        }
      }
    }
  }

  dds.finalize();
  return 1; // Plugins return 1 when they are successful
}

DECLARE_APPLY( LHCb_DumpConditions, dump_conditions )
