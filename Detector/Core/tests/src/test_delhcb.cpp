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
#include <DD4hep/Detector.h>
#include <DD4hep/Factories.h>
#include <DD4hep/Printout.h>

#include <Core/DetectorDataService.h>
#include <Core/Keys.h>
#include <Core/MagneticFieldExtension.h>
#include <Core/MagneticFieldGridReader.h>
#include <Core/yaml_converters.h>

#include <Detector/LHCb/DeLHCb.h>

#include <exception>
#include <string>
#include <vector>

void _compare_lists( std::vector<std::string> actual, std::vector<std::string> expected, std::string name ) {
  if ( expected.size() == actual.size() ) {
    for ( size_t i = 0; i < expected.size(); i++ ) {
      if ( actual[i] != actual[i] ) {
        throw std::logic_error( "Incorrect DeLHCb " + name + " " + actual[i] + " instead of " + expected[i] );
      }
    }
  } else {
    throw std::logic_error(
        fmt::format( "Mismatch in {} list size, found: {} instead of {}", name, actual.size(), expected.size() ) );
  }
}

static long test_load_delhcb( dd4hep::Detector& description, int argc, char** argv ) {

  // XXX Boiler plate for the plugin, to be improved/factorized
  bool        help = false;
  std::string conditions;
  std::string field_map_path = "";
  for ( int i = 0; i < argc && argv[i]; ++i ) {
    if ( argv[i][0] == '-' || argv[i][0] == '/' ) {
      if ( 0 == ::strncmp( "-help", argv[i], 4 ) )
        help = true;
      else if ( 0 == ::strncmp( "-conditions", argv[i], 11 ) )
        conditions = argv[++i];
      else if ( 0 == ::strncmp( "-fieldmappath", argv[i], 13 ) ) {
        field_map_path = argv[++i];
      } else
        help = true;
    }
  }
  if ( help || conditions.empty() ) {
    /// Help printout describing the basic command line interface
    std::cout << "Usage: -plugin <name> -arg [-arg]                                   \n"
                 "     name:   factory name     LHCb_TEST_Magnet                      \n"
                 "     -detector   <name>       Name of the sub-detector to analyze.  \n"
                 "     -conditions <directory>  Top-directory with conditions files.  \n"
                 "     -fieldmappath <directory>  Directory containing the filed map files.  \n"
                 "                              Fully qualified: <protocol>://<path>  \n"
                 "     -help                    Show this help.                       \n"
                 "\tArguments given: "
              << dd4hep::arguments( argc, argv ) << std::endl;
    ::exit( EINVAL );
  }

  // Loading the detector and retrieving the condition with the file names for the Magnetic field service
  LHCb::Magnet::setup_magnetic_field_extension( description, field_map_path );
  std::vector<std::string>            detector_list{"/world", "Magnet", "VP"};
  LHCb::Detector::DetectorDataService dds( description, detector_list );
  dds.initialize( nlohmann::json{{"repository", conditions}} );

  int                    slice1_id     = 100;
  auto                   slice         = dds.get_slice( slice1_id );
  dd4hep::DetElement     lhcbdet       = description.detector( "/world" );
  LHCb::Detector::DeLHCb lhcb          = slice->get( lhcbdet, LHCb::Detector::Keys::deKey );
  std::string            detector_path = lhcb.placement().volume().name();

  if ( detector_path != "world_volume" ) { throw std::logic_error( "Detector path does not match" ); }

  std::vector<std::string> delhcb_children;
  std::vector<std::string> delhcb_grandchildren;
  lhcb.applyToAllChildren( [&delhcb_children, &delhcb_grandchildren]( LHCb::Detector::DeIOV d ) {
    delhcb_children.push_back( d.detector().path() );
    std::cout << "children" << d.detector().path() << '\n';
    try {
      d.applyToAllChildren( [&delhcb_grandchildren]( LHCb::Detector::DeIOV d2 ) {
        std::cout << "grandchildren" << d2.detector().path() << '\n';
        delhcb_grandchildren.push_back( d2.detector().path() );
      } );
    } catch ( const LHCb::Detector::NotImplemented& /*nie*/ ) {
      std::cout << "LHCb::Detector::NotImplemented for applyToAllChildren in " << d.detector().path() << '\n';
    }
  } );

  std::vector<std::string> expected_delhcb_children = {"/world/MagnetRegion/Magnet", "/world/BeforeMagnetRegion/VP"};
  std::vector<std::string> expected_delhcb_grandchildren = {"/world/BeforeMagnetRegion/VP/MotionVPLeft/VPLeft",
                                                            "/world/BeforeMagnetRegion/VP/MotionVPRight/VPRight"};
  _compare_lists( delhcb_children, expected_delhcb_children, "children" );
  _compare_lists( delhcb_grandchildren, expected_delhcb_grandchildren, "grandchildren" );

  // Finalizing the service and returning
  dds.finalize();
  return 1; // Plugins return 1 when they are successful
}
DECLARE_APPLY( LHCb_TEST_load_delhcb, test_load_delhcb )
