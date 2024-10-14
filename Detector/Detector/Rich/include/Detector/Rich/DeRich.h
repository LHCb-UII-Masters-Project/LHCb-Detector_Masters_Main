/*****************************************************************************\
* (c) Copyright 2000-2019 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*******************************************************************************/

#pragma once

#include "Core/DeIOV.h"
#include "Core/FloatComparison.h"
#include "Detector/Rich/Types.h"
#include "Detector/Rich/Utilities.h"
#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>

namespace LHCb::Detector {

  namespace detail {

    struct DeRichObject : DeIOVObject {

      Rich::DetectorType m_rich = Rich::InvalidDetector;

      std::int32_t m_NumSides          = -1;
      std::int32_t m_nSphMirrorSegRows = -1;
      std::int32_t m_nSphMirrorSegCols = -1;
      std::int32_t m_nSecMirrorSegRows = -1;
      std::int32_t m_nSecMirrorSegCols = -1;

      XYTable m_NominalPmtQE;
      XYTable m_SphMNominalRefl;
      XYTable m_SecMNominalRefl;
      XYTable m_QWNominalAbsLen;

      double m_sphMirrorNominalROC = 0;

      Rich::PanelArray<dd4hep::Position> m_NominalPanelCoC;

      // using DeIOVObject::DeIOVObject;
      DeRichObject( const Rich::DetectorType              rich, //
                    const dd4hep::DetElement&             de,   //
                    dd4hep::cond::ConditionUpdateContext& ctxt );

      // mimic 'param' method from DetDesc
      template <typename TYPE>
      inline auto param( std::string name ) const {
        return dd4hep_param<TYPE>( std::move( name ) );
      }
    };

  } // namespace detail

  template <typename ObjectType>
  struct DeRichElement : DeIOVElement<ObjectType> {

    using DeIOVElement<ObjectType>::DeIOVElement;

    /// RICH type enum
    inline auto rich() const noexcept { return this->access()->m_rich; }

    /// Number of Sides
    inline auto NumSides() const noexcept { return ( this->access()->m_NumSides ); }

    /// number of primary mirror rows
    inline auto nSphMirrorSegRows() const noexcept { return this->access()->m_nSphMirrorSegRows; }
    /// number of primary mirror columns
    inline auto nSphMirrorSegCols() const noexcept { return this->access()->m_nSphMirrorSegCols; }
    /// number of primary mirror rows
    inline auto nSecMirrorSegRows() const noexcept { return this->access()->m_nSecMirrorSegRows; }
    /// number of primary mirror columns
    inline auto nSecMirrorSegCols() const noexcept { return this->access()->m_nSecMirrorSegCols; }

    /// table of (x,y) values for the nominal PD QE curve
    inline auto nominalPDQuantumEff() const noexcept {
      return detail::scalePhotonEnergies( this->access()->m_NominalPmtQE );
    }
    /// table of (x,y) values for the nominal primary mirror reflectivity
    inline auto nominalSphMirrorRefl() const noexcept {
      return detail::scalePhotonEnergies( this->access()->m_SphMNominalRefl );
    }
    /// table of (x,y) values for the nominal secondary mirror reflectivity
    inline auto nominalSecMirrorRefl() const noexcept {
      return detail::scalePhotonEnergies( this->access()->m_SecMNominalRefl );
    }
    /// table of (x,y) values for the nominal gas quartz window absorption length
    inline auto gasWinAbsLength() const noexcept {
      auto tab = detail::scaleLengths( detail::scalePhotonEnergies( this->access()->m_QWNominalAbsLen ) );
      // CRJ : To be checked but seems reco downstream expects length units in cm here.
      //       Not entirely sure why ... ??
      if constexpr ( !essentiallyEqual( 1.0, detail::GaudiCentimeter ) ) {
        for ( auto& i : tab ) { i.second /= detail::GaudiCentimeter; }
      }
      return tab;
    }

    /// 'Nominal' radius of curvature of primary mirror
    inline auto sphMirrorRadius() const noexcept {
      return detail::toLHCbLengthUnits( this->access()->m_sphMirrorNominalROC );
    }

    /// 'nominal' centre of curvature of the spherical mirror for given side
    inline auto nominalCentreOfCurvature( const Rich::Side side ) const noexcept {
      return detail::toLHCbLengthUnits( this->access()->m_NominalPanelCoC[side] );
    }

    /// Access parameters
    template <typename TYPE>
    inline auto param( std::string name ) const {
      return this->access()->template param<TYPE>( std::move( name ) );
    }
  };

  using DeRich = detail::DeRichObject;

} // namespace LHCb::Detector
