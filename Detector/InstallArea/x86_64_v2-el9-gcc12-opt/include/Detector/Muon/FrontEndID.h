/*****************************************************************************\
* (c) Copyright 2000-2022 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once

#if __cplusplus >= 202002
#  include <bit>
#endif

namespace LHCb::Detector::Muon {

  struct FrontEndID {
    using value_t = unsigned int;

    constexpr FrontEndID() = default;
    constexpr FrontEndID( value_t value ) : m_channelID{value} {}

    enum class Mask : value_t {
      Readout = 0b0000000000000000000000000011,
      Layer   = 0b0000000000000000000000001100,
      FEGridX = 0b0000000000000000011111110000,
      FEGridY = 0b0000000000001111100000000000,
      FEIDX   = 0b0000011111110000000000000000,
      FEIDY   = 0b1111100000000000000000000000
    };

  private:
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
#else
    constexpr static int countr_zero( Mask m ) noexcept { return std::countr_zero( static_cast<value_t>( m ) ); }
#endif

    // helper to make sure offset is computed at compile time
    template <Mask m, int offset>
    constexpr FrontEndID& set( value_t value ) {

      const value_t m_ = static_cast<value_t>( m );
      m_channelID      = ( m_channelID & ~m_ ) | ( ( value << offset ) & m_ );

      return *this;
    }

    // helper to make sure offset is computed at compile time
    template <Mask m, int offset>
    constexpr value_t get() const {
      return ( m_channelID & static_cast<value_t>( m ) ) >> offset;
    }

  public:
    template <Mask m>
    constexpr FrontEndID& set( value_t value ) {
      return set<m, countr_zero( m )>( value );
    }

    template <Mask m>
    constexpr value_t get() const {
      return get<m, countr_zero( m )>();
    }

    constexpr value_t getReadout() const { return get<Mask::Readout>(); }
    constexpr value_t getLayer() const { return get<Mask::Layer>(); }
    constexpr value_t getFEGridX() const { return get<Mask::FEGridX>(); }
    constexpr value_t getFEGridY() const { return get<Mask::FEGridY>(); }
    constexpr value_t getFEIDX() const { return get<Mask::FEIDX>(); }
    constexpr value_t getFEIDY() const { return get<Mask::FEIDY>(); }

    constexpr FrontEndID& setReadout( value_t value ) { return set<Mask::Readout>( value ); }
    constexpr FrontEndID& setLayer( value_t value ) { return set<Mask::Layer>( value ); }
    constexpr FrontEndID& setFEGridX( value_t value ) { return set<Mask::FEGridX>( value ); }
    constexpr FrontEndID& setFEGridY( value_t value ) { return set<Mask::FEGridY>( value ); }
    constexpr FrontEndID& setFEIDX( value_t value ) { return set<Mask::FEIDX>( value ); }
    constexpr FrontEndID& setFEIDY( value_t value ) { return set<Mask::FEIDY>( value ); }

    value_t m_channelID = 0;
  };

} // namespace LHCb::Detector::Muon
