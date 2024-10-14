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
#include "Core/MagneticFieldExtension.h"
#include "Core/yaml_converters.h"

#include "Detector/Magnet/DeMagnet.h"
#include "Detector/VP/DeVP.h"
#include <Detector/Test/Fixture.h>

#include <cstring>
#include <exception>
#include <nlohmann/json.hpp>
#include <regex>
#include <string>
#include <vector>

#include <range/v3/action/split.hpp>
#include <range/v3/range/primitives.hpp>
#include <range/v3/view/all.hpp>
#include <range/v3/view/c_str.hpp>
#include <range/v3/view/filter.hpp>

namespace {
  namespace ra = ranges::actions;
  namespace rv = ranges::views;
} // namespace

static long test_DetectorDataService( dd4hep::Detector& description, int argc, char** argv ) {

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
    std::cout << "Usage: -plugin <name> -arg [-arg]                                   \n"
                 "     name:   factory name     LHCb_TEST_cond_content                \n"
                 "     -detector   <name>       Name of the sub-detector to analyze.  \n"
                 "     -conditions <directory>  Top-directory with conditions files.  \n"
                 "                              Fully qualified: <protocol>://<path>  \n"
                 "     -help                    Show this help.                       \n"
                 "\tArguments given: "
              << dd4hep::arguments( argc, argv ) << std::endl;
    ::exit( EINVAL );
  }

  // Attaching the MagneticFieldExtension to the Magnet DetElement
  LHCb::Magnet::setup_magnetic_field_extension( description, "/cvmfs/lhcb.cern.ch/lib/lhcb/DBASE/FieldMap/v5r7/cdf" );

  // Now creating the DetectorDataService that will load the conditions
  std::vector<std::string>            detector_list{"/world", "VP", "Magnet"};
  LHCb::Detector::DetectorDataService dds( description, detector_list );
  dds.initialize( nlohmann::json{{"repository", conditions}} );

  // Checking the value of a given conditions
  auto get_test_value = [&dds, &description]( int iov ) -> double {
    auto        slice = dds.get_slice( iov );
    const auto& cond =
        slice->get( description.detector( "VP" ), LHCb::Detector::item_key( "TestCond" ) ).get<nlohmann::json>();
    const auto& values = cond["values"];
    for ( std::size_t i = 0; i < values.size(); i++ ) {
      dd4hep::printout( dd4hep::INFO, "test_DetectorDataService", "value[%d]: %f", i, values[i].get<double>() );
    }
    return values[0].get<double>();
  };

  double test_value = get_test_value( 100 );
  if ( !LHCb::essentiallyEqual( test_value, 4.2 ) ) {
    dd4hep::printout( dd4hep::ERROR, "test_DetectorDataService", "Wrong value[0]: %f", test_value );
    ::exit( EINVAL );
  }

  test_value = get_test_value( 200 );
  if ( !LHCb::essentiallyEqual( test_value, 0.1 ) ) {
    dd4hep::printout( dd4hep::ERROR, "test_DetectorDataService", "Wrong value[0]: %f", test_value );
    ::exit( EINVAL );
  }

  // Now checking the MagneticField
  auto                            slice     = dds.get_slice( 200 );
  dd4hep::DetElement              magnetdet = description.detector( "Magnet" );
  const LHCb::Detector::DeMagnet& m         = slice->get( magnetdet, LHCb::Detector::Keys::deKey );
  const auto                      f         = m.fieldVector( ROOT::Math::XYZPoint{-400, 0, 0} );

  double maxdiff = 1e-5;
  auto   check   = [&maxdiff]( double val, double ref, std::string name ) {
    if ( abs( ( val - ref ) / val ) > maxdiff ) {
      dd4hep::printout( dd4hep::ERROR, "test_DetectorDataService", "Error with %s", name.c_str() );
      ::exit( EINVAL );
    }
  };

  // Field should be -1.18417e-07,-6.66836e-06,1.30723e-06 at that point...
  // std::cout << "field:" << f << std::endl;
  check( f.X(), -1.18417e-07, "field map X component" );
  check( f.Y(), -6.66836e-06, "field map Y component" );
  check( f.Z(), 1.30723e-06, "field map Z component" );

  dds.finalize();
  return 1; // Plugins return 1 when they are successful
}
DECLARE_APPLY( LHCb_TEST_DetectorDataService, test_DetectorDataService )

