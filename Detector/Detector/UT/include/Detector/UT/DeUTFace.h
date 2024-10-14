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
#include "Detector/UT/ChannelID.h"
#include "Detector/UT/DeUTModule.h"
#include "Detector/UT/DeUTSector.h"

#include <array>
#include <deque>
#include <vector>

namespace LHCb::Detector::UT {

  namespace detail {

    struct DeUTFaceObject : LHCb::Detector::detail::DeIOVObject {
      DeUTFaceObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );
      const unsigned int           m_id;
      std::deque<DeUTModuleObject> m_modules;
      std::string                  m_type;
      ChannelID                    m_channelID;

      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& module : m_modules ) { func( &module ); };
      }

      void applyToAllModules( const std::function<void( DeUTModule )>& func ) const { applyToAllChildren( func ); }

      void applyToAllSectors( const std::function<void( DeUTSector )>& func ) const {
        for ( auto& module : m_modules ) module.applyToAllSectors( func );
      }

      /// returns nb of active sectors and total number of sectors, in this order
      std::pair<double, unsigned int> countSectors() {
        std::pair<double, unsigned int> ret{0, 0};
        auto& [active, total] = ret;
        for ( auto& module : m_modules ) {
          auto [a, t] = module.countSectors();
          active += a;
          total += t;
        }
        return ret;
      }

      float z() const { return m_modules[0].z(); }
    };
  } // End namespace detail

  /**
   *  This object defines the behaviour of the objects's data
   *
   *  \author  Markus Frank
   *  \date    2018-03-08
   *  \version  1.0
   */
  struct DeUTFace : DeIOVElement<detail::DeUTFaceObject> {
    using DeIOVElement::DeIOVElement;

    auto                    detector() const noexcept { return this->access()->detector(); }
    ChannelID               channelID() const { return this->access()->m_channelID; }
    unsigned int            id() const { return this->access()->m_id; }
    std::vector<DeUTModule> modules() const {
      std::vector<DeUTModule> tmodules;
      for ( const auto& module : this->access()->m_modules ) { tmodules.emplace_back( &module ); }
      return tmodules;
    }
    const std::string& stavetype() const { return this->access()->m_type; }

    bool contains( const ChannelID aChannel ) const { return aChannel.uniqueFace() == channelID().uniqueFace(); }
    const DeUTSector findSector( const ChannelID aChannel ) const {
      auto module = findModule( aChannel );
      return module.isValid() ? module.findSector( aChannel ) : DeUTSector{};
    }
    const DeUTSector findSector( const ROOT::Math::XYZPoint& point ) const {
      auto module = findModule( point );
      return module.isValid() ? module.findSector( point ) : DeUTSector{};
    }
    const DeUTModule findModule( const ChannelID aChannel ) const {
      const auto& modules = this->access()->m_modules;
      auto        iter    = std::find_if( modules.begin(), modules.end(), [&]( const detail::DeUTModuleObject& s ) {
        return DeUTModule{&s}.contains( aChannel );
      } );
      return iter != modules.end() ? &( *iter ) : nullptr;
    }
    const DeUTModule findModule( const ROOT::Math::XYZPoint& point ) const {
      const auto& modules = this->access()->m_modules;
      auto        iter    = std::find_if( modules.begin(), modules.end(),
                                [&]( const detail::DeUTModuleObject& s ) { return DeUTModule{&s}.isInside( point ); } );
      return iter != modules.end() ? &( *iter ) : nullptr;
    }
    double fractionActive() const {
      auto [active, total] = this->access()->countSectors();
      return active / total;
      ;
    }
  };

} // namespace LHCb::Detector::UT
