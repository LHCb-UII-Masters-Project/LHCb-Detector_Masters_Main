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
#include <Core/Units.h>
#include <DD4hep/Detector.h>
#include <DD4hep/Printout.h>
#include <Detector/Calo/DeCalorimeter.h>
#include <TError.h> // gErrorIgnoreLevel

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main()
#include <catch2/catch.hpp>

#include <iostream>
#include <string>

namespace {
  auto cond_db_path() {
    // If LHCB_CONDITION_DB is set respect its value
    const auto db = getenv( "LHCB_CONDITION_DB" );
    const auto conditions =
        ( db ? db : "git:/cvmfs/lhcb.cern.ch/lib/lhcb/git-conddb/lhcb-conditions-database.git@master" );
    std::cout << "Using Conditions DB " << conditions << std::endl;
    return conditions;
  }
} // namespace

TEST_CASE( "Ecal" ) {

  // turn off DD4hep printouts
  dd4hep::setPrintLevel( static_cast<dd4hep::PrintLevel>( dd4hep::ALWAYS + 1 ) );
  // turn off ROOT printouts
  gErrorIgnoreLevel = kError + 1;

  auto& description = dd4hep::Detector::getInstance();
  description.fromXML( "compact/run3/trunk/LHCb.xml" );

  REQUIRE( description.state() == dd4hep::Detector::READY );

  auto ecal = description.detector( "Ecal" );

  // the `!!` is needed because handles have `operator!` but not `operator bool`
  REQUIRE( !!ecal );

  LHCb::Detector::DetectorDataService dds( description, {"/world", "Ecal"} );
  dds.initialize( nlohmann::json{{"repository", cond_db_path()}} );
  auto slice = dds.get_slice( 100 );
  REQUIRE( slice );

  using namespace LHCb::Detector::Calo;

  DeCalorimeter de = slice->get( ecal, LHCb::Detector::Keys::deKey );
  REQUIRE( !!de );

  CHECK( de.caloName() == "EcalDet" );
  CHECK( de.index() == CellCode::Index::EcalCalo );

  CHECK_THAT( LHCb::Detector::detail::toDD4hepUnits( de.xSize() ),
              Catch::Matchers::WithinRel( 64. * ( 121.7 + 2 * 0.1 ) * dd4hep::mm, 1.e-4 ) );
  CHECK_THAT( LHCb::Detector::detail::toDD4hepUnits( de.ySize() ),
              Catch::Matchers::WithinRel( dd4hep::_toDouble( "EcalYSize" ), 1.e-4 ) );
  CHECK_THAT( LHCb::Detector::detail::toDD4hepUnits( de.zSize() ),
              Catch::Matchers::WithinRel( 432. * dd4hep::mm, 1.e-4 ) );
  CHECK_THAT( LHCb::Detector::detail::toDD4hepUnits( de.zOffset() ),
              Catch::Matchers::WithinAbs( -113.5 * dd4hep::mm, 1.e-6 ) );

  CHECK( de.adcMax() == 4095 );          // from YAML
  CHECK( de.access()->maxRowCol == 63 ); // from YAML
  CHECK( de.access()->firstRowUp == 32 );
  CHECK_THAT( de.access()->centerRowCol, Catch::Matchers::WithinRel( 31.5, 1.e-4 ) );
  CHECK( de.access()->centralHoleX == 8 ); // from YAML
  CHECK( de.access()->centralHoleY == 6 ); // from YAML

  CHECK_THAT( de.pedestalShift(), Catch::Matchers::WithinRel( 0.4, 1.e-4 ) ); // from test/ConditionsIOV

  dd4hep::Detector::destroyInstance();
}

TEST_CASE( "Hcal" ) {

  // turn off DD4hep printouts
  dd4hep::setPrintLevel( static_cast<dd4hep::PrintLevel>( dd4hep::ALWAYS + 1 ) );
  // turn off ROOT printouts
  gErrorIgnoreLevel = kError + 1;

  auto& description = dd4hep::Detector::getInstance();
  description.fromXML( "compact/run3/trunk/LHCb.xml" );

  REQUIRE( description.state() == dd4hep::Detector::READY );

  auto hcal = description.detector( "Hcal" );

  // the `!!` is needed because handles have `operator!` but not `operator bool`
  REQUIRE( !!hcal );

  LHCb::Detector::DetectorDataService dds( description, {"/world", "Hcal"} );
  dds.initialize( nlohmann::json{{"repository", cond_db_path()}} );
  auto slice = dds.get_slice( 100 );
  REQUIRE( slice );

  using namespace LHCb::Detector::Calo;

  DeCalorimeter de = slice->get( hcal, LHCb::Detector::Keys::deKey );
  REQUIRE( !!de );

  CHECK( de.caloName() == "HcalDet" );
  CHECK( de.index() == CellCode::Index::HcalCalo );

  CHECK_THAT( LHCb::Detector::detail::toDD4hepUnits( de.xSize() ),
              Catch::Matchers::WithinRel( 2. * 8. * 2. * 262.6 * dd4hep::mm, 1.e-4 ) );
  CHECK_THAT( LHCb::Detector::detail::toDD4hepUnits( de.ySize() ),
              Catch::Matchers::WithinRel( dd4hep::_toDouble( "HcalTotYSize" ), 1.e-4 ) );
  CHECK_THAT( LHCb::Detector::detail::toDD4hepUnits( de.zSize() ),
              Catch::Matchers::WithinRel( 1200. * dd4hep::mm, 1.e-4 ) );
  CHECK_THAT( LHCb::Detector::detail::toDD4hepUnits( de.zOffset() ),
              Catch::Matchers::WithinAbs( -213.5 * dd4hep::mm, 1.e-6 ) );

  CHECK( de.adcMax() == 4095 );          // from YAML
  CHECK( de.access()->maxRowCol == 31 ); // from YAML
  CHECK( de.access()->firstRowUp == 16 );
  CHECK_THAT( de.access()->centerRowCol, Catch::Matchers::WithinRel( 15.5, 1.e-4 ) );
  CHECK( de.access()->centralHoleX == 2 ); // from YAML
  CHECK( de.access()->centralHoleY == 2 ); // from YAML

  CHECK_THAT( de.pedestalShift(), Catch::Matchers::WithinRel( 0.4, 1.e-4 ) ); // from test/ConditionsIOV

  dd4hep::Detector::destroyInstance();
}
