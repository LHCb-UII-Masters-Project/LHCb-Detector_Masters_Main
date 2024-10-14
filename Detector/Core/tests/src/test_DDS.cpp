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

TEST_CASE( "DetectorDataService" ) {
  namespace fs = std::filesystem;

  Detector::Test::Fixture f;

  auto& description = f.description();

  description.fromXML( "compact/components/debug/VP.xml" );
  // description.fromXML( "compact/run3/trunk/LHCb.xml" );

  REQUIRE( description.state() == dd4hep::Detector::READY );

  auto det = description.detector( "VP" );
  // the `!!` is needed because handles have `operator!` but not `operator bool`
  REQUIRE( !!det );

  LHCb::Detector::DetectorDataService dds( description, {"/world", "VP"} );
  dds.initialize( nlohmann::json( {{"repository", "file:tests/ConditionsIOV"}} ) );

  // get a condition slice
  auto slice = dds.get_slice( 100 );
  REQUIRE( slice );

  // check that the slice we got has an IOV spanning from 0 (included) to 200 (excluded)
  CHECK( slice->iov().contains( dds.make_iov( 100 ) ) );
  CHECK( slice->iov().contains( dds.make_iov( 0 ) ) );
  CHECK( slice->iov().contains( dds.make_iov( 199 ) ) );
  CHECK( !slice->iov().contains( dds.make_iov( 200 ) ) );

  // check that we always get the same slice for every point in the IOV
  CHECK( dds.get_slice( 0 ) == slice );
  CHECK( dds.get_slice( 50 ) == slice );
  CHECK( dds.get_slice( 100 ) == slice );
  CHECK( dds.get_slice( 150 ) == slice );
  CHECK( dds.get_slice( 199 ) == slice );

  auto old_slice = slice;
  slice          = dds.get_slice( 250 );
  CHECK( slice != old_slice );

  // check that the slice we got has an IOV spanning from 200 (included) to 400 (excluded)
  CHECK( slice->iov().contains( dds.make_iov( 250 ) ) );
  CHECK( slice->iov().contains( dds.make_iov( 200 ) ) );
  CHECK( slice->iov().contains( dds.make_iov( 399 ) ) );
  CHECK( !slice->iov().contains( dds.make_iov( 400 ) ) );

  // check that we always get the same slice for every point in the IOV
  CHECK( dds.get_slice( 200 ) == slice );
  CHECK( dds.get_slice( 250 ) == slice );
  CHECK( dds.get_slice( 300 ) == slice );
  CHECK( dds.get_slice( 350 ) == slice );
  CHECK( dds.get_slice( 399 ) == slice );
}
