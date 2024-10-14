/***************************************************************************** \
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
#include "FTConstants.h"
#include <algorithm>
#include <cassert>
#include <fmt/format.h>
#include <ostream>
#include <vector>

namespace FTConstants = LHCb::Detector::FT;

namespace LHCb::Detector {

  /** @class FTChannelID FTChannelID.h
   *
   * Channel ID for the Fibre Tracker (LHCb Upgrade)
   *
   * @author FT software team
   *
   */

  class FTChannelID final {

    /// Bitmasks for bitfield channelID
    // FIXME: LHCBSCIFI-195
    enum struct Mask : unsigned {
      channel         = 0x7f,    // 00000000000001111111
      sipm            = 0x180,   // 00000000000110000000
      mat             = 0x600,   // 00000000011000000000
      module          = 0x3800,  // 00000011100000000000
      quarter         = 0xc000,  // 00001100000000000000
      layer           = 0x30000, // 00110000000000000000
      station         = 0xc0000, // 11000000000000000000
      uniqueLayer     = layer | station,
      uniqueQuarter   = quarter | uniqueLayer,
      uniqueModule    = module | uniqueQuarter,
      uniqueMat       = mat | uniqueModule,
      uniqueSiPM      = sipm | uniqueMat,
      die             = 0x40,
      sipmInModule    = mat | sipm,
      channelInModule = channel | sipmInModule
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
    [[nodiscard]] friend constexpr bool         is_X( LayerID id ) { return id == LayerID{0} || id == LayerID{3}; }

    enum struct QuarterID : unsigned int {};
    [[nodiscard]] friend constexpr unsigned int to_unsigned( QuarterID id ) { return static_cast<unsigned>( id ); }
    [[nodiscard]] friend constexpr unsigned int side( QuarterID id ) { return to_unsigned( id ) % 2; }

    [[nodiscard]] friend constexpr bool is_bottom( QuarterID id ) { return id == QuarterID{0} || id == QuarterID{1}; }
    [[nodiscard]] friend constexpr bool is_top( QuarterID id ) { return id == QuarterID{2} || id == QuarterID{3}; }
    [[nodiscard]] friend constexpr bool is_left( QuarterID id ) { return id == QuarterID{1} || id == QuarterID{3}; }
    [[nodiscard]] friend constexpr bool is_right( QuarterID id ) { return id == QuarterID{0} || id == QuarterID{2}; }
    [[nodiscard]] static constexpr bool is_bottom_from_qidx( unsigned int uQua ) {
      return is_bottom( QuarterID{uQua % 4} );
    }
    [[nodiscard]] static constexpr bool is_top_from_qidx( unsigned int uQua ) { return is_top( QuarterID{uQua % 4} ); }
    [[nodiscard]] static constexpr bool is_left_from_qidx( unsigned int uQua ) {
      return is_left( QuarterID{uQua % 4} );
    }
    [[nodiscard]] static constexpr bool is_right_from_qidx( unsigned int uQua ) {
      return is_right( QuarterID{uQua % 4} );
    }

    enum struct ModuleID : unsigned int {};
    [[nodiscard]] friend constexpr unsigned int to_unsigned( ModuleID id ) { return static_cast<unsigned>( id ); }

    enum struct MatID : unsigned int {};
    [[nodiscard]] friend constexpr unsigned int to_unsigned( MatID id ) { return static_cast<unsigned>( id ); }

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
    static constexpr auto to_quarterID = to_ID_t<QuarterID>{};
    static constexpr auto to_moduleID  = to_ID_t<ModuleID>{};
    static constexpr auto to_matID     = to_ID_t<MatID>{};

    /// Default Constructor
    constexpr FTChannelID() = default;

    /// Partial constructor using the unique sipm and the channel
    constexpr FTChannelID( unsigned int uniqueSiPM, unsigned int channel )
        : m_channelID{( shift<Mask::uniqueSiPM>( uniqueSiPM ) ) + ( shift<Mask::channel>( channel ) )} {};

    /// Constructor from int
    constexpr explicit FTChannelID( unsigned int id ) : m_channelID{id} {}

    /// Explicit constructor from the geometrical location
    constexpr FTChannelID( StationID station, LayerID layer, QuarterID quarter, ModuleID module, MatID mat,
                           unsigned int sipm, unsigned int channel )
        : FTChannelID{shift<Mask::station>( station ) | shift<Mask::layer>( layer ) | shift<Mask::quarter>( quarter ) |
                      shift<Mask::module>( module ) | shift<Mask::mat>( mat ) | shift<Mask::sipm>( sipm ) |
                      shift<Mask::channel>( channel )} {}

    /// Explicit constructor from the geometrical location
    constexpr FTChannelID( StationID station, LayerID layer, QuarterID quarter, ModuleID module,
                           unsigned int channelInModule )
        : FTChannelID{shift<Mask::station>( station ) | shift<Mask::layer>( layer ) | shift<Mask::quarter>( quarter ) |
                      shift<Mask::module>( module ) | shift<Mask::channelInModule>( channelInModule )} {}

    // Explicit constructor from the hardware IDs
    constexpr FTChannelID( StationID station, LayerID layer, QuarterID quarter, ModuleID module, unsigned int halfRob,
                           unsigned int dataLink, unsigned int channel )
        : FTChannelID{shift<Mask::station>( station ) | shift<Mask::layer>( layer ) | shift<Mask::quarter>( quarter ) |
                      shift<Mask::module>( module ) | shift<Mask::mat>( 2 * halfRob + ( dataLink / 4 ) ) |
                      shift<Mask::sipm>( dataLink % 4 ) | shift<Mask::channel>( channel )} {}

    /// Operator overload, to cast channel ID to unsigned int. Used by linkers where the key (channel id) is an int
    constexpr operator unsigned int() const { return m_channelID; }

    /// Comparison equality
    constexpr friend bool operator==( FTChannelID lhs, FTChannelID rhs ) { return lhs.channelID() == rhs.channelID(); }

    /// Comparison <
    constexpr friend bool operator<( FTChannelID lhs, FTChannelID rhs ) { return lhs.channelID() < rhs.channelID(); }

    /// Comparison >
    constexpr friend bool operator>( FTChannelID lhs, FTChannelID rhs ) { return rhs < lhs; }

    /// Useful for decoding: maximum number of SiPMs
    static unsigned int maxNumberOfSiPMs();

    // FTChannelID with channelID set to kInvalid bits
    static constexpr FTChannelID kInvalidChannel() {
      return {StationID{0u}, LayerID{0u}, QuarterID{0u}, ModuleID{7u}, MatID{0u}, 0, 0};
    };

    // FTChannelID with channelID set to kInvalid bits
    static constexpr unsigned int kInvalidChannelID() { return kInvalidChannel().channelID(); };

    /// Increment the channelID
    constexpr FTChannelID& advance() {
      ++m_channelID;
      return *this;
    }

    /// Return the SiPM number within the module (0-15)
    [[nodiscard]] constexpr unsigned int sipmInModule() const { return extract<Mask::sipmInModule>( m_channelID ); }

    /// Return the die number (0 or 1)
    [[nodiscard]] constexpr unsigned int die() const { return extract<Mask::die>( m_channelID ); }

    /// Return true if channelID is in x-layer
    [[nodiscard]] constexpr bool isX() const { return is_X( layer() ); }

    /// Return true if channelID is in bottom part of detector
    [[nodiscard]] constexpr bool isBottom() const { return is_bottom( quarter() ); }

    /// Return true if channelID is in top part of detector
    [[nodiscard]] constexpr bool isTop() const { return is_top( quarter() ); }

    /// Return true if channelID is in left part of detector
    [[nodiscard]] constexpr bool isLeft() const { return is_left( quarter() ); }

    /// Return true if channelID is in right part of detector
    [[nodiscard]] constexpr bool isRight() const { return is_right( quarter() ); }

    /// Return true if channelID is in right part of detector
    [[nodiscard]] constexpr unsigned int getSide() const { return side( quarter() ); }

    // Return true if the readout direction is in the same direction as LHCb global x
    [[nodiscard]] constexpr bool normalReadoutDir() const {
      return ( to_unsigned( layer() ) % 2 == 0 ) ? isBottom() : isTop();
    }

    /// Return true if the readout is installed from centre to edge
    [[nodiscard]] constexpr bool readoutFromCtoE() const {
      return ( to_unsigned( layer() ) % 2 == 0 ) ? ( to_unsigned( quarter() ) == 1 || to_unsigned( quarter() ) == 2 )
                                                 : ( to_unsigned( quarter() ) == 0 || to_unsigned( quarter() ) == 3 );
    }

    /// Retrieve const  FT Channel ID
    [[nodiscard]] constexpr unsigned int channelID() const { return m_channelID; }

    /// Retrieve Channel in the 128 channel SiPM
    [[nodiscard]] constexpr unsigned int channel() const { return extract<Mask::channel>( m_channelID ); }

    /// Retrieve ID of the SiPM in the mat
    [[nodiscard]] constexpr unsigned int sipm() const { return extract<Mask::sipm>( m_channelID ); }

    /// Retrieve ID of the mat in the module
    [[nodiscard]] constexpr MatID mat() const { return MatID{extract<Mask::mat>( m_channelID )}; }

    /// Retrieve Module id (0 - 5 or 0 - 6)
    [[nodiscard]] constexpr ModuleID module() const { return ModuleID{extract<Mask::module>( m_channelID )}; }

    /// Retrieve Quarter ID (0 - 3)
    [[nodiscard]] constexpr QuarterID quarter() const { return QuarterID{extract<Mask::quarter>( m_channelID )}; }

    /// Retrieve Layer id
    [[nodiscard]] constexpr LayerID layer() const { return LayerID{extract<Mask::layer>( m_channelID )}; }

    /// Retrieve Station id
    [[nodiscard]] constexpr StationID station() const { return StationID{extract<Mask::station>( m_channelID )}; }

    /// Retrieve HalfRob ID within module [0, 1]
    [[nodiscard]] constexpr unsigned int halfRob() const { return to_unsigned( mat() ) / FTConstants::nHalfRobs; }

    /// Retrieve data link ID within HalfRob [0..7]
    [[nodiscard]] constexpr unsigned int dataLink() const { return ( FTConstants::nSiPM * halfRob() + sipm() ); }

    /// Retrieve ClusterBoard ID within HalfRob [0..3]
    [[nodiscard]] constexpr unsigned int clusterBoard() const {
      return dataLink() / FTConstants::nLinksPerClusterBoard;
    }

    /// Retrieve PacificBoard ID within HalfRob [0..3]
    [[nodiscard]] constexpr unsigned int pacificBoard() const { return clusterBoard(); }

    /// Retrieve Pacific ID within PacificBoard [0, 1]
    [[nodiscard]] constexpr unsigned int pacific() const {
      return ( channel() / FTConstants::nChannelsInDie ) + 2 * ( dataLink() % 2 );
    }

    /// Retrieve Pacific channel ID within Pacific [0..63]
    [[nodiscard]] constexpr unsigned int pacificChannel() const {
      auto ch = channel() % FTConstants::nChannelsInDie;
      return ( FTConstants::nChannelsInDie / 2 ) * ( 1 - ch % 2 ) + ( ch / 2 );
    }

    /// Retrieve how many modules there are in that quarter
    [[nodiscard]] constexpr unsigned int nModulesInQuarter() const { return ( to_unsigned( station() ) == 3 ) ? 6 : 5; }

    /// Retrieve unique layer
    [[nodiscard]] constexpr unsigned int globalLayerID() const { return extract<Mask::uniqueLayer>( m_channelID ); }

    /// Retrieve unique quarter
    [[nodiscard]] constexpr unsigned int globalQuarterID() const { return extract<Mask::uniqueQuarter>( m_channelID ); }

    /// Retrieve unique module
    [[nodiscard]] constexpr unsigned int globalModuleID() const { return extract<Mask::uniqueModule>( m_channelID ); }

    /// Retrieve unique mat
    [[nodiscard]] constexpr unsigned int globalMatID() const { return extract<Mask::uniqueMat>( m_channelID ); }

    /// Retrieve matID for Allen dumping
    // FIX ME
    [[nodiscard]] constexpr unsigned int globalMatID_shift() const {
      /// Five modules per quarter in station 1 and 2 (IDs 0 to 159)
      return globalMatID() - 512;
    }

    /// Retrieve unique SiPM
    [[nodiscard]] constexpr unsigned int globalSiPMID() const { return extract<Mask::uniqueSiPM>( m_channelID ); }

    /// Global "Idx" functions. Useful for indexing and histogramming

    /// Retrieve global station index [0..2]
    [[nodiscard]] constexpr unsigned int globalStationIdx() const {
      assert( to_unsigned( station() ) != 0 &&
              "Trying to get the station idx of a station 0! This typically happens when having a kInvalidChannel." );
      return to_unsigned( station() ) - 1;
    }

    /// Retrieve global layer index [0..11]
    [[nodiscard]] constexpr unsigned int globalLayerIdx() const {
      return ( FTConstants::nLayers * globalStationIdx() + to_unsigned( layer() ) );
    }

    /// Retrieve local layer Index
    [[nodiscard]] constexpr unsigned int localLayerIdx() const { return to_unsigned( layer() ); }

    /// Retrieve global quarter index [0..47]
    [[nodiscard]] constexpr unsigned int globalQuarterIdx() const {
      return ( FTConstants::nQuarters * globalLayerIdx() + to_unsigned( quarter() ) );
    }

    /// Retrieve local quarter Index
    [[nodiscard]] constexpr unsigned int localQuarterIdx() const { return to_unsigned( quarter() ); }

    /// Retrieve global module index [0..255]
    [[nodiscard]] constexpr unsigned int globalModuleIdx() const {
      /// Five modules per quarter in station 1 and 2 (IDs 0 to 159)
      unsigned int iModule = to_unsigned( module() );
      if ( to_unsigned( station() ) < 3 ) {
        return 5 * globalQuarterIdx() + iModule;
      }
      /// Six modules per quarter in station 3 (IDs 160 to 255)
      else {
        return 5 * globalQuarterIdx() + FTConstants::nQuarters * ( to_unsigned( layer() ) ) + to_unsigned( quarter() ) +
               iModule;
      }
    }

    /// Retrieve global mat index [0..1023]
    [[nodiscard]] constexpr unsigned int globalMatIdx() const {
      return FTConstants::nMats * globalModuleIdx() + localMatIdx();
    }

    /// Retrieve global sipm index [4095]
    /// Center-to-edge convention
    [[nodiscard]] constexpr unsigned int globalSipmIdx() const {
      return FTConstants::nSiPM * globalMatIdx() + localSiPMIdx();
    }

    /// Local idx functions. Starting at 0, centre-to-edge, contiguous
    /// Returns the local index of the module ordered from left to right
    [[nodiscard]] constexpr unsigned int localModuleIdx_Xorder() const {
      return ( is_left( quarter() ) ) ? ( nModulesInQuarter() - 1 - to_unsigned( module() ) ) : to_unsigned( module() );
    }
    [[nodiscard]] constexpr unsigned int localModuleIdx() const { return to_unsigned( module() ); }

    [[nodiscard]] constexpr unsigned int localModuleIdx_station() const {
      return nModulesInQuarter() * ( FTConstants::nQuarters * to_unsigned( layer() ) + to_unsigned( quarter() ) ) +
             localModuleIdx();
    }
    // Center-to-edge convention
    [[nodiscard]] constexpr unsigned int localChannelIdx() const {
      return ( readoutFromCtoE() ) ? channel() : FTConstants::nChannels - 1 - channel();
    }

    // Center-to-edge convention
    [[nodiscard]] constexpr unsigned int localSiPMIdx() const {
      return ( readoutFromCtoE() ) ? sipm() : FTConstants::nSiPM - 1 - sipm();
    }

    // Center-to-edge convention
    [[nodiscard]] constexpr unsigned int localMatIdx() const {
      return ( readoutFromCtoE() ) ? to_unsigned( mat() ) : FTConstants::nMats - 1 - to_unsigned( mat() );
    }

    [[nodiscard]] constexpr unsigned int localMatIdx_quarter() const {
      return localMatIdx() + FTConstants::nMats * localModuleIdx();
    }

    /// Retrieve SiPMID for monitoring (continuous)
    [[nodiscard]] constexpr unsigned int localSiPMIdx_quarter() const {
      return localSiPMIdx() + FTConstants::nSiPM * localMatIdx_quarter();
    }

    /// Retrieve channelID for monitoring
    [[nodiscard]] constexpr unsigned int localChannelIdx_module() const {
      return localChannelIdx() + FTConstants::nChannels * ( localSiPMIdx() + FTConstants::nSiPM * localMatIdx() );
    }

    /// Retrieve channelID for monitoring
    /// This should exactly be the pseudochannel
    [[nodiscard]] constexpr unsigned int localChannelIdx_quarter() const {
      return localChannelIdx_module() +
             FTConstants::nChannels * FTConstants::nSiPM * FTConstants::nMats * localModuleIdx();
    }

    friend std::ostream& operator<<( std::ostream& s, const FTChannelID& obj ) {
      return s << "{ FTChannelID : "
               << " channel =" << obj.channel() << " sipm =" << obj.sipm() << " mat =" << to_unsigned( obj.mat() )
               << " module=" << to_unsigned( obj.module() ) << " quarter=" << to_unsigned( obj.quarter() )
               << " layer=" << to_unsigned( obj.layer() ) << " station=" << to_unsigned( obj.station() ) << " }";
    }

    /// Operator overload, to cast channel ID to std::string
    friend std::string toString( const FTChannelID& id ) {
      return fmt::format( "T{}L{}Q{}M{}M{}S{}C{}", to_unsigned( id.station() ), to_unsigned( id.layer() ),
                          to_unsigned( id.quarter() ), to_unsigned( id.module() ), to_unsigned( id.mat() ), id.sipm(),
                          id.channel() );
    }

    /// Operator overload, to cast channel ID to std::string
    std::string toString() const {
      return fmt::format( "T{}L{}Q{}M{}M{}S{}C{}", to_unsigned( station() ), to_unsigned( layer() ),
                          to_unsigned( quarter() ), to_unsigned( module() ), to_unsigned( mat() ), sipm(), channel() );
    }

  private:
    unsigned int m_channelID{0}; ///< FT Channel ID

  }; // class FTChannelID

} // namespace LHCb::Detector

// -----------------------------------------------------------------------------
// end of class
// -----------------------------------------------------------------------------
