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
#include <Detector/Muon/MuonVolumeID.h>

#include <iomanip>
#include <iosfwd>
#include <iostream>
#include <tuple>
#include <vector>

#include <catch2/catch.hpp>

namespace LHCb::Detector {
  std::ostream& operator<<( std::ostream& s, LHCb::Detector::MuonVolumeID const& mvid ) {
    return s << "MuonVolumeID{StationId{" << mvid.station().value() << "}, SideId{" << mvid.side().value()
             << "}, RegionId{" << mvid.region().value() << "}, ChamberId{" << mvid.chamber().value() << "}, GapId{"
             << mvid.gap().value() << "}} (0x" << std::hex << mvid.volumeID() << ")";
  }
} // namespace LHCb::Detector

TEST_CASE( "MuonVolumeID" ) {
  using LHCb::Detector::MuonVolumeID;
  using namespace LHCb::Detector::Muon;

  // test some valid MuonVolumeIDs
  std::vector<std::tuple<unsigned int, unsigned int, unsigned int, unsigned int, unsigned int>> test_data = {
      {2, 0, 1, 1, 1}, {5, 1, 4, 157, 3}};
  for ( auto [station, side, region, chamber, gap] : test_data ) {
    auto mvid = MuonVolumeID{StationN{station}, SideN{side}, RegionN{region}, ChamberN{chamber}, GapN{gap}};
    CHECK( mvid.isValid() );
    CHECK( mvid == MuonVolumeID{
                       200                         // system id
                       | ( ( station - 2 ) << 8 )  //
                       | ( ( side ) << 11 )        //
                       | ( ( region - 1 ) << 12 )  //
                       | ( ( chamber - 1 ) << 15 ) //
                       | ( ( gap - 1 ) << 23 )     //
                       | ( ( 1 ) << 25 )           // gap layer (unused)
                   } );
    // test round trip
    CHECK( mvid == MuonVolumeID{mvid.station(), mvid.side(), mvid.region(), mvid.chamber(), mvid.gap()} );
    CHECK( mvid == MuonVolumeID{mvid.volumeID()} );
  }

  // test error bit
  auto bad_mvid = MuonVolumeID::error();
  CHECK( bad_mvid.isValid() );
  CHECK( bad_mvid == MuonVolumeID{200 | ( 1u << 31 )} );

  // test of a special case
  auto special = MuonVolumeID{50500040};
  CHECK( special == MuonVolumeID{StationN{3}, SideN{0}, RegionN{2}, ChamberN{6}, GapN{3}} );
  CHECK( special ==
         MuonVolumeID{special.station(), special.side(), special.region(), special.chamber(), special.gap()} );
}
