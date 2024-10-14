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
#include <Detector/Test/TmpDir.h>
#include <Detector/VP/DeVP.h>
#include <cstdio>
#include <filesystem>

#include <catch2/catch.hpp>

namespace {
  auto getDelta( const LHCb::Detector::DeIOV det ) { return det.detectorAlignment().delta(); }
} // namespace

TEST_CASE( "DDS with overlay" ) {
  namespace fs = std::filesystem;

  Detector::Test::Fixture f;

  auto& description = f.description();

  description.fromXML( "compact/components/debug/VP.xml" );
  // description.fromXML( "compact/run3/trunk/LHCb.xml" );

  REQUIRE( description.state() == dd4hep::Detector::READY );

  auto det = description.detector( "VP" );
  // the `!!` is needed because handles have `operator!` but not `operator bool`
  REQUIRE( !!det );

  Detector::Test::TmpDir tmp;

  // phase 1: prepare the directory with the initial alignments (to fill the overlay)
  auto cond_dir = tmp.path / "Conditions" / "VP" / "Alignment";
  fs::create_directories( cond_dir );
  std::ofstream( cond_dir / "Global.yml" ) << R"(---
VPSystem: !alignment
  position: [0.0 * mm, 5.0 * mm, 7.0 * mm]
VPLeft: !alignment
  position: [1.0 * mm, -2.0 * mm, 0.0 * mm]
VPRight: !alignment
  position: [-1.0 * mm, -2.0 * mm, 0.0 * mm]
)";
  REQUIRE( !fs::exists( cond_dir / "Ladders.yml" ) );

  // phase 2: load conditions with the overlay filled from the injected data
  LHCb::Detector::DetectorDataService dds( description, {"/world", "VP"} );
  dds.initialize( nlohmann::json( {{"repository", "file:tests/ConditionsIOV"}, {"overlay", {{"path", tmp.path}}}} ) );
  auto slice = dds.get_slice( 100 );
  REQUIRE( slice );

  using LHCb::Detector::DeVP;
  DeVP de = slice->get( det, LHCb::Detector::Keys::deKey );
  REQUIRE( !!de );

  using dd4hep::deg;
  using dd4hep::mm;
  using dd4hep::Position;
  using dd4hep::RotationZ;
  using dd4hep::RotationZYX;
  using dd4hep::Translation3D;

  // check that we have what we expect
  auto align = getDelta( de );
  CHECK( align.hasTranslation() );
  CHECK( !align.hasRotation() );
  CHECK( !align.hasPivot() );
  CHECK_THAT( align.translation.X(), Catch::Matchers::WithinAbs( 0 * mm, 1.e-6 ) );
  CHECK_THAT( align.translation.Y(), Catch::Matchers::WithinRel( 5 * mm, 1.e-4 ) );
  CHECK_THAT( align.translation.Z(), Catch::Matchers::WithinRel( 7 * mm, 1.e-4 ) );

  // phase 3: change the alignment paramters and update the overlay
  {
    RotationZYX   rot{RotationZ{5 * deg}};
    dd4hep::Delta new_align{Position{1 * mm, 4 * mm, 2.5 * mm}, rot};
    dds.update_alignment( de.detector(), new_align );
    // We cannot check anything here becase the overlay instance is private to the system
  }

  // phase 4: reload alignments (from overlay)
  dds.clear_slice_cache(); // this is to make sure we force a reload of everything
  auto slice2 = dds.get_slice( 100 );

  REQUIRE( slice2 );
  REQUIRE( slice != slice2 );
  de = slice2->get( det, LHCb::Detector::Keys::deKey );
  CHECK( !!de );

  align = getDelta( de );
  CHECK( align.hasTranslation() );
  CHECK( align.hasRotation() );
  CHECK( !align.hasPivot() );
  CHECK_THAT( align.translation.X(), Catch::Matchers::WithinRel( 1 * mm, 1.e-4 ) );
  CHECK_THAT( align.translation.Y(), Catch::Matchers::WithinRel( 4 * mm, 1.e-4 ) );
  CHECK_THAT( align.translation.Z(), Catch::Matchers::WithinRel( 2.5 * mm, 1.e-4 ) );
  CHECK_THAT( align.rotation.Phi(), Catch::Matchers::WithinRel( 5 * deg, 1.e-4 ) );
  CHECK_THAT( align.rotation.Theta(), Catch::Matchers::WithinAbs( 0, 1.e-6 ) );
  CHECK_THAT( align.rotation.Psi(), Catch::Matchers::WithinAbs( 0, 1.e-6 ) );

  // phase 5: dump alignments to files
  dds.dump_conditions( tmp.path );
  {
    REQUIRE( fs::exists( cond_dir / "Global.yml" ) );
    // dump_conditions (by default) should not write unchanged entries
    CHECK( !fs::exists( cond_dir / "Ladders.yml" ) );

    // check that the YAML file contains the expected changes
    auto gbl = YAML::LoadFile( tmp.path / "Conditions" / "VP" / "Alignment" / "Global.yml" );
    auto sys = gbl["VPSystem"];
    REQUIRE( sys );
    CHECK( sys.Tag() == "!alignment" );
    std::stringstream ss;
    ss << sys["position"];
    CHECK( ss.str() == "[1 * mm, 4 * mm, 2.5 * mm]" );
    CHECK( sys["rotation"] );
    CHECK( !sys["pivot"] );
  }

  // phase 6: reload the overlay from files
  dds.load_conditions( tmp.path );

  // phase 7: reload alignments (from overlay)
  dds.clear_slice_cache();
  auto slice3 = dds.get_slice( 100 );

  REQUIRE( slice3 );
  REQUIRE( slice2 != slice3 );
  de = slice3->get( det, LHCb::Detector::Keys::deKey );
  CHECK( !!de );

  align = getDelta( de );
  CHECK( align.hasTranslation() );
  CHECK( align.hasRotation() );
  CHECK( !align.hasPivot() );
  CHECK_THAT( align.translation.X(), Catch::Matchers::WithinRel( 1 * mm, 1.e-4 ) );
  CHECK_THAT( align.translation.Y(), Catch::Matchers::WithinRel( 4 * mm, 1.e-4 ) );
  CHECK_THAT( align.translation.Z(), Catch::Matchers::WithinRel( 2.5 * mm, 1.e-4 ) );
  CHECK_THAT( align.rotation.Phi(), Catch::Matchers::WithinRel( 5 * deg, 1.e-4 ) );
  CHECK_THAT( align.rotation.Theta(), Catch::Matchers::WithinAbs( 0, 1.e-6 ) );
  CHECK_THAT( align.rotation.Psi(), Catch::Matchers::WithinAbs( 0, 1.e-6 ) );
}
