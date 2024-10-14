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

#include "Core/DeIOV.h"
#include "Detector/FT/DeFTMat.h"

#include <array>

namespace LHCb::Detector {

  namespace detail {

    /**
     *  Generic FT iov dependent detector element of type FT module
     *  \author  Markus Frank
     *  \date    2018-03-08
     *  \version  1.0
     */
    struct DeFTModuleObject : DeIOVObject {
      /// Reference to the static information of mats
      unsigned int                         m_id;
      int                                  m_nChannelsInModule{dd4hep::_toInt( "FT:nChannelsInModule" )};
      std::array<DeFTMatObject, FT::nMats> m_mats;
      ROOT::Math::Plane3D                  m_plane; ///< xy-plane in the z-middle of the module
      bool                                 m_reversed;
      FTChannelID                          m_elementID;
      DeFTModuleObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt, unsigned int iModule,
                        unsigned int quarterID );

      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& mat : m_mats ) { func( &mat ); };
      };
      FTChannelID::StationID stationID() const { return m_elementID.station(); }
      FTChannelID::ModuleID  moduleID() const { return FTChannelID::ModuleID{m_id}; }

      /// Get the pseudo-channel for a FTChannelID (useful in the monitoring)
      int channelInModule( const FTChannelID channelID ) const {
        return channelID.channelID() & ( m_nChannelsInModule - 1u );
      }

      /// Kept for retrocompatibility
      int pseudoChannel( const FTChannelID channelID ) const {
        return ( channelID.localChannelIdx_quarter() );
        //        int chanInModule = channelInModule( channelID );
        //        if ( m_reversed ) { chanInModule = m_nChannelsInModule - 1 - chanInModule; }
        //        return chanInModule + to_unsigned( moduleID() ) * m_nChannelsInModule;
      }

      FTChannelID channelFromPseudo( const int pseudoChannel ) const {
        int chanInModule = pseudoChannel & ( m_nChannelsInModule - 1u );
        if ( m_reversed ) { chanInModule = m_nChannelsInModule - 1 - chanInModule; }
        return FTChannelID( m_elementID + chanInModule );
      }
    };
  } // End namespace detail
  struct DeFTModule : DeIOVElement<detail::DeFTModuleObject> {
    using DeIOVElement::DeIOVElement;
    FTChannelID::ModuleID  moduleID() const { return this->access()->moduleID(); }
    FTChannelID::StationID stationID() const { return this->access()->stationID(); }

    /// Find the layer for a given XYZ point
    const std::optional<DeFTMat> findMat( const ROOT::Math::XYZPoint& aPoint ) const {
      /// Find the layer and return a pointer to the layer from XYZ point
      const auto iter =
          std::find_if( this->access()->m_mats.begin(), this->access()->m_mats.end(),
                        [&aPoint]( const detail::DeFTMatObject& m ) { return DeFTMat{&m}.isInside( aPoint ); } );
      return iter != this->access()->m_mats.end() ? DeFTMat{&*iter} : std::optional<DeFTMat>{}; // DeFTMat{};
    }

    const std::optional<DeFTMat> findMat( const FTChannelID& id ) const {
      return ( to_unsigned( id.mat() ) < this->access()->m_mats.size() )
                 ? &( this->access()->m_mats[to_unsigned( id.mat() )] )
                 : std::optional<DeFTMat>{};
    }

    /// Find a mat from global X and Y coordinates
    const std::optional<DeFTMat> findMatWithXY( const ROOT::Math::XYZPoint& globalPoint ) const {
      const auto iter =
          std::find_if( this->access()->m_mats.begin(), this->access()->m_mats.end(),
                        [&globalPoint]( const detail::DeFTMatObject& m ) {
                          auto localPoint = m.toLocal( globalPoint );
                          return m.isInside( m.toGlobal( ROOT::Math::XYZPoint( localPoint.x(), localPoint.y(), 0. ) ) );
                        } );
      return iter != this->access()->m_mats.end() ? iter : std::optional<DeFTMat>{};
    }

    [[nodiscard]] FTChannelID elementID() const { return this->access()->m_elementID; }

    [[nodiscard]] int pseudoChannel( const FTChannelID channelID ) const {
      return this->access()->pseudoChannel( channelID );
    }

    [[nodiscard]] FTChannelID channelFromPseudo( const int pseudoChannel ) const {
      const auto& obj             = this->access();
      int         channelInModule = pseudoChannel & ( obj->m_nChannelsInModule - 1u );
      if ( obj->m_reversed ) { channelInModule = obj->m_nChannelsInModule - 1 - channelInModule; }
      return FTChannelID( this->elementID() + channelInModule );
    }
    ROOT::Math::Plane3D plane() const { return this->access()->m_plane; }
  };
} // End namespace LHCb::Detector
