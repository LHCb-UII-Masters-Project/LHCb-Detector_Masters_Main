/*****************************************************************************\
* (c) Copyright 2024 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#include <Core/ConditionHelper.h>
#include <Core/DeConditionCall.h>
#include <Core/DeIOV.h>
#include <Core/DetectorDataService.h>
#include <Core/Keys.h>
#include <DD4hep/Detector.h>
#include <DD4hep/DetectorLoad.h>
#include <Detector/Test/Fixture.h>
#include <nlohmann/json.hpp>

#include <catch2/catch.hpp>

namespace {
  struct DummyDeIOVObject final : LHCb::Detector::detail::DeIOVObject {
    using DeIOVObject::DeIOVObject;
    void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOVElement<DeIOVObject> )>& ) const override {}
  };

  struct DummyConditionCall : LHCb::Detector::DeConditionCall {
    using DeConditionCall::DeConditionCall;
    virtual dd4hep::Condition operator()( const dd4hep::ConditionKey&,
                                          dd4hep::cond::ConditionUpdateContext& context ) override final {
      auto det = dd4hep::Detector::getInstance().detector( "test" );
      return LHCb::Detector::DeIOV{new DummyDeIOVObject( det, context, 9000001, false )};
    }
  };
  long create_conditions_recipes( dd4hep::Detector& description, xml_h e ) {
    // Use the helper to load the XML, setup the callback according
    LHCb::Detector::ConditionConfigHelper<DummyConditionCall> config_helper{description, "test", e};
    config_helper.configure();
    return 1;
  }
} // namespace
DECLARE_XML_DOC_READER( Test_Dummy_cond, create_conditions_recipes )

static constexpr const char* main_xml = R"(<lccdd>
  <info name="Test" status="development"/>
  <geometry open="true" close="false"/>
  <includes>
    <!-- FIXME: it would be nice to embed the materials needed for the test. but I didn't manage -->
    <gdmlFile ref="tests/testscope/sc/elements.xml"/>
    <gdmlFile ref="tests/testscope/sc/materials.xml"/>
  </includes>
  <define>
    <constant name="world_side" value="10*m" />
    <constant name="world_x" value="world_side/2" />
    <constant name="world_y" value="world_side/2" />
    <constant name="world_z" value="world_side/2" />
  </define>
  <detectors>
    <detector name="test" type="DD4hep_VolumeAssembly" parent="/world">
      <envelope material="Air">
        <shape name="BiggerBox" type="Box" dx="100*cm" dy="100*cm" dz="100*cm"/>
      </envelope>
      <position x="0*cm" y="0*cm" z="0*cm"/>
    </detector>
  </detectors>
  <plugins>
    <plugin name="Test_Dummy_cond_XML_reader" type="xml">
      <conditions_config prefix="Conditions/" />
      <conditions path="test/no0cond.yml">
        <condition name="cond"/>
      </conditions>
    </plugin>
  </plugins>
</lccdd>)";

TEST_CASE( "ConditionGitReader.missing_0" ) {
  Detector::Test::Fixture f;

  auto& description = f.description();
  dd4hep::DetectorLoad{description}.processXMLString( main_xml );

  auto det = description.detector( "test" );
  // the `!!` is needed because handles have `operator!` but not `operator bool`
  REQUIRE( !!det );

  std::vector<std::string>            detector_list{"/world", "test"};
  LHCb::Detector::DetectorDataService dds( description, detector_list );
  // this DB is invalid as it misses the special condition for time point 0
  // and hitting it must throw an exception
  dds.initialize( nlohmann::json{{"repository",
                                  R"(json:{
    "Conditions": { "test": { "no0cond.yml": {".condition": "", "1000": "cond: {value: 42}"} } },
    ".schema.json": "{\"Conditions/test/no0cond.yml\":[\"cond\"]}"
  })"}} );

  // get a valid condition slice
  auto slice = dds.get_slice( 2000 );
  REQUIRE( slice );

  // this is a condition actually in the DB
  auto cond = slice->get( det, LHCb::Detector::item_key( "cond" ) ).get<nlohmann::json>();
  REQUIRE( cond.is_object() );
  CHECK( cond.contains( "value" ) );

  // hit the problematic IOV
  using Catch::Matchers::Contains;
  CHECK_THROWS_WITH( dds.get_slice( 100 ), Contains( "Conditions/test/no0cond.yml" ) && Contains( "missing 0 value" ) );
}

#include "DD4hep/detail/Handle.inl"
DD4HEP_INSTANTIATE_HANDLE_UNNAMED( DummyDeIOVObject, LHCb::Detector::detail::DeIOVObject, ConditionObject );
