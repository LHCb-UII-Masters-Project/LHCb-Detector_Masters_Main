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
#include "Detector/Rich2/DetElemAccess/DeRich2PDModule.h"
#include <array>
#include <cassert>
#include <cstdint>
#include <optional>

namespace LHCb::Detector {

  namespace detail {
    /**
     *  Rich2 detector element data
     *  \author  Sajan Easo
     *  \date    2022-02-10
     *  \version  1.0
     */

    struct DeRich2PhDetPanelObject : DeRichPhDetPanelObject {
      std::int32_t m_NumMapmtGrandModulesInAPanel = -1;

      using StdMods = std::array<std::optional<DeRich2StdPDModuleObject>, Rich2::NumRich2MapmtStdModulesInAPanel>;
      StdMods m_Rich2StdPDModuleDetElem;
      using GrandMods = std::array<std::optional<DeRich2GrandPDModuleObject>, Rich2::NumRich2MapmtGrandModulesInAPanel>;
      GrandMods m_Rich2GrandPDModuleDetElem;

      DeRich2PhDetPanelObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );
      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& pmt : m_Rich2StdPDModuleDetElem ) {
          if ( pmt.has_value() ) func( &pmt.value() );
        };
        for ( auto& pmt : m_Rich2GrandPDModuleDetElem ) {
          if ( pmt.has_value() ) func( &pmt.value() );
        };
      }
    };

  } // End namespace detail

  struct DeRich2PhDetPanel : detail::DeRichPhDetPanelElement<detail::DeRich2PhDetPanelObject> {
    using DeRichPhDetPanelElement::DeRichPhDetPanelElement;

    auto NumMapmtGrandModulesInAPanel() const noexcept { return this->access()->m_NumMapmtGrandModulesInAPanel; }

    auto StdPDModule( const unsigned int aM ) const noexcept {
      assert( aM < this->access()->m_Rich2StdPDModuleDetElem.size() );
      assert( this->access()->m_Rich2StdPDModuleDetElem[aM].has_value() );
      return DeRich2StdPDModule( &( this->access()->m_Rich2StdPDModuleDetElem[aM].value() ) );
    }
    auto GrandPDModule( const unsigned int aM ) const noexcept {
      assert( aM < this->access()->m_Rich2GrandPDModuleDetElem.size() );
      assert( this->access()->m_Rich2GrandPDModuleDetElem[aM].has_value() );
      return DeRich2GrandPDModule( &( this->access()->m_Rich2GrandPDModuleDetElem[aM].value() ) );
    }

    /// Access all R type (standard) PMTs
    inline auto allPMTsR() const noexcept {
      std::vector<DeRich2StdMapmt> pmts;
      pmts.reserve( Rich1::MaxNumRich1MapmtsInStdModule * Rich2::NumRich2MapmtStdModulesInAPanel );
      // loop over all modules and fill PMTs
      for ( int i = 0; i < this->NumMapmtStdModulesInAPanel(); ++i ) {
        const auto module_pmts = StdPDModule( i ).Mapmts();
        pmts.insert( pmts.end(), module_pmts.begin(), module_pmts.end() );
      }
      return pmts;
    }

    /// Access all H type (grand) PMTs
    inline auto allPMTsH() const noexcept {
      std::vector<DeRich2GrandMapmt> pmts;
      pmts.reserve( Rich2::MaxNumRich2MapmtsInGrandModule * Rich2::NumRich2MapmtGrandModulesInAPanel );
      // loop over all modules and fill PMTs
      for ( int i = 0; i < this->NumMapmtGrandModulesInAPanel(); ++i ) {
        const auto module_pmts = GrandPDModule( i ).Mapmts();
        pmts.insert( pmts.end(), module_pmts.begin(), module_pmts.end() );
      }
      return pmts;
    }
  };
} // End namespace LHCb::Detector
