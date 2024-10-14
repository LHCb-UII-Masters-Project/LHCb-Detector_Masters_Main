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
#include "Detector/Muon/DeMuonRegion.h"
#include "Detector/Muon/MuonConstants.h"

#include <array>
#include <string>

namespace LHCb::Detector {

  namespace detail {

    /**
     *  Generic Muon iov dependent detector element of a Muon Side
     *  \author  Markus Frank
     *  \date    2018-03-08
     *  \version  1.0
     */
    struct DeMuonSideObject : DeIOVObject {
      //
      // Indexes of the sides by name
      std::array<std::string, 2> m_Side   = {"A", "C"};
      unsigned int               m_sideID = 0;
      double                     m_Xside  = 0.0; ///< Global x position of side
      double                     m_Yside  = 0.0; ///< Global y position of side
      double                     m_Zside  = 0.0; ///< Global z position of side
      //
      std::array<DeMuonRegionObject, Muon::nRegions> m_regions;
      //
      DeMuonSideObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt, unsigned int iSide,
                        unsigned int stationID );
      //

      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& region : m_regions ) { func( &region ); };
      }
    };

  } // End namespace detail

  struct DeMuonSide : DeIOVElement<detail::DeMuonSideObject> {
    using DeIOVElement::DeIOVElement;

    unsigned int sideID() const { return this->access()->m_sideID; }
    double       XPos() const { return this->access()->m_Xside; }
    double       YPos() const { return this->access()->m_Yside; }
    double       ZPos() const { return this->access()->m_Zside; }
    //
    const DeMuonRegion getRegion( unsigned int iReg ) const {
      return DeMuonRegion( &this->access()->m_regions[iReg] );
    };
    const std::array<DeMuonRegion, Muon::nRegions> regions() const {
      const auto obj = this->access();
      return {DeMuonRegion( &obj->m_regions[0] ), DeMuonRegion( &obj->m_regions[1] ),
              DeMuonRegion( &obj->m_regions[2] ), DeMuonRegion( &obj->m_regions[3] )};
    };
  };
} // End namespace LHCb::Detector
