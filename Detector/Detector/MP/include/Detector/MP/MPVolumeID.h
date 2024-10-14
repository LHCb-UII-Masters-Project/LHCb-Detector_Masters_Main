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

#include "Detector/MP/MPChannelID.h"

#include <fmt/format.h>

#include <cassert>
#include <iosfwd>

namespace LHCb::Detector {

  class MPVolumeID final {
    /// Bitmasks for bitfield volumeID
    enum struct Mask : unsigned {
      system  = 0x00000FF, // 0000,0000,0000,0000,0000,1111,1111
      station = 0x0000F00, // 0000,0000,0000,0000,1111,0000,0000
      layer   = 0x001F000, // 0000,0000,0001,1111,0000,0000,0000
      half    = 0x0020000, // 0000,0000,0010,0000,0000,0000,0000
      module  = 0x0FC0000, // 0000,1111,1100,0000,0000,0000,0000
      mat     = 0xF000000  // 1111,0000,0000,0000,0000,0000,0000
    };

    template <Mask m>
    [[nodiscard]] static constexpr unsigned int extract( unsigned int i ) {
      constexpr auto b =
          __builtin_ctz( static_cast<unsigned int>( m ) ); // FIXME: C++20 replace __builtin_ctz with std::countr_zero
      return ( i & static_cast<unsigned int>( m ) ) >> b;
    }

  public:
    unsigned system() const { return extract<Mask::system>( m_volumeID ); }
    unsigned station() const { return extract<Mask::station>( m_volumeID ); }
    unsigned layer() const { return extract<Mask::layer>( m_volumeID ); }
    unsigned half() const { return extract<Mask::half>( m_volumeID ); }
    unsigned module() const { return extract<Mask::module>( m_volumeID ); }
    unsigned mat() const { return extract<Mask::mat>( m_volumeID ); }

    MPVolumeID( const unsigned int id ) : m_volumeID( id ) {}

  private:
    static constexpr unsigned int m_systemID{10}; ///< MP system ID
    unsigned int                  m_volumeID{0};  ///< the full DD4hep Volume ID

  }; // class MPVolumeID

} // namespace LHCb::Detector
