/*****************************************************************************\
* (c) Copyright 2000-2021 CERN for the benefit of the LHCb Collaboration      *
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
#include <type_traits>

#include "UTConstants.h"

namespace LHCb::Detector::UT {

  /**
   * Channel ID for class for UT
   * @author A Beiter (based on code by M Needham, J. Wang)
   */
  class ChannelID final {

    /// Bitmasks for UT bitfield that are different from TT/IT
    enum struct Mask : unsigned {
      strip           = 0x1ffL,
      asic            = 0x180L,
      subsector       = 0x100L,
      sector          = 0x200L,
      module          = 0x1c00L,
      face            = 0x2000L,
      stave           = 0x3c000L,
      layer           = 0xc0000L,
      side            = 0x100000,
      type            = 0x600000,
      uniquelayer     = layer | side,
      uniquestave     = uniquelayer | stave,
      uniqueface      = uniquestave | face,
      uniquemodule    = uniqueface | module,
      uniquesector    = uniquemodule | sector,
      uniquesubsector = uniquesector | subsector,
      uniqueasic      = uniquesector | asic
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

  public:
    /// types of sub-detector channel ID
    enum detType { typeUT = 2 };

    /// Default Constructor
    constexpr ChannelID() = default;

    /// constructor with int
    constexpr explicit ChannelID( unsigned int id ) : m_channelID( id ) {}

    /// constructor with side, halflayer, stave, face, module, sector, strip. New style
    ChannelID( const unsigned int iType, const unsigned int iSide, const unsigned int iHalflayer,
               const unsigned int iStave, const unsigned int iFace, const unsigned int iModule,
               const unsigned int iSector, const unsigned int iStrip )
        : ChannelID{shift<Mask::type>( iType ) | shift<Mask::side>( iSide ) | shift<Mask::layer>( iHalflayer ) |
                    shift<Mask::stave>( iStave ) | shift<Mask::face>( iFace ) | shift<Mask::module>( iModule ) |
                    shift<Mask::sector>( iSector ) | shift<Mask::strip>( iStrip )} {}

    /// cast
    constexpr operator unsigned int() const { return m_channelID; }

    /// Retrieve type
    [[nodiscard]] constexpr unsigned int type() const { return extract<Mask::type>( m_channelID ); }

    /// test whether UT or not
    [[nodiscard]] constexpr bool isUT() const { return type() == ChannelID::detType::typeUT; }

    /// Retrieve sector
    [[nodiscard]] constexpr unsigned int sector() const { return extract<Mask::sector>( m_channelID ); }

    /// Retrieve subsector
    [[nodiscard]] constexpr unsigned int subSector() const { return extract<Mask::subsector>( m_channelID ); }

    /// Retrieve detRegion
    [[nodiscard]] constexpr unsigned int detRegion() const { return stave() < 2 ? 2 : side() == 0 ? 1 : 3; }

    /// Retrieve layer
    [[nodiscard]] constexpr unsigned int layer() const { return extract<Mask::layer>( m_channelID ); }

    /// Retrieve unique layer
    [[nodiscard]] constexpr unsigned int uniqueLayer() const { return extract<Mask::uniquelayer>( m_channelID ); }

    /// Trim to keep until sector part
    [[nodiscard]] constexpr unsigned int getFullSector() const {
      unsigned int m = (unsigned int)Mask::uniquesector | (unsigned int)Mask::type;
      return channelID() & m;
    }

    /// Trim to keep until subsector part
    [[nodiscard]] constexpr unsigned int getFullSubSector() const {
      unsigned int m = (unsigned int)Mask::uniquesubsector | (unsigned int)Mask::type;
      return channelID() & m;
    }

    /// Trim to keep until asic part
    [[nodiscard]] constexpr unsigned int getFullAsic() const {
      unsigned int m = (unsigned int)Mask::uniqueasic | (unsigned int)Mask::type;
      return channelID() & m;
    }

    /// Print method for python NOT NEEDED + SLOW IN C++ use operator<<
    [[nodiscard]] std::string toString() const;

    /// Retrieve const  UT Channel ID
    [[nodiscard]] constexpr unsigned int channelID() const { return m_channelID; }

    /// Update  UT Channel ID
    [[deprecated]] constexpr ChannelID& setChannelID( unsigned int value ) {
      m_channelID = value;
      return *this;
    }

    /// Retrieve strip
    [[nodiscard]] constexpr unsigned int strip() const { return extract<Mask::strip>( m_channelID ); }

    /// Retrive asic
    [[nodiscard]] constexpr unsigned int asic() const { return extract<Mask::asic>( m_channelID ); }

    /// Retrieve station
    [[nodiscard]] constexpr unsigned int station() const { return ( this->layer() / 2 + 1u ); }

    /// Retrieve unique sector
    [[nodiscard]] constexpr unsigned int uniqueSector() const { return extract<Mask::uniquesector>( m_channelID ); }

    /// Retrieve unique sub sector
    [[nodiscard]] constexpr unsigned int uniqueSubSector() const {
      return extract<Mask::uniquesubsector>( m_channelID );
    }

    /// Retrieve unique asic
    [[nodiscard]] constexpr unsigned int uniqueAsic() const { return extract<Mask::uniqueasic>( m_channelID ); }

    /// Retrieve side
    [[nodiscard]] constexpr unsigned int side() const { return extract<Mask::side>( m_channelID ); }

    /// Retrieve stave
    [[nodiscard]] constexpr unsigned int stave() const { return extract<Mask::stave>( m_channelID ); }

    /// Retrieve face
    [[nodiscard]] constexpr unsigned int face() const { return extract<Mask::face>( m_channelID ); }

    /// Retrieve module
    [[nodiscard]] constexpr unsigned int module() const { return extract<Mask::module>( m_channelID ); }

    /// Retrieve unique Stave
    [[nodiscard]] constexpr unsigned int uniqueStave() const { return extract<Mask::uniquestave>( m_channelID ); }

    /// Retrieve unique Face
    [[nodiscard]] constexpr unsigned int uniqueFace() const { return extract<Mask::uniqueface>( m_channelID ); }

    /// Retrieve unique Module
    [[nodiscard]] constexpr unsigned int uniqueModule() const { return extract<Mask::uniquemodule>( m_channelID ); }

    // Retrive the unique identifier used for hit-data indexing
    [[nodiscard]] unsigned int sectorFullID() const {
      return ( ( ( ( ( ( side() * nHalfLayers + layer() ) * nStavesPerLayer ) + stave() ) * nFaces + face() ) *
                 nModules ) +
               module() ) *
                 nSubSectors + // FIXME: sector is misnamed as subsector here
             sector();
    }

    friend std::ostream& operator<<( std::ostream& str, const ChannelID& obj );

  private:
    unsigned int m_channelID{0}; ///< UT Channel ID

  }; // class ChannelID

} // namespace LHCb::Detector::UT
