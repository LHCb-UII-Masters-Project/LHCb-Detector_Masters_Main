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
#include "Detector/UP/DeUPModule.h"
#include "Detector/UP/DeUPSector.h"
#include "Detector/UP/UPChannelID.h"

#include <array>
#include <vector>

namespace LHCb::Detector {

  namespace detail {

    struct DeUPFaceObject : LHCb::Detector::detail::DeIOVObject {
      DeUPFaceObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );
      std::vector<DeUPModuleObject> m_modules;
      ChannelID                     m_channelID;
      unsigned int                  m_firstSector{0};
      double                        m_fractionActive;
      unsigned int                  m_numSectorsExpected;
      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& module : m_modules ) { func( &module ); };
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
  struct DeUPFaceElement : DeIOVElement<ObjectType> {

    using DeIOVElement<ObjectType>::DeIOVElement;

    ChannelID channelID() const { return this->access()->m_channelID; }
    auto      modules() { return this->access()->m_modules; }

    inline bool      contains( const ChannelID ) const { throw NotImplemented(); }
    const DeUPSector findSector( const ChannelID aChannel ) const {
      auto* module = findModule( aChannel );
      if ( !module ) return nullptr;
      return module->findSector( aChannel );
    }
    const DeUPSector findSector( const ROOT::Math::XYZPoint& point ) const {
      auto* module = findModule( point );
      if ( !module ) return nullptr;
      return module->findSector( point );
    }
    const DeUPModule findModule( const ChannelID aChannel ) const {
      auto iter = std::find_if( modules().begin(), modules().end(),
                                [&]( const DeUPModule s ) { return s.contains( aChannel ); } );
      return iter != modules().end() ? *iter : nullptr;
    }
    const DeUPModule findModule( const ROOT::Math::XYZPoint& point ) const {
      auto iter =
          std::find_if( modules().begin(), modules().end(), [&]( const DeUPModule s ) { return s.isInside( point ); } );
      return iter != modules().end() ? *iter : nullptr;
    }
    double       fractionActive() const { return this->access()->m_fractionActive; }
    unsigned int lastSector() const { return firstSector() + this->access()->m_numSectorsExpected - 1u; }
    unsigned int firstSector() const { return this->access()->m_firstSector; }
  };

  using DeUPFace = DeUPFaceElement<detail::DeUPFaceObject>;

} // namespace LHCb::Detector
