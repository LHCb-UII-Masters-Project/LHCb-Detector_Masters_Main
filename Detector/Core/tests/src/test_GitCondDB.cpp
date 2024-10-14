/*****************************************************************************\
* (c) Copyright 2020-2022 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#include <GitCondDB.h>
#include <catch2/catch.hpp>
#include <iostream>
#include <yaml-cpp/yaml.h>

TEST_CASE( "GitCondDB" ) {
  auto db = GitCondDB::connect( "file:tests/ConditionsIOV/Conditions" );
  REQUIRE( db.connected() );

  {
    auto [data, iov] = db.get( {"v0", "VP/Alignment/Global.yml", 0} );
    CHECK( iov.since == 0 );
    CHECK( iov.until == 200 );
    auto value = YAML::Load( data );
    CHECK( value["VPSystem"]["position"][2].as<std::string>() == "0.0 * mm" );
  }

  {
    auto [data, iov] = db.get( {"v0", "VP/Alignment/Global.yml", 200} );
    CHECK( iov.since == 200 );
    CHECK( iov.until == 500 );
    auto value = YAML::Load( data );
    CHECK( value["VPSystem"]["position"][2].as<std::string>() == "1000 * um" );
  }
}
