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
#include "Core/MagneticFieldGrid.h"

#include <array>
#include <vector>

namespace LHCb::Detector {

  struct DeMagnet;

  namespace detail {

    struct DeMagnetObject : DeIOVObject {

      friend DeMagnet;

      DeMagnetObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt,
                      std::shared_ptr<LHCb::Magnet::MagneticFieldGrid> magneticFieldGrid, bool useRealMap, bool isDown,
                      double signedRelativeCurrent );

      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& ) const override {
        // no children
      }

    private:
      // The field grid itself
      std::shared_ptr<LHCb::Magnet::MagneticFieldGrid> m_magneticFieldGrid;

      // Check whether this is the real map
      bool m_useRealMap;

      // Field direction
      bool m_isDown;

      // Current
      double m_signedRelativeCurrent;
    };
  } // End namespace detail

  struct DeMagnet : DeIOVElement<detail::DeMagnetObject> {
    using DeIOVElement::DeIOVElement;

    ROOT::Math::XYZVector fieldVector( const ROOT::Math::XYZPoint& pos ) const {
      return this->access()->m_magneticFieldGrid->fieldVectorLinearInterpolation( pos );
    }

    /// Returns the field grid
    const LHCb::Magnet::MagneticFieldGrid* fieldGrid() const { return this->access()->m_magneticFieldGrid.get(); }

    bool useRealMap() const { return this->access()->m_useRealMap; }

    bool isDown() const { return this->access()->m_isDown; }

    double signedRelativeCurrent() const { return this->access()->m_signedRelativeCurrent; }
  };
} // End namespace LHCb::Detector
