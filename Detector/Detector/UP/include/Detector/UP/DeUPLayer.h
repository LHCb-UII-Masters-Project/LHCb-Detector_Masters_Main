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
#include "Detector/UP/DeUPStave.h"
#include "Detector/UP/UPChannelID.h"

#include <array>
#include <vector>

namespace LHCb::Detector {

  namespace detail {

    struct DeUPLayerObject : LHCb::Detector::detail::DeIOVObject {
      DeUPLayerObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );
      std::vector<DeUPStaveObject> m_staves;
      ChannelID                    ChanID;
      ROOT::Math::Plane3D          m_plane;
      double                       m_angle{};
      double                       m_sinAngle{};
      double                       m_cosAngle{};

      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& stave : m_staves ) { func( &stave ); };
      }
      ROOT::Math::XYZPoint globalCentre() const { throw "Not implemented"; }
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
  struct DeUPLayerElement : DeIOVElement<ObjectType> {

    using DeIOVElement<ObjectType>::DeIOVElement;

    auto const&         staves() const { return this->access()->m_staves; }
    ROOT::Math::Plane3D plane() const { return this->access()->m_plane; }

    /// detector element id - for experts only !
    ChannelID elementID() const { throw "Not implemented"; }

    ROOT::Math::XYZPoint globalCentre() const { throw "Not implemented"; }

    DeUPSector sector( unsigned int ) const { throw "Not implemented"; }
    void       applyToAllSectors( const std::function<void( DeUPSector const& )>& ) const { throw "Not implemented"; }
    void applyToAllSectorsAllen( const std::function<void( DeUPSector const& )>& ) const { throw "Not implemented"; }

    auto   chanID() { return this->access()->ChanID; }
    double fractionActive() const {
      return std::accumulate( staves().begin(), staves().end(), 0.0,
                              []( double f, const DeUPStave m ) { return f + m.fractionActive(); } ) /
             double( staves().size() );
    }
    const DeUPStave findStave( const ChannelID aChannel ) const {
      auto iter =
          std::find_if( staves().begin(), staves().end(), [&]( const DeUPStave m ) { return m.contains( aChannel ); } );
      return iter != staves().end() ? *iter : nullptr;
    }
    const DeUPStave findStave( const ROOT::Math::XYZPoint& point ) const {
      auto iter =
          std::find_if( staves().begin(), staves().end(), [&]( const DeUPStave m ) { return m.isInside( point ); } );
      return iter != staves().end() ? *iter : nullptr;
    }

    bool   contains( const ChannelID ) const { throw "Not implemented"; }
    double sinAngle() const { return this->access()->m_sinAngle; }
    double cosAngle() const { return this->access()->m_cosAngle; }
    double angle() const { return this->access()->m_angle; }
  };

  using DeUPLayer = DeUPLayerElement<detail::DeUPLayerObject>;

  inline std::ostream& operator<<( std::ostream&, const DeUPLayer& ) { throw "Not implemented"; }

} // namespace LHCb::Detector