static long test_VPPosition( dd4hep::Detector& description, int argc, char** argv ) {

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
    std::cout << "Usage: -plugin <name> -arg [-arg]                                   \n"
                 "     name:   factory name     LHCb_TEST_cond_content                \n"
                 "     -detector   <name>       Name of the sub-detector to analyze.  \n"
                 "     -conditions <directory>  Top-directory with conditions files.  \n"
                 "                              Fully qualified: <protocol>://<path>  \n"
                 "     -help                    Show this help.                       \n"
                 "\tArguments given: "
              << dd4hep::arguments( argc, argv ) << std::endl;
    ::exit( EINVAL );
  }

  std::vector<std::string>            detector_list{"/world", "VP"};
  LHCb::Detector::DetectorDataService dds( description, detector_list );
  dds.initialize( nlohmann::json{{"repository", conditions}} );

  auto print_position = []( int slice_id, const char* det, const ROOT::Math::XYZPoint& pos ) {
    dd4hep::printout( dd4hep::INFO, "Test_VPPosition", "Slice %d, %s Position: (%f, %f, %f)", slice_id, det, pos.X(),
                      pos.Y(), pos.Z() );
  };

  // Getting the condition object for the DeVP
  int                         slice1_id = 100;
  auto                        slice     = dds.get_slice( slice1_id );
  dd4hep::DetElement          vpdet     = description.detector( "VP" );
  const LHCb::Detector::DeVP& vp        = slice->get( vpdet, LHCb::Detector::Keys::deKey );
  auto                        vppos     = vp.toGlobal( ROOT::Math::XYZPoint{0, 0, 0} );
  print_position( slice1_id, "VP", vppos );

  // Getting the slice for time 200 with displacement
  int                         slice2_id = 200;
  auto                        slice2    = dds.get_slice( slice2_id );
  const LHCb::Detector::DeVP& vp2       = slice2->get( vpdet, LHCb::Detector::Keys::deKey );
  auto                        vppos2    = vp2.toGlobal( ROOT::Math::XYZPoint{0, 0, 0} );
  print_position( slice2_id, "VP", vppos2 );
  auto diff2 = vppos2 - ROOT::Math::XYZPoint{1, 1, 1}; // LHCb units, so in mm;
  if ( diff2.Mag2() > 0.001 ) {
    std::cout << "slice for time 200 has wrong displacement " << vppos2 << " - expected {1, 1, 1}\n";
    ::exit( EINVAL );
  }
  // Checking the position of the right side of the VP
  const auto side2    = vp2.right();
  auto       sidepos2 = side2.toGlobal( ROOT::Math::XYZPoint{0, 0, 0} );
  print_position( slice2_id, "Right Side", sidepos2 );

  // Checking the position
  int                         slice3_id = 550;
  auto                        slice3    = dds.get_slice( slice3_id );
  const LHCb::Detector::DeVP& vp3       = slice3->get( vpdet, LHCb::Detector::Keys::deKey );
  auto                        vppos3    = vp3.toGlobal( ROOT::Math::XYZPoint{0, 0, 0} );
  print_position( slice3_id, "VP", vppos3 );

  // Checking the position of the right side of the VP
  const auto side3    = vp3.right();
  auto       sidepos3 = side3.toGlobal( ROOT::Math::XYZPoint{0, 0, 0} );
  print_position( slice3_id, "Right Side", sidepos3 );
  auto diff3 = sidepos3 - ROOT::Math::XYZPoint{6, 6, 6}; // LHCb units, so in mm
  if ( diff3.Mag2() > 0.001 ) {
    std::cout << "position of the right side of the VP wrong : " << sidepos3 << " - expected {6, 6, 6}\n";
    ::exit( EINVAL );
  }

  // Finalizing the service and returning
  dds.finalize();
  return 1; // Plugins return 1 when they are successful
}
DECLARE_APPLY( LHCb_TEST_VPPosition, test_VPPosition )

