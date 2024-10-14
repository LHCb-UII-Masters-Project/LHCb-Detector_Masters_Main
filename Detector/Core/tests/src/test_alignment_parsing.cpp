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
#include <Core/yaml_converters.h>
#include <DD4hep/AlignmentData.h>
#include <string_view>
#include <yaml-cpp/yaml.h>

// this has to be before the include of Catch2 to avoid errors in clang
namespace {
  bool operator==( const dd4hep::Delta& lhs, const dd4hep::Delta& rhs ) {
    return lhs.flags == rhs.flags && ( !lhs.hasTranslation() || lhs.translation == rhs.translation ) &&
           ( !lhs.hasRotation() || lhs.rotation == rhs.rotation ) && ( !lhs.hasPivot() || lhs.pivot == rhs.pivot );
  }
} // namespace

#include <catch2/catch.hpp>

using namespace LHCb::YAMLConverters;
using Catch::Matchers::Contains;
using dd4hep::Delta;

TEST_CASE( "alignment parsing" ) {
  YAML::Node data = YAML::Load( R"(---
no-align: this is not an alignment

empty: !alignment
just-pos: !alignment
    position: [0.,0.,0.]
just-rot: !alignment
    rotation: [0.,0.,0.]
just-piv: !alignment
    pivot: [0.,0.,0.]
rot1: !alignment
    position: [0.,0.,0.]
    rotation: [0.,0.,0.]
rot2: !alignment
    rotation: [0.,0.,0.]
    pivot: [0.,0.,0.]
full: !alignment
    position: [0.,0.,0.]
    rotation: [0.,0.,0.]
    pivot: [0.,0.,0.]

with-units: !alignment
    position: [ 5 * mm, 1 * m, 7.5 * cm ]
default-units: !alignment
    position: [ 1, 2, 3 ]
default-units-2: !alignment
    position: [ 1e0, +2, -3 ]

bad-expression: !alignment
    position: [ bad expression, 0, 0 ]
wrong-size-1: !alignment
    position: [ 1 ]
wrong-size-2: !alignment
    rotation: [ 1, 2, 3, 4 ]
)" );

  auto get = [&data]( const std::string& name ) { return make_condition( name, data[name] ).get<dd4hep::Delta>(); };

  SECTION( "not a condition" ) { CHECK_THROWS( get( "no-align" ) ); }

  SECTION( "optional fields" ) {
    CHECK( get( "empty" ) == Delta{} );
    CHECK( get( "just-pos" ) == Delta( dd4hep::Position{0, 0, 0} ) );
    CHECK( get( "just-rot" ) == Delta( dd4hep::RotationZYX{0, 0, 0} ) );
    CHECK_THROWS_WITH( get( "just-piv" ), Contains( "has a pivot, but no rotation" ) );
    CHECK( get( "rot1" ) == Delta( dd4hep::Position{0, 0, 0}, dd4hep::RotationZYX{0, 0, 0} ) );
    CHECK( get( "rot2" ) == Delta( dd4hep::Translation3D{0, 0, 0}, dd4hep::RotationZYX{0, 0, 0} ) );
    CHECK( get( "full" ) ==
           Delta( dd4hep::Position{0, 0, 0}, dd4hep::Translation3D{0, 0, 0}, dd4hep::RotationZYX{0, 0, 0} ) );
  }

  SECTION( "units" ) {
    CHECK( get( "with-units" ) == Delta( dd4hep::Position{5 * dd4hep::mm, 1 * dd4hep::m, 7.5 * dd4hep::cm} ) );
    CHECK( get( "default-units" ) == Delta( dd4hep::Position{1 * dd4hep::mm, 2 * dd4hep::mm, 3 * dd4hep::mm} ) );
    CHECK( get( "default-units-2" ) == Delta( dd4hep::Position{1 * dd4hep::mm, 2 * dd4hep::mm, -3 * dd4hep::mm} ) );
  }

  SECTION( "failure modes" ) {
    CHECK_THROWS_WITH( get( "bad-expression" ), Contains( "Evaluation error" ) && Contains( "bad expression" ) );
    CHECK_THROWS_WITH( get( "wrong-size-1" ),
                       Contains( "invalid argument" ) && Contains( "wrong-size-1" ) && Contains( "position" ) );
    CHECK_THROWS_WITH( get( "wrong-size-2" ),
                       Contains( "invalid argument" ) && Contains( "wrong-size-2" ) && Contains( "rotation" ) );
  }
}
