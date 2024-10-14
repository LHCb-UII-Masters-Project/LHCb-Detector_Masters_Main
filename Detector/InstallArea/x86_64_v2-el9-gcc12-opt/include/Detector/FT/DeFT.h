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

#include "Detector/FT/DeFTStation.h"
#include "Detector/FT/FTChannelID.h"
#include "Detector/FT/FTConstants.h"

#include <functional>
namespace DeFTLocation {
  // FT location defined in the XmlDDDB
  inline const std::string Default = "/dd/Structure/LHCb/AfterMagnetRegion/T/FT";
} // namespace DeFTLocation

namespace LHCb::Detector {
  namespace detail {
    /**
     *  FT detector element data
     *  \author  Markus Frank
     *  \date    2018-11-08
     *  \version  1.0
     */
    struct DeFTObject : DeIOVObject {
      int m_version{dd4hep::_toInt( "FT:version" )};                     // FT Geometry Version
      int m_nModulesT1{dd4hep::_toInt( "FT:nModulesT1" )};               // Number of modules in T1
      int m_nModulesT2{dd4hep::_toInt( "FT:nModulesT2" )};               // Number of modules in T2
      int m_nModulesT3{dd4hep::_toInt( "FT:nModulesT3" )};               // Number of modules in T3
      int m_nStations{dd4hep::_toInt( "FT:nStations" )};                 // Number of stations
      int m_nLayers{dd4hep::_toInt( "FT:nLayers" )};                     // Number of layers per station
      int m_nQuarters{dd4hep::_toInt( "FT:nQuarters" )};                 // Number of quarters per layer
      int m_nChannelsInModule{dd4hep::_toInt( "FT:nChannelsInModule" )}; // Number of channels per SiPM
      int m_nTotQuarters = m_nStations * m_nLayers * m_nQuarters;
      int m_nTotModules  = m_nModulesT1 + m_nModulesT2 + m_nModulesT3;
      int m_nTotChannels = m_nTotModules * m_nChannelsInModule;

      std::array<DeFTStationObject, FTConstants::nStations> m_stations;

      DeFTObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );

      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& station : m_stations ) { func( &station ); };
      }

      template <typename F>
      void applyToAllLayers( F func ) const {
        for ( auto& station : m_stations ) { station.applyToAllChildren( func ); };
      }

      template <typename F>
      void applyToAllQuarters( F func ) const {
        for ( auto& station : m_stations ) { station.applyToAllQuarters( func ); };
      }

      template <typename F>
      void applyToAllMats( F func ) const {
        for ( auto& station : m_stations ) { station.applyToAllMats( func ); };
      }

      const DeFTMatObject& firstMat() const {
        return m_stations[0].m_layers[0].m_halflayers[0].m_quarters[0].m_modules[0].m_mats[0];
      };
    }; // End namespace detail
  }    // namespace detail
  /**
   *  FT interface
   *
   *  \author  Louis Henry
   *  \date    2021-09-21
   *  \version  1.0
   */
  struct DeFT : DeIOVElement<detail::DeFTObject> {
    using DeIOVElement::DeIOVElement;

    int                                                   version() const { return this->access()->m_version; };
    const std::array<DeFTStation, FTConstants::nStations> stations() const {
      const auto obj = this->access();
      return {&obj->m_stations[0], &obj->m_stations[1], &obj->m_stations[2]};
    };
    const std::optional<DeFTQuarter> quarter( unsigned int quarter ) const {
      const auto obj            = this->access();
      auto       quarter_id     = quarter / FTConstants::nQuarters;
      auto       this_halflayer = quarter_id % 2;
      auto       this_quarter   = quarter_id / 2;

      return std::optional<DeFTQuarter>{
          &obj->m_stations[quarter / FTConstants::nQuartersStation]
               .m_layers[( quarter % FTConstants::nQuartersStation ) / FTConstants::nLayers]
               .m_halflayers[this_halflayer]
               .m_quarters[this_quarter]};
    }
    template <typename F>
    void applyToAllLayers( F func ) const {
      this->access()->applyToAllLayers( func );
    }
    template <typename F>
    void applyToAllMats( F func ) const {
      this->access()->applyToAllMats( func );
    }
    template <typename F>
    void applyToAllQuarters( F func ) const {
      this->access()->applyToAllQuarters( func );
    }
    DeFTMat firstMat() const { return &( this->access()->firstMat() ); }

    int nStations() const { return this->access()->m_nStations; }
    int nChannels() const { return this->access()->m_nTotChannels; }

    /// Need to override this function since lvFT is now an assembly, so does not have its own solid
    bool isInside( const ROOT::Math::XYZPoint& globalPoint ) const override {
      return findStation( globalPoint ).has_value();
    }

    /** Find the FT Station corresponding to the point
     *  @return Pointer to the relevant station
     */
    [[nodiscard]] const std::optional<DeFTStation> findStation( const ROOT::Math::XYZPoint& aPoint ) const {
      const auto iS = std::find_if(
          std::begin( this->access()->m_stations ), std::end( this->access()->m_stations ),
          [&aPoint]( detail::DeFTStationObject const& s ) { return DeFTStation{&s}.isInside( aPoint ); } );
      return iS != this->access()->m_stations.end() ? std::optional<DeFTStation>{iS} : std::optional<DeFTStation>{};
    }

    /// Find the layer for a given XYZ point
    [[nodiscard]] const std::optional<DeFTLayer> findLayer( const ROOT::Math::XYZPoint& aPoint ) const {
      const auto s = findStation( aPoint );
      return s ? s->findLayer( aPoint ) : std::optional<DeFTLayer>{};
    }

    /// Find the quarter for a given XYZ point
    [[nodiscard]] const std::optional<DeFTQuarter> findQuarter( const ROOT::Math::XYZPoint& aPoint ) const {
      const auto l = findLayer( aPoint );
      return l ? l->findQuarter( aPoint ) : std::optional<DeFTQuarter>{};
    }

    /// Find the module for a given XYZ point
    [[nodiscard]] const std::optional<DeFTModule> findModule( const ROOT::Math::XYZPoint& aPoint ) const {
      const auto l = findLayer( aPoint ); // is faster than via DeFTQuarter
      return l ? l->findModule( aPoint ) : std::optional<DeFTModule>{};
    }

    /// Find the mat for a given XYZ point
    [[nodiscard]] const std::optional<DeFTMat> findMat( const ROOT::Math::XYZPoint& aPoint ) const {
      const auto m = findModule( aPoint );
      return m ? m->findMat( aPoint ) : std::optional<DeFTMat>{};
    }

    /** Find the FT Station corresponding to the channel id
     *  @return Pointer to the relevant station
     */
    [[nodiscard]] const std::optional<DeFTStation> findStation( const FTChannelID& aChannel ) const {
      return ( to_unsigned( aChannel.station() ) - 1 < this->access()->m_stations.size() )
                 ? &( this->access()->m_stations[to_unsigned( aChannel.station() ) - 1] )
                 : std::optional<DeFTStation>{};
    }

    /** Find the FT Layer corresponding to the channel id
     *  @return Pointer to the relevant layer
     */
    [[nodiscard]] const std::optional<DeFTLayer> findLayer( const FTChannelID& aChannel ) const {
      const auto s = findStation( aChannel );
      return s ? s->findLayer( aChannel ) : std::optional<DeFTLayer>{};
    }

    /** Find the FT Quarter corresponding to the channel id
     *  @return Pointer to the relevant quarter
     */
    [[nodiscard]] const std::optional<DeFTQuarter> findQuarter( const FTChannelID& aChannel ) const {
      const auto l = findLayer( aChannel );
      return l ? l->findQuarter( aChannel ) : std::optional<DeFTQuarter>{};
    }

    /** Find the FT Module corresponding to the channel id
     *  @return Pointer to the relevant module
     */
    [[nodiscard]] const std::optional<DeFTModule> findModule( const FTChannelID& aChannel ) const {
      const auto q = findQuarter( aChannel );
      return q ? q->findModule( aChannel ) : std::optional<DeFTModule>{};
    }

    /** Find the FT Mat corresponding to the channel id
     *  @return Pointer to the relevant module
     */
    [[nodiscard]] const std::optional<DeFTMat> findMat( const FTChannelID& aChannel ) const {
      const auto m = findModule( aChannel );
      return m ? m->findMat( aChannel ) : std::optional<DeFTMat>{};
    }

    /** Get the station according to it's index
     * @param stationIndex Index of the station [0,1,2]
     * @return Pointer to the station
     */
    [[nodiscard]] const std::optional<DeFTStation> getStation( unsigned int stationIndex ) const {
      return ( stationIndex < this->access()->m_stations.size() ) ? &( this->access()->m_stations[stationIndex] )
                                                                  : std::optional<DeFTStation>{};
    }

    int sensitiveVolumeID( const ROOT::Math::XYZPoint& point ) const override {
      const auto& mat = findMat( point );
      return mat ? mat->sensitiveVolumeID( point ) : -1;
    }

    /// Get a random FTChannelID (useful for the thermal noise, which is ~flat)
    FTChannelID getRandomChannelFromSeed( const float seed ) const {
      if ( seed < 0.f || seed > 1.f ) return FTChannelID::kInvalidChannel();
      const auto&  obj             = this->access();
      unsigned int flatChannel     = int( seed * obj->m_nTotChannels );
      unsigned int channelInModule = flatChannel & ( obj->m_nChannelsInModule - 1u );
      flatChannel /= obj->m_nChannelsInModule;
      unsigned int quarter = flatChannel & ( obj->m_nQuarters - 1u );
      flatChannel /= obj->m_nQuarters;
      unsigned int layer = flatChannel & ( obj->m_nLayers - 1u );
      flatChannel /= obj->m_nLayers;
      unsigned int station = 1;
      unsigned int module  = flatChannel;
      if ( flatChannel >= static_cast<unsigned int>( obj->m_nModulesT1 + obj->m_nModulesT2 ) ) {
        station = 3;
        module  = flatChannel - obj->m_nModulesT1 - obj->m_nModulesT2;
      } else if ( flatChannel >= static_cast<unsigned int>( obj->m_nModulesT1 ) ) {
        station = 2;
        module  = flatChannel - obj->m_nModulesT1;
      }
      return FTChannelID( FTChannelID::StationID{station}, FTChannelID::LayerID{layer}, FTChannelID::QuarterID{quarter},
                          FTChannelID::ModuleID{module}, channelInModule );
    }

    /// Get a random FTChannelID from a pseudoChannel (useful for the AP noise)
    /// A pseudoChannel should exactly be FTChannelID::localChannelIdx_quarter
    FTChannelID getRandomChannelFromPseudo( const int pseudoChannel, const float seed ) const {
      if ( seed < 0.f || seed > 1.f ) return FTChannelID::kInvalidChannel();
      const auto&  obj         = this->access();
      unsigned int flatQuarter = int( seed * obj->m_nTotQuarters );
      auto         quarter     = FTChannelID::QuarterID{flatQuarter & ( obj->m_nQuarters - 1u )};
      flatQuarter /= obj->m_nQuarters;
      auto layer = FTChannelID::LayerID{flatQuarter & ( obj->m_nLayers - 1u )};
      flatQuarter /= obj->m_nLayers;
      auto station = FTChannelID::StationID{static_cast<unsigned int>( flatQuarter & obj->m_stations.size() ) + 1u};

      auto        module = FTChannelID::ModuleID{static_cast<unsigned int>( pseudoChannel / obj->m_nChannelsInModule )};
      const auto& moduleDet = findModule( FTChannelID( station, layer, quarter, module, 0u ) );
      return moduleDet->channelFromPseudo( pseudoChannel & ( obj->m_nChannelsInModule - 1u ) );
    }
  };
} // End namespace LHCb::Detector
