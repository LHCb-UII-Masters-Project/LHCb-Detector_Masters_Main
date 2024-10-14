/*****************************************************************************\
* (c) Copyright 2023 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#include <Core/CondDBSchema.h>
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
      <conditions path="test/conds.yml">
        <condition name="cond1"/>
        <condition name="cond2"/>
      </conditions>
    </plugin>
  </plugins>
</lccdd>)";

TEST_CASE( "CondDBSchema.basic" ) {
  Detector::Test::Fixture f;

  auto& description = f.description();
  dd4hep::DetectorLoad{description}.processXMLString( main_xml );

  auto det = description.detector( "test" );
  // the `!!` is needed because handles have `operator!` but not `operator bool`
  REQUIRE( !!det );

  std::vector<std::string>            detector_list{"/world", "test"};
  LHCb::Detector::DetectorDataService dds( description, detector_list );
  dds.initialize( nlohmann::json{{"repository",
                                  R"(json:{
    "Conditions": { "test": { "conds.yml": "cond1: { par1: 1, par2: \"text\" }\ncond2: { value: 42 }" } },
    ".schema.json": "{\"Conditions/test/conds.yml\":[\"cond1\",\"cond2\"]}"
  })"}} );

  // get a condition slice
  auto slice = dds.get_slice( 200 );
  REQUIRE( slice );

  // this is a condition actually in the DB
  auto cond1 = slice->get( det, LHCb::Detector::item_key( "cond1" ) ).get<nlohmann::json>();
  REQUIRE( cond1.is_object() );
  CHECK( cond1.contains( "par1" ) );

  // this is condition is not in the DB so it should be implicitly created as null
  auto cond2 = slice->get( det, LHCb::Detector::item_key( "cond2" ) ).get<nlohmann::json>();
  REQUIRE( cond2.is_object() );
  CHECK( cond2.contains( "value" ) );
}

TEST_CASE( "CondDBSchema.missing_condition" ) {
  using Catch::Matchers::Contains;

  Detector::Test::Fixture f;

  auto& description = f.description();
  dd4hep::DetectorLoad{description}.processXMLString( main_xml );

  auto det = description.detector( "test" );
  // the `!!` is needed because handles have `operator!` but not `operator bool`
  REQUIRE( !!det );

  std::vector<std::string>            detector_list{"/world", "test"};
  LHCb::Detector::DetectorDataService dds( description, detector_list );

  CHECK_THROWS_WITH( dds.initialize( nlohmann::json{{"repository",
                                                     R"(json:{
    "Conditions": { "test": { "conds.yml": "cond1: { par1: 1, par2: \"text\" }" } },
    ".schema.json": "{\"Conditions/test/conds.yml\":[\"cond1\"]}"
  })"}} ),
                     Contains( "Conditions/test/conds.yml[cond2]" ) );
}

TEST_CASE( "CondDBSchema.no_schema" ) {
  Detector::Test::Fixture f;

  auto& description = f.description();
  dd4hep::DetectorLoad{description}.processXMLString( main_xml );

  auto det = description.detector( "test" );
  // the `!!` is needed because handles have `operator!` but not `operator bool`
  REQUIRE( !!det );

  std::vector<std::string>            detector_list{"/world", "test"};
  LHCb::Detector::DetectorDataService dds( description, detector_list );
  dds.initialize( nlohmann::json{{"repository",
                                  R"(json:{
    "Conditions": { "test": { "conds.yml": "cond1: { par1: 1, par2: \"text\" }" } }
  })"}} );

  // get a condition slice
  auto slice = dds.get_slice( 200 );
  REQUIRE( slice );

  // this is a condition actually in the DB
  auto cond1 = slice->get( det, LHCb::Detector::item_key( "cond1" ) ).get<nlohmann::json>();
  REQUIRE( cond1.is_object() );
  CHECK( cond1.contains( "par1" ) );

  // this is condition is not in the DB so it should be implicitly created as null
  auto cond2 = slice->get( det, LHCb::Detector::item_key( "cond2" ) ).get<nlohmann::json>();
  REQUIRE( cond2.is_null() );
}

static constexpr const char* optional_xml = R"(<lccdd>
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
      <conditions path="test/conds.yml">
        <condition name="cond1"/>
        <condition name="cond2" optional="1"/>
      </conditions>
    </plugin>
  </plugins>
</lccdd>)";

TEST_CASE( "CondDBSchema.no_schema_optional_condition" ) {

  Detector::Test::Fixture f;

  auto& description = f.description();
  dd4hep::DetectorLoad{description}.processXMLString( optional_xml );

  auto det = description.detector( "test" );
  // the `!!` is needed because handles have `operator!` but not `operator bool`
  REQUIRE( !!det );

  std::vector<std::string>            detector_list{"/world", "test"};
  LHCb::Detector::DetectorDataService dds( description, detector_list );
  dds.initialize( nlohmann::json{{"repository",
                                  R"(json:{
    "Conditions": { "test": { "conds.yml": "cond1: { par1: 1, par2: \"text\" }" } },
    ".schema.json": "{\"Conditions/test/conds.yml\":[\"cond1\",\"cond2\"]}"
  })"}} );

  // get a condition slice
  auto slice = dds.get_slice( 200 );
  REQUIRE( slice );

  // this is a condition actually in the DB
  auto cond1 = slice->get( det, LHCb::Detector::item_key( "cond1" ) ).get<nlohmann::json>();
  REQUIRE( cond1.is_object() );
  CHECK( cond1.contains( "par1" ) );

  // this is condition is not in the DB so it should be implicitly created as null
  auto cond2 = slice->get( det, LHCb::Detector::item_key( "cond2" ) ).get<nlohmann::json>();
  REQUIRE( cond2.is_null() );
}

static constexpr const char* optional_pattern_xml = R"delim(<lccdd>
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
      <condition_pattern path_regex=".*/(\w+)$"  path="test/conds.yml" name="cond1" optional="1"/>
    </plugin>
  </plugins>
