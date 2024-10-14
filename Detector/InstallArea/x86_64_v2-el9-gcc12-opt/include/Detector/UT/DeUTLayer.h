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
#include "Detector/UT/DeUTStave.h"

#include <array>
#include <deque>
#include <vector>

namespace LHCb::Detector::UT {

  namespace detail {

    struct DeUTLayerObject : LHCb::Detector::detail::DeIOVObject {

      DeUTLayerObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );
      const unsigned int m_id;
      const unsigned int m_nStaves; // can be 8 or 9
      // not always fully used, when nStaves = 8, last one is replicaeted and unusued
      std::array<DeUTStaveObject, 9> m_staves;
      ChannelID                      m_channelID;
      ROOT::Math::Plane3D            m_plane;
      double                         m_angle    = 0;
      double                         m_sinAngle = 0;
      double                         m_cosAngle = 0;

      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( unsigned int i = 0; i < m_nStaves; i++ ) { func( &m_staves[i] ); };
      }

      void applyToAllStaves( const std::function<void( DeUTStave )>& func ) const { applyToAllChildren( func ); }

      void applyToAllFaces( const std::function<void( DeUTFace )>& func ) const {
        for ( unsigned int i = 0; i < m_nStaves; i++ ) m_staves[i].applyToAllFaces( func );
      }

      void applyToAllModules( const std::function<void( DeUTModule )>& func ) const {
        for ( unsigned int i = 0; i < m_nStaves; i++ ) m_staves[i].applyToAllModules( func );
      }

      void applyToAllSectors( const std::function<void( DeUTSector )>& func ) const {
        for ( unsigned int i = 0; i < m_nStaves; i++ ) m_staves[i].applyToAllSectors( func );
      }

      /// returns nb of active sectors and total number of sectors, in this order
      std::pair<double, unsigned int> countSectors() {
        std::pair<double, unsigned int> ret{0, 0};
        auto& [active, total] = ret;
        for ( unsigned int i = 0; i < m_nStaves; i++ ) {
          auto [a, t] = m_staves[i].countSectors();
          active += a;
          total += t;
        }
        return ret;
      }

      float z() { return m_staves[0].z(); }
    };
  } // End namespace detail

  /**
   *  This object defines the behaviour of the objects's data
   *
   *  \author  Markus Frank
   *  \date    2018-03-08
   *  \version  1.0
   */
  struct DeUTLayer : DeIOVElement<detail::DeUTLayerObject> {
    using DeIOVElement::DeIOVElement;

    unsigned int id() const { return this->access()->m_id; }
    unsigned int nStaves() const { return this->access()->m_nStaves; }
    auto         detector() const noexcept { return this->access()->detector(); }
    bool contains( const ChannelID aChannel ) const { return channelID().uniqueLayer() == aChannel.uniqueLayer(); };

    double angle() const { return this->access()->m_angle; }
    double cosAngle() const { return this->access()->m_cosAngle; }
    double sinAngle() const { return this->access()->m_sinAngle; }

    const DeUTStave findStave( const ChannelID aChannel ) const {
      for ( unsigned int i = 0; i < nStaves(); i++ ) {
        if ( DeUTStave{&this->access()->m_staves[i]}.contains( aChannel ) ) return &this->access()->m_staves[i];
      }
      return nullptr;
    }

    const DeUTStave findStave( const ROOT::Math::XYZPoint& point ) const {
      for ( unsigned int i = 0; i < nStaves(); i++ ) {
        if ( DeUTStave{&this->access()->m_staves[i]}.isInside( point ) ) return &this->access()->m_staves[i];
      }
      return nullptr;
    }

    const DeUTSector findSector( const ChannelID aChannel ) const {
      auto stave = findStave( aChannel );
      return stave.isValid() ? stave.findSector( aChannel ) : DeUTSector{};
    }

    const DeUTSector findSector( const ROOT::Math::XYZPoint& aPoint ) const {
      auto stave = findStave( aPoint );
      return stave.isValid() ? stave.findSector( aPoint ) : DeUTSector{};
    }

    const ROOT::Math::Plane3D& plane() const { return this->access()->m_plane; }

    const std::vector<DeUTStave> staves() const {
      std::vector<DeUTStave> tstaves;
      for ( unsigned int i = 0; i < nStaves(); i++ ) { tstaves.emplace_back( &this->access()->m_staves[i] ); }
      return tstaves;
    }

    double fractionActive() const {
      auto [active, total] = this->access()->countSectors();
      return active / total;
      ;
    }

    ROOT::Math::XYZPoint globalCentre() const { return this->toGlobal( ROOT::Math::XYZPoint{0, 0, 0} ); }
    float                z() const { return this->access()->z(); }
    ChannelID            channelID() const { return this->access()->m_channelID; }
    ChannelID            elementID() const { return this->access()->m_channelID; }
  };

  inline std::ostream& operator<<( std::ostream&, const DeUTLayer& ) { throw "Not implemednted"; }

} // namespace LHCb::Detector::UT
