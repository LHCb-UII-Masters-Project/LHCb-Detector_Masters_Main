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
#include <Detector/LHCb/Tell40Links.h>
#include <nlohmann/json.hpp>

bool operator==( const LHCb::Detector::Tell40Links::LinksStates& lhs,
                 const LHCb::Detector::Tell40Links::LinksStates& rhs ) {
  return lhs.offset == rhs.offset && lhs.size == rhs.size && lhs.mask == rhs.mask;
}

#include <catch2/catch.hpp>

TEST_CASE( "Tell40Links" ) {
  using LHCb::Detector::Tell40Links;

  nlohmann::json links_data = nlohmann::json::parse( R"(
    {
        "22550": {
            "tell40": "ECTEL011",
            "links": [0,1,2,3,4,5,6,7,8,9,10,11],
            "enabled": [1,1,1,1,1,1,1,1,1,1,1,1]
        },
        "24621": {
            "tell40": "HCTEL022",
            "links": [12,13,14,15,16,17,18,19,20,21,22,23],
            "enabled": [1,1,1,1,0,0,0,0,0,0,0,0]
        },
        "6148": {
            "tell40": "VCTEL102",
            "links": [0,1,2,3,4,5,6,7,8,9,10,11],
            "enabled": [1,1,1,1,1,1,1,1,1,1,0,0]
        },
        "9237": {
            "tell40": "R1TEL152",
            "links": [24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47],
            "enabled": [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]
        },
        "14866": {
            "tell40": "SATEL022",
            "links": [0,1,2,3,4,5,6,7,12,13,14,15,16,17,18,19],
            "enabled": [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]
        },
        "14867":{
            "tell40": "SATEL022",
            "links": [24,25,26,27,28,29,30,31,36,37,38,39,40,41,42,43],
            "enabled": [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]
        }
    }
  )" );

  SECTION( "default initialization" ) {
    Tell40Links t{};
    CHECK( t.links.empty() );
  }

  SECTION( "initialize from JSON" ) {
    Tell40Links t{links_data};
    CHECK( t.links.size() == links_data.size() );
    CHECK( *t[22550] == Tell40Links::LinksStates{0, 12, 0xfff} );
    CHECK( *t[24621] == Tell40Links::LinksStates{12, 12, 0xf} );
    CHECK( *t[6148] == Tell40Links::LinksStates{0, 12, 0x3ff} );
    CHECK( *t[9237] == Tell40Links::LinksStates{24, 24, 0xffffff} );
    CHECK( *t[14866] == Tell40Links::LinksStates{0, 24, 0x0ff0ff} );
    CHECK( *t[14867] == Tell40Links::LinksStates{24, 24, 0x0ff0ff} );

    CHECK( !t[0].has_value() );
  }

  SECTION( "LinksStates accessors" ) {
    Tell40Links::LinksStates link{{7, 8, 9, 10, 11, 12, 13, 14, 16}, {0, 1, 1, 0, 0, 1, 0, 0, 1}};
    CHECK( link == Tell40Links::LinksStates{0, 24, 0x11300} );
    CHECK( !link.isEnabled( 7 ) );
    CHECK( link.isEnabled( 8 ) );
    CHECK( link.isEnabled( 9 ) );
    CHECK( !link.isEnabled( 10 ) );
    CHECK( !link.isEnabled( 15 ) );
    CHECK( link.isEnabled( 16 ) );

    CHECK_THROWS_MATCHES( link.isEnabled( 25 ), Tell40Links::InvalidLinkIndex,
                          Catch::Message( "invalid Tell40 link index 25" ) );
  }
}
