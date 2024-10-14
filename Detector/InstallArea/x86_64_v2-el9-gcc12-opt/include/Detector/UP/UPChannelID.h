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

namespace LHCb::Detector {

  /**
   * Channel ID for class for UP
   * @author A Beiter (based on code by M Needham, J. Wang)
   */
  class ChannelID final {

    /// Bitmasks for UP bitfield that are different from TT/IT
    enum struct Mask : unsigned {
      pixel = 0xffff, // FIXME: maxstrip seems to be 512, so why not 0x1ff, but then what is the meaning of the bit
                      // corresponding to 0x200 ?
      chip         = 0xf0000,
      module       = 0x1f00000,
      face         = 0x2000000,
      stave        = 0x1c000000,
      layer        = 0x60000000,
      side         = 0x80000000,
      uniqueLayer  = side | layer,
      uniqueStave  = uniqueLayer | stave,
      uniqueFace   = uniqueStave | face,
      uniqueModule = uniqueFace | module,
      uniqueChip   = uniqueModule | chip
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
    enum detType { typeUP = 2 };

    /// Default Constructor
    constexpr ChannelID() = default;

    /// constructor with int
    constexpr explicit ChannelID( unsigned int id ) : m_channelID( id ) {}

    /// constructor with station, layer, detRegion, sector , strip,
    ChannelID( const unsigned int iSide, const unsigned int iLayer, const unsigned int iStave, const unsigned int iFace,
               const unsigned int iModule, const unsigned int iChip, const unsigned int iPixel )
        : ChannelID{shift<Mask::side>( iSide ) | shift<Mask::layer>( iLayer ) | shift<Mask::stave>( iStave ) |
                    shift<Mask::face>( iFace ) | shift<Mask::module>( iModule ) | shift<Mask::chip>( iChip ) |
                    shift<Mask::pixel>( iPixel )} {} // Do we need IPixel?

    ChannelID( const unsigned int iSide, const unsigned int iLayer, const unsigned int iStave, const unsigned int iFace,
               const unsigned int iModule, const unsigned int iChip )
        : ChannelID{shift<Mask::side>( iSide ) | shift<Mask::layer>( iLayer ) | shift<Mask::stave>( iStave ) |
                    shift<Mask::face>( iFace ) | shift<Mask::module>( iModule ) | shift<Mask::chip>( iChip )} {}

    /// cast
    constexpr operator unsigned int() const { return m_channelID; }

    /// Retrieve type
    [[nodiscard]] constexpr unsigned int type() const { return ChannelID::detType::typeUP; }

    /// test whether UP or not
    [[nodiscard]] constexpr bool isUP() const { return type() == ChannelID::detType::typeUP; } // always should be UP

    // For Mingjie: with the similar method, we need the interface like pixel(), chip() ... side(), station()...
    // Also for uniqueLayer...
    // For DetRegion, we define the most inside four staves are in Region 1, and -x -> Region 0 and +x->Region 2
    /// Retrieve sector
    [[nodiscard]] constexpr unsigned int pixel() const { return extract<Mask::pixel>( m_channelID ); }
    [[nodiscard]] constexpr unsigned int strip() const { return pixel(); } // Need change name from "strip" to "pixel"?
    [[nodiscard]] constexpr unsigned int chip() const { return extract<Mask::chip>( m_channelID ); }
    [[nodiscard]] constexpr unsigned int module() const { return extract<Mask::module>( m_channelID ); }
    [[nodiscard]] constexpr unsigned int sector() const { return chip(); } // Need change name from "sector" to "chip"?
    [[nodiscard]] constexpr unsigned int face() const { return extract<Mask::face>( m_channelID ); }
    [[nodiscard]] constexpr unsigned int stave() const { return extract<Mask::stave>( m_channelID ); }
    [[nodiscard]] constexpr unsigned int layer() const { return extract<Mask::layer>( m_channelID ); }
    [[nodiscard]] constexpr unsigned int side() const { return extract<Mask::side>( m_channelID ); }
    [[nodiscard]] constexpr unsigned int station() const { return ( layer() / 2 == 0 ) ? 0 : 1; }

    /// Retrieve detRegion
    [[nodiscard]] constexpr unsigned int detRegion() const { return stave() < 2 ? 2 : side() == 0 ? 1 : 3; }

    /// Retrieve unique layer
    [[nodiscard]] constexpr unsigned int uniqueLayer() const { return extract<Mask::uniqueLayer>( m_channelID ); }
    /// Retrieve unique stave
    [[nodiscard]] constexpr unsigned int uniqueStave() const { return extract<Mask::uniqueStave>( m_channelID ); }
    /// Retrieve unique face
    [[nodiscard]] constexpr unsigned int uniqueFace() const { return extract<Mask::uniqueFace>( m_channelID ); }
    /// Retrieve unique module
    [[nodiscard]] constexpr unsigned int uniqueModule() const { return extract<Mask::uniqueModule>( m_channelID ); }
    /// Retrieve unique chip
    [[nodiscard]] constexpr unsigned int uniqueChip() const { return extract<Mask::uniqueChip>( m_channelID ); }

    /// Retrieve unique sector
    [[nodiscard]] constexpr unsigned int uniqueSector() const { return uniqueChip(); }

    /// Retrieve unique detRegion
    [[nodiscard]] constexpr unsigned int uniqueDetRegion() const { return layer() << 0x3 | detRegion(); }

    /// Print method for python NOT NEEDED + SLOW IN C++ use operator<<
    [[nodiscard]] std::string toString() const;

    /// Retrieve const  UP Channel ID
    [[nodiscard]] constexpr unsigned int channelID() const { return m_channelID; }

    /// Update  UP Channel ID
    [[deprecated]] constexpr ChannelID& setChannelID( unsigned int value ) {
      m_channelID = value;
      return *this;
    }

    /// Retrieve strip
    //[[nodiscard]] constexpr unsigned int strip() const { return pixel(); }

    friend std::ostream& operator<<( std::ostream& str, const ChannelID& obj );

  private:
    unsigned int m_channelID{0}; ///< UP Channel ID

  }; // class ChannelID

} // namespace LHCb::Detector
