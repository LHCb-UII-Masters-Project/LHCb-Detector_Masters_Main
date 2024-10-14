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
/**  DeRich2PDModule detector element implementation in DD4HEP    SE 2022-02-10    ***/
/************************************************************************************/

#include "Detector/Rich2/DetElemAccess/DeRich2PDModule.h"
#include "Detector/Rich1/RichDD4HepAccessUtil.h"
#include "Detector/Rich1/RichPmtGeoAux.h"

using namespace LHCb::Detector::detail;

DeRich2StdPDModuleObject::DeRich2StdPDModuleObject( const dd4hep::DetElement&             de,
                                                    dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeRichPDModuleObject( de, ctxt ) {
  detail::init_param( m_NumMapmtsInModule, "Rh1NumPMTInStdModule" );
  const auto auxPmt     = RichPmtGeoAux::getRichPmtGeoAuxInstance();
  m_PDModuleCopyNum     = auxPmt->Rich2PmtModuleCopyNumber( de.id() );
  m_PDModuleSide        = auxPmt->Rich2ModuleSideFromR2ModuleNum( de.id() );
  m_PDPanelLocalModuleN = auxPmt->Rich2ModuleNumInPanelFromR2ModuleNum( de.id() );
  const auto R2StdPmt   = auxPmt->Rich2MapmtListInModuleDE( de, ctxt, m_PDModuleCopyNum, 0 );
  assert( R2StdPmt.size() <= m_Rich2StdMapmtDetElem.capacity() );
  m_Rich2StdMapmtDetElem.clear();
  m_Rich2StdMapmtDetElem.resize( R2StdPmt.size() );
  detail::print( *this );
  for ( std::size_t i = 0; i < m_Rich2StdMapmtDetElem.size(); ++i ) {
    auto& pmt                    = m_Rich2StdMapmtDetElem[i].emplace( R2StdPmt[i].first, R2StdPmt[i].second );
    pmt.m_MapmtPanelLocalModuleN = m_PDPanelLocalModuleN;
    pmt.m_MapmtModuleCopyNum     = m_PDModuleCopyNum;
    detail::print( pmt );
  }
}

DeRich2GrandPDModuleObject::DeRich2GrandPDModuleObject( const dd4hep::DetElement&             de,
                                                        dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeRichPDModuleObject( de, ctxt ) {
  detail::init_param( m_NumMapmtsInModule, "RhNumGrandPMTInModule" );
  const auto auxPmt     = RichPmtGeoAux::getRichPmtGeoAuxInstance();
  m_PDModuleCopyNum     = auxPmt->Rich2PmtModuleCopyNumber( de.id() );
  m_PDModuleSide        = auxPmt->Rich2ModuleSideFromR2ModuleNum( de.id() );
  m_PDPanelLocalModuleN = auxPmt->Rich2ModuleNumInPanelFromR2ModuleNum( de.id() );
  const auto R2GrandPmt = auxPmt->Rich2MapmtListInModuleDE( de, ctxt, m_PDModuleCopyNum, 1 );
  assert( R2GrandPmt.size() <= m_Rich2GrandMapmtDetElem.capacity() );
  m_Rich2GrandMapmtDetElem.clear();
  m_Rich2GrandMapmtDetElem.resize( R2GrandPmt.size() );
  detail::print( *this );
  for ( std::size_t i = 0; i < m_Rich2GrandMapmtDetElem.size(); ++i ) {
    auto& pmt                    = m_Rich2GrandMapmtDetElem[i].emplace( R2GrandPmt[i].first, R2GrandPmt[i].second );
    pmt.m_MapmtPanelLocalModuleN = m_PDPanelLocalModuleN;
    pmt.m_MapmtModuleCopyNum     = m_PDModuleCopyNum;
    detail::print( pmt );
  }
}
