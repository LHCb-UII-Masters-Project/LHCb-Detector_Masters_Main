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
#include <Detector/Muon/DeMuon.h>

#include <Core/DetectorDataService.h>
#include <Core/Keys.h>
#include <DD4hep/Detector.h>
#include <Detector/Test/Fixture.h>

#include <catch2/catch.hpp>

TEST_CASE( "DeMuon" ) {
  using TileID = LHCb::Detector::Muon::TileID;
  using Layout = LHCb::Detector::Muon::Layout;
  Detector::Test::Fixture f;

  auto& description = f.description();

  description.fromXML( "compact/components/debug/Muon.xml" );

  REQUIRE( description.state() == dd4hep::Detector::READY );

  auto muon = description.detector( "Muon" );

  // the `!!` is needed because handles have `operator!` but not `operator bool`
  REQUIRE( !!muon );

  // load conditions
  LHCb::Detector::DetectorDataService dds( description, {"/world", "Muon"} );
  dds.initialize( nlohmann::json{{"repository", "file:tests/ConditionsIOV"}} );
  auto slice = dds.get_slice( 100 );
  REQUIRE( slice );

  using LHCb::Detector::DeMuon;
  DeMuon de = slice->get( muon, LHCb::Detector::Keys::deKey );
  REQUIRE( !!de );

  CHECK( de.stations() == 4 );
  CHECK( de.getStationName( 2 ) == "M4" );

  CHECK( de.regions() == 16 );

  CHECK( de.getRegionName( 1, 1 ) == "M3R2" );
  auto& rout = de.getReadoutCond( 1, 1 );
  REQUIRE( rout.size() == 2 );
  CHECK( rout.readoutType( 0 ) == LHCb::Detector::Muon::ReadoutCond::Anode );
  CHECK( rout.readoutType( 1 ) == LHCb::Detector::Muon::ReadoutCond::Cathode );
  CHECK_THAT( rout.syncDrift( 0 ), Catch::Matchers::WithinRel( 3., 1.e-4 ) );

  int  station       = 2;
  int  region        = 1;
  int  chamberNumber = 1;
  auto tile =
      de.Chamber2Tile( chamberNumber - LHCb::Detector::Muon::firstChamber, station - LHCb::Detector::Muon::firstStation,
                       region - LHCb::Detector::Muon::firstRegion );

  CHECK( tile.station() == 0 );
  CHECK( tile.region() == 0 );
  CHECK( de.getChamberNumber( tile ) == chamberNumber );
  auto chamber         = de.getChamber( station - LHCb::Detector::Muon::firstStation,
                                region - LHCb::Detector::Muon::firstRegion, chamberNumber );
  auto chamberFromTile = de.getChamberFromTile( tile );
  CHECK( chamber.chamberNumber() == chamberFromTile.chamberNumber() );
  CHECK( chamber.name() == chamberFromTile.name() );
  CHECK( chamber.stationNumber() == chamberFromTile.stationNumber() );

  CHECK_THAT( dd4hep::mm, Catch::Matchers::WithinRel( 1, 1.e-4 ) );
  CHECK( de.getStationFromZ( 15200 * dd4hep::mm ) == 2 );
  CHECK( de.getStationFromZ( 16400 * dd4hep::mm ) == 3 );
  CHECK( de.getStationFromZ( 17600 * dd4hep::mm ) == 4 );
  CHECK( de.getStationFromZ( 18800 * dd4hep::mm ) == 5 );

  // PAD tile
  // TileID (Station, Layout, Region, Quadrant, NX, NY)
  TileID pTile( 0, Layout( 48, 16 ), 1, 0, 0, 16 );

  auto position = de.position( pTile );
  CHECK( position->x() > 13.87 );
  CHECK( position->x() < 13.88 );
  CHECK( position->dX() > 6.37 );
  CHECK( position->dX() < 6.38 );

  // Logical channel - StripX
  TileID sXTile( 0, Layout( 48, 1 ), 0, 0, 0, 1 );

  position = de.position( sXTile );
  CHECK( position->x() > 7.4 );
  CHECK( position->x() < 7.5 );
  CHECK( position->dX() > 3.2 );
  CHECK( position->dX() < 3.3 );

  // Logical channel - StripY
  TileID sYTile( 0, Layout( 4, 16 ), 1, 0, 0, 16 );

  position = de.position( sYTile );
  CHECK( position->x() >= 84.00 );
  CHECK( position->x() < 85.00 );
  CHECK( position->dX() >= 76.45 );
  CHECK( position->dX() < 76.55 );

  CHECK_THAT( de.getStationZ( 0 ), Catch::Matchers::WithinRel( 15270 * dd4hep::mm, 1.e-4 ) );
  CHECK_THAT( de.getStationZ( 1 ), Catch::Matchers::WithinRel( 16470 * dd4hep::mm, 1.e-4 ) );
  CHECK_THAT( de.getStationZ( 2 ), Catch::Matchers::WithinRel( 17670 * dd4hep::mm, 1.e-4 ) );
  CHECK_THAT( de.getStationZ( 3 ), Catch::Matchers::WithinRel( 18870 * dd4hep::mm, 1.e-4 ) );

  // hit outside a gap gives a negative volume id
  // CHECK( de.sensitiveVolumeID( {616.721 * dd4hep::mm, 4.26087 * dd4hep::mm, 15412 * dd4hep::mm} ) < 0 );
}
