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

TEST_CASE( "Tell40Links loading" ) {
  namespace fs = std::filesystem;
  using Catch::Matchers::Contains;

  Detector::Test::Fixture f;

  auto& description = f.description();

  description.fromXML( "compact/run3/trunk/LHCb.xml" );
  // description.fromXML( "compact/run3/trunk/LHCb.xml" );

  REQUIRE( description.state() == dd4hep::Detector::READY );

  auto det = description.detector( "/world" );
  // the `!!` is needed because handles have `operator!` but not `operator bool`
  REQUIRE( !!det );

  LHCb::Detector::DetectorDataService dds( description, {"/world"} );
  dds.initialize( nlohmann::json( {{"repository", "file:tests/ConditionsIOV"}} ) );

  {
    // get a condition slice
    auto slice = dds.get_slice( 200 );
    REQUIRE( slice );

    LHCb::Detector::DeLHCb lhcb = slice->get( det, LHCb::Detector::Keys::deKey );
    REQUIRE( !!lhcb );

    std::vector<std::tuple<std::uint16_t, std::uint8_t, std::uint32_t>> expected{
        {22550, 0, 0xfff}, {24621, 12, 0xf}, {6148, 0, 0x3ff}};

    for ( const auto& [id, offset, mask] : expected ) {
      auto link = lhcb.tell40links( id ).value();
      INFO( ( fmt::format( "Tell40 link {:5d} found: offset {:2d}, mask {:#06x}\n", id, link.offset, link.mask ) ) );
      INFO( ( fmt::format( "               expected: offset {:2d}, mask {:#06x}\n", offset, mask ) ) );
      CHECK( ( offset == link.offset && mask == link.mask ) );
    }

    CHECK_THROWS_AS( lhcb.tell40links( 1 ).value(), std::bad_optional_access );
  }

  {
    // get the condition slice with empty Tell40Links
    auto slice = dds.get_slice( 0 );
    REQUIRE( slice );

    LHCb::Detector::DeLHCb lhcb = slice->get( det, LHCb::Detector::Keys::deKey );
    REQUIRE( !!lhcb );

    CHECK( lhcb.tell40links().links.empty() );
    CHECK_THROWS_AS( lhcb.tell40links( 22550 ).value(), std::bad_optional_access );
  }
}
