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
#include "Detector/UT/DeUTSector.h"

#include <array>
#include <deque>
#include <numeric>
#include <vector>

// FOR HANGYI: do the similar modification as we did in DeUTStave
namespace LHCb::Detector::UT {

  namespace detail {

    struct DeUTModuleObject : LHCb::Detector::detail::DeIOVObject {

      const unsigned int           m_id;
      std::deque<DeUTSectorObject> m_sectors;
      DeUTModuleObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );

      std::string m_type;
      ChannelID   m_channelID;
      std::string m_versionString{"DC11"};

      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& sector : m_sectors ) { func( &sector ); };
      }

      void applyToAllSectors( const std::function<void( DeUTSector )>& func ) const { applyToAllChildren( func ); }

      /// returns active sectors and total number of sectors, in this order
      std::pair<double, unsigned int> countSectors() {
        std::pair<unsigned int, unsigned int> ret{0, 0};
        auto& [active, total] = ret;
        for ( auto& sector : m_sectors ) {
          active += DeUTSector{&sector}.fractionActive();
          total += 1;
        };
        return ret;
      }

      float z() const { return DeUTSector( &m_sectors[0] ).globalCentre().Z(); }
    };
  } // End namespace detail

  /**
   *  This object defines the behaviour of the objects's data
   *
   *  \author  Markus Frank
   *  \date    2018-03-08
   *  \version  1.0
   */
  struct DeUTModule : DeIOVElement<detail::DeUTModuleObject> {

    using DeIOVElement::DeIOVElement;

    ChannelID               channelID() const { return this->access()->m_channelID; }
    unsigned int            id() const { return this->access()->m_id; }
    std::vector<DeUTSector> sectors() const {
      std::vector<DeUTSector> tsectors;
      tsectors.reserve( 2 );
      for ( const auto& sector : this->access()->m_sectors ) { tsectors.emplace_back( &sector ); }
      return tsectors;
    }
    auto               detector() const { return this->access()->detector(); }
    const std::string& versionString() const { return this->access()->m_versionString; }

    const std::string& staveType() const { return this->access()->m_type; }
    const std::string& type() const { return this->access()->m_type; }

    bool   contains( const ChannelID aChannel ) const { return aChannel.uniqueModule() == channelID().uniqueModule(); }
    double fractionActive() const {
      auto [active, total] = this->access()->countSectors();
      return active / total;
      ;
    }
    const DeUTSector findSector( const ROOT::Math::XYZPoint& point ) const {
      const auto& sectors = this->access()->m_sectors;
      auto        iter    = std::find_if( sectors.begin(), sectors.end(),
                                [&]( const detail::DeUTSectorObject& s ) { return DeUTSector{&s}.isInside( point ); } );
      return iter != sectors.end() ? &( *iter ) : nullptr;
    }
    const DeUTSector findSector( const ChannelID aChannel ) const {
      const auto& sectors = this->access()->m_sectors;
      auto        iter    = std::find_if( sectors.begin(), sectors.end(), [&]( const detail::DeUTSectorObject& s ) {
        return DeUTSector{&s}.contains( aChannel );
      } );
      return iter != sectors.end() ? &( *iter ) : nullptr;
    }
  };

} // namespace LHCb::Detector::UT
