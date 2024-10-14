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
#include <Detector/Test/Fixture.h>
#include <Detector/VP/DeVP.h>
#include <cstdio>

#include <catch2/catch.hpp>

TEST_CASE( "DDS limit IOV length" ) {
  namespace fs = std::filesystem;
  using Catch::Matchers::Contains;

  Detector::Test::Fixture f;

  auto& description = f.description();

  description.fromXML( "compact/components/debug/VP.xml" );
  // description.fromXML( "compact/run3/trunk/LHCb.xml" );

  REQUIRE( description.state() == dd4hep::Detector::READY );

  auto det = description.detector( "VP" );
  // the `!!` is needed because handles have `operator!` but not `operator bool`
  REQUIRE( !!det );

  LHCb::Detector::DetectorDataService dds( description, {"/world", "VP"} );
  dds.initialize( nlohmann::json( {{"repository", "file:tests/ConditionsIOV"},
                                   // limit IOV of Conditions/VP/Alignment/Global.yml to exactly 1 unit (run)
                                   {"online_conditions", {"Conditions/VP/Alignment/Global.yml"}}} ) );

  // get a condition slice
  auto slice = dds.get_slice( 200 );
  REQUIRE( slice );

  // check that the slice we got has IOV exactly for point 200 and nothing more
  CHECK( slice->iov().contains( dds.make_iov( 200 ) ) );
  CHECK( !slice->iov().contains( dds.make_iov( 199 ) ) );
  CHECK( !slice->iov().contains( dds.make_iov( 201 ) ) );

  // the condition for point 201 is not present so we expect an error
  CHECK_THROWS_WITH( dds.get_slice( 201 ), Contains( "cannot find data for point 201" ) );

  // the next valid point in the test DB is 400
  slice = dds.get_slice( 500 );
  REQUIRE( slice );
  // check that the slice we got has IOV exactly for point 500 and nothing more
  CHECK( slice->iov().contains( dds.make_iov( 500 ) ) );
  CHECK( !slice->iov().contains( dds.make_iov( 499 ) ) );
  CHECK( !slice->iov().contains( dds.make_iov( 501 ) ) );
}
