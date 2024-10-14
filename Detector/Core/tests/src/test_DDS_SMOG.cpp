/*****************************************************************************\
* (c) Copyright 2024 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#include <Core/DetectorDataService.h>
#include <Core/Keys.h>
#include <DD4hep/Detector.h>
#include <Detector/LHCb/DeLHCb.h>
#include <Detector/LHCb/SMOGInfo.h>
#include <Detector/Test/Fixture.h>
#include <cstdio>

#include <catch2/catch.hpp>

TEST_CASE( "SMOGCondition_loading" ) {
  namespace fs = std::filesystem;
  using Catch::Matchers::Contains;

  Detector::Test::Fixture f;

  auto& description = f.description();

  description.fromXML( "compact/run3/trunk/LHCb.xml" );

  REQUIRE( description.state() == dd4hep::Detector::READY );

  auto det = description.detector( "/world" );
  // the `!!` is needed because handles have `operator!` but not `operator bool`
  REQUIRE( !!det );

  LHCb::Detector::DetectorDataService dds( description, {"/world"} );
  dds.initialize( nlohmann::json( {{"repository", "file:tests/ConditionsIOV"}} ) );

  {
    // get a condition slice where the condition exists
    // - we should get the values from the condition

    auto slice = dds.get_slice( 200 );
    REQUIRE( slice );

    LHCb::Detector::DeLHCb lhcb = slice->get( det, LHCb::Detector::Keys::deKey );
    REQUIRE( !!lhcb );

    auto SMOG = lhcb.SMOG();
    REQUIRE( SMOG.has_value() );

    CHECK( SMOG.value().injection_mode == LHCb::Detector::SMOGInfo::Mode::SMOG2 );
    CHECK( SMOG.value().injected_gas == LHCb::Detector::SMOGInfo::Gas::ARGON );
    CHECK( SMOG.value().stable_injection == true );
  }

  {
    // get a condition slice where the condition doesn't exist
    // - we should get the fallback values
    auto slice = dds.get_slice( 0 );
    REQUIRE( slice );

    LHCb::Detector::DeLHCb lhcb = slice->get( det, LHCb::Detector::Keys::deKey );
    REQUIRE( !!lhcb );

    auto SMOG = lhcb.SMOG();
    REQUIRE( !SMOG.has_value() );
  }

  {
    // get a condition slice where the condition is empty
    // - we should get the default values

    auto slice = dds.get_slice( 1000 );
    REQUIRE( slice );

    LHCb::Detector::DeLHCb lhcb = slice->get( det, LHCb::Detector::Keys::deKey );
    REQUIRE( !!lhcb );

    auto SMOG = lhcb.SMOG();
    REQUIRE( SMOG.has_value() );

    CHECK( SMOG.value().injection_mode == LHCb::Detector::SMOGInfo::Mode::NONE );
    CHECK( SMOG.value().injected_gas == LHCb::Detector::SMOGInfo::Gas::NONE );
    CHECK( SMOG.value().stable_injection == false );
  }
}
