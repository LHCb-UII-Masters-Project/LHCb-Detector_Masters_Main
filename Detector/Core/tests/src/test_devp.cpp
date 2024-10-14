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
#include "Core/Keys.h"
#include "Core/MagneticFieldExtension.h"
#include "Core/MagneticFieldGridReader.h"
#include "Core/yaml_converters.h"

#include "Detector/VP/DeVP.h"

#include <exception>
#include <string>
#include <vector>

static long test_load_devp( dd4hep::Detector& description, int argc, char** argv ) {

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
                 "     name:   factory name     LHCb_TEST_load_devp_detailed          \n"
                 "     -conditions <directory>  Top-directory with conditions files.  \n"
                 "     -fieldmappath <directory>  Directory containing the filed map files.  \n"
                 "                              Fully qualified: <protocol>://<path>  \n"
                 "     -help                    Show this help.                       \n"
                 "\tArguments given: "
              << dd4hep::arguments( argc, argv ) << '\n';
    ::exit( EINVAL );
  }

  // Loading the detector and retrieving the condition with the file names for the Magnetic field service
  LHCb::Magnet::setup_magnetic_field_extension( description, field_map_path );
  std::vector<std::string>            detector_list{"/world", "Magnet", "Muon", "VP"};
  LHCb::Detector::DetectorDataService dds( description, detector_list );
  dds.initialize( nlohmann::json{{"repository", conditions}} );

  // The fake run number 100 should give conditions with null alignment and motion
  auto                        slice = dds.get_slice( 100 );
  dd4hep::DetElement          vpdet = description.detector( "VP" );
  const LHCb::Detector::DeVP& vp    = slice->get( vpdet, LHCb::Detector::Keys::deKey );

  bool allChecksOK{true};

  const ROOT::Math::XYZPoint zero{0.0, 0.0, 0.0};

  // Check the global position of the VP
  auto diff = vp.toGlobal( zero ) - zero;
  if ( diff.r() > 1e-5 ) {
    std::cerr << "VP {0, 0, 0} is " << vp.toGlobal( zero ) << ", expected " << zero << std::endl;
    allChecksOK = false;
  }

  // Check the number of sensors
  const auto nSensors = vp.numberSensors();
  if ( nSensors != 208 ) {
    std::cout << "Number of sensors = " << nSensors << std::endl;
    allChecksOK = false;
  }

  // Check the number of modules and global position of the C-side
  const LHCb::Detector::DeVPSide rightSide = vp.right();
  if ( rightSide.size() != 26 ) {
    std::cerr << "Number of modules on right side = " << rightSide.size() << std::endl;
    allChecksOK = false;
  }
  diff = rightSide.toGlobal( zero ) - zero;
  if ( diff.r() > 1e-5 ) {
    std::cerr << "C-side {0, 0, 0} is " << rightSide.toGlobal( zero ) << ", expected " << zero << std::endl;
    allChecksOK = false;
  }

  // Check the number of modules and global position of the A-side
  const LHCb::Detector::DeVPSide leftSide = vp.left();
  if ( leftSide.size() != 26 ) {
    std::cerr << "Number of modules on left side = " << leftSide.size() << std::endl;
    allChecksOK = false;
  }
  diff = leftSide.toGlobal( zero ) - zero;
  if ( diff.r() > 1e-5 ) {
    std::cerr << "A-side {0, 0, 0} is " << leftSide.toGlobal( zero ) << ", expected " << zero << std::endl;
    allChecksOK = false;
  }

  // Check that accessing the sensors by sensor number works ok
  for ( size_t iSensor{0}; iSensor < nSensors; ++iSensor ) {
    const LHCb::Detector::DeVPSensor sensor = vp.sensor( LHCb::Detector::VPChannelID::SensorID( iSensor ) );
    if ( to_unsigned( sensor.sensorNumber() ) != iSensor ) {
      std::cerr << "Sensor " << iSensor << " returns sensor number = " << to_unsigned( sensor.sensorNumber() )
                << std::endl;
      allChecksOK = false;
    }
    if ( sensor.isLeft() != ( ( iSensor / 4 ) % 2 ) ) {
      std::cerr << "Sensor " << iSensor << " returns isLeft = " << sensor.isLeft() << std::endl;
      allChecksOK = false;
    }
  }

  const double shimSize{0.0 * dd4hep::mm};

  const std::array<double, VP::NModules> moduleZPositions{
      -287.5, -275.0, -262.5, -250.0, -237.5, -225.0, -212.5, -200.0, -137.5, -125.0, -62.5, -50.0, -37.5,
      -25.0,  -12.5,  0.0,    12.5,   25.0,   37.5,   50.0,   62.5,   75.0,   87.5,   100.0, 112.5, 125.0,
      137.5,  150.0,  162.5,  175.0,  187.5,  200.0,  212.5,  225.0,  237.5,  250.0,  262.5, 275.0, 312.5,
      325.0,  387.5,  400.0,  487.5,  500.0,  587.5,  600.0,  637.5,  650.0,  687.5,  700.0, 737.5, 750.0};

  const std::array<double, VP::NSensorsPerModule> sensorXPositions{-12.7173, 26.1347, 9.8394, 49.6636};

  const std::array<double, VP::NSensorsPerModule> sensorYPositions{39.8243, 20.4177, -17.2675, 3.1113};

  const std::array<std::array<double, 4>, VP::NSensorsPerModule> cornerXPositions{{{-12.717, -22.635, 17.345, 7.428},
                                                                                   {26.096, 16.179, -3.967, -13.884},
                                                                                   {9.839, -0.078, 39.902, 29.985},
                                                                                   {49.625, 39.708, 19.562, 9.645}}};

  const std::array<std::array<double, 4>, VP::NSensorsPerModule> cornerYPositions{{{39.785, 29.868, 9.723, -0.194},
                                                                                   {20.418, 10.500, 50.480, 40.563},
                                                                                   {-17.229, -7.311, 12.834, 22.751},
                                                                                   {3.111, 13.029, -26.951, -17.034}}};

  vp.runOnAllSensors( [&]( const LHCb::Detector::DeVPSensor& sensor ) {
    const auto sNumID    = sensor.sensorNumber();
    const auto sNum      = to_unsigned( sNumID );
    const auto mNum      = sNum / VP::NSensorsPerModule;
    const auto sNumOnMod = sNum % VP::NSensorsPerModule;
    if ( sNumOnMod == 0 ) {
      // first sensor in module, check module info
      auto                 module             = vp.module( sNumID );
      auto                 moduleZeroInGlobal = module.toGlobal( zero );
      auto                 moduleXPos         = ( mNum % 2 == 0 ) ? -shimSize : shimSize;
      ROOT::Math::XYZPoint expectedPos{moduleXPos, 0.0, moduleZPositions[mNum]};
      diff = moduleZeroInGlobal - expectedPos;
      if ( diff.r() > 1e-4 ) {
        std::cerr << "Module " << mNum << " global position is " << moduleZeroInGlobal << ", expected " << expectedPos
                  << std::endl;
        std::cerr << "Difference = " << diff.r() << std::endl;
        allChecksOK = false;
      }
    }

    auto sensorZPos = moduleZPositions[mNum];
    if ( sNumOnMod == 0 || sNumOnMod == 3 ) {
      sensorZPos -= 0.641;
    } else {
      sensorZPos += 0.641;
    }

    // Check the global position of the sensor
    auto                 globalZero = sensor.toGlobal( zero );
    ROOT::Math::XYZPoint expectedPos;
    if ( mNum % 2 == 0 ) {
      expectedPos = {-sensorXPositions[sNumOnMod] - shimSize, -sensorYPositions[sNumOnMod], sensorZPos};
    } else {
      expectedPos = {sensorXPositions[sNumOnMod] + shimSize, sensorYPositions[sNumOnMod], sensorZPos};
    }
    diff = globalZero - expectedPos;
    if ( diff.r() > 1e-4 ) {
      std::cerr << "Sensor " << sNum << " global position is " << globalZero << ", expected " << expectedPos
                << std::endl;
      std::cerr << "Difference = " << diff.r() << std::endl;
      allChecksOK = false;
    }

    // Verifying the transform of points and vectors
    ROOT::Math::XYZVector v1( 1., 1., 1. );
    ROOT::Math::XYZVector v2 = sensor.toLocal( v1 );
    ROOT::Math::XYZPoint  p0( 0., 0., 0. );
    ROOT::Math::XYZPoint  p1    = p0 + v1;
    auto                  pp0   = sensor.toLocal( p0 );
    auto                  pp1   = sensor.toLocal( p1 );
    ROOT::Math::XYZVector vv2   = pp1 - pp0;
    ROOT::Math::XYZVector vdiff = v2 - vv2;

    if ( vdiff.r() > 1e-4 ) {
      std::cerr << "Error in conversion of vector from global to local frame, expected " << vv2 << " but got " << v2
                << std::endl;
      allChecksOK = false;
    }

    // Then check the global position of each corner pixel
    for ( unsigned int corner{0}; corner < 4; ++corner ) {
      unsigned int chip, row, column;
      if ( corner == 0 ) {
        chip   = 0;
        row    = 0;
        column = 0;
      } else if ( corner == 1 ) {
        chip   = 0;
        row    = 255;
        column = 0;
      } else if ( corner == 2 ) {
        chip   = 2;
        row    = 0;
        column = 255;
      } else {
        chip   = 2;
        row    = 255;
        column = 255;
      }
      LHCb::Detector::VPChannelID vID{sNumID,
                                      LHCb::Detector::VPChannelID::ChipID{chip},
                                      LHCb::Detector::VPChannelID::ColumnID{column},
                                      LHCb::Detector::VPChannelID::RowID{row},
                                      LHCb::Detector::VPChannelID::OrfxID{0},
                                      LHCb::Detector::VPChannelID::OrfyID{0}};
      auto                        globalChannelPos = sensor.channelToGlobalPoint( vID );
      if ( mNum % 2 == 0 ) {
        expectedPos = {-cornerXPositions[sNumOnMod][corner] - shimSize, -cornerYPositions[sNumOnMod][corner],
                       sensorZPos};
      } else {
        expectedPos = {cornerXPositions[sNumOnMod][corner] + shimSize, cornerYPositions[sNumOnMod][corner], sensorZPos};
      }
      diff = globalChannelPos - expectedPos;
      if ( diff.r() > 1e-3 ) {
        std::cerr << "Sensor " << sNum << " corner " << corner << " is at " << globalChannelPos << ", expected "
                  << expectedPos << std::endl;
        std::cerr << "Difference = " << diff.r() << std::endl;
        allChecksOK = false;
      }
    }
  } );

  // test DeVP::sensitiveVolumeID
  for ( std::size_t iModule{0}; iModule < VP::NModules; ++iModule ) {
    for ( std::size_t iSensor{0}; iSensor < VP::NSensorsPerModule; ++iSensor ) {

      const int sensorNumber = iModule * VP::NSensorsPerModule + iSensor;

      auto sensorZPos = moduleZPositions[iModule];
      if ( iSensor == 0 || iSensor == 3 ) {
        sensorZPos -= 0.641;
      } else {
        sensorZPos += 0.641;
      }

      ROOT::Math::XYZPoint sensorPos;
      if ( iModule % 2 == 0 ) {
        sensorPos = {-sensorXPositions[iSensor] - shimSize, -sensorYPositions[iSensor], sensorZPos};
      } else {
        sensorPos = {sensorXPositions[iSensor] + shimSize, sensorYPositions[iSensor], sensorZPos};
      }

      const int sensitiveVolumeID = vp.sensitiveVolumeID( sensorPos );
      if ( sensorNumber != sensitiveVolumeID ) {
        std::cerr << "DeVP::sensitiveVolumeID returns " << sensitiveVolumeID << " for position " << sensorPos
                  << " which should give sensor number = " << sensorNumber << std::endl;
        allChecksOK = false;
      }
    }
  }

  // Finalizing the service and returning
  dds.finalize();

  if ( allChecksOK ) {
    std::cout << "All checks of DeVP succeeded!" << std::endl;
  } else {
    ::exit( EINVAL );
  }

  return 1; // Plugins return 1 when they are successful
}
DECLARE_APPLY( LHCb_TEST_load_devp_detailed, test_load_devp )
