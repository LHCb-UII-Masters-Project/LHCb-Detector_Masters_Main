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
#include <cstdlib>
#include <fmt/format.h>
#include <ostream>
#include <string>

#include "FTChannelID.h"

namespace LHCb::Detector {
  // Forward declarations

  /** @class FTSourceID FTSourceID.h
   *
   * Source ID for the Fibre Tracker (LHCb Upgrade)
   *
   * @author FT software team
   *
   */

  class FTSourceID final {
  private:
    /// Bitmasks for bitfield sourceID
    // Format is [station station][layer layer][quarter quarter][datalink datalink][port]
    // FIXME: LHCBSCIFI-195
    enum struct Mask : std::uint16_t {
      port           = 0x1,    // 0000000000000001
      dataLink       = 0xe,    // 0000000000001110
      quarter        = 0x30L,  // 0000000000110000
      layer          = 0xc0L,  // 0000000011000000
      station        = 0x300L, // 0000001100000000
      uniqueLayer    = layer | station,
      uniqueQuarter  = quarter | uniqueLayer,
      uniqueDataLink = dataLink | uniqueQuarter,
      uniquePort     = port | uniqueDataLink,
      header         = 0xF800 // 1111100000000000
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

    /// Additional station to start the monitoring at station 1 and not 0
    [[nodiscard]] constexpr unsigned int moniAddStation() const {
      // replace station with 1 + station...
      //
      // ugh... seems like a really bad idea that will confuse someone at some point,
      // and will result in an 'off-by-one' conclusion in eg. some conditions generated
      // from the monitoring in the future... but if you really want that, instead of
      // sticking to a single convention throughout, go ahead...

      return ( m_sourceID & ~static_cast<unsigned int>( Mask::station ) ) |
             shift<Mask::station>( extract<Mask::station>( m_sourceID ) + FTConstants::stationShift );
    }

    /// Helper method to check if digit is valid
    static constexpr unsigned digit( char c ) {
      if ( c < '0' || c > '9' ) throw std::invalid_argument( "Invalid digit" );
      return c - '0';
    }
    unsigned int m_sourceID{0}; ///< FT Source ID
  public:
    /// Explicit constructor from the geometrical location
    constexpr FTSourceID( FTChannelID::StationID station, FTChannelID::LayerID layer, FTChannelID::QuarterID quarter,
                          unsigned int dataLink, unsigned int port ) {
      unsigned int sourceHeader =
          is_left( quarter ) ? LHCb::Detector::FT::RawBank::leftHeader : LHCb::Detector::FT::RawBank::rightHeader;
      m_sourceID = shift<Mask::header>( sourceHeader ) | shift<Mask::station>( station ) | shift<Mask::layer>( layer ) |
                   shift<Mask::quarter>( quarter ) | shift<Mask::dataLink>( dataLink ) | shift<Mask::port>( port );
    }

    /// Explicit constructor from geometrical location string
    /// Assumes a 'name' formatted as TxLxQxDxPx
    constexpr FTSourceID( std::string_view name )
        : FTSourceID{FTChannelID::StationID{digit( name[1] )}, FTChannelID::LayerID{digit( name[3] )},
                     FTChannelID::QuarterID{digit( name[5] )}, digit( name[7] ), digit( name[9] )} {
      assert( ( name.size() == 10 ) );
      assert( ( name[0] == 'T' && name[2] == 'L' && name[4] == 'Q' && name[6] == 'D' && name[8] == 'P' ) );
      assert( digit( name[1] ) < FTConstants::stationShift + FTConstants::nStations ); // note: test setup runs from
                                                                                       // 0-2, actual detector from 1-3
    }
    /// Explicit constructor from sourceID
    constexpr explicit FTSourceID( unsigned int id ) : m_sourceID( id ){};

    /// Default Constructor
    constexpr FTSourceID() = default;

    /// Operator overload, to cast channel ID to unsigned int. Used by linkers where the key
    /// (channel id) is an int
    [[nodiscard]] constexpr operator unsigned int() const { return m_sourceID; }

    /// Operator overload, to cast channel ID to std::string
    friend std::string toString( const FTSourceID& id ) {
      return fmt::format( "T{}L{}Q{}D{}P{}", to_unsigned( id.station() ), to_unsigned( id.layer() ),
                          to_unsigned( id.quarter() ), id.dataLink(), id.port() );
    }

    /// Needed in order to compare to the reports.
    friend std::string toStringReport( const FTSourceID& id ) {
      return fmt::format( "T{}L{}Q{}_D{}_{}", to_unsigned( id.station() ), to_unsigned( id.layer() ),
                          to_unsigned( id.quarter() ), id.dataLink() + 1, id.port() );
    }

    /// Comparison equality
    constexpr friend bool operator==( const FTSourceID& lhs, const FTSourceID& rhs ) {
      return lhs.sourceID() == rhs.sourceID();
    }

    /// Comparison <
    friend bool operator<( const FTSourceID& lhs, const FTSourceID& rhs ) { return lhs.sourceID() < rhs.sourceID(); }

