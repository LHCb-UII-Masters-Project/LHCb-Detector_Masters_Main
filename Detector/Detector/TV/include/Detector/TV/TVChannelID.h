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
#include <cassert>
#include <ostream>

namespace LHCb::Detector {

  /** @class TVChannelID TVChannelID.h
   *
   * This class identifies a single pixel in the TV
   *
   * @author Victor Coco
   *
   */

  class TVChannelID final {
    /// Bitmasks for bitfield channelID
    enum struct Mask : unsigned {
      row    = 0xffL,
      col    = 0xff00L,
      chip   = 0x30000L,
      sensor = 0x3fc0000L,
      orfy   = 0x4000000L,
      orfx   = 0x8000000L,
      scol   = chip | col
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

    template <Mask m, typename T>
    constexpr TVChannelID& set( T val ) {
      m_channelID &= ~static_cast<unsigned int>( m );
      m_channelID |= shift<m>( to_unsigned( val ) );
      return *this;
    }

  public:
    enum struct OrfxID : unsigned int {};
    [[nodiscard]] friend constexpr unsigned int to_unsigned( OrfxID id ) { return static_cast<unsigned>( id ); }
    enum struct OrfyID : unsigned int {};
    [[nodiscard]] friend constexpr unsigned int to_unsigned( OrfyID id ) { return static_cast<unsigned>( id ); }
    enum struct SensorID : unsigned int {};
    [[nodiscard]] friend constexpr unsigned int to_unsigned( SensorID id ) { return static_cast<unsigned>( id ); }
    enum struct ChipID : unsigned int {};
    [[nodiscard]] friend constexpr unsigned int to_unsigned( ChipID id ) { return static_cast<unsigned>( id ); }
    enum struct ColumnID : unsigned int {};
    [[nodiscard]] friend constexpr unsigned int to_unsigned( ColumnID id ) { return static_cast<unsigned>( id ); }
    enum struct ScolID : unsigned int {};
    [[nodiscard]] friend constexpr unsigned int to_unsigned( ScolID id ) { return static_cast<unsigned>( id ); }
    enum struct RowID : unsigned int {};
    [[nodiscard]] friend constexpr unsigned int to_unsigned( RowID id ) { return static_cast<unsigned>( id ); }

    /// Default Constructor
    constexpr TVChannelID() = default;

    /// Constructor with channelID
    constexpr explicit TVChannelID( unsigned int id ) : m_channelID( id ) {}

    /// Constructor with sensor, chip, column, row, orfx and orfy
    constexpr TVChannelID( SensorID sensor, ChipID chip, ColumnID col, RowID row, OrfxID orfx = OrfxID{0},
                           OrfyID orfy = OrfyID{0} )
        : TVChannelID{shift<Mask::orfx>( orfx ) | shift<Mask::orfy>( orfy ) | shift<Mask::sensor>( sensor ) |
                      shift<Mask::chip>( chip ) | shift<Mask::col>( col ) | shift<Mask::row>( row )} {}

    /// Constructor with sensor, scol, row, orfx and orfy
    constexpr TVChannelID( SensorID sensor, ScolID scol, RowID row, OrfxID orfx = OrfxID{0}, OrfyID orfy = OrfyID{0} )
        : TVChannelID{shift<Mask::orfx>( orfx ) | shift<Mask::orfy>( orfy ) | shift<Mask::sensor>( sensor ) |
                      shift<Mask::scol>( scol ) | shift<Mask::row>( row )} {}

    /// Cast
    constexpr operator unsigned int() const { return m_channelID; }

    /// Get sensor column number
    [[nodiscard]] constexpr unsigned int scol() const { return extract<Mask::scol>( m_channelID ); }

    /// Get module number
    [[nodiscard]] constexpr unsigned int module() const { return to_unsigned( sensor() ) / 4; }

    /// Get station number
    [[nodiscard]] constexpr unsigned int station() const { return module() / 2; }

    /// Get side (left/right)
    [[nodiscard]] constexpr unsigned int sidepos() const { return module() % 2; }

    /// Retrieve const  TV Channel ID
    [[nodiscard]] constexpr unsigned int channelID() const { return m_channelID; }

    /// Retrieve pixel row
    [[nodiscard]] constexpr RowID row() const { return RowID{extract<Mask::row>( m_channelID )}; }

    /// Update pixel row
    TVChannelID& setRow( RowID row ) { return set<Mask::row>( row ); }

    /// Retrieve pixel column
    [[nodiscard]] constexpr ColumnID col() const { return ColumnID{extract<Mask::col>( m_channelID )}; }

    /// Update pixel column
    TVChannelID& setCol( ColumnID col ) { return set<Mask::col>( col ); }

    /// Retrieve chip number
    [[nodiscard]] constexpr ChipID chip() const { return ChipID{extract<Mask::chip>( m_channelID )}; }

    /// Update chip number
    TVChannelID& setChip( ChipID chip ) { return set<Mask::chip>( chip ); }

    /// Retrieve sensor number
    [[nodiscard]] constexpr SensorID sensor() const { return SensorID{extract<Mask::sensor>( m_channelID )}; }

    /// Update sensor number
    TVChannelID& setSensor( SensorID sensor ) { return set<Mask::sensor>( sensor ); }

    /// Retrieve orfy
    [[nodiscard]] constexpr OrfyID orfy() const { return OrfyID{extract<Mask::orfy>( m_channelID )}; }

    /// Update orfy
    TVChannelID& setOrfy( OrfyID orfy ) { return set<Mask::orfy>( orfy ); }

    /// Retrieve orfx
    [[nodiscard]] constexpr OrfxID orfx() const { return OrfxID{extract<Mask::orfx>( m_channelID )}; }

    /// Update orfx
    TVChannelID& setOrfx( OrfxID orfx ) { return set<Mask::orfx>( orfx ); }

    /// Retrieve scol
    [[nodiscard]] constexpr ScolID scol2() const { return ScolID{extract<Mask::scol>( m_channelID )}; }

    /// Update scol
    TVChannelID& setScol( ScolID scol2 ) { return set<Mask::scol>( scol2 ); }

    friend std::ostream& operator<<( std::ostream& str, const TVChannelID& obj );

  private:
    unsigned int m_channelID{0}; ///< TV Channel ID

  }; // class TVChannelID

} // namespace LHCb::Detector
