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
#include "Detector/UP/DeUPSector.h"
#include "Detector/UP/UPChannelID.h"

#include <array>
#include <numeric>
#include <vector>

namespace LHCb::Detector {

  namespace detail {

    struct DeUPModuleObject : LHCb::Detector::detail::DeIOVObject {
      DeUPModuleObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );

      unsigned int                  id;
      std::vector<DeUPSectorObject> m_sectors;
      ChannelID                     m_channelID;

      unsigned int m_firstSector{0};
      std::string  m_versionString{"DC11"};
      void         applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& sector : m_sectors ) { func( &sector ); };
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
  struct DeUPModuleElement : DeIOVElement<ObjectType> {

    using DeIOVElement<ObjectType>::DeIOVElement;

    ChannelID          channelID() const { return this->access()->m_channelID; }
    auto const&        module() { return this->access()->id; }
    auto               sectors() { return this->access()->m_sectors; }
    unsigned int       lastSector() const { return this->access()->m_firstSector + 1u; }
    unsigned int       firstSector() const { return this->access()->m_firstSector; }
    const std::string& versionString() const { return this->access()->m_versionString; }

    inline bool contains( const ChannelID ) const { throw NotImplemented(); }
    double      fractionActive() const {
      return std::accumulate( sectors().begin(), sectors().end(), 0.0,
                              []( double f, const DeUPSector s ) { return f + s.fractionActive(); } ) /
             double( sectors().size() );
    }
    const DeUPSector findSector( const ROOT::Math::XYZPoint& point ) const {
      auto iter =
          std::find_if( sectors().begin(), sectors().end(), [&]( const DeUPSector s ) { return s.isInside( point ); } );
      return iter != sectors().end() ? *iter : nullptr;
    }
    const DeUPSector findSector( const ChannelID aChannel ) const {
      auto iter = std::find_if( sectors().begin(), sectors().end(),
                                [&]( const DeUPSector s ) { return s.contains( aChannel ); } );
      return iter != sectors().end() ? *iter : nullptr;
    }
  };

  using DeUPModule = DeUPModuleElement<detail::DeUPModuleObject>;

} // namespace LHCb::Detector
