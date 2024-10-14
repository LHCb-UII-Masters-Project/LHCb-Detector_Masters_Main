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
#include <Detector/Muon/FrontEndID.h>

#include <catch2/catch.hpp>

namespace {
  using value_t = unsigned int;
  using Mask    = LHCb::Detector::Muon::FrontEndID::Mask;
#if __cplusplus < 202002
  // helper to count the number of rightmost 0 bits, to be replaced with std::countr_zero in C++20
  constexpr static int countr_zero( Mask m ) noexcept {
    auto      tmp = static_cast<value_t>( m );
    const int max = static_cast<int>( 8 * sizeof( Mask ) );
    for ( int offset = 0; offset < max; ++offset ) {
      if ( tmp & 1 ) return offset;
      tmp = tmp >> 1;
    }
    return max;
  }
  constexpr static int countr_one( unsigned int m ) noexcept {
    auto      tmp = static_cast<value_t>( m );
    const int max = static_cast<int>( 8 * sizeof( Mask ) );
    for ( int offset = 0; offset < max; ++offset ) {
      if ( !( tmp & 1 ) ) return offset;
      tmp = tmp >> 1;
    }
    return max;
  }
#else
  constexpr static int countr_zero( Mask m ) noexcept { return std::countr_zero( static_cast<value_t>( m ) ); }
  constexpr static int countr_one( unsigned int m ) noexcept { return std::countr_one( m ); }
#endif
  constexpr static int get_mask_width( Mask m ) noexcept {
    auto tmp = static_cast<value_t>( m );
    tmp >>= countr_zero( m );
    return countr_one( tmp );
  }
} // namespace

TEST_CASE( "FrontEndID" ) {
  using LHCb::Detector::Muon::FrontEndID;
  using Mask = LHCb::Detector::Muon::FrontEndID::Mask;

  // FIXME: should we covert this to a static assert?
  SECTION( "check masks" ) {
    auto check_mask_info = []( Mask m, int offset, int width ) {
      REQUIRE( countr_zero( m ) == offset );
      REQUIRE( get_mask_width( m ) == width );
    };
    check_mask_info( Mask::Readout, 0, 2 );
    check_mask_info( Mask::Layer, 2, 2 );
    check_mask_info( Mask::FEGridX, 4, 7 );
    check_mask_info( Mask::FEGridY, 11, 5 );
    check_mask_info( Mask::FEIDX, 16, 7 );
    check_mask_info( Mask::FEIDY, 23, 5 );
  }

  SECTION( "default initialization" ) {
    const FrontEndID feid;
    REQUIRE( feid.get<Mask::Readout>() == 0 );
    REQUIRE( feid.get<Mask::Layer>() == 0 );
    REQUIRE( feid.get<Mask::FEGridX>() == 0 );
    REQUIRE( feid.get<Mask::FEGridY>() == 0 );
    REQUIRE( feid.get<Mask::FEIDX>() == 0 );
    REQUIRE( feid.get<Mask::FEIDY>() == 0 );
  }

  SECTION( "initialize from bitfield" ) {
    const FrontEndID feid{0b1010101010101010101010101010};
    REQUIRE( feid.get<Mask::Readout>() == 0b10 );
    REQUIRE( feid.get<Mask::Layer>() == 0b10 );
    REQUIRE( feid.get<Mask::FEGridX>() == 0b0101010 );
    REQUIRE( feid.get<Mask::FEGridY>() == 0b10101 );
    REQUIRE( feid.get<Mask::FEIDX>() == 0b0101010 );
    REQUIRE( feid.get<Mask::FEIDY>() == 0b10101 );
  }

  SECTION( "set-get roundtrip" ) {
    for ( value_t i = 0; i < 4; ++i ) {
      FrontEndID feid;
      REQUIRE( feid.set<Mask::Readout>( i ).get<Mask::Readout>() == i );
      REQUIRE( feid.set<Mask::Layer>( i ).get<Mask::Layer>() == i );
      REQUIRE( feid.set<Mask::FEGridX>( i ).get<Mask::FEGridX>() == i );
      REQUIRE( feid.set<Mask::FEGridY>( i ).get<Mask::FEGridY>() == i );
      REQUIRE( feid.set<Mask::FEIDX>( i ).get<Mask::FEIDX>() == i );
      REQUIRE( feid.set<Mask::FEIDY>( i ).get<Mask::FEIDY>() == i );
    }
  }
}
