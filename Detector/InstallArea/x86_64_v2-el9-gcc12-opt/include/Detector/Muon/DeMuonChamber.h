/*****************************************************************************\
* (c) Copyright 2000-2021 CERN for the benefit of the LHCb Collaboration      *
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
#include "Core/Utils.h"
#include "DD4hep/Detector.h"
#include "DD4hep/Printout.h"
#include "Detector/Muon/DeMuonGasGap.h"
#include "Detector/Muon/MuonConstants.h"
#include "Math/Point3D.h"
#include <array>

namespace LHCb::Detector {

  namespace detail {

    struct DeMuonChamberObject : DeIOVObject {
      //
      DeMuonChamberObject( dd4hep::DetElement const& de, dd4hep::cond::ConditionUpdateContext& ctxt );
      //
      double m_Xcham = 0.0;
      double m_Ycham = 0.0;
      double m_Zcham = 0.0;
      //
      double m_DXcham = 0.0; ///< DeltaX outer frame chamber dimension
      double m_DYcham = 0.0; ///< DeltaY outer frame chamber dimension
      double m_DZcham = 0.0; ///< DeltaZ outer frame chamber dimension
      //
      int m_ChamberNumber = 0;
      int m_RegionNumber  = 0;
      int m_StationNumber = 0;
      //
      std::array<DeMuonGasGapObject, Muon::nGaps> m_gasGaps;
      //
      //
      struct PointInGasGap {
        const ROOT::Math::XYZPoint pointInGap;
        int                        number = 0;
        const DeMuonGasGap         gasGap;
      };
      std::optional<PointInGasGap> checkPointInGasGap( ROOT::Math::XYZPoint GlobalPoint ) const;
      ROOT::Math::XYZPoint         getPosition() const;
      //
      std::tuple<ROOT::Math::XYZPoint, ROOT::Math::XYZPoint> getGapPoints( int gap, double LocalX,
                                                                           double LocalY ) const;
      bool checkHitAndGapInChamber( double GlobalX, double GlobalY ) const;
      bool calculateHitPosInGap( int gapNumber, double xpos, double ypos, double xSlope, double ySlope, double zavegaps,
                                 ROOT::Math::XYZPoint& entryGlobal, ROOT::Math::XYZPoint& exitGlobal ) const;
      //
      double calculateAverageGap( int gapNumberStart, int gapNumberStop, double xpos, double ypos ) const;
      //

      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& gasGap : m_gasGaps ) { func( &gasGap ); };
      }
    };

  } // namespace detail

  struct DeMuonChamber : DeIOVElement<detail::DeMuonChamberObject> {
    using DeIOVElement::DeIOVElement;

    double XPos() const { return this->access()->m_Xcham; }
    double YPos() const { return this->access()->m_Ycham; }
    double ZPos() const { return this->access()->m_Zcham; }
    //
    double DX() const { return this->access()->m_DXcham; }
    double DY() const { return this->access()->m_DYcham; }
    double DZ() const { return this->access()->m_DZcham; }
    //
    const DeMuonGasGap getGap( unsigned int iG ) const { return DeMuonGasGap( &this->access()->m_gasGaps[iG] ); };

    const std::array<DeMuonGasGap, Muon::nGaps> gaps() const {
      const auto obj = this->access();
      return {&obj->m_gasGaps[0], &obj->m_gasGaps[1], &obj->m_gasGaps[2], &obj->m_gasGaps[3]};
    };

    int stationNumber() const { return this->access()->m_StationNumber; }
    int regionNumber() const { return this->access()->m_RegionNumber; }
    int chamberNumber() const { return this->access()->m_ChamberNumber; }

    using NotImplemented = LHCb::Detector::NotImplemented;

    using PointInGasGap = typename detail::DeMuonChamberObject::PointInGasGap;

    std::optional<PointInGasGap> checkPointInGasGap( ROOT::Math::XYZPoint GlobalPoint ) const {
      return this->access()->checkPointInGasGap( GlobalPoint );
    }
    ROOT::Math::XYZPoint getPosition() { return this->access()->getPosition(); }
    //
    //
    bool checkHitAndGapInChamber( double GlobalX, double GlobalY ) const {
      return this->access()->checkHitAndGapInChamber( GlobalX, GlobalY );
    }

    bool calculateHitPosInGap( int gapNumber, double xpos, double ypos, double xSlope, double ySlope, double zavegaps,
                               ROOT::Math::XYZPoint& entryGlobal, ROOT::Math::XYZPoint& exitGlobal ) const {
      return this->access()->calculateHitPosInGap( gapNumber, xpos, ypos, xSlope, ySlope, zavegaps, entryGlobal,
                                                   exitGlobal );
    }

    double calculateAverageGap( int gapNumberStart, int gapNumberStop, double xpos, double ypos ) const {
      return this->access()->calculateAverageGap( gapNumberStart, gapNumberStop, xpos, ypos );
    }
    //
    std::tuple<ROOT::Math::XYZPoint, ROOT::Math::XYZPoint> getGapPoints( int gap, double LocalX, double LocalY ) const {
      return this->access()->getGapPoints( gap, LocalX, LocalY );
    }
  };
} // End namespace LHCb::Detector
