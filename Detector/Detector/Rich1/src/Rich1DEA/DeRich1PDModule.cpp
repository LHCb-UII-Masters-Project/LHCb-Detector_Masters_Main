/*****************************************************************************\
* (c) Copyright 2000-2019 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
/**  DeRich1PDModule detector element implementation in DD4HEP    SE 2022-02-10    ***/
/************************************************************************************/

#include "Detector/Rich1/DetElemAccess/DeRich1PDModule.h"
#include "Detector/Rich1/RichDD4HepAccessUtil.h"
#include "Detector/Rich1/RichPmtGeoAux.h"

using namespace LHCb::Detector::detail;

DeRich1StdPDModuleObject::DeRich1StdPDModuleObject( const dd4hep::DetElement&             de,
                                                    dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeRichPDModuleObject( de, ctxt ) {
  m_PDModuleCopyNum = de.id();
  detail::init_param( m_NumMapmtsInModule, "Rh1NumPMTInStdModule" );
  const auto auxPmt     = RichPmtGeoAux::getRichPmtGeoAuxInstance();
  m_PDModuleSide        = auxPmt->Rich1ModuleSideFromCopyNum( m_PDModuleCopyNum );
  m_PDPanelLocalModuleN = auxPmt->Rich1ModuleNumInPanelFromCopyNum( m_PDModuleCopyNum );
  const auto R1StdPmt   = auxPmt->Rich1MapmtListInModuleDE( de, ctxt, m_PDModuleCopyNum );
  assert( R1StdPmt.size() <= m_Rich1StdMapmtDetElem.capacity() );
  m_Rich1StdMapmtDetElem.clear();
  m_Rich1StdMapmtDetElem.resize( R1StdPmt.size() );
  detail::print( *this );
  for ( std::size_t i = 0; i < m_Rich1StdMapmtDetElem.size(); ++i ) {
    auto& pmt                    = m_Rich1StdMapmtDetElem[i].emplace( R1StdPmt[i].first, R1StdPmt[i].second );
    pmt.m_MapmtPanelLocalModuleN = m_PDPanelLocalModuleN;
    pmt.m_MapmtModuleCopyNum     = m_PDModuleCopyNum;
    detail::print( pmt );
  }
}

DeRich1EdgePDModuleObject::DeRich1EdgePDModuleObject( const dd4hep::DetElement&             de,
                                                      dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeRichPDModuleObject( de, ctxt ) {
  m_PDModuleCopyNum = de.id();
  detail::init_param( m_NumMapmtsInModule, "Rh1NumPMTInEdgeModule" );
  const auto auxPmt     = RichPmtGeoAux::getRichPmtGeoAuxInstance();
  m_PDModuleSide        = auxPmt->Rich1ModuleSideFromCopyNum( m_PDModuleCopyNum );
  m_PDPanelLocalModuleN = auxPmt->Rich1ModuleNumInPanelFromCopyNum( m_PDModuleCopyNum );
  const auto R1EdgePmt  = auxPmt->Rich1MapmtListInModuleDE( de, ctxt, m_PDModuleCopyNum );
  assert( R1EdgePmt.size() <= m_Rich1EdgeMapmtDetElem.capacity() );
  m_Rich1EdgeMapmtDetElem.clear();
  m_Rich1EdgeMapmtDetElem.resize( R1EdgePmt.size() );
  detail::print( *this );
  for ( std::size_t i = 0; i < m_Rich1EdgeMapmtDetElem.size(); ++i ) {
    auto& pmt                    = m_Rich1EdgeMapmtDetElem[i].emplace( R1EdgePmt[i].first, R1EdgePmt[i].second );
    pmt.m_MapmtPanelLocalModuleN = m_PDPanelLocalModuleN;
    pmt.m_MapmtModuleCopyNum     = m_PDModuleCopyNum;
    detail::print( pmt );
  }
}

DeRich1OuterCornerPDModuleObject::DeRich1OuterCornerPDModuleObject( const dd4hep::DetElement&             de,
                                                                    dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeRichPDModuleObject( de, ctxt ) {
  m_PDModuleCopyNum = de.id();
  detail::init_param( m_NumMapmtsInModule, "Rh1NumPMTInOuterCornerModule" );
  const auto auxPmt           = RichPmtGeoAux::getRichPmtGeoAuxInstance();
  m_PDModuleSide              = auxPmt->Rich1ModuleSideFromCopyNum( m_PDModuleCopyNum );
  m_PDPanelLocalModuleN       = auxPmt->Rich1ModuleNumInPanelFromCopyNum( m_PDModuleCopyNum );
  const auto R1OuterCornerPmt = auxPmt->Rich1MapmtListInModuleDE( de, ctxt, m_PDModuleCopyNum );
  assert( R1OuterCornerPmt.size() <= m_Rich1OuterCornerMapmtDetElem.capacity() );
  m_Rich1OuterCornerMapmtDetElem.clear();
  m_Rich1OuterCornerMapmtDetElem.resize( R1OuterCornerPmt.size() );
  detail::print( *this );
  for ( std::size_t i = 0; i < m_Rich1OuterCornerMapmtDetElem.size(); ++i ) {
    auto& pmt = m_Rich1OuterCornerMapmtDetElem[i].emplace( R1OuterCornerPmt[i].first, R1OuterCornerPmt[i].second );
    pmt.m_MapmtPanelLocalModuleN = m_PDPanelLocalModuleN;
    pmt.m_MapmtModuleCopyNum     = m_PDModuleCopyNum;
    detail::print( pmt );
  }
}
