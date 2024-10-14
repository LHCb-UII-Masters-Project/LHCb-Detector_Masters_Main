/*****************************************************************************\
* (c) Copyright 2021 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#include "../../include/Core/ConditionsOverlay.h"
#include <Detector/Test/TmpDir.h>
#include <catch2/catch.hpp>
#include <filesystem>

using Detector::Test::TmpDir;
using LHCb::Detector::ConditionsOverlay;

TEST_CASE( "simple get" ) {
  ConditionsOverlay ovly;
  CHECK( ovly.size() == 0 );

  {
    auto doc = ovly.get( "some/path", R"(---
node1: 42
node2:
  pos: 1
  rot: 2
)" );

    CHECK( ovly.size() == 1 );

    CHECK( doc.size() == 2 );
    CHECK( doc["node1"].as<int>() == 42 );
    CHECK( doc["node2"]["pos"].as<int>() == 1 );
    CHECK( doc["node2"]["rot"].as<int>() == 2 );
  }

  {
    auto doc1 = ovly.get( "some/path", R"(---
node1: 43
node2:
  pos: 5
  rot: 2
)" );

    auto doc2 = ovly.get( "another/path", R"(---
data: some text
)" );

    CHECK( ovly.size() == 2 );

    CHECK( doc1.size() == 2 );
    CHECK( doc1["node1"].as<int>() == 43 );
    CHECK( doc1["node2"]["pos"].as<int>() == 5 );
    CHECK( doc1["node2"]["rot"].as<int>() == 2 );

    CHECK( doc2.size() == 1 );
    CHECK( doc2["data"].as<std::string>() == "some text" );
  }
}

TEST_CASE( "override get" ) {
  ConditionsOverlay ovly;
  CHECK( ovly.size() == 0 );

  {
    auto doc = ovly.get( "some/path", R"(---
node1: 42
node2:
  pos: 1
  rot: 2
)" );

    CHECK( ovly.size() == 1 );

    CHECK( doc.size() == 2 );
    CHECK( doc["node1"].as<int>() == 42 );
    CHECK( doc["node2"]["pos"].as<int>() == 1 );
    CHECK( doc["node2"]["rot"].as<int>() == 2 );
  }

  ovly.update( "some/path", "node2", YAML::Load( R"({"pos": 3, "rot": 7})" ) );

  {
    auto doc = ovly.get( "some/path", R"(---
node1: 43
node2:
  pos: 5
  rot: 2
)" );

    CHECK( ovly.size() == 1 );

    CHECK( doc.size() == 2 );
    CHECK( doc["node1"].as<int>() == 42 );
    CHECK( doc["node2"]["pos"].as<int>() == 3 );
    CHECK( doc["node2"]["rot"].as<int>() == 7 );
  }
}

TEST_CASE( "add" ) {
  ConditionsOverlay ovly;
  CHECK( ovly.size() == 0 );

  ovly.add( "some/path", YAML::Load( R"({"initial": 0})" ) );
  CHECK( ovly.size() == 1 );

  // an added document is not visible unless also updated
  ovly.update( "some/path", "another", YAML::Load( "1" ) );
  CHECK( ovly.size() == 1 );

  {
    auto doc = ovly.get( "some/path", "{}" );

    CHECK( doc.size() == 2 );
    CHECK( doc["initial"].as<int>() == 0 );
    CHECK( doc["another"].as<int>() == 1 );
  }
  // replace the doc
  ovly.add( "some/path", YAML::Load( R"({"whatever": 10})" ) );
  ovly.update( "some/path", "another", YAML::Load( "1" ) );

  {
    auto doc = ovly.get( "some/path", "{}" );

    CHECK( doc.size() == 2 );
    CHECK( doc["whatever"].as<int>() == 10 );
    CHECK( doc["another"].as<int>() == 1 );
  }
}

TEST_CASE( "clear" ) {
  ConditionsOverlay ovly;
  CHECK( ovly.size() == 0 );

  ovly.add( "some/path", YAML::Load( R"({})" ) );
  CHECK( ovly.size() == 1 );

  ovly.clear();
  CHECK( ovly.size() == 0 );
}

TEST_CASE( "dump" ) {
  namespace fs = std::filesystem;

  Detector::Test::TmpDir tmp;

  ConditionsOverlay ovly;
  ovly.add( "path/to/a/condition.yaml", YAML::Load( R"(---
condition1: [0, 1, 2]
displacement: !alignment
  position: [0, 0, 0]
  rotation: [0, 0, 0]
)" ) );
  ovly.add( "/this_is/another.yaml", YAML::Load( R"(---
value1: 100
value2: -3
)" ) );
  const bool all = true;
  ovly.dump( tmp.path, all );

  REQUIRE( fs::is_regular_file( tmp.path / "path" / "to" / "a" / "condition.yaml" ) );
  auto f1 = YAML::LoadFile( tmp.path / "path" / "to" / "a" / "condition.yaml" );
  CHECK( f1.size() == 2 );
  CHECK( ( f1["condition1"].as<std::vector<int>>() == std::vector<int>{0, 1, 2} ) );
  auto d = f1["displacement"];
  CHECK( d.Tag() == "!alignment" );
  CHECK( d.size() == 2 );
  CHECK( ( d["position"].as<std::vector<int>>() == std::vector<int>{0, 0, 0} ) );
  CHECK( ( d["rotation"].as<std::vector<int>>() == std::vector<int>{0, 0, 0} ) );

  REQUIRE( fs::is_regular_file( tmp.path / "this_is" / "another.yaml" ) );
  auto f2 = YAML::LoadFile( tmp.path / "this_is" / "another.yaml" );
  CHECK( f2.size() == 2 );
  CHECK( f2["value1"].as<int>() == 100 );
  CHECK( f2["value2"].as<int>() == -3 );

  std::size_t files = 0, dirs = 0, others = 0;
  for ( const auto& entry : fs::recursive_directory_iterator( tmp.path ) ) {
    if ( entry.is_regular_file() ) {
      ++files;
    } else if ( entry.is_directory() ) {
      ++dirs;
    } else {
      ++others;
    }
  }
  CHECK( files == 2 );
  CHECK( dirs == 4 );
  CHECK( others == 0 );
}

TEST_CASE( "load" ) {
  namespace fs = std::filesystem;

  TmpDir tmp;

  fs::create_directories( tmp.path / "path" / "to" / "a" );
  fs::create_directories( tmp.path / "this_is" );
  std::ofstream( tmp.path / "path" / "to" / "a" / "condition.yaml" ) << R"(---
condition1: [0, 1, 2]
displacement: !alignment
  position: [0, 0, 0]
  rotation: [0, 0, 0]
)";
  std::ofstream( tmp.path / "this_is" / "another.yaml" ) << R"(---
value1: 100
value2: -3
)";
  std::ofstream( tmp.path / "invalid.txt" ) << "invalid YAML: {\n";

  ConditionsOverlay ovly;
  ovly.load( tmp.path );

  {
    auto cond = ovly.get( "path/to/a/condition.yaml", "" );
    CHECK( cond.size() == 2 );
    {
      std::vector<int> expected{0, 1, 2};
      CHECK( cond["condition1"].as<std::vector<int>>() == expected );
    }
    CHECK( cond["displacement"].Tag() == "!alignment" );
    CHECK( cond["displacement"]["position"].IsDefined() );
    CHECK( cond["displacement"]["rotation"].IsDefined() );
  }

  {
    auto cond = ovly.get( "this_is/another.yaml", "" );
    CHECK( cond.size() == 2 );
    CHECK( cond["value1"].as<int>() == 100 );
    CHECK( cond["value2"].as<int>() == -3 );
  }

  CHECK( ovly.get( "unknown.yaml", "---\ndata: 42" )["data"].as<int>() == 42 );
  CHECK( ovly.get( "invalid.txt", "never loaded" ).as<std::string>() == "never loaded" );
}
