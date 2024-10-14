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
#include "Detector/UT/DeUTFace.h"
#include "Detector/UT/DeUTSector.h"

#include <array>
#include <vector>

namespace LHCb::Detector::UT {

  namespace detail {

    struct DeUTStaveObject : LHCb::Detector::detail::DeIOVObject {

      DeUTStaveObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );
      const unsigned int            m_id;
      std::string                   m_type;
      std::string                   m_nickname;
      std::string                   m_nicknameOld;
      std::array<DeUTFaceObject, 2> m_faces;
      ChannelID                     m_channelID;
      unsigned int                  m_version;
      unsigned int                  m_column;
      unsigned int                  m_detRegion;

      std::string getStaveName( const ChannelID& aChan ) const {
        static constexpr const char* sides[2]  = {"C", "A"};
        static constexpr const char* layers[4] = {"aX", "aU", "bV", "bX"};
        return fmt::format( "UT{side}sideUT{layer}Stave{stave}", fmt::arg( "side", sides[aChan.side()] ),
                            fmt::arg( "layer", layers[aChan.layer()] ), fmt::arg( "stave", aChan.stave() + 1 ) );
      }

      std::string getStaveNameOld( const ChannelID& aChan ) const {
        static constexpr const char* layers[4] = {"aX", "aU", "bV", "bX"};
        return fmt::format( "UT{layer}LayerR{region}Stave{column}", fmt::arg( "layer", layers[aChan.layer()] ),
                            fmt::arg( "region", m_detRegion ), fmt::arg( "column", m_column ) );
      }

      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& face : m_faces ) { func( &face ); };
      }

      void applyToAllFaces( const std::function<void( DeUTFace )>& func ) const { applyToAllChildren( func ); }

      void applyToAllModules( const std::function<void( DeUTModule )>& func ) const {
        for ( auto& face : m_faces ) face.applyToAllModules( func );
      }

      void applyToAllSectors( const std::function<void( DeUTSector )>& func ) const {
        for ( auto& face : m_faces ) face.applyToAllSectors( func );
      }

      /// returns nb of active sectors and total number of sectors, in this order
      std::pair<double, unsigned int> countSectors() {
        std::pair<double, unsigned int> ret{0, 0};
        auto& [active, total] = ret;
        for ( auto& face : m_faces ) {
          auto [a, t] = face.countSectors();
          active += a;
          total += t;
        }
        return ret;
      }

      float z() const { return m_faces[0].z(); }
    };
  } // End namespace detail

  /**
   *  This object defines the behaviour of the objects's data
   *
   *  \author  Markus Frank
   *  \date    2018-03-08
   *  \version  1.0
   */
  struct DeUTStave : public DeIOVElement<detail::DeUTStaveObject> {
    using DeIOVElement::DeIOVElement;

    unsigned int          id() const { return this->access()->m_id; }
    auto                  detector() const noexcept { return this->access()->detector(); }
    ChannelID             channelID() const { return this->access()->m_channelID; }
    std::vector<DeUTFace> faces() const {
      std::vector<DeUTFace> tfaces;
      for ( const auto& face : this->access()->m_faces ) { tfaces.emplace_back( &face ); }
      return tfaces;
    }
    unsigned int       layer() const { return channelID().layer(); }
    unsigned int       side() const { return channelID().side(); }
    unsigned int       stave() const { return channelID().stave(); }
    unsigned int       column() const { return this->access()->m_column; }
    unsigned int       detRegion() const { return this->access()->m_detRegion; }
    const std::string& nickname() const { return this->access()->m_nickname; }
    const std::string& nicknameOld() const { return this->access()->m_nicknameOld; }

    const std::string& type() const { return this->access()->m_type; }

    bool contains( const ChannelID aChannel ) const { return aChannel.uniqueStave() == channelID().uniqueStave(); };

    const DeUTSector findSector( const ChannelID aChannel ) const {
      auto face = findFace( aChannel );
      return face.isValid() ? face.findSector( aChannel ) : DeUTSector{};
    }

    const DeUTSector findSector( const ROOT::Math::XYZPoint& point ) const {
      auto face = findFace( point );
      return face.isValid() ? face.findSector( point ) : DeUTSector{};
    }

    const DeUTModule findModule( const ChannelID aChannel ) const {
      auto face = findFace( aChannel );
      return face.isValid() ? face.findModule( aChannel ) : DeUTModule{};
    }
    const DeUTModule findModule( const ROOT::Math::XYZPoint& point ) const {
      auto face = findFace( point );
      return face.isValid() ? face.findModule( point ) : DeUTModule{};
    }

    const DeUTFace findFace( const ChannelID aChannel ) const {
      const auto& faces = this->access()->m_faces;
      auto        iter  = std::find_if( faces.begin(), faces.end(),
                                [&]( const detail::DeUTFaceObject& s ) { return DeUTFace{&s}.contains( aChannel ); } );
      return iter != faces.end() ? &( *iter ) : nullptr;
    }
    const DeUTFace findFace( const ROOT::Math::XYZPoint& point ) const {
      const auto& faces = this->access()->m_faces;
      auto        iter  = std::find_if( faces.begin(), faces.end(),
                                [&]( const detail::DeUTFaceObject& s ) { return DeUTFace{&s}.isInside( point ); } );
      return iter != faces.end() ? &( *iter ) : nullptr;
    }

    double fractionActive() const {
      auto [active, total] = this->access()->countSectors();
      return active / total;
      ;
    }
  };

} // namespace LHCb::Detector::UT
