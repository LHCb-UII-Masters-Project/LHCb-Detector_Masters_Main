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
#include "Detector/FT/DeFTLayer.h"

namespace LHCb::Detector {

  namespace detail {

    /**
     *  Generic FT iov dependent detector element of a FT station
     *  \author  Markus Frank
     *  \date    2018-03-08
     *  \version  1.0
     */
    struct DeFTStationObject : DeIOVObject {
      /// Reference to the static information of the layers
      unsigned int                             m_id;
      std::array<DeFTLayerObject, FT::nLayers> m_layers;
      DeFTStationObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt,
                         unsigned int iStation );

      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& layer : m_layers ) { func( &layer ); };
      }

      template <typename F>
      void applyToAllMats( F func ) const {
        for ( auto& layer : m_layers ) layer.applyToAllMats( func );
      }

      template <typename F>
      void applyToAllQuarters( F func ) const {
        for ( auto& layer : m_layers ) layer.applyToAllQuarters( func );
      }
    };
  } // End namespace detail

  struct DeFTStation : DeIOVElement<detail::DeFTStationObject> {
    using DeIOVElement::DeIOVElement;
    unsigned int stationID() const { return this->access()->m_id; }
    unsigned int stationIdx() const { return this->access()->m_id - FTConstants::stationShift; }

    /** Find the FT Layer corresponding to the point
     *  @return Pointer to the relevant layer
     */
    [[nodiscard]] const std::optional<DeFTLayer> findLayer( const ROOT::Math::XYZPoint& aPoint ) const {
      auto iter =
          std::find_if( this->access()->m_layers.begin(), this->access()->m_layers.end(),
                        [&aPoint]( const detail::DeFTLayerObject& l ) { return DeFTLayer{&l}.isInside( aPoint ); } );
      return iter != this->access()->m_layers.end() ? iter : std::optional<DeFTLayer>{}; // DeFTLayer{};
    }

    /** Const method to return the layer for a given channel id
     * @param  aChannel an FT channel id
     * @return pointer to detector element
     */
    [[nodiscard]] const std::optional<DeFTLayer> findLayer( const FTChannelID& id ) const {
      return ( to_unsigned( id.layer() ) < this->access()->m_layers.size() )
                 ? &( this->access()->m_layers[to_unsigned( id.layer() )] )
                 : std::optional<DeFTLayer>{};
    }

    /** Get the layer according to it's index
     * @param layerIndex Index of the layer [0,1,2,3]
     * @return Pointer to the layer
     */
    [[nodiscard]] const std::optional<DeFTLayer> getLayer( unsigned int layerIndex ) const {
      return ( layerIndex < this->access()->m_layers.size() ) ? &( this->access()->m_layers[layerIndex] )
                                                              : std::optional<DeFTLayer>{};
    }
  };
} // End namespace LHCb::Detector
