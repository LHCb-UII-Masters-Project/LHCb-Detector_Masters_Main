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
#include "../../src/ReadoutCondYaml.h"
#include <Core/Utils.h>
#include <map>
#include <string>
#include <vector>

#include <catch2/catch.hpp>

using namespace LHCb::Detector::Muon;
using LHCb::Detector::utils::yaml2json;

namespace {
  constexpr char yaml_example[] = R"(
ReadoutModules:
  M1R1:
    - ReadoutType: Cathode
      Efficiency: 0.995000
      SyncDrift: 3.000000
      ChamberNoise: 0.000000
      ElectronicsNoise: 100.000000
      MeanDeadTime: 50.000000
      RMSDeadTime: 10.000000
      TimeGateStart: 0.000000
      PadEdgeSizeX: -0.0670000
      PadEdgeSigmaX: 1.00000
      ClusterX: [{size: 1, prob: 0.980}, {size: 2, prob: 0.020}]
      PadEdgeSizeY: -0.067
      PadEdgeSigmaY: 1.000
      ClusterY: [{size: 1, prob: 0.980}, {size: 2, prob: 0.020}]
      JitterMin: 0.
      JitterMax: 58.
      JitterValues: [
        0.00001, 0.00005, 0.00022, 0.00065, 0.00136, 0.00248, 0.00373, 0.00555, 0.00760, 0.00971,
        0.01198, 0.01443, 0.01662, 0.01893, 0.02130, 0.02310, 0.02506, 0.02661, 0.02800, 0.02906,
        0.02968, 0.03050, 0.03097, 0.03100, 0.03128, 0.03093, 0.03023, 0.02991, 0.02911, 0.02826,
        0.02751, 0.02653, 0.02550, 0.02458, 0.02373, 0.02252, 0.02136, 0.02045, 0.01906, 0.01800,
        0.01688, 0.01591, 0.01478, 0.01403, 0.01299, 0.01212, 0.01137, 0.01044, 0.00967, 0.00888,
        0.00834, 0.00771, 0.00718, 0.00657, 0.00591, 0.00548, 0.00512, 0.00458, 0.00423, 0.00386,
        0.00351, 0.00323, 0.00296, 0.00265, 0.00239, 0.00222, 0.00202, 0.00182, 0.00164, 0.00152,
        0.00136, 0.00125, 0.00111, 0.00101, 0.00087, 0.00078, 0.00072, 0.00065, 0.00060, 0.00050,
        0.00047, 0.00039, 0.00038, 0.00034, 0.00031, 0.00027, 0.00022, 0.00019, 0.00018, 0.00014,
        0.00012, 0.00012, 0.00010, 0.00008, 0.00008, 0.00006, 0.00005, 0.00004, 0.00004, 0.00004,
      ]
)";
}

TEST_CASE( "ReadoutCond (convert from JSON)" ) {
  auto j       = yaml2json( YAML::Load( yaml_example ) );
  auto modules = j["ReadoutModules"].get<std::map<std::string, ReadoutCond>>();

  REQUIRE( modules.find( "M1R1" ) != modules.end() );
  auto& rout = modules["M1R1"];

  REQUIRE( rout.size() == 1 );

  CHECK( rout.readoutType( 0 ) == ReadoutCond::Cathode );

  double jMin = 0., jMax = 0.;
  auto   jitter = rout.timeJitter( jMin, jMax, 0 );
  CHECK_THAT( jMin, Catch::Matchers::WithinAbs( 0, 1.e-4 ) );
  CHECK_THAT( jMax, Catch::Matchers::WithinRel( 58., 1.e-4 ) );
  CHECK( jitter.size() == 100 );
  CHECK_THAT( jitter[0], Catch::Matchers::WithinRel( 0.00001, 1.e-4 ) );
  CHECK_THAT( jitter[27], Catch::Matchers::WithinRel( 0.02991, 1.e-4 ) );
  CHECK_THAT( jitter[99], Catch::Matchers::WithinRel( 0.00004, 1.e-4 ) );

  CHECK( rout.singleGapClusterX( 0.3, 5, 0 ) == 1 );
  CHECK( rout.singleGapClusterX( 0.99, 5, 0 ) == 2 );
  CHECK( rout.singleGapClusterX( 0.99, 0.1, 0 ) == 2 );
  CHECK( rout.singleGapClusterX( 0.90, 0.1, 0 ) == 1 );
  CHECK( rout.singleGapClusterX( 0.92, 0.1, 0 ) == 2 );
}

TEST_CASE( "ReadoutCond (constructor from YAML)" ) {
  auto node    = YAML::Load( yaml_example );
  auto modules = node["ReadoutModules"].as<std::map<std::string, ReadoutCond>>();

  REQUIRE( modules.find( "M1R1" ) != modules.end() );
  auto& rout = modules["M1R1"];

  REQUIRE( rout.size() == 1 );

  CHECK( rout.readoutType( 0 ) == ReadoutCond::Cathode );

  double jMin = 0., jMax = 0.;
  auto   jitter = rout.timeJitter( jMin, jMax, 0 );
  CHECK_THAT( jMin, Catch::Matchers::WithinAbs( 0, 1.e-4 ) );
  CHECK_THAT( jMax, Catch::Matchers::WithinRel( 58., 1.e-4 ) );
  CHECK( jitter.size() == 100 );
  CHECK_THAT( jitter[0], Catch::Matchers::WithinRel( 0.00001, 1.e-4 ) );
  CHECK_THAT( jitter[27], Catch::Matchers::WithinRel( 0.02991, 1.e-4 ) );
  CHECK_THAT( jitter[99], Catch::Matchers::WithinRel( 0.00004, 1.e-4 ) );

  CHECK( rout.singleGapClusterX( 0.3, 5, 0 ) == 1 );
  CHECK( rout.singleGapClusterX( 0.99, 5, 0 ) == 2 );
  CHECK( rout.singleGapClusterX( 0.99, 0.1, 0 ) == 2 );
  CHECK( rout.singleGapClusterX( 0.90, 0.1, 0 ) == 1 );
  CHECK( rout.singleGapClusterX( 0.92, 0.1, 0 ) == 2 );
}
