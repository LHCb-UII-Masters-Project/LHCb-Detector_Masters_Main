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
#include <array>
#include <cstdint>

namespace LHCb::Detector {

  //====================================================================================//

  namespace detail {
    /**
     *  Rich1 detector element data
     *  \author  Sajan Easo
     *  \date    2021-10-01
     *  \version  1.0
     */

    struct DeRich1Mirror1SegObject : DeRichMirrorSegObject {
      void PrintR1M1ReflectivityFromDBInObj();
      DeRich1Mirror1SegObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );
    };

    struct DeRich1Mirror1EnvObject : DeIOVObject {
      std::int32_t            m_Rich1Mirror1QN = -1;
      DeRich1Mirror1SegObject m_Rich1Mirror1SegDetElem;
      DeRich1Mirror1EnvObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );
      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        func( &m_Rich1Mirror1SegDetElem );
      }
    };

  } // End namespace detail

  struct DeRich1Mirror1Seg : detail::DeRichMirrorSegElement<detail::DeRich1Mirror1SegObject> {
    using DeRichMirrorSegElement::DeRichMirrorSegElement;
    void PrintReflectivityFromDB() const noexcept { this->access()->PrintR1M1ReflectivityFromDBInObj(); }
  };

  struct DeRich1Mirror1Env : DeIOVElement<detail::DeRich1Mirror1EnvObject> {
    using DeIOVElement::DeIOVElement;
    auto mirrorNumber() const noexcept { return this->access()->m_Rich1Mirror1QN; }
    auto MirrorSeg() const noexcept { return DeRich1Mirror1Seg( &( this->access()->m_Rich1Mirror1SegDetElem ) ); }
    auto LocalCOC() const noexcept { return MirrorSeg().LocalCOC(); }
    auto GlobalCOC() const noexcept { return MirrorSeg().GlobalCOC(); }
  };
} // End namespace LHCb::Detector
