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
#include <cassert>
#include <cstdint>
#include <optional>

namespace LHCb::Detector {

  namespace detail {
    /**
     *  Rich2 Sec Mirror detector element data
     *  \author  Sajan Easo
     *  \date    2022-02-02
     *  \version  1.0
     */

    struct DeRich2SecMirrorSegObject : DeRichMirrorSegObject {
      void PrintR2M2ReflectivityFromDBInObj();
      DeRich2SecMirrorSegObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );
    };

    struct DeRich2SecMirrorMasterObject : DeIOVObject {
      std::int32_t m_Rich2SecMirrorMasterCopyIdRef = -1;
      std::int32_t m_Rich2SecMirrorMasterN         = -1;
      std::int32_t m_Rich2SecMirrorNumSegTotal     = -1;
      std::int32_t m_Rich2SecMirrorNumSegSide      = -1;

      using Mirrors = std::array<std::optional<DeRich2SecMirrorSegObject>, Rich2::NumRich2SecMirrorSegSide>;
      Mirrors m_Rich2SecMirrorSegDetElem;

      DeRich2SecMirrorMasterObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );
      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& mirror : m_Rich2SecMirrorSegDetElem ) {
          if ( mirror.has_value() ) func( &mirror.value() );
        };
      }
    };

  } // end namespace detail

  struct DeRich2SecMirrorSeg : detail::DeRichMirrorSegElement<detail::DeRich2SecMirrorSegObject> {
    using DeRichMirrorSegElement::DeRichMirrorSegElement;
    void PrintReflectivityFromDB() const noexcept { this->access()->PrintR2M2ReflectivityFromDBInObj(); }
  };

  struct DeRich2SecMirrorMaster : DeIOVElement<detail::DeRich2SecMirrorMasterObject> {

    using DeIOVElement::DeIOVElement;
    auto SecMirrorMasterN() const noexcept { return this->access()->m_Rich2SecMirrorMasterN; }
    auto SecMirrorMasterCopyIdRef() const noexcept { return this->access()->m_Rich2SecMirrorMasterCopyIdRef; }
    auto SecMirrorNumSegTotal() const noexcept { return this->access()->m_Rich2SecMirrorNumSegTotal; }
    auto SecMirrorNumSegSide() const noexcept { return this->access()->m_Rich2SecMirrorNumSegSide; }

    auto SecMirrorSeg( int aS ) const noexcept {
      assert( this->access()->m_Rich2SecMirrorSegDetElem[aS].has_value() );
      return DeRich2SecMirrorSeg( &( this->access()->m_Rich2SecMirrorSegDetElem[aS].value() ) );
    }
    auto SecMirrorSegVect() const noexcept {
      std::vector<DeRich2SecMirrorSeg> aRich2SecMSegVect( SecMirrorNumSegSide() );
      for ( int i = 0; i < ( SecMirrorNumSegSide() ); i++ ) { aRich2SecMSegVect[i] = SecMirrorSeg( i ); }
      return aRich2SecMSegVect;
    }
  };
} // End namespace LHCb::Detector
