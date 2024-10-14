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

#include <cassert>
#include <iosfwd>

namespace LHCb::Detector {

  // system:8,sector:1,chip:4,module:5,face:1,stave:3,layer:2,side:1
  class UPVolumeID final {
    /// Bitmasks for bitfield volumeID
    enum struct Mask : unsigned {
      system = 0x0000FFFF, // 0000,0000,0000,0000,1111,1111,1111,1111
      chip   = 0x000F0000, // 0000,0000,0000,1111,0000,0000,0000,0000
      module = 0x01f00000, // 0000,0001,1111,0000,0000,0000,0000,0000
      face   = 0x02000000, // 0000,0010,0000,0000,0000,0000,0000,0000
      stave  = 0x1c000000, // 0001,1100,0000,0000,0000,0000,0000,0000
      layer  = 0x60000000, // 0110,0000,0000,0000,0000,0000,0000,0000
      side   = 0x80000000  // 1000,0000,0000,0000,0000,0000,0000,0000
    };

    template <Mask m>
    [[nodiscard]] static constexpr unsigned int extract( unsigned int i ) {
      constexpr auto b =
          __builtin_ctz( static_cast<unsigned int>( m ) ); // FIXME: C++20 replace __builtin_ctz with std::countr_zero
      return ( i & static_cast<unsigned int>( m ) ) >> b;
    }

  public:
    unsigned system() const { return extract<Mask::system>( m_volumeID ); }
    unsigned chip() const { return extract<Mask::chip>( m_volumeID ); }
    unsigned module() const { return extract<Mask::module>( m_volumeID ); }
    unsigned face() const { return extract<Mask::face>( m_volumeID ); }
    unsigned stave() const { return extract<Mask::stave>( m_volumeID ); }
    unsigned layer() const { return extract<Mask::layer>( m_volumeID ); }
    unsigned side() const { return extract<Mask::side>( m_volumeID ); }

    UPVolumeID( const unsigned int id ) : m_volumeID( id ) {}

  private:
    static constexpr unsigned int m_systemID{10}; ///< UP system ID
    unsigned int                  m_volumeID{0};  ///< the full DD4hep Volume ID

  }; // class UPVolumeID

} // namespace LHCb::Detector
