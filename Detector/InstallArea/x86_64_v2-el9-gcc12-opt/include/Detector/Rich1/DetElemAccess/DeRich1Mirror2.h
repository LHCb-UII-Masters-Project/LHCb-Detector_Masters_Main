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
     *  Rich1 detector element data
     *  \author  Sajan Easo
     *  \date    2022-02-01
     *  \version  1.0
     */

    struct DeRich1Mirror2SegObject : DeRichMirrorSegObject {
      void PrintR1M2ReflectivityFromDBInObj();
      DeRich1Mirror2SegObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );
    };

    struct DeRich1Mirror2MasterObject : DeIOVObject {
      std::int32_t m_Rich1Mirror2MasterCopyIdRef = -1;
      std::int32_t m_Rich1Mirror2MasterN         = -1;
      std::int32_t m_Rich1Mirror2NumSegTotal     = -1;
      std::int32_t m_Rich1Mirror2NumSegSide      = -1;

      using Mirrors = std::array<std::optional<DeRich1Mirror2SegObject>, Rich1::NumRich1Mirror2SegSide>;
      Mirrors m_Rich1Mirror2SegDetElem;

      DeRich1Mirror2MasterObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );
      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& mirror : m_Rich1Mirror2SegDetElem ) {
          if ( mirror.has_value() ) func( &mirror.value() );
        };
      }
    };

  } // End namespace detail

  struct DeRich1Mirror2Seg : detail::DeRichMirrorSegElement<detail::DeRich1Mirror2SegObject> {
    using DeRichMirrorSegElement::DeRichMirrorSegElement;
    void PrintReflectivityFromDB() const noexcept { this->access()->PrintR1M2ReflectivityFromDBInObj(); }
  };

  struct DeRich1Mirror2Master : DeIOVElement<detail::DeRich1Mirror2MasterObject> {
    using DeIOVElement::DeIOVElement;

    auto Mirror2MasterCopyIdRef() const noexcept { return this->access()->m_Rich1Mirror2MasterCopyIdRef; }
    auto Mirror2MasterN() const noexcept { return this->access()->m_Rich1Mirror2MasterN; }
    auto Mirror2NumSegTotal() const noexcept { return this->access()->m_Rich1Mirror2NumSegTotal; }
    auto Mirror2NumSegSide() const noexcept { return this->access()->m_Rich1Mirror2NumSegSide; }

    auto Mirror2Seg( int aS ) const noexcept {
      assert( this->access()->m_Rich1Mirror2SegDetElem[aS].has_value() );
      return DeRich1Mirror2Seg( &( this->access()->m_Rich1Mirror2SegDetElem[aS].value() ) );
    }
    auto Mirror2SegVect() const noexcept {
      std::vector<DeRich1Mirror2Seg> aRich1Mirror2SegVect( Mirror2NumSegSide() );
      for ( int i = 0; i < Mirror2NumSegSide(); ++i ) { aRich1Mirror2SegVect[i] = Mirror2Seg( i ); }
      return aRich1Mirror2SegVect;
    }
  };
} // namespace LHCb::Detector
