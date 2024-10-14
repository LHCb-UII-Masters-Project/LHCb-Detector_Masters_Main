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
#include "Detector/UP/DeUPFace.h"
#include "Detector/UP/DeUPSector.h"
#include "Detector/UP/UPChannelID.h"

#include <array>
#include <vector>

namespace LHCb::Detector {

  namespace detail {

    struct DeUPStaveObject : LHCb::Detector::detail::DeIOVObject {
      DeUPStaveObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );
      std::vector<DeUPFaceObject> m_faces;
      ChannelID                   m_channelID;

      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& face : m_faces ) { func( &face ); };
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
  struct DeUPStaveElement : DeIOVElement<ObjectType> {

    using DeIOVElement<ObjectType>::DeIOVElement;

    auto      sectors() { return this->access()->m_sectors; }
    ChannelID channelID() const { return this->access()->m_channelID; }

    inline bool      contains( const ChannelID ) const { throw NotImplemented(); }
    const DeUPSector findSector( const ChannelID aChannel ) const {
      auto& face = findFace( aChannel );
      if ( !face ) return nullptr;
      return face.findSector( aChannel );
    }
    const DeUPSector findSector( const ROOT::Math::XYZPoint& point ) const {
      auto& face = findFace( point );
      if ( !face ) return nullptr;
      return face.findSector( point );
    }
    const DeUPFace findFace( const ChannelID aChannel ) const {
      auto iter =
          std::find_if( faces().begin(), faces().end(), [&]( const DeUPFace s ) { return s.contains( aChannel ); } );
      return iter != faces().end() ? *iter : nullptr;
    }
    const DeUPFace findFace( const ROOT::Math::XYZPoint& point ) const {
      auto iter =
          std::find_if( faces().begin(), faces().end(), [&]( const DeUPFace s ) { return s.isInside( point ); } );
      return iter != faces().end() ? *iter : nullptr;
    }
    const DeUPModule findModule( const ChannelID aChannel ) const {
      auto* face = findFace( aChannel );
      if ( !face ) return nullptr;
      return face->findModule( aChannel );
    }
    const DeUPModule findModule( const ROOT::Math::XYZPoint& point ) const {
      auto* face = findFace( point );
      if ( !face ) return nullptr;
      return face->findModule( point );
    }
    double fractionActive() const {
      return std::accumulate( sectors().begin(), sectors().end(), 0.0,
                              []( double f, const DeUPSector s ) { return f + s.fractionActive(); } ) /
             double( sectors().size() );
    }
    auto const& faces() const { return this->access()->m_faces; }
  };

  using DeUPStave = DeUPStaveElement<detail::DeUPStaveObject>;

} // namespace LHCb::Detector
