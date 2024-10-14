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
#include "Detector/Rich/DeRichPDModule.h"
#include "Detector/Rich/Rich_Geom_Constants_De.h"
#include "Detector/Rich2/DetElemAccess/DeRich2Mapmt.h"
#include "boost/container/static_vector.hpp"
#include <optional>

namespace LHCb::Detector {

  //====================================================================================//
  namespace detail {
    /**
     *  Rich2 Mapmt Module  data
     *  \author  Sajan Easo
     *  \date    2022-02-10
     *  \version  1.0
     */

    struct DeRich2StdPDModuleObject : DeRichPDModuleObject {
      using PMTs = boost::container::static_vector<std::optional<DeRich2StdMapmtObject>, //
                                                   Rich1::MaxNumRich1MapmtsInStdModule>;
      PMTs m_Rich2StdMapmtDetElem;
      DeRich2StdPDModuleObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );
      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& pmt : m_Rich2StdMapmtDetElem ) {
          if ( pmt.has_value() ) func( &pmt.value() );
        };
      }
    };

    struct DeRich2GrandPDModuleObject : DeRichPDModuleObject {
      using PMTs = boost::container::static_vector<std::optional<DeRich2GrandMapmtObject>, //
                                                   Rich2::MaxNumRich2MapmtsInGrandModule>;
      PMTs m_Rich2GrandMapmtDetElem;
      DeRich2GrandPDModuleObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );
      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& pmt : m_Rich2GrandMapmtDetElem ) {
          if ( pmt.has_value() ) func( &pmt.value() );
        };
      }
    };

  } // End namespace detail

  struct DeRich2StdPDModule : detail::DeRichPDModuleElement<detail::DeRich2StdPDModuleObject> {
    using DeRichPDModuleElement::DeRichPDModuleElement;
    auto Mapmt( const unsigned int aP ) const noexcept {
      assert( aP < this->access()->m_Rich2StdMapmtDetElem.size() );
      assert( this->access()->m_Rich2StdMapmtDetElem[aP].has_value() );
      return DeRich2StdMapmt( &( this->access()->m_Rich2StdMapmtDetElem[aP].value() ) );
    }
    auto Mapmts() const noexcept {
      std::vector<DeRich2StdMapmt> pmts;
      const auto                   nPmts = this->access()->m_Rich2StdMapmtDetElem.size();
      pmts.reserve( nPmts );
      for ( std::size_t i = 0; i < nPmts; ++i ) { pmts.emplace_back( this->Mapmt( i ) ); }
      return pmts;
    }
  };

  struct DeRich2GrandPDModule : detail::DeRichPDModuleElement<detail::DeRich2GrandPDModuleObject> {
    using DeRichPDModuleElement::DeRichPDModuleElement;
    auto Mapmt( const unsigned int aP ) const noexcept {
      assert( aP < this->access()->m_Rich2GrandMapmtDetElem.size() );
      assert( this->access()->m_Rich2GrandMapmtDetElem[aP].has_value() );
      return DeRich2GrandMapmt( &( this->access()->m_Rich2GrandMapmtDetElem[aP].value() ) );
    }
    auto Mapmts() const noexcept {
      std::vector<DeRich2GrandMapmt> pmts;
      const auto                     nPmts = this->access()->m_Rich2GrandMapmtDetElem.size();
      pmts.reserve( nPmts );
      for ( std::size_t i = 0; i < nPmts; ++i ) { pmts.emplace_back( this->Mapmt( i ) ); }
      return pmts;
    }
  };
} // End namespace LHCb::Detector