void _iterate_detector_tree( dd4hep::DetElement det, const dd4hep::cond::ConditionsSlice& slice,
                             const std::string& searchName ) {

  // std::cout << det.path() << "|" << det.name() << "|" << det.volumeID() << "|" << det.placementPath() << std::endl;
  const char* detname = det.name();
  if ( ::strncmp( detname, searchName.c_str(), ::strlen( detname ) ) == 0 ) {
    std::cout << "Found: " << det.path() << std::endl;
    dd4hep::Alignment a = slice.get( det, LHCb::Detector::Keys::alignmentKey );
    std::cout << a.localToWorld( dd4hep::Position{0, 0, 0} ) << std::endl;
  }

  // Iterating on children
  for ( const auto& [name, child_det] : det.children() ) { _iterate_detector_tree( child_det, slice, searchName ); }
}

static long test_VPLadders( dd4hep::Detector& description, int argc, char** argv ) {

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
    std::cout << "Usage: -plugin <name> -arg [-arg]                                   \n"
                 "     name:   factory name     LHCb_TEST_cond_content                \n"
                 "     -detector   <name>       Name of the sub-detector to analyze.  \n"
                 "     -conditions <directory>  Top-directory with conditions files.  \n"
                 "                              Fully qualified: <protocol>://<path>  \n"
                 "     -help                    Show this help.                       \n"
                 "\tArguments given: "
              << dd4hep::arguments( argc, argv ) << std::endl;
    ::exit( EINVAL );
  }

  std::vector<std::string>            detector_list{"/world", "VP"};
  LHCb::Detector::DetectorDataService dds( description, detector_list );
  dds.initialize( nlohmann::json{{"repository", conditions}} );
  int  slice1_id = 100;
  auto slice     = dds.get_slice( slice1_id );

  _iterate_detector_tree( description.detector( "VP" ), *slice, "ladder_0" );

  return 1; // Plugins return 1 when they are successful
}
DECLARE_APPLY( LHCb_TEST_VPLadders, test_VPLadders )

static long test_VPLadders_conditions( dd4hep::Detector& description, int argc, char** argv ) {

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
    std::cout << "Usage: -plugin <name> -arg [-arg]                                   \n"
                 "     name:   factory name     LHCb_TEST_cond_content                \n"
                 "     -detector   <name>       Name of the sub-detector to analyze.  \n"
                 "     -conditions <directory>  Top-directory with conditions files.  \n"
                 "                              Fully qualified: <protocol>://<path>  \n"
                 "     -help                    Show this help.                       \n"
                 "\tArguments given: "
              << dd4hep::arguments( argc, argv ) << std::endl;
    ::exit( EINVAL );
  }

  std::vector<std::string>            detector_list{"/world", "VP"};
  LHCb::Detector::DetectorDataService dds( description, detector_list );
  dds.initialize( nlohmann::json{{"repository", conditions}} );
  int  slice1_id = 100;
  auto slice     = dds.get_slice( slice1_id );

  const double shimSize{0.0 * dd4hep::mm};

  auto              det = description.detector( "/world/BeforeMagnetRegion/VP/MotionVPLeft/VPLeft/Module01/ladder_0" );
  dd4hep::Alignment a   = slice->get( det, LHCb::Detector::Keys::alignmentKey );
  auto              position = a.localToWorld( dd4hep::Position{0, 0, 0} );
  std::cout << "Ladder position: " << position << std::endl;
  dd4hep::Position expected{-1.27244 * dd4hep::cm + shimSize, 3.9803 * dd4hep::cm, -127.535 * dd4hep::cm};
  auto             diff = expected - position;
  if ( diff.r() > 1e-5 * dd4hep::cm ) {
    std::cout << "Incorrect ladder position from Conditions. distance from expected: " << diff.r() / dd4hep::cm << "cm"
              << std::endl;
    ::exit( EINVAL );
  }
  return 1; // Plugins return 1 when they are successful
}
DECLARE_APPLY( LHCb_TEST_VPLadders_conditions, test_VPLadders_conditions )

