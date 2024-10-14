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
#include <Detector/Muon/Types.h>

#include <catch2/catch.hpp>

using namespace LHCb::Detector::Muon;

TEST_CASE( "Types" ) {
  StationId s = StationN{3};
  CHECK( s.value() == 1 );
  CHECK( StationN{s}.value() == 3 );
}
