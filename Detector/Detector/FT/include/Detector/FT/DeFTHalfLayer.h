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
// Half layer object, authored zehua xu

#pragma once

#include "Core/DeIOV.h"
#include "Detector/FT/DeFTQuarter.h"

#include <array>
#include <functional>

namespace LHCb::Detector {

  namespace detail {

    /**
     *  Generic FT iov dependent detector element of a FT layer
     *  \author  Zehua Xu
     *  \date    2022-05-01
     *  \version  1.0
     */
    struct DeFTHalfLayerObject : DeIOVObject {
      // Reference to the static information of the quarters
      // LayerID does not mean the same thing as in DetDesc. Here it is a global layer ID.
      unsigned int                     m_halflayerID; ///< layer ID number
      float                            m_globalZ;     ///< Global z position of layer closest to y-axis
      ROOT::Math::Plane3D              m_plane;       ///< xy-plane in the z-middle of the layer
      float                            m_dzdy;        ///< dz/dy of the layer (tan of the beam angle)
      float                            m_stereoAngle; ///< stereo angle of the layer
      float                            m_sizeX = {dd4hep::_toFloat( "FT:StationSizeX" )}; ///< Size of the layer in x
      float                            m_sizeY = {dd4hep::_toFloat( "FT:StationSizeY" )}; ///< Size of the layer in y
      float                            m_dxdy; ///< dx/dy of the layer (ie. tan(m_stereoAngle))
      std::array<DeFTQuarterObject, 2> m_quarters;

      DeFTHalfLayerObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt,
                           unsigned int iHalfLayer, unsigned int layerID );

      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& quarter : m_quarters ) { func( &quarter ); };
      }

      template <typename F>
      void applyToAllQuarters( F func ) const {
        applyToAllChildren( func );
      }

      template <typename F>
      void applyToAllMats( F func ) const {
        for ( auto& quarter : m_quarters ) quarter.applyToAllMats( func );
      }

      using DeIOVObject::toGlobal;
      using DeIOVObject::toLocal;
    };
  } // End namespace detail

  struct DeFTHalfLayer : DeIOVElement<detail::DeFTHalfLayerObject> {
    using DeIOVElement::DeIOVElement;

    unsigned int halflayerID() const { return this->access()->m_halflayerID; }

    /** Find the FT Quarter corresponding to the point
     *  @return Pointer to the relevant quarter
     */
    [[nodiscard]] const std::optional<DeFTQuarter> findQuarter( const ROOT::Math::XYZPoint& aPoint ) const {
      const auto iQ = std::find_if(
          this->access()->m_quarters.begin(), this->access()->m_quarters.end(),
          [&aPoint]( const detail::DeFTQuarterObject& q ) { return DeFTQuarter{&q}.isInside( aPoint ); } );
      return iQ != this->access()->m_quarters.end() ? iQ : std::optional<DeFTQuarter>{};
    }

    /// Find the module for a given XYZ point
    [[nodiscard]] const std::optional<DeFTModule> findModule( const ROOT::Math::XYZPoint& aPoint ) const {
      const auto iQ = findQuarter( aPoint );
      return iQ ? iQ->findModule( aPoint ) : std::optional<DeFTModule>{};
    }

    /** Const method to return the layer for a given channel id
     * @param  aChannel an FT channel id
     * @return pointer to detector element
     */
    [[nodiscard]] const std::optional<DeFTQuarter> findQuarter( const FTChannelID& id ) const {
      auto this_quarter = to_unsigned( id.quarter() ) / 2;

      return ( this_quarter < this->access()->m_quarters.size() ) ? &( this->access()->m_quarters[this_quarter] )
                                                                  : std::optional<DeFTQuarter>{};
    }

    /// Find a quarter from global X and Y coordinates
    const std::optional<DeFTQuarter> findQuarterWithXY( const ROOT::Math::XYZPoint& globalPoint ) const {
      const auto iter =
          std::find_if( this->access()->m_quarters.begin(), this->access()->m_quarters.end(),
                        [&globalPoint]( const detail::DeFTQuarterObject& q ) {
                          auto localPoint = q.toLocal( globalPoint );
                          return q.isInside( q.toGlobal( ROOT::Math::XYZPoint( localPoint.x(), localPoint.y(), 0. ) ) );
                        } );
      return iter != this->access()->m_quarters.end() ? DeFTQuarter{&*iter} : std::optional<DeFTQuarter>{};
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
