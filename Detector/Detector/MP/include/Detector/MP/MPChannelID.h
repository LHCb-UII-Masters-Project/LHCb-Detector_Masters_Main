/*****************************************************************************\
* (c) Copyright 2000-2019 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once

#include <array>
#include <cassert>
#include <climits>
#include <cstdint>
#include <exception>
#include <fmt/format.h>
#include <functional>
#include <iostream>
#include <type_traits>
#include <utility>

namespace LHCb::Detector {
  /** @brief a faked MIGHTY tracker channel ID
   *
   * This class encodes a MIGHTY tracker channel ID. It has the following
   * features:
   *
   * - It uses the MCKey of the particle.
   * - There is no physical meaning to the LHCBID
   */
  class MPChannelID final {
    /// Bitmasks
    enum struct Mask : unsigned {
      key         = 0x1ffff, // 00011111111111111111 -> 131071
      layer       = 0x20000, // 00100000000000000000
      station     = 0xc0000, // 11000000000000000000
      uniqueLayer = layer | station,
    };

    template <Mask m>
    [[nodiscard]] static constexpr unsigned int extract( unsigned int i ) {
      constexpr auto b =
          __builtin_ctz( static_cast<unsigned int>( m ) ); // FIXME: C++20 replace __builtin_ctz with std::countr_zero
      return ( i & static_cast<unsigned int>( m ) ) >> b;
    }

    template <Mask m>
    [[nodiscard]] static constexpr unsigned int shift( unsigned int i ) {
      constexpr auto b =
          __builtin_ctz( static_cast<unsigned int>( m ) ); // FIXME: C++20 replace __builtin_ctz with std::countr_zero
      auto v = ( i << static_cast<unsigned int>( b ) );
      assert( extract<m>( v ) == i );
      return v;
    }

    template <Mask m, typename T>
    [[nodiscard]] static constexpr unsigned int shift( T i ) {
      return shift<m>( to_unsigned( i ) );
    }

  public:
    enum struct StationID : unsigned int {};
    [[nodiscard]] friend constexpr unsigned int to_unsigned( StationID id ) { return static_cast<unsigned>( id ); }

    enum struct LayerID : unsigned int {};
    [[nodiscard]] friend constexpr unsigned int to_unsigned( LayerID id ) { return static_cast<unsigned>( id ); }

    template <typename IDtype>
    struct to_ID_t {
      constexpr std::vector<IDtype> operator()( const std::vector<int>& v ) const {
        std::vector<IDtype> ret;
        ret.reserve( v.size() );
        std::transform( v.begin(), v.end(), std::back_inserter( ret ),
                        []( int a ) { return IDtype{static_cast<unsigned int>( a )}; } );
        return ret;
      };
    };

    static constexpr auto to_stationID = to_ID_t<StationID>{};
    static constexpr auto to_layerID   = to_ID_t<LayerID>{};

    /// Default Constructor
    constexpr MPChannelID() = default;

    /// Constructor from int
    constexpr explicit MPChannelID( unsigned int id ) : m_channelID{id} {}

    /// Explicit constructor from the geometrical location and key
    constexpr MPChannelID( unsigned int station, unsigned int layer, unsigned int key )
        : MPChannelID{shift<Mask::station>( station ) | shift<Mask::layer>( layer ) | shift<Mask::key>( key )} {}

    // Operator overload, to cast channel ID to unsigned int. Used by linkers where the key (channel id) is an int
    constexpr operator unsigned int() const { return m_channelID; }

    /// Comparison equality
    constexpr friend bool operator==( MPChannelID lhs, MPChannelID rhs ) { return lhs.channelID() == rhs.channelID(); }

    /// Comparison <
    constexpr friend bool operator<( MPChannelID lhs, MPChannelID rhs ) { return lhs.channelID() < rhs.channelID(); }

    /// Comparison >
    constexpr friend bool operator>( MPChannelID lhs, MPChannelID rhs ) { return rhs < lhs; }

    /// Increment the channelID
    constexpr MPChannelID& advance() {
      ++m_channelID;
      return *this;
    }

    /// Retrieve const  MP Channel ID
    [[nodiscard]] constexpr unsigned int channelID() const { return m_channelID; }

    /// Retrieve MCKey of particle
    [[nodiscard]] constexpr unsigned int key() const { return extract<Mask::key>( m_channelID ); }

    /// Retrieve Layer id
    [[nodiscard]] constexpr LayerID layer() const { return LayerID{extract<Mask::layer>( m_channelID )}; }

    /// Retrieve Station id
    [[nodiscard]] constexpr StationID station() const { return StationID{extract<Mask::station>( m_channelID )}; }

    /// Retrieve unique layer
    [[nodiscard]] constexpr unsigned int globalLayerID() const { return extract<Mask::uniqueLayer>( m_channelID ); }

    /// Retrieve global station index [0..2]
    [[nodiscard]] constexpr unsigned int globalStationIdx() const {
      assert( to_unsigned( station() ) != 0 &&
              "Trying to get the station idx of a station 0! This typically happens when having a kInvalidChannel." );
      return to_unsigned( station() ) - 1;
    }

    /// Retrieve global layer index [0..11]
    [[nodiscard]] constexpr unsigned int globalLayerIdx() const {
      return ( 2 * globalStationIdx() + to_unsigned( layer() ) );
    }

    /// Retrieve local layer Index
    [[nodiscard]] constexpr unsigned int localLayerIdx() const { return to_unsigned( layer() ); }

    // /// Retrieve channelID for monitoring
    // [[nodiscard]] constexpr unsigned int key() const {
    //   return to_unsigned( key() );
    // }

    friend std::ostream& operator<<( std::ostream& s, const MPChannelID& obj ) {
      return s << "{ MPChannelID : "
               << " key =" << obj.key() << " layer=" << to_unsigned( obj.layer() )
               << " station=" << to_unsigned( obj.station() ) << " }";
    }

    /// Operator overload, to cast channel ID to std::string
    friend std::string toString( const MPChannelID& id ) {
      return fmt::format( "T{}L{}K{}", to_unsigned( id.station() ), to_unsigned( id.layer() ), id.key() );
    }

    /// Operator overload, to cast channel ID to std::string
    std::string toString() const {
      return fmt::format( "T{}L{}K{}", to_unsigned( station() ), to_unsigned( layer() ), key() );
    }

  private:
    unsigned int m_channelID{0}; /// MP Channel ID

  }; // class MPChannelID
} // namespace LHCb::Detector