    /// Comparison >
    friend bool operator>( const FTSourceID& lhs, const FTSourceID& rhs ) { return rhs < lhs; }

    // << Operator
    friend std::ostream& operator<<( std::ostream& str, const FTSourceID& obj ) { return obj.fillStream( str ); }

    // Get the header
    constexpr int header() const { return extract<Mask::header>( m_sourceID ); }

    // Checks that the sourceID is indeed a source ID
    constexpr bool isIndeedFT() const {
      return ( header() == LHCb::Detector::FT::RawBank::leftHeader ||
               header() == LHCb::Detector::FT::RawBank::rightHeader );
    } // see RawBank in FTConstants

    /// Is the source from an X layer
    [[nodiscard]] constexpr bool isX() const { return is_X( layer() ); }

    /// Return true if sourceID is in bottom part of detector
    [[nodiscard]] constexpr bool isBottom() const { return is_bottom( quarter() ); }

    /// Retuitrn true if sourceID is in top part of detector
    [[nodiscard]] constexpr bool isTop() const { return is_top( quarter() ); }

    /// Return true if information in that source is supposed to be
    /// read from left to right
    [[nodiscard]] constexpr bool isFromLeftToRight() const {
      return ( ( to_unsigned( layer() ) % 2 == 0 && isTop() ) || ( to_unsigned( layer() ) % 2 == 1 && isBottom() ) );
    }

    /// Dumb help function
    static constexpr bool isReversedInDecoding( unsigned int iLayer, unsigned int iQuarter ) {
      return ( ( iLayer % 2 == 0 && ( iQuarter == 0 || iQuarter == 3 ) ) ||
               ( iLayer % 2 == 1 && ( iQuarter == 1 || iQuarter == 2 ) ) );
    }

    static constexpr bool isReversedInDecoding( unsigned int iUQuarter ) {
      return isReversedInDecoding( ( iUQuarter / FTConstants::nQuarters ) % FTConstants::nLayers,
                                   iUQuarter % FTConstants::nQuarters );
    }

    constexpr bool isReversedInDecoding() const {
      return isReversedInDecoding( to_unsigned( layer() ), to_unsigned( quarter() ) );
    }

    /// Print this FTSourceID in a human readable way
    std::ostream& fillStream( std::ostream& s ) const {
      return s << "{ FTSourceID : "
               << " port = " << port() << " dataLink =" << dataLink() << " quarter=" << to_unsigned( quarter() )
               << " layer=" << to_unsigned( layer() ) << " station=" << to_unsigned( station() ) << " }";
    }

    /// Retrieve const  FT Source ID
    [[nodiscard]] constexpr unsigned int sourceID() const { return m_sourceID; }

    /// Retrieve P (0-1)
    [[nodiscard]] constexpr unsigned int port() const { return extract<Mask::port>( m_sourceID ); }

    /// Retrieve D (0-2)
    [[nodiscard]] constexpr unsigned int dataLink() const { return extract<Mask::dataLink>( m_sourceID ); }

    /// Retrieve Quarter ID (0 - 3)
    [[nodiscard]] constexpr FTChannelID::QuarterID quarter() const {
      return FTChannelID::QuarterID{extract<Mask::quarter>( m_sourceID )};
    }

    /// Retrieve Layer id
    [[nodiscard]] constexpr FTChannelID::LayerID layer() const {
      return FTChannelID::LayerID{extract<Mask::layer>( m_sourceID )};
    }

    /// Retrieve Station id
    [[nodiscard]] constexpr FTChannelID::StationID station() const {
      return FTChannelID::StationID{extract<Mask::station>( m_sourceID )};
    }

    /// Retrieve unique layer
    [[nodiscard]] constexpr unsigned int uniqueLayer() const { return extract<Mask::uniqueLayer>( m_sourceID ); }

    /// Retrieve unique quarter
    [[nodiscard]] constexpr unsigned int uniqueQuarter() const { return extract<Mask::uniqueQuarter>( m_sourceID ); }

    /// Retrieve unique dataLink
    [[nodiscard]] constexpr unsigned int uniqueDataLink() const { return extract<Mask::uniqueDataLink>( m_sourceID ); }

    /// Retrieve Station id
    [[nodiscard]] constexpr unsigned int moniStation() const { return extract<Mask::station>( moniAddStation() ); }

    /// Retrieve unique layer
    [[nodiscard]] constexpr unsigned int uniqueMoniLayer() const {
      return extract<Mask::uniqueLayer>( moniAddStation() );
    }

    /// Retrieve unique quarter
    [[nodiscard]] constexpr unsigned int uniqueMoniQuarter() const {
      return extract<Mask::uniqueQuarter>( moniAddStation() );
    }

    /// Retrieve unique dataLink
    [[nodiscard]] constexpr unsigned int uniqueMoniDataLink() const {
      return extract<Mask::uniqueDataLink>( moniAddStation() );
    }

  }; // class FTSourceID

} // namespace LHCb::Detector