static long test_VPSensors( dd4hep::Detector& description, int argc, char** argv ) {

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
    std::cout << "Usage: -plugin <name> -arg [-arg]                                   \n"
                 "     name:   factory name     LHCb_TEST_cond_content                \n"
                 "     -detector   <name>       Name of the sub-detector to analyze.  \n"
                 "     -conditions <directory>  Top-directory with conditions files.  \n"
                 "                              Fully qualified: <protocol>://<path>  \n"
                 "     -help                    Show this help.                       \n"
                 "\tArguments given: "
              << dd4hep::arguments( argc, argv ) << std::endl;
    ::exit( EINVAL );
  }

  std::vector<std::string>            detector_list{"/world", "VP"};
  LHCb::Detector::DetectorDataService dds( description, detector_list );
  dds.initialize( nlohmann::json{{"repository", conditions}} );
  int  slice1_id = 100;
  auto slice     = dds.get_slice( slice1_id );

  _iterate_detector_tree( description.detector( "VP" ), *slice, "sensor" );

  return 1; // Plugins return 1 when they are successful
}
DECLARE_APPLY( LHCb_TEST_VPSensors, test_VPSensors )

static long test_VPSensors_conditions( dd4hep::Detector& description, int argc, char** argv ) {

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
    std::cout << "Usage: -plugin <name> -arg [-arg]                                   \n"
                 "     name:   factory name     LHCb_TEST_VPSensors                   \n"
                 "     -detector   <name>       Name of the sub-detector to analyze.  \n"
                 "     -conditions <directory>  Top-directory with conditions files.  \n"
                 "                              Fully qualified: <protocol>://<path>  \n"
                 "     -help                    Show this help.                       \n"
                 "\tArguments given: "
              << dd4hep::arguments( argc, argv ) << std::endl;
    ::exit( EINVAL );
  }

  std::vector<std::string>            detector_list{"/world", "VP"};
  LHCb::Detector::DetectorDataService dds( description, detector_list );
  dds.initialize( nlohmann::json{{"repository", conditions}} );
  int  slice1_id = 0;
  auto slice     = dds.get_slice( slice1_id );

  const double shimSize{0.0 * dd4hep::mm};

  constexpr std::size_t                         nSensorsToTest{4};
  const std::array<std::string, nSensorsToTest> locations{
      "/world/BeforeMagnetRegion/VP/MotionVPRight/VPRight/Module00/ladder_0/sensor",
      "/world/BeforeMagnetRegion/VP/MotionVPRight/VPRight/Module00/ladder_1/sensor",
      "/world/BeforeMagnetRegion/VP/MotionVPRight/VPRight/Module00/ladder_2/sensor",
      "/world/BeforeMagnetRegion/VP/MotionVPRight/VPRight/Module00/ladder_3/sensor"};
  const std::array<dd4hep::Position, nSensorsToTest> expected{
      dd4hep::Position{1.27173155 * dd4hep::cm - shimSize, -3.98242539 * dd4hep::cm, ( -28.75 - 0.0641 ) * dd4hep::cm},
      dd4hep::Position{-2.61346666 * dd4hep::cm - shimSize, -2.04177085 * dd4hep::cm, ( -28.75 + 0.0641 ) * dd4hep::cm},
      dd4hep::Position{-0.98393919 * dd4hep::cm - shimSize, 1.72675476 * dd4hep::cm, ( -28.75 + 0.0641 ) * dd4hep::cm},
      dd4hep::Position{-4.96636488 * dd4hep::cm - shimSize, -0.31112698 * dd4hep::cm,
                       ( -28.75 - 0.0641 ) * dd4hep::cm}};

  for ( std::size_t i{0}; i < nSensorsToTest; ++i ) {
    auto              det      = description.detector( locations[i] );
    dd4hep::Alignment a        = slice->get( det, LHCb::Detector::Keys::alignmentKey );
    auto              position = a.localToWorld( dd4hep::Position{0, 0, 0} );
    std::cout << "Sensor position: " << position << std::endl;
    auto diff = expected[i] - position;
    if ( diff.r() > 1e-5 ) {
      std::cout << "Incorrect sensor position from Conditions. Distance from expected: " << diff.r() / dd4hep::cm
                << "cm" << std::endl;
      std::cout << position << " vs " << expected[i] << std::endl;
      ::exit( EINVAL );
    }
  }

  return 1; // Plugins return 1 when they are successful
}
DECLARE_APPLY( LHCb_TEST_VPSensors_conditions, test_VPSensors_conditions )

