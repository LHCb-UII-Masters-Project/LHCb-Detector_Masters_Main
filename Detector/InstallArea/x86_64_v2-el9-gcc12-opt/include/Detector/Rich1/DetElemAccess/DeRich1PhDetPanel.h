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

#include "Detector/Rich/DeRichPhDetPanel.h"
#include "Detector/Rich/Rich_Geom_Constants_De.h"
#include "Detector/Rich1/DetElemAccess/DeRich1PDModule.h"
#include <array>
#include <cassert>
#include <cstdint>
#include <optional>

namespace LHCb::Detector {

  namespace detail {

    /**
     *  Rich1 detector element data
     *  \author  Sajan Easo
     *  \date    2022-02-10
     *  \version  1.0
     */

    struct DeRich1PhDetPanelObject : DeRichPhDetPanelObject {

      std::int32_t m_NumMapmtEdgeModulesInAPanel        = -1;
      std::int32_t m_NumMapmtOuterCornerModulesInAPanel = -1;

      // The following in three lists since the corresponding Mapmts are initialized by DD4HEP at the same time
      // Each list below has a different number of Mapmts in the corresponding Modules
      // DD4HEP does not allow null pointers to non-existant pmts.

      using StdMods = std::array<std::optional<DeRich1StdPDModuleObject>, Rich1::NumRich1MapmtStdModulesInAPanel>;
      StdMods m_Rich1StdPDModuleDetElem;
      using EdgeMods = std::array<std::optional<DeRich1EdgePDModuleObject>, Rich1::NumRich1MapmtEdgeModulesInAPanel>;
      EdgeMods m_Rich1EdgePDModuleDetElem;
      using CornerMods =
          std::array<std::optional<DeRich1OuterCornerPDModuleObject>, Rich1::NumRich1MapmtOuterCornerModulesInAPanel>;
      CornerMods m_Rich1OuterCornerPDModuleDetElem;

      DeRich1PhDetPanelObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );
      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& module : m_Rich1StdPDModuleDetElem ) {
          if ( module.has_value() ) func( &module.value() );
        };
        for ( auto& module : m_Rich1EdgePDModuleDetElem ) {
          if ( module.has_value() ) func( &module.value() );
        };
        for ( auto& module : m_Rich1OuterCornerPDModuleDetElem ) {
          if ( module.has_value() ) func( &module.value() );
        };
      }
    };

  } // End namespace detail

  struct DeRich1PhDetPanel : detail::DeRichPhDetPanelElement<detail::DeRich1PhDetPanelObject> {
    using DeRichPhDetPanelElement::DeRichPhDetPanelElement;

    auto NumMapmtEdgeModulesInAPanel() const noexcept { return this->access()->m_NumMapmtEdgeModulesInAPanel; }
    auto NumMapmtOuterCornerModulesInAPanel() const noexcept {
      return this->access()->m_NumMapmtOuterCornerModulesInAPanel;
    }

    auto StdPDModule( const unsigned int aM ) const noexcept {
      assert( aM < this->access()->m_Rich1StdPDModuleDetElem.size() );
      assert( this->access()->m_Rich1StdPDModuleDetElem[aM].has_value() );
      return DeRich1StdPDModule( &( this->access()->m_Rich1StdPDModuleDetElem[aM].value() ) );
    }
    auto EdgePDModule( const unsigned int aM ) const noexcept {
      assert( aM < this->access()->m_Rich1EdgePDModuleDetElem.size() );
      assert( this->access()->m_Rich1EdgePDModuleDetElem[aM].has_value() );
      return DeRich1EdgePDModule( &( this->access()->m_Rich1EdgePDModuleDetElem[aM].value() ) );
    }
    auto OuterCornerPDModule( const unsigned int aM ) const noexcept {
      assert( aM < this->access()->m_Rich1OuterCornerPDModuleDetElem.size() );
      assert( this->access()->m_Rich1OuterCornerPDModuleDetElem[aM].has_value() );
      return DeRich1OuterCornerPDModule( &( this->access()->m_Rich1OuterCornerPDModuleDetElem[aM].value() ) );
    }

    /// Access all R type (standard) PMTs
    inline auto allPMTsR() const noexcept {
      std::vector<DeRich1Mapmt> pmts;
      pmts.reserve( Rich1::MaxNumRich1MapmtsInStdModule * Rich1::NumRich1MapmtModulesInAPanel );
      // loop over all modules and fill PMTs
      for ( int i = 0; i < this->NumMapmtStdModulesInAPanel(); ++i ) {
        const auto module_pmts = StdPDModule( i ).Mapmts();
        pmts.insert( pmts.end(), module_pmts.begin(), module_pmts.end() );
      }
      for ( int i = 0; i < this->NumMapmtEdgeModulesInAPanel(); ++i ) {
        const auto module_pmts = EdgePDModule( i ).Mapmts();
        pmts.insert( pmts.end(), module_pmts.begin(), module_pmts.end() );
      }
      for ( int i = 0; i < this->NumMapmtOuterCornerModulesInAPanel(); ++i ) {
        const auto module_pmts = OuterCornerPDModule( i ).Mapmts();
        pmts.insert( pmts.end(), module_pmts.begin(), module_pmts.end() );
      }
      return pmts;
    }

    /// Return empty H type vector for RICH1
    inline auto allPMTsH() const noexcept { return std::vector<DeRich1Mapmt>{}; }
  };
} // End namespace LHCb::Detector
