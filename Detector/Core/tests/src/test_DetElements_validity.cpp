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
#include <DD4hep/Detector.h>
#include <Detector/Test/Fixture.h>
#include <cstdio>

#include <catch2/catch.hpp>

void _iterate_det_tree( dd4hep::DetElement de, std::vector<std::string>& invalid_placements, int level = 0 ) {
  dd4hep::PlacedVolume                place    = de.placement();
  const dd4hep::DetElement::Children& children = de.children();
  if ( !place.isValid() ) { invalid_placements.emplace_back( de.path() ); }

  for ( const auto& c : children ) { _iterate_det_tree( c.second, invalid_placements, level + 1 ); }
}

void _test_validity( std::string description_path, std::string test_name ) {
  namespace fs = std::filesystem;
  Detector::Test::Fixture f;
  auto&                   description = f.description();
  description.fromXML( description_path.c_str() );

  REQUIRE( description.state() == dd4hep::Detector::READY );

  auto det = description.world();
  // the `!!` is needed because handles have `operator!` but not `operator bool`
  REQUIRE( !!det );

  std::vector<std::string> invalid_placements;
  _iterate_det_tree( det, invalid_placements );

  dd4hep::setPrintLevel( dd4hep::ERROR );
  for ( const auto& p : invalid_placements ) {
    dd4hep::printout( dd4hep::ERROR, test_name.c_str(), "Invalid DetElement: %s", p.c_str() );
  }

  REQUIRE( invalid_placements.size() == 0 );
}

TEST_CASE( "DetElements_validity_run3_trunk" ) {
  _test_validity( "compact/run3/trunk/LHCb.xml", "DetElements_validity_run3_trunk" );
}

TEST_CASE( "DetElements_validity_run4_trunk" ) {
  _test_validity( "compact/run4/trunk/LHCb.xml", "DetElements_validity_run4_trunk" );
}

TEST_CASE( "DetElements_validity_run5_baseline" ) {
  _test_validity( "compact/run5/branch-baseline/LHCb.xml", "DetElements_validity_run5_baseline" );
}
