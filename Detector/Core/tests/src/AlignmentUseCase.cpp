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
#include "DD4hep/Detector.h"
#include "DD4hep/Factories.h"
#include "DD4hep/Printout.h"

#include "Core/DetectorDataService.h"
#include "Core/FloatComparison.h"
#include "Core/Keys.h"
#include "Core/yaml_converters.h"

#include "Detector/VP/DeVP.h"

#include <exception>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace {
  bool operator==( const dd4hep::Delta& lhs, const dd4hep::Delta& rhs ) {
    return lhs.flags == rhs.flags && ( !lhs.hasTranslation() || lhs.translation == rhs.translation ) &&
           ( !lhs.hasRotation() || lhs.rotation == rhs.rotation ) && ( !lhs.hasPivot() || lhs.pivot == rhs.pivot );
  }
  bool operator!=( const dd4hep::Delta& lhs, const dd4hep::Delta& rhs ) { return !( lhs == rhs ); }

  long test_AlignmentUseCase( dd4hep::Detector& description, int argc, char** argv ) {
    bool        help = false;
    std::string conditions;
    for ( int i = 0; i < argc && argv[i]; ++i ) {
      if ( argv[i][0] == '-' || argv[i][0] == '/' ) {
        if ( 0 == ::strncmp( "-help", argv[i], 4 ) )
          help = true;
        else if ( 0 == ::strncmp( "-conditions", argv[i], 4 ) )
          conditions = argv[++i];
        else
          help = true;
      }
    }
    if ( help || conditions.empty() ) {
      /// Help printout describing the basic command line interface
      std::cout << R"(
Usage: -plugin LHCb_TEST_AlignmentUseCase -conditions <directory> [-arg]
     -conditions <directory>  Top-directory with conditions files.
                              Fully qualified: <protocol>://<path> 
     -help                    Show this help.
  Arguments given: )"
                << dd4hep::arguments( argc, argv ) << std::endl;
      ::exit( EINVAL );
    }

    using nlohmann::json;

    std::vector<std::string>            detector_list{"/world", "VP"};
    LHCb::Detector::DetectorDataService dds( description, detector_list );
    dds.initialize( nlohmann::json{{"repository", conditions}, {"overlay", true}} );
    dd4hep::printout( dd4hep::INFO, "AlignmentUseCase", "Initialize done %s", conditions.c_str() );

    auto get_test_value = [&dds, &description]( int iov ) -> double {
      auto        slice = dds.get_slice( iov );
      const auto& cond = slice->get( description.detector( "VP" ), LHCb::Detector::item_key( "TestCond" ) ).get<json>();
      const auto& values = cond["values"];
      for ( std::size_t i = 0; i < values.size(); i++ ) {
        dd4hep::printout( dd4hep::INFO, "YamlCondition", "values[%d]: %f", i, values[i].get<double>() );
      }
      return values[0].get<double>();
    };

    double test_value = get_test_value( 100 );
    if ( !LHCb::essentiallyEqual( test_value, 4.2 ) ) {
      dd4hep::printout( dd4hep::ERROR, "YamlCondition", "Wrong values[0]: %f", test_value );
      ::exit( EINVAL );
    }

    // test error conditions
    try {
      dds.update_condition( description.detector( "VP" ), "Whatever",
                            YAML::Load( "values: [3.14, 123.456, 654.321]" ) );
      dd4hep::printout( dd4hep::ERROR, "AlignmentUseCase", "I was expecting an exception" );
      ::exit( EINVAL );
    } catch ( std::exception& err ) {
      if ( err.what() != std::string_view{"cannot find Whatever in VP's ConditionsRepository"} ) {
        dd4hep::printout( dd4hep::ERROR, "AlignmentUseCase", "I was expecting another exception, not '%s'",
                          err.what() );
        ::exit( EINVAL );
      }
      dd4hep::printout( dd4hep::INFO, "AlignmentUseCase", "got (expected) exception: '%s'", err.what() );
    }

    // update the test condition
    dds.update_condition( description.detector( "VP" ), "TestCond", YAML::Load( "values: [3.14, 123.456, 654.321]" ) );

    // these numbers were chosen to make the double comparison work
    dd4hep::Delta new_align( dd4hep::Position( 0.001 * dd4hep::mm, 0.002 * dd4hep::mm, 0.003 * dd4hep::mm ) );
    dds.update_alignment( description.detector( "VP" ), new_align );

    // force reload of the slice
    dds.drop_slice( 100 );

    test_value = get_test_value( 100 );
    if ( !LHCb::essentiallyEqual( test_value, 3.14 ) ) {
      dd4hep::printout( dd4hep::ERROR, "YamlCondition", "Wrong values[0]: %f", test_value );
      ::exit( EINVAL );
    }

    // helper to make sure we start with a clean slate and we clean up when we are done
    {
      namespace fs = std::filesystem;
      struct clean_up {
        clean_up() { fs::remove_all( "new_conditions" ); }
        ~clean_up() { fs::remove_all( "new_conditions" ); }
      } cleaner;
      dds.dump_conditions( "new_conditions" );

      if ( !fs::is_regular_file( "new_conditions/Conditions/VP/conditions.yml" ) ) {
        dd4hep::printout( dd4hep::ERROR, "YamlCondition", "Missing expected file %s",
                          "new_conditions/Conditions/VP/conditions.yml" );
        ::exit( EINVAL );
      }
      {
        auto doc = YAML::LoadFile( "new_conditions/Conditions/VP/conditions.yml" );
        if ( !( doc["TestCond"].IsMap() && doc["TestCond"]["values"].IsSequence() &&
                doc["TestCond"]["values"].as<std::vector<double>>() == std::vector<double>{3.14, 123.456, 654.321} ) ) {
          YAML::Emitter out;
          out << doc;
          dd4hep::printout( dd4hep::ERROR, "YamlCondition", "Wrong new conditions data\n%s", out.c_str() );
          ::exit( EINVAL );
        }
      }

      if ( !fs::is_regular_file( "new_conditions/Conditions/VP/Alignment/Global.yml" ) ) {
        dd4hep::printout( dd4hep::ERROR, "YamlCondition", "Missing expected file %s",
                          "new_conditions/Conditions/VP/Alignment/Global.yml" );
        ::exit( EINVAL );
      }
      {
        auto doc  = YAML::LoadFile( "new_conditions/Conditions/VP/Alignment/Global.yml" );
        auto cond = LHCb::YAMLConverters::make_condition( "VPSystem", doc["VPSystem"] );

        if ( auto delta = cond.get<dd4hep::Delta>(); delta != new_align ) {
          YAML::Emitter out;
          out << YAML::BeginMap;
          out << YAML::Key << "VPSystem";
          out << YAML::Value << doc["VPSystem"];
          out << YAML::EndMap;
          dd4hep::printout( dd4hep::ERROR, "YamlCondition", "Wrong new conditions data\n%s", out.c_str() );
          ::exit( EINVAL );
        }
      }
    }
    dds.finalize();

    return 1; // Plugins return 1 when they are successful
  }
} // namespace
DECLARE_APPLY( LHCb_TEST_AlignmentUseCase, test_AlignmentUseCase )
