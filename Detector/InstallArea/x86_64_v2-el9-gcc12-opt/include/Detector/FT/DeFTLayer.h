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
#include "Detector/FT/DeFTHalfLayer.h"
#include "Detector/FT/DeFTQuarter.h"

#include <array>
#include <cmath>
#include <functional>

namespace LHCb::Detector {

  namespace detail {

    /**
     *  Generic FT iov dependent detector element of a FT layer
     *  \author  Markus Frank
     *  \date    2018-03-08
     *  \version  1.0
     *
     *  modified by Zehua Xu for half layer construction
     */
    struct DeFTLayerObject : DeIOVObject {
      /// Reference to the static information of the quarters
      unsigned int        m_layerID{};     ///< layer ID number
      float               m_globalZ{};     ///< Global z position of layer closest to y-axis
      ROOT::Math::Plane3D m_plane{};       ///< xy-plane in the z-middle of the layer
      float               m_dzdy{};        ///< dz/dy of the layer (tan of the beam angle)
      float               m_stereoAngle{}; ///< stereo angle of the layer
      float               m_sizeX{dd4hep::_toFloat( "FT:StationSizeX" )}; ///< Size of the layer in x
      float               m_sizeY{dd4hep::_toFloat( "FT:StationSizeY" )}; ///< Size of the layer in y
      float               m_dxdy{}; ///< dx/dy of the layer (ie. tan(m_stereoAngle))

      std::array<DeFTHalfLayerObject, 2> m_halflayers;

      DeFTLayerObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt, unsigned int iLayer,
                       unsigned int stationID );

      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& halflayer : m_halflayers ) { func( &halflayer ); };
      }

      template <typename F>
      void applyToAllQuarters( F func ) const {
        for ( auto& halflayer : m_halflayers ) halflayer.applyToAllQuarters( func );
      }

      template <typename F>
      void applyToAllMats( F func ) const {
        for ( auto& halflayer : m_halflayers ) halflayer.applyToAllMats( func );
      }
    };
  } // End namespace detail

  struct DeFTLayer : DeIOVElement<detail::DeFTLayerObject> {
    using DeIOVElement::DeIOVElement;
    unsigned int layerID() const { return this->access()->m_layerID; }
    unsigned int layerIdx() const { return this->access()->m_layerID - FTConstants::layerShift; }

    [[nodiscard]] const std::optional<DeFTHalfLayer> findHalfLayer( const ROOT::Math::XYZPoint& aPoint ) const {
      const auto iHL = std::find_if(
          this->access()->m_halflayers.begin(), this->access()->m_halflayers.end(),
          [&aPoint]( const detail::DeFTHalfLayerObject& q ) { return DeFTHalfLayer{&q}.isInside( aPoint ); } );

      return iHL != this->access()->m_halflayers.end() ? iHL : std::optional<DeFTHalfLayer>{}; // DeFTQuarter{};
    }

    [[nodiscard]] const std::optional<DeFTQuarter> findQuarter( const ROOT::Math::XYZPoint& aPoint ) const {
      const auto iHL = findHalfLayer( aPoint );
      return iHL ? iHL->findQuarter( aPoint ) : std::optional<DeFTQuarter>{};
    }

    /// Find the module for a given XYZ point
    [[nodiscard]] const std::optional<DeFTModule> findModule( const ROOT::Math::XYZPoint& aPoint ) const {
      const auto iQ = findQuarter( aPoint );
      return iQ ? iQ->findModule( aPoint ) : std::optional<DeFTModule>{}; // DeFTModule{};
    }

    /** Const method to return the layer for a given channel id
     * @param  aChannel an FT channel id
     * @return pointer to detector element
     */
    [[nodiscard]] const std::optional<DeFTHalfLayer> findHalfLayer( const FTChannelID& id ) const {
      auto this_halflayer = to_unsigned( id.quarter() ) % 2;

      return ( to_unsigned( id.quarter() ) < 2 * this->access()->m_halflayers.size() )
                 ? &( this->access()->m_halflayers[this_halflayer] )
                 : std::optional<DeFTHalfLayer>{};
    }

    [[nodiscard]] const std::optional<DeFTQuarter> findQuarter( const FTChannelID& id ) const {
      const auto iHL = findHalfLayer( id );
      return iHL ? iHL->findQuarter( id ) : std::optional<DeFTQuarter>{};
    }

    /// Find a half layer from global X and Y coordinates
    const std::optional<DeFTHalfLayer> findHalfLayerWithXY( const ROOT::Math::XYZPoint& globalPoint ) const {
      const auto iter = std::find_if( this->access()->m_halflayers.begin(), this->access()->m_halflayers.end(),
                                      [&globalPoint]( const detail::DeFTHalfLayerObject& hl ) {
                                        auto localPoint = hl.toLocal( globalPoint );
                                        return hl.isInside(
                                            hl.toGlobal( ROOT::Math::XYZPoint( localPoint.x(), localPoint.y(), 0. ) ) );
                                      } );
      return iter != this->access()->m_halflayers.end() ? DeFTHalfLayer{&*iter} : std::optional<DeFTHalfLayer>{};
    }

    /// Find a quarter from global X and Y coordinates
    const std::optional<DeFTQuarter> findQuarterWithXY( const ROOT::Math::XYZPoint& globalPoint ) const {
      const auto hl = findHalfLayerWithXY( globalPoint );
      return hl ? hl->findQuarterWithXY( globalPoint ) : std::optional<DeFTQuarter>{};
    }

    /// Find a module from global X and Y coordinates
    const std::optional<DeFTModule> findModuleWithXY( const ROOT::Math::XYZPoint& globalPoint ) const {
      const auto q = findQuarterWithXY( globalPoint );
      return q ? q->findModuleWithXY( globalPoint ) : std::optional<DeFTModule>{};
    }

    /// Find a mat from global X and Y coordinates
    const std::optional<DeFTMat> findMatWithXY( const ROOT::Math::XYZPoint& globalPoint ) const {
      const auto m = findModuleWithXY( globalPoint );
      return m ? m->findMatWithXY( globalPoint ) : std::optional<DeFTMat>{};
    }

    float               globalZ() const { return this->access()->m_globalZ; }
    ROOT::Math::Plane3D plane() const { return this->access()->m_plane; }
    float               dxdy() const { return this->access()->m_dxdy; }
    float               stereoAngle() const { return this->access()->m_stereoAngle; }
    float               dzdy() const { return this->access()->m_dzdy; }
    float               sizeX() const { return this->access()->m_sizeX; }
    float               sizeY() const { return this->access()->m_sizeY; }
  };
} // End namespace LHCb::Detector
