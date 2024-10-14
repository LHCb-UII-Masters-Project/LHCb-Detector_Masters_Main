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

#include "Detector/Muon/MuonConstants.h"
#include "Math/Point3D.h"

namespace LHCb::Detector {

  namespace detail {

    struct DeMuonGasGapObject : DeIOVObject {
      DeMuonGasGapObject( dd4hep::DetElement const& de, dd4hep::cond::ConditionUpdateContext& ctxt, unsigned int iG );
      //
      double m_Xgap = 0.0;
      double m_Ygap = 0.0;
      double m_Zgap = 0.0;
      //
      double m_DXgap = 0.0; ///< DeltaX sensitive chamber dimension
      double m_DYgap = 0.0; ///< DeltaY sensitive chamber dimension
      double m_DZgap = 0.0; ///< DeltaZ sensitive chamber dimension
      //
      unsigned int m_gapID = 0;
      //
      ROOT::Math::XYZPoint getPosition() const;
      //
      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& ) const override {
        // no children
      }
    };

  } // namespace detail

  struct DeMuonGasGap : DeIOVElement<detail::DeMuonGasGapObject> {
    using DeIOVElement::DeIOVElement;

    double XPos() const { return this->access()->m_Xgap; }
    double YPos() const { return this->access()->m_Ygap; }
    double ZPos() const { return this->access()->m_Zgap; }

    double DX() const { return this->access()->m_DXgap; }
    double DY() const { return this->access()->m_DYgap; }
    double DZ() const { return this->access()->m_DZgap; }
    //
    ROOT::Math::XYZPoint getPosition() { return this->access()->getPosition(); }
    //
    // To be dropped once everything is implemented
    using NotImplemented = LHCb::Detector::NotImplemented;
    //
    bool checkHitAndGapInChamber( float, float ) const { throw NotImplemented(); }
    bool calculateHitPosInGap( int, float, float, float, float, float, ROOT::Math::XYZPoint&,
                               ROOT::Math::XYZPoint& ) const {
      throw NotImplemented();
    }
    float calculateAverageGap( int, int, float, float ) const { throw NotImplemented(); }
    std::tuple<ROOT::Math::XYZPoint, ROOT::Math::XYZPoint> getGapPoints( int, double, double ) const {
      throw NotImplemented();
    }
  };
} // End namespace LHCb::Detector
