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

#include "Detector/Rich/DeRichPDModule.h"
#include "Detector/Rich/Rich_Geom_Constants_De.h"
#include "Detector/Rich1/DetElemAccess/DeRich1Mapmt.h"
#include "boost/container/static_vector.hpp"
#include <optional>
#include <vector>

namespace LHCb::Detector {

  //====================================================================================//
  namespace detail {
    /**
     *  Rich1 Mapmt Module data
     *  \author  Sajan Easo
     *  \date    2022-02-10
     *  \version  1.0
     */

    struct DeRich1StdPDModuleObject : DeRichPDModuleObject {
      using PMTs = boost::container::static_vector<std::optional<DeRich1MapmtObject>, //
                                                   Rich1::MaxNumRich1MapmtsInStdModule>;
      PMTs m_Rich1StdMapmtDetElem;
      DeRich1StdPDModuleObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );
      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& pmt : m_Rich1StdMapmtDetElem ) {
          if ( pmt.has_value() ) func( &pmt.value() );
        };
      }
    };

    struct DeRich1EdgePDModuleObject : DeRichPDModuleObject {
      using PMTs = boost::container::static_vector<std::optional<DeRich1MapmtObject>, //
                                                   Rich1::MaxNumRich1MapmtsInEdgeModule>;
      PMTs m_Rich1EdgeMapmtDetElem;
      DeRich1EdgePDModuleObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );
      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& pmt : m_Rich1EdgeMapmtDetElem ) {
          if ( pmt.has_value() ) func( &pmt.value() );
        };
      }
    };

    struct DeRich1OuterCornerPDModuleObject : DeRichPDModuleObject {
      using PMTs = boost::container::static_vector<std::optional<DeRich1MapmtObject>, //
                                                   Rich1::MaxNumRich1MapmtsInOuterCornerModule>;
      PMTs m_Rich1OuterCornerMapmtDetElem;
      DeRich1OuterCornerPDModuleObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );
      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& pmt : m_Rich1OuterCornerMapmtDetElem ) {
          if ( pmt.has_value() ) func( &pmt.value() );
        };
      }
    };

  } // End namespace detail

  struct DeRich1StdPDModule : detail::DeRichPDModuleElement<detail::DeRich1StdPDModuleObject> {
    using DeRichPDModuleElement::DeRichPDModuleElement;
    // extra test for valid pmt , avoided for now.
    // auto Mapmt ( int aP ) const noexcept {
    //  return (RichPmtGeoAux::getRichPmtGeoAuxInstance()->Rich1PmtModulePmtValid(Rich1StdPDModuleCopyNum(),aP)) ?
    //    (DeRich1Mapmt( &( this->access()->m_Rich1StdMapmtDetElem[aP] ) )) : 0   ;
    //  }
    auto Mapmt( const unsigned int aP ) const noexcept {
      assert( aP < this->access()->m_Rich1StdMapmtDetElem.size() );
      assert( this->access()->m_Rich1StdMapmtDetElem[aP].has_value() );
      return DeRich1Mapmt( &( this->access()->m_Rich1StdMapmtDetElem[aP].value() ) );
    }
    auto Mapmts() const noexcept {
      std::vector<DeRich1Mapmt> pmts;
      const auto                nPmts = this->access()->m_Rich1StdMapmtDetElem.size();
      pmts.reserve( nPmts );
      for ( std::size_t i = 0; i < nPmts; ++i ) { pmts.emplace_back( this->Mapmt( i ) ); }
      return pmts;
    }
  };

  struct DeRich1EdgePDModule : detail::DeRichPDModuleElement<detail::DeRich1EdgePDModuleObject> {
    using DeRichPDModuleElement::DeRichPDModuleElement;
    auto Mapmt( const unsigned int aP ) const noexcept {
      assert( aP < this->access()->m_Rich1EdgeMapmtDetElem.size() );
      assert( this->access()->m_Rich1EdgeMapmtDetElem[aP].has_value() );
      return DeRich1Mapmt( &( this->access()->m_Rich1EdgeMapmtDetElem[aP].value() ) );
    }
    auto Mapmts() const noexcept {
      std::vector<DeRich1Mapmt> pmts;
      const auto                nPmts = this->access()->m_Rich1EdgeMapmtDetElem.size();
      pmts.reserve( nPmts );
      for ( std::size_t i = 0; i < nPmts; ++i ) { pmts.emplace_back( this->Mapmt( i ) ); }
      return pmts;
    }
  };

  struct DeRich1OuterCornerPDModule : detail::DeRichPDModuleElement<detail::DeRich1OuterCornerPDModuleObject> {
    using DeRichPDModuleElement::DeRichPDModuleElement;
    auto Mapmt( const unsigned int aP ) const noexcept {
      assert( aP < this->access()->m_Rich1OuterCornerMapmtDetElem.size() );
      assert( this->access()->m_Rich1OuterCornerMapmtDetElem[aP].has_value() );
      return DeRich1Mapmt( &( this->access()->m_Rich1OuterCornerMapmtDetElem[aP].value() ) );
    }
    auto Mapmts() const noexcept {
      std::vector<DeRich1Mapmt> pmts;
      const auto                nPmts = this->access()->m_Rich1OuterCornerMapmtDetElem.size();
      pmts.reserve( nPmts );
      for ( std::size_t i = 0; i < nPmts; ++i ) { pmts.emplace_back( this->Mapmt( i ) ); }
      return pmts;
    }
  };
} // End namespace LHCb::Detector
