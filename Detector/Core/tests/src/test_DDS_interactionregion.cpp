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
#include <Core/DetectorDataService.h>
#include <Core/Keys.h>
#include <DD4hep/Detector.h>
#include <Detector/LHCb/DeLHCb.h>
#include <Detector/Test/Fixture.h>
#include <cstdio>

#include <catch2/catch.hpp>

TEST_CASE( "InteractionRegion loading" ) {
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

    auto ir = lhcb.interactionRegion();
    REQUIRE( ir.has_value() );

    CHECK( ir.value().avgPosition == ROOT::Math::XYZPoint{0.0, 0.0, 0.0} );
    CHECK( ir.value().spread.At( 0, 0 ) == 0.0064 );
    CHECK( ir.value().spread.At( 0, 1 ) == 0.0 );
    CHECK( ir.value().spread.At( 0, 2 ) == 0.0 );
    CHECK( ir.value().spread.At( 1, 1 ) == 0.0064 );
    CHECK( ir.value().spread.At( 1, 2 ) == 0.0 );
    CHECK( ir.value().spread.At( 2, 2 ) == 2809.0 );
    CHECK( ir.value().tX() == 0.0 );
    CHECK( ir.value().tY() == 0.0 );
  }

  {
    // get a condition slice where the condition doesn't exist
    // - we should get the fallback values
    auto slice = dds.get_slice( 0 );
    REQUIRE( slice );

    LHCb::Detector::DeLHCb lhcb = slice->get( det, LHCb::Detector::Keys::deKey );
    REQUIRE( !!lhcb );

    auto ir = lhcb.interactionRegion();
    REQUIRE( !ir.has_value() );
  }
}
