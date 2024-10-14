/***************************************************************************** \
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
#include "Detector/Muon/DeMuonSide.h"
#include "Detector/Muon/MuonConstants.h"
#include <array>

namespace LHCb::Detector {

  namespace detail {

    /**
     *  Generic Muon iov dependent detector element of a Muon station
     *  \author  Markus Frank
     *  \date    2018-03-08
     *  \version  1.0
     */
    struct DeMuonStationObject : DeIOVObject {
      //
      unsigned int m_stationID = 0; // first station is == 2
      double       m_Xstation  = 0.0;
      double       m_Ystation  = 0.0;
      double       m_Zstation  = 0.0;
      //
      DeMuonStationObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt,
                           unsigned int iStation );
      //
      std::array<DeMuonSideObject, Muon::nSides> m_sides;
      //

      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& side : m_sides ) { func( &side ); };
      }
    };

  } // End namespace detail

  struct DeMuonStation : DeIOVElement<detail::DeMuonStationObject> {
    using DeIOVElement::DeIOVElement;

    unsigned int stationID() const { return this->access()->m_stationID; }
    double       XPos() const { return this->access()->m_Xstation; }
    double       YPos() const { return this->access()->m_Ystation; }
    double       ZPos() const { return this->access()->m_Zstation; }

    const DeMuonSide getSide( unsigned int iSide ) const { return DeMuonSide( &this->access()->m_sides[iSide] ); };

    const std::array<DeMuonSide, 2> sides() const {
      const auto obj = this->access();
      return {DeMuonSide( &obj->m_sides[0] ), DeMuonSide( &obj->m_sides[1] )};
    };
  };
} // End namespace LHCb::Detector
