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
#include "Detector/UP/DeUPLayer.h"

#include <array>
#include <vector>

namespace LHCb::Detector {

  namespace detail {

    struct DeUPSideObject : LHCb::Detector::detail::DeIOVObject {
      DeUPSideObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );
      unsigned int                 id;
      std::vector<DeUPLayerObject> m_layers;
      ChannelID                    channelID;

      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& layer : m_layers ) { func( &layer ); };
      }
    };
  } // End namespace detail

  /**
   *  This object defines the behaviour of the objects's data
   *
   *  \author  Markus Frank
   *  \date    2018-03-08
   *  \version  1.0
   */
  template <typename ObjectType>
  struct DeUPSideElement : DeIOVElement<ObjectType> {
    using DeIOVElement<ObjectType>::DeIOVElement;

    auto const& layers() const { return this->access()->m_layers; }
    ChannelID   channelID() const { return this->access()->m_channelID; }

    const DeUPLayer findLayer( const ChannelID aChannel ) const {
      auto iter =
          std::find_if( layers().begin(), layers().end(), [&]( const DeUPLayer l ) { return l.contains( aChannel ); } );
      return iter != layers().end() ? *iter : nullptr;
    }
    const DeUPLayer findLayer( const ROOT::Math::XYZPoint& point ) const {
      auto iter =
          std::find_if( layers().begin(), layers().end(), [&]( const DeUPLayer l ) { return l.isInside( point ); } );
      return iter != layers().end() ? *iter : nullptr;
    }
    double fractionActive() const {
      return std::accumulate( layers().begin(), layers().end(), 0.0,
                              [&]( double f, const DeUPLayer l ) { return f + l.fractionActive(); } ) /
             double( layers().size() );
    }
    bool contains( const ChannelID ) const {
      throw NotImplemented(); // return elementID().side() == aChannel.side();
    }
    ROOT::Math::XYZPoint globalCentre() const { throw NotImplemented(); }
  };

  using DeUPSide = DeUPSideElement<detail::DeUPSideObject>;

} // namespace LHCb::Detector
