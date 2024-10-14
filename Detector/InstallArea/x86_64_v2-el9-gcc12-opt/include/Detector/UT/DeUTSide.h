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
#include "Detector/UT/DeUTLayer.h"

#include <array>
#include <vector>

namespace LHCb::Detector::UT {

  namespace detail {

    struct DeUTSideObject : LHCb::Detector::detail::DeIOVObject {
      const unsigned int             m_id;
      ChannelID                      m_channelID;
      std::array<DeUTLayerObject, 4> m_layers;
      DeUTSideObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );

      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& layer : m_layers ) { func( &layer ); };
      }

      void applyToAllLayers( const std::function<void( DeUTLayer )>& func ) const { applyToAllChildren( func ); }

      void applyToAllStaves( const std::function<void( DeUTStave )>& func ) const {
        for ( auto& layer : m_layers ) layer.applyToAllStaves( func );
      }

      void applyToAllFaces( const std::function<void( DeUTFace )>& func ) const {
        for ( auto& layer : m_layers ) layer.applyToAllFaces( func );
      }

      void applyToAllModules( const std::function<void( DeUTModule )>& func ) const {
        for ( auto& layer : m_layers ) layer.applyToAllModules( func );
      }

      void applyToAllSectors( const std::function<void( DeUTSector )>& func ) const {
        for ( auto& layer : m_layers ) layer.applyToAllSectors( func );
      }

      /// returns nb of active sectors and total number of sectors, in this order
      std::pair<double, unsigned int> countSectors() {
        std::pair<double, unsigned int> ret{0, 0};
        auto& [active, total] = ret;
        for ( auto& layer : m_layers ) {
          auto [a, t] = layer.countSectors();
          active += a;
          total += t;
        }
        return ret;
      }
    };
  } // End namespace detail

  struct DeUTSide : public DeIOVElement<detail::DeUTSideObject> {

    using DeIOVElement::DeIOVElement;

    std::vector<DeUTLayer> layers() const {
      std::vector<DeUTLayer> tlayers;
      for ( const auto& layer : this->access()->m_layers ) { tlayers.emplace_back( &layer ); }
      return tlayers;
    }

    unsigned int id() const { return this->access()->m_id; }
    auto         detector() const { return this->access()->detector(); }

    bool contains( const ChannelID aChannel ) const { return channelID().side() == aChannel.side(); }

    const DeUTLayer findLayer( const ChannelID aChannel ) const {
      const auto& layers = this->access()->m_layers;
      const auto  iter   = std::find_if( layers.begin(), layers.end(), [&]( const detail::DeUTLayerObject& l ) {
        return DeUTLayer{&l}.contains( aChannel );
      } );
      return iter != layers.end() ? &( *iter ) : nullptr;
    }

    const DeUTLayer findLayer( const ROOT::Math::XYZPoint& point ) const {
      const auto& layers = this->access()->m_layers;
      const auto  iter   = std::find_if( layers.begin(), layers.end(), [&]( const detail::DeUTLayerObject& l ) {
        return DeUTLayer{&l}.isInside( point );
      } );
      return iter != layers.end() ? &( *iter ) : nullptr;
    }

    const DeUTSector findSector( const ChannelID aChannel ) const {
      auto layer = findLayer( aChannel );
      return layer.isValid() ? layer.findSector( aChannel ) : DeUTSector{};
    }

    const DeUTSector findSector( const ROOT::Math::XYZPoint& aPoint ) const {
      auto layer = findLayer( aPoint );
      return layer.isValid() ? layer.findSector( aPoint ) : DeUTSector{};
    }

    ChannelID channelID() const { return ChannelID{this->access()->m_channelID}; }
    double    fractionActive() const {
      auto [active, total] = this->access()->countSectors();
      return active / total;
      ;
    }

    ROOT::Math::XYZPoint globalCentre() const { return this->toGlobal( ROOT::Math::XYZPoint{0, 0, 0} ); }
  };

} // namespace LHCb::Detector::UT
