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
#include "Detector/Rich/DeRichMirror.h"
#include "Detector/Rich/Rich_Geom_Constants_De.h"
#include <array>
#include <cstdint>
#include <optional>

namespace LHCb::Detector {

  namespace detail {
    /**
     *  Rich2 Sph Mirror detector element data
     *  \author  Sajan Easo
     *  \date    2022-01-11
     *  \version  1.0
     */
    struct DeRich2SphMirrorSegObject : DeRichMirrorSegObject {
      void PrintR2M1ReflectivityFromDBInObj();
      DeRich2SphMirrorSegObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );
    };

    struct DeRich2SphMirrorMasterObject : DeIOVObject {
      std::int32_t m_Rich2SphMMCopyIdRef       = -1;
      std::int32_t m_Rich2SphMMSN              = -1;
      std::int32_t m_Rich2NumSphMirrorSegTotal = -1;
      std::int32_t m_Rich2NumSphMirrorSegSide  = -1;

      using Segments = std::array<std::optional<DeRich2SphMirrorSegObject>, Rich2::NumRich2SphMirrorSegSide>;
      Segments m_Rich2SphMirrorSegDetElem;

      DeRich2SphMirrorMasterObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );
      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& segment : m_Rich2SphMirrorSegDetElem ) {
          if ( segment.has_value() ) func( &segment.value() );
        };
      }
    };

  } // End namespace detail

  struct DeRich2SphMirrorSeg : detail::DeRichMirrorSegElement<detail::DeRich2SphMirrorSegObject> {
    using DeRichMirrorSegElement::DeRichMirrorSegElement;
    void PrintReflectivityFromDB() const noexcept { this->access()->PrintR2M1ReflectivityFromDBInObj(); }
  };

  struct DeRich2SphMirrorMaster : DeIOVElement<detail::DeRich2SphMirrorMasterObject> {
    using DeIOVElement::DeIOVElement;

    auto SphMMSN() const noexcept { return this->access()->m_Rich2SphMMSN; }
    auto SphMMCopyIdRef() const noexcept { return this->access()->m_Rich2SphMMCopyIdRef; }

    auto NumSphMirrorSegTotal() const noexcept { return this->access()->m_Rich2NumSphMirrorSegTotal; }
    auto NumSphMirrorSegSide() const noexcept { return this->access()->m_Rich2NumSphMirrorSegSide; }
    auto SphMirrorSeg( int aS ) const noexcept {
      assert( this->access()->m_Rich2SphMirrorSegDetElem[aS].has_value() );
      return DeRich2SphMirrorSeg( &( this->access()->m_Rich2SphMirrorSegDetElem[aS].value() ) );
    }
    auto SphMirrorSegVect() const noexcept {
      std::vector<DeRich2SphMirrorSeg> aRich2SphMSegVect( NumSphMirrorSegSide() );
      for ( int i = 0; i < ( NumSphMirrorSegSide() ); i++ ) { aRich2SphMSegVect[i] = SphMirrorSeg( i ); }
      return aRich2SphMSegVect;
    }
  };
} // End namespace LHCb::Detector