static long test_VP_motion( dd4hep::Detector& description, int argc, char** argv ) {

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
    std::cout << "Usage: -plugin <name> -arg [-arg]                                   \n"
                 "     name:   factory name     LHCb_TEST_cond_content                \n"
                 "     -detector   <name>       Name of the sub-detector to analyze.  \n"
                 "     -conditions <directory>  Top-directory with conditions files.  \n"
                 "                              Fully qualified: <protocol>://<path>  \n"
                 "     -help                    Show this help.                       \n"
                 "\tArguments given: "
              << dd4hep::arguments( argc, argv ) << std::endl;
    ::exit( EINVAL );
  }

  std::vector<std::string>            detector_list{"/world", "VP"};
  LHCb::Detector::DetectorDataService dds( description, detector_list );
  dds.initialize( nlohmann::json{{"repository", conditions}} );

  std::string det_path       = "/world/BeforeMagnetRegion/VP/MotionVPLeft/VPLeft";
  auto        get_vpleft_pos = [&dds, &det_path, &description]( int sliceid ) {
    auto              slice    = dds.get_slice( sliceid );
    auto              det      = description.detector( det_path );
    dd4hep::Alignment a        = slice->get( det, LHCb::Detector::Keys::alignmentKey );
    auto              position = a.localToWorld( dd4hep::Position{0, 0, 0} );
    return position;
  };

  // Getting the first position where motion should be 0
  auto position100 = get_vpleft_pos( 100 );

  std::cout << "run 100 - VP Left position: " << position100 << '\n';

  auto position1000 = get_vpleft_pos( 1000 );

  std::cout << "run 1000 - VP Left position: " << position1000 << '\n';

  auto diff = position1000 - position100;
  if ( abs( diff.r() - 0.5 * dd4hep::cm ) > 1e-5 ) {
    std::cout << "Incorrect VP motion. Difference with expected: " << diff.r() / dd4hep::cm << "cm" << std::endl;
    ::exit( EINVAL );
  }

  std::cout << "Getting VP for run 1000\n";

  auto get_vp = [&dds, &description]( int sliceid ) {
    auto                        slice = dds.get_slice( sliceid );
    auto                        det   = description.detector( "VP" );
    const LHCb::Detector::DeVP& vp    = slice->get( det, LHCb::Detector::Keys::deKey );
    return vp;
  };

  const LHCb::Detector::DeVP& vp       = get_vp( 1000 );
  auto                        leftpos  = vp.left().motionSystemTransform().value()( ROOT::Math::XYZPoint{0, 0, 0} );
  auto                        rightpos = vp.right().motionSystemTransform().value()( ROOT::Math::XYZPoint{0, 0, 0} );

  std::cout << "VPLeft motionSystemTransform: " << leftpos << '\n';
  std::cout << "VPright motionSystemTransform: " << rightpos << '\n';

  if ( abs( leftpos.X() - 5 ) > 1e-5 || abs( leftpos.X() - 5 ) > 1e-5 ) {
    std::cout << "motionSystemTransform\n";
    ::exit( EINVAL );
  }

  auto beamspot =
      ( ROOT::Math::XYZVector{vp.toGlobal( leftpos )} + ROOT::Math::XYZVector{vp.toGlobal( rightpos )} ) / 2.0;
  std::cout << "BeamSpot:" << beamspot << '\n';

  return 1; // Plugins return 1 when they are successful
}
DECLARE_APPLY( LHCb_TEST_VP_motion, test_VP_motion )