</lccdd>)delim";

TEST_CASE( "CondDBSchema.optional_pattern" ) {

  Detector::Test::Fixture f;

  auto& description = f.description();
  dd4hep::DetectorLoad{description}.processXMLString( optional_pattern_xml );

  auto det = description.detector( "test" );
  // the `!!` is needed because handles have `operator!` but not `operator bool`
  REQUIRE( !!det );

  std::vector<std::string>            detector_list{"/world", "test"};
  LHCb::Detector::DetectorDataService dds( description, detector_list );
  dds.initialize( nlohmann::json{{"repository",
                                  R"(json:{
    "Conditions": { "test": { "conds.yml": "" } },
    ".schema.json": "{\"Conditions/test/conds.yml\":[\"cond1\"]}"
  })"}} );

  // get a condition slice
  auto slice = dds.get_slice( 200 );
  REQUIRE( slice );

  // this is a condition actually in the DB
  auto cond1 = slice->get( det, LHCb::Detector::item_key( "cond1" ) ).get<nlohmann::json>();
  REQUIRE( cond1.is_null() );
}

static constexpr const char* non_optional_pattern_xml = R"delim(<lccdd>
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
      <condition_pattern path_regex=".*/(\w+)$"  path="test/conds.yml" name="cond1"/>
    </plugin>
  </plugins>
</lccdd>)delim";

TEST_CASE( "CondDBSchema.missing_non_optional_pattern" ) {
  using Catch::Matchers::Contains;
  Detector::Test::Fixture f;

  auto& description = f.description();
  dd4hep::DetectorLoad{description}.processXMLString( non_optional_pattern_xml );

  auto det = description.detector( "test" );
  // the `!!` is needed because handles have `operator!` but not `operator bool`
  REQUIRE( !!det );

  std::vector<std::string>            detector_list{"/world", "test"};
  LHCb::Detector::DetectorDataService dds( description, detector_list );
  CHECK_THROWS_WITH( dds.initialize( nlohmann::json{{"repository",
                                                     R"(json:{
    "Conditions": { "test": { "conds.yml": "" } },
    ".schema.json": "{\"Conditions/test/conds.yml\":[ ]}"
  })"}} ),
                     Contains( "Conditions/test/conds.yml[cond1]" ) );
}

#include "DD4hep/detail/Handle.inl"
DD4HEP_INSTANTIATE_HANDLE_UNNAMED( DummyDeIOVObject, LHCb::Detector::detail::DeIOVObject, ConditionObject );
