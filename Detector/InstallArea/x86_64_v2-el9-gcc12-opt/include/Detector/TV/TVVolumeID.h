/*****************************************************************************\
* (c) Copyright 2000-2023 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once

#include "Detector/TV/TVChannelID.h"
#include "Detector/TV/TVConstants.h"

#include <fmt/format.h>

#include <cassert>
#include <iosfwd>

namespace LHCb::Detector {

  /** @class TVVolumeID TVVolumeID.h
   *
   * This class converts from DD4hep volume IDs to TV sensor number etc.
   *
   * The volume IDs are a bit pattern defined in the TV detector.xml
   *
   * @author Thomas Latham
   *
   */

  class TVVolumeID final {
    /// Bitmasks for bitfield volumeID
    enum struct Mask : unsigned {
      system     = 0xff,    // 0000,0000,0000,0000,1111,1111
      motionside = 0x100,   // 0000,0000,0000,0001,0000,0000
      side       = 0x200,   // 0000,0000,0000,0010,0000,0000
      module     = 0x3fc00, // 0000,0011,1111,1100,0000,0000
      ladder     = 0x1c0000 // 0001,1100,0000,0000,0000,0000
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
    constexpr TVVolumeID& set( T val ) {
      m_volumeID &= ~static_cast<unsigned int>( m );
      m_volumeID |= shift<m>( to_unsigned( val ) );
      return *this;
    }

  public:
    enum struct SystemID : unsigned int {};
    [[nodiscard]] friend constexpr unsigned int to_unsigned( SystemID id ) { return static_cast<unsigned>( id ); }

    enum struct SideID : unsigned int {};
    [[nodiscard]] friend constexpr unsigned int to_unsigned( SideID id ) { return static_cast<unsigned>( id ); }

    enum struct ModuleID : unsigned int {};
    [[nodiscard]] friend constexpr unsigned int to_unsigned( ModuleID id ) { return static_cast<unsigned>( id ); }

    enum struct LadderID : unsigned int {};
    [[nodiscard]] friend constexpr unsigned int to_unsigned( LadderID id ) { return static_cast<unsigned>( id ); }

    /// Default Constructor
    constexpr TVVolumeID() = default;

    /// Constructor with volume ID
    constexpr explicit TVVolumeID( unsigned int id ) : m_volumeID{id} {
      if ( !isValid() ) { throw std::runtime_error{fmt::format( "Invalid TVVolumeID {0:#010X}", id )}; }
    }

    /// Constructor with module and ladder
    constexpr TVVolumeID( ModuleID module, LadderID ladder )
        : TVVolumeID{shift<Mask::system>( SystemID{m_systemID} ) |
                     shift<Mask::side>( SideID{!( to_unsigned( module ) % 2 )} ) |
                     shift<Mask::motionside>( SideID{!( to_unsigned( module ) % 2 )} ) | shift<Mask::module>( module ) |
                     shift<Mask::ladder>( ladder )} {}

    /// Constructor with sensor number
    constexpr TVVolumeID( TVChannelID::SensorID sensor )
        : TVVolumeID{ModuleID{to_unsigned( sensor ) / TV::NSensorsPerModule},
                     LadderID{to_unsigned( sensor ) % TV::NSensorsPerModule}} {}

    /// Cast
    constexpr operator unsigned int() const { return m_volumeID; }

    /// Retrieve const TV Volume ID
    [[nodiscard]] constexpr unsigned int volumeID() const { return m_volumeID; }

    /// Get system volume ID
    [[nodiscard]] constexpr SystemID system() const { return SystemID{extract<Mask::system>( m_volumeID )}; }

    /// Get side volume ID
    [[nodiscard]] constexpr SideID side() const { return SideID{extract<Mask::side>( m_volumeID )}; }

    /// Get motionside volume ID
    [[nodiscard]] constexpr SideID motionside() const { return SideID{extract<Mask::motionside>( m_volumeID )}; }

    /// Get module volume ID
    [[nodiscard]] constexpr ModuleID module() const { return ModuleID{extract<Mask::module>( m_volumeID )}; }

    /// Get ladder volume ID
    [[nodiscard]] constexpr LadderID ladder() const { return LadderID{extract<Mask::ladder>( m_volumeID )}; }

    /// Get TV sensor number
    [[nodiscard]] constexpr TVChannelID::SensorID sensor() const {
      return TVChannelID::SensorID{TV::NSensorsPerModule * to_unsigned( module() ) + to_unsigned( ladder() )};
    }

    /// Check validity of ID
    [[nodiscard]] constexpr bool isValid() const {
      if ( to_unsigned( system() ) != m_systemID ) { return false; }
      if ( to_unsigned( side() ) != to_unsigned( motionside() ) ) { return false; }
      if ( to_unsigned( module() ) % 2 + to_unsigned( side() ) != 1 ) { return false; }
      if ( to_unsigned( module() ) >= TV::NModules ) { return false; }
      if ( to_unsigned( ladder() ) >= TV::NSensorsPerModule ) { return false; }
      return true;
    }

    /// Update module number (and possibly side)
    TVVolumeID& setModule( ModuleID module ) {
      SideID newSide{!( to_unsigned( module ) % 2 )};
      set<Mask::side>( newSide );
      set<Mask::motionside>( newSide );
      return set<Mask::module>( module );
    }

    /// Update ladder number
    TVVolumeID& setLadder( LadderID ladder ) { return set<Mask::ladder>( ladder ); }

    /// Update sensor number
    TVVolumeID& setSensor( TVChannelID::SensorID sensor ) {
      ModuleID newModule{to_unsigned( sensor ) / TV::NSensorsPerModule};
      LadderID newLadder{to_unsigned( sensor ) % TV::NSensorsPerModule};
      setLadder( newLadder );
      return setModule( newModule );
    }

    friend std::ostream& operator<<( std::ostream& str, const TVVolumeID& obj );

  private:
    static constexpr unsigned int m_systemID{10}; ///< TV system ID
    unsigned int                  m_volumeID{0};  ///< the full DD4hep Volume ID

  }; // class TVVolumeID

} // namespace LHCb::Detector
