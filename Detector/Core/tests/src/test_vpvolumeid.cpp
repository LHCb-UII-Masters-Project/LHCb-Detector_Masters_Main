/*****************************************************************************\
* (c) Copyright 2023 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#include "Core/DetectorDataService.h"

#include "Detector/Test/Fixture.h"
#include "Detector/VP/DeVP.h"
#include "Detector/VP/VPVolumeID.h"

#include <catch2/catch.hpp>

template <typename... Args>
auto makeVPVolumeID( Args&&... args ) {
  return LHCb::Detector::VPVolumeID{std::forward<Args>( args )...};
}

TEST_CASE( "VPVolumeID" ) {

  Detector::Test::Fixture f;

  auto& description = f.description();

  description.fromXML( "compact/components/debug/VP.xml" );

  // run the VolumeManager plugin so that the volume IDs are available
  description.apply( "DD4hep_VolumeManager", 0, nullptr );

  REQUIRE( description.state() == dd4hep::Detector::READY );

  // First test that construction with bad values throws
  REQUIRE_THROWS_AS( makeVPVolumeID( 0u ), std::runtime_error );
  REQUIRE_THROWS_AS( makeVPVolumeID( LHCb::Detector::VPChannelID::SensorID{VP::NSensors} ), std::runtime_error );
  REQUIRE_THROWS_AS(
      makeVPVolumeID( LHCb::Detector::VPVolumeID::ModuleID{VP::NModules}, LHCb::Detector::VPVolumeID::LadderID{0} ),
      std::runtime_error );
  REQUIRE_THROWS_AS( makeVPVolumeID( LHCb::Detector::VPVolumeID::ModuleID{0},
                                     LHCb::Detector::VPVolumeID::LadderID{VP::NSensorsPerModule} ),
                     std::runtime_error );

  // Check that all VP sensors have volume IDs that match the expectation
  LHCb::Detector::DetectorDataService dds( description, {"/world", "VP"} );
  dds.initialize( nlohmann::json( {{"repository", "file:tests/ConditionsIOV"}} ) );

  auto                        slice = dds.get_slice( 100 );
  dd4hep::DetElement          vpdet = description.detector( "VP" );
  const LHCb::Detector::DeVP& vp    = slice->get( vpdet, LHCb::Detector::Keys::deKey );

  const auto nSensors = vp.numberSensors();
  REQUIRE( nSensors == 208 );

  for ( unsigned int iSensor{0}; iSensor < nSensors; ++iSensor ) {

    const auto                       sensor_id = LHCb::Detector::VPChannelID::SensorID{iSensor};
    const LHCb::Detector::DeVPSensor sensor    = vp.sensor( sensor_id );

    const auto dd4hep_vol_id = sensor.detector().volumeID();

    LHCb::Detector::VPVolumeID expected_vp_vol_id;
    REQUIRE_NOTHROW( expected_vp_vol_id = makeVPVolumeID( sensor_id ) );

    LHCb::Detector::VPVolumeID actual_vp_vol_id;
    REQUIRE_NOTHROW( actual_vp_vol_id = makeVPVolumeID( static_cast<unsigned int>( dd4hep_vol_id ) ) );

    REQUIRE( expected_vp_vol_id == actual_vp_vol_id );
  }

  // Then check that some known values give all the right answers when "decoded"

  auto deMod38Lad1 = description.detector( "BeforeMagnetRegion/VP/MotionVPRight/VPRight/Module38/ladder_1/sensor" );

  // the `!!` is needed because handles have `operator!` but not `operator bool`
  REQUIRE( !!deMod38Lad1 );

  unsigned int volID = deMod38Lad1.volumeID();

  REQUIRE( volID == 0x00049B0A );

  LHCb::Detector::VPVolumeID id1;

  REQUIRE_NOTHROW( id1 = makeVPVolumeID( volID ) );

  REQUIRE( id1.isValid() );
  REQUIRE( id1.volumeID() == volID );
  REQUIRE( to_unsigned( id1.system() ) == 10 );
  REQUIRE( to_unsigned( id1.side() ) == 1 );
  REQUIRE( to_unsigned( id1.motionside() ) == 1 );
  REQUIRE( to_unsigned( id1.module() ) == 38 );
  REQUIRE( to_unsigned( id1.ladder() ) == 1 );
  REQUIRE( to_unsigned( id1.sensor() ) == 153 );

  LHCb::Detector::VPVolumeID id2;
  REQUIRE_NOTHROW( id2 = makeVPVolumeID( LHCb::Detector::VPChannelID::SensorID{153} ) );

  REQUIRE( id2.isValid() );
  REQUIRE( id2.volumeID() == volID );
  REQUIRE( to_unsigned( id2.system() ) == 10 );
  REQUIRE( to_unsigned( id2.side() ) == 1 );
  REQUIRE( to_unsigned( id2.motionside() ) == 1 );
  REQUIRE( to_unsigned( id2.module() ) == 38 );
  REQUIRE( to_unsigned( id2.ladder() ) == 1 );
  REQUIRE( to_unsigned( id2.sensor() ) == 153 );

  LHCb::Detector::VPVolumeID id3;
  REQUIRE_NOTHROW(
      id3 = makeVPVolumeID( LHCb::Detector::VPVolumeID::ModuleID{38}, LHCb::Detector::VPVolumeID::LadderID{1} ) );

  REQUIRE( id3.isValid() );
  REQUIRE( id3.volumeID() == volID );
  REQUIRE( to_unsigned( id3.system() ) == 10 );
  REQUIRE( to_unsigned( id3.side() ) == 1 );
  REQUIRE( to_unsigned( id3.motionside() ) == 1 );
  REQUIRE( to_unsigned( id3.module() ) == 38 );
  REQUIRE( to_unsigned( id3.ladder() ) == 1 );
  REQUIRE( to_unsigned( id3.sensor() ) == 153 );

  REQUIRE( id1 == id2 );
  REQUIRE( id1 == id3 );

  auto deMod37Lad1 = description.detector( "BeforeMagnetRegion/VP/MotionVPLeft/VPLeft/Module37/ladder_1/sensor" );

  REQUIRE( !!deMod37Lad1 );

  volID = deMod37Lad1.volumeID();

  REQUIRE( volID == 0x0004940A );

  id3.setModule( LHCb::Detector::VPVolumeID::ModuleID{37} );

  REQUIRE( id3.isValid() );
  REQUIRE( id3.volumeID() == volID );
  REQUIRE( to_unsigned( id3.system() ) == 10 );
  REQUIRE( to_unsigned( id3.side() ) == 0 );
  REQUIRE( to_unsigned( id3.motionside() ) == 0 );
  REQUIRE( to_unsigned( id3.module() ) == 37 );
  REQUIRE( to_unsigned( id3.ladder() ) == 1 );
  REQUIRE( to_unsigned( id3.sensor() ) == 149 );

  auto deMod37Lad3 = description.detector( "BeforeMagnetRegion/VP/MotionVPLeft/VPLeft/Module37/ladder_3/sensor" );

  REQUIRE( !!deMod37Lad3 );

  volID = deMod37Lad3.volumeID();

  REQUIRE( volID == 0x000C940A );

  id3.setLadder( LHCb::Detector::VPVolumeID::LadderID{3} );

  REQUIRE( id3.isValid() );
  REQUIRE( id3.volumeID() == volID );
  REQUIRE( to_unsigned( id3.system() ) == 10 );
  REQUIRE( to_unsigned( id3.side() ) == 0 );
  REQUIRE( to_unsigned( id3.motionside() ) == 0 );
  REQUIRE( to_unsigned( id3.module() ) == 37 );
  REQUIRE( to_unsigned( id3.ladder() ) == 3 );
  REQUIRE( to_unsigned( id3.sensor() ) == 151 );

  auto deMod00Lad0 = description.detector( "BeforeMagnetRegion/VP/MotionVPRight/VPRight/Module00/ladder_0/sensor" );

  REQUIRE( !!deMod00Lad0 );

  volID = deMod00Lad0.volumeID();

  REQUIRE( volID == 0x0000030A );

  id3.setSensor( LHCb::Detector::VPChannelID::SensorID{0} );

  REQUIRE( id3.isValid() );
  REQUIRE( id3.volumeID() == volID );
  REQUIRE( to_unsigned( id3.system() ) == 10 );
  REQUIRE( to_unsigned( id3.side() ) == 1 );
  REQUIRE( to_unsigned( id3.motionside() ) == 1 );
  REQUIRE( to_unsigned( id3.module() ) == 0 );
  REQUIRE( to_unsigned( id3.ladder() ) == 0 );
  REQUIRE( to_unsigned( id3.sensor() ) == 0 );
}
