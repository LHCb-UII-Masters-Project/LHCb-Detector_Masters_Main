/*****************************************************************************\
* (c) Copyright 2023 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once

#include <Detector/Muon/MuonConstants.h>
#include <Detector/Muon/Types.h>

#include <cassert>
#include <iosfwd>

namespace LHCb::Detector {

  /**
   * Converts a DD4hep volumeIDs to Muon parameters.
   * The bit pattern for the readout is defined in Muon/detector.xml
   *
   * In practice the rule is:
   * gapID     = ( ( hit >> 23 ) & 0x3 );
   * chamberID = ( ( hit >> 15 ) & 0xFF );
   * regionID  = ( ( hit >> 12 ) & 0x7 );
   * sideID    = ( ( hit >> 11 ) & 0x1 );
   * stationID = ( ( hit >> 8 ) & 0x7 );
   */

  class MuonVolumeID final {
    using StationId  = Muon::StationId;
    using SideId     = Muon::SideId;
    using RegionId   = Muon::RegionId;
    using ChamberId  = Muon::ChamberId;
    using GapId      = Muon::GapId;
    using QuadrantId = Muon::QuadrantId;

  public:
    // using: <id>system:8,Station:3,Side:1,Region:3,Chamber:8,GasGap:2,GasGapLayer:1</id>
    enum struct Mask : unsigned {
      system   = 0xff,
      station  = 0x700,
      side     = 0x800,
      region   = 0x7000,
      chamber  = 0x7F8000,
      gap      = 0x1800000,
      gaplayer = 0x2000000,
      error    = 0x80000000
    };

    template <Mask m>
    static constexpr unsigned int extract( unsigned int i ) {
      constexpr auto b =
          __builtin_ctz( static_cast<unsigned int>( m ) ); // FIXME: C++20 replace __builtin_ctz with std::countr_zero
      return ( i & static_cast<unsigned int>( m ) ) >> b;
    }

    template <Mask m>
    static constexpr unsigned int shift( unsigned int i ) {
      constexpr auto b =
          __builtin_ctz( static_cast<unsigned int>( m ) ); // FIXME: C++20 replace __builtin_ctz with std::countr_zero
      auto v = ( i << static_cast<unsigned int>( b ) );
      assert( extract<m>( v ) == i );
      return v;
    }

    template <Mask m, typename T>
    static constexpr unsigned int shift( T i ) {
      return shift<m>( static_cast<unsigned>( i ) );
    }

    template <Mask m, typename T>
    constexpr MuonVolumeID& set( T val ) {
      m_volumeID &= ~static_cast<unsigned int>( m );
      m_volumeID |= shift<m>( static_cast<unsigned>( val ) );
      return *this;
    }

    /// Default Constructor
    constexpr MuonVolumeID() = default;

    /// Constructor with volume ID
    constexpr explicit MuonVolumeID( unsigned int id ) : m_volumeID( id ) { assert( isValid() ); }

    /// Constructor with volume ID
    constexpr explicit MuonVolumeID( StationId stationID, SideId sideID, RegionId regionID, ChamberId chamberID,
                                     GapId gapID )
        : MuonVolumeID{
              shift<Mask::system>( m_systemID ) | shift<Mask::station>( stationID.value() ) |
              shift<Mask::side>( sideID.value() ) | shift<Mask::region>( regionID.value() ) |
              shift<Mask::chamber>( chamberID.value() ) | shift<Mask::gap>( gapID.value() ) |
              shift<Mask::gaplayer>( 1u ) // Not really used, but this is just to make sure the ID matches with DD4hep
          } {}

    constexpr static MuonVolumeID error() {
      return MuonVolumeID{shift<Mask::system>( m_systemID ) | shift<Mask::error>( 1 )};
    }

    /// Cast
    constexpr operator unsigned int() const { return m_volumeID; }

    /// Retrieve Volume ID
    constexpr unsigned int volumeID() const { return m_volumeID; }

    /// Get system volume ID
    constexpr unsigned int system() const { return extract<Mask::system>( m_volumeID ); }

    /// Get station volume ID
    constexpr StationId station() const { return StationId{extract<Mask::station>( m_volumeID )}; }

    /// Get side volume ID
    constexpr SideId side() const { return SideId{extract<Mask::side>( m_volumeID )}; }

    /// Get Region volume ID
    constexpr RegionId region() const { return RegionId{extract<Mask::region>( m_volumeID )}; }

    /// Get Chamber volume ID
    constexpr ChamberId chamber() const { return ChamberId{extract<Mask::chamber>( m_volumeID )}; }

    /// Get gap volume ID
    constexpr GapId gap() const { return GapId{extract<Mask::gap>( m_volumeID )}; }

    constexpr bool is_error() const { return extract<Mask::error>( m_volumeID ); }

    /// Derive the quandrant ID from the other data
    constexpr QuadrantId quadrant() const {
      // Get the numbers of chambers per station
      unsigned int nc = Muon::nChambers[region().value()];
      // Now deriving the quadrant
      if ( side().value() == 0u ) {
        // SideA
        if ( chamber().value() < ( nc / 2u ) ) {
          return QuadrantId( 0u );
        } else {
          return QuadrantId( 1u );
        }
      } else {
        // SideC
        if ( chamber().value() < ( nc / 2u ) ) {
          return QuadrantId( 3u );
        } else {
          return QuadrantId( 2u );
        }
      }
    }

    /// Check validity of ID
    constexpr bool isValid() const { return system() == m_systemID; }

    friend std::ostream& operator<<( std::ostream& str, const MuonVolumeID& obj );

  private:
    static constexpr unsigned int m_systemID{200}; ///< system ID
    unsigned int                  m_volumeID{0};   ///< the full DD4hep Volume ID

  }; // class MuonVolumeID

} // namespace LHCb::Detector
