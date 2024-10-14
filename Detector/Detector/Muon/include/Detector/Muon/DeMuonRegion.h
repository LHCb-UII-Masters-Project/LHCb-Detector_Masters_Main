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
#include "Detector/Muon/DeMuonChamber.h"
#include "Detector/Muon/MuonConstants.h"
#include <array>
#include <deque>

namespace LHCb::Detector {

  namespace detail {

    /**
     *  Generic Muon iov dependent detector element of a Muon region
     *  \author  Markus Frank
     *  \date    2018-03-08
     *  \version  1.0
     */
    struct DeMuonRegionObject : DeIOVObject {
      //
      unsigned int m_regionID;
      unsigned int m_sideID;
      unsigned int m_stationID;

      double m_Xreg = 0.0; ///< Global x position of side
      double m_Yreg = 0.0; ///< Global y position of side
      double m_Zreg = 0.0; ///< Global z position of side

      //
      std::deque<DeMuonChamberObject> m_chambers;
      //
      DeMuonRegionObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt,
                          unsigned int iRegion, unsigned int sideID, unsigned int stationID );
      //

      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& chamber : m_chambers ) { func( &chamber ); };
      }
    };
  } // End namespace detail

  struct DeMuonRegion : DeIOVElement<detail::DeMuonRegionObject> {
    using DeIOVElement::DeIOVElement;

    const DeMuonChamber getChamber( unsigned int iCh ) const {
      return DeMuonChamber( &this->access()->m_chambers[iCh] );
    };

    unsigned int chambersNumber() const { return this->access()->m_chambers.size(); }

    const std::deque<detail::DeMuonChamberObject>& chambers() const {
      //
      return this->access()->m_chambers;
    }
    unsigned int regionID() const { return this->access()->m_regionID; }

    unsigned int sideID() const { return this->access()->m_sideID; }

    unsigned int stationID() const { return this->access()->m_stationID; }

    //
    double XPos() const { return this->access()->m_Xreg; }
    double YPos() const { return this->access()->m_Yreg; }
    double ZPos() const { return this->access()->m_Zreg; }
  };
} // End namespace LHCb::Detector
