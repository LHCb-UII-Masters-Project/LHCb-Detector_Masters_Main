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
/**  DeRich2PhDetPanel detector element implementation in DD4HEP    SE 2022-02-10    ***/
/************************************************************************************/

#include "Detector/Rich2/DetElemAccess/DeRich2PhDetPanel.h"
#include "Detector/Rich/Utilities.h"
#include "Detector/Rich1/RichDD4HepAccessUtil.h"
#include "Detector/Rich1/RichPmtGeoAux.h"

using namespace LHCb::Detector::detail;

DeRich2PhDetPanelObject::DeRich2PhDetPanelObject( const dd4hep::DetElement&             de,
                                                  dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeRichPhDetPanelObject( de, ctxt ) {

  detail::init_param( m_NumMapmtGrandModulesInAPanel, "Rh2TotalNumGrandPMTModulesInPanel" );
  detail::init_param( m_PhDetPanelCopyIdRef, "pvRich2PhDetPanel0CopyNumber" );
  detail::init_param( m_NumMapmtModulesInAPanel, "Rh2TotalNumPMTModulesInPanel" );
  detail::init_param( m_NumMapmtStdModulesInAPanel, "Rh2TotalNumStdPMTModulesInPanel" );
  m_PhDetPanelN = de.id() - m_PhDetPanelCopyIdRef;

  const auto pmtAux     = RichPmtGeoAux::getRichPmtGeoAuxInstance();
  const auto R2StdPDM   = pmtAux->Rich2PDModuleListDE( de, ctxt, m_PhDetPanelN, 0 );
  const auto R2GrandPDM = pmtAux->Rich2PDModuleListDE( de, ctxt, m_PhDetPanelN, 1 );

  detail::print( *this );

  assert( R2StdPDM.size() == m_Rich2StdPDModuleDetElem.size() );
  for ( std::size_t i = 0; i < R2StdPDM.size(); ++i ) {
    m_Rich2StdPDModuleDetElem[i].emplace( R2StdPDM[i].first, R2StdPDM[i].second );
  }

  assert( m_Rich2GrandPDModuleDetElem.size() == R2GrandPDM.size() );
  for ( std::size_t i = 0; i < R2GrandPDM.size(); ++i ) {
    m_Rich2GrandPDModuleDetElem[i].emplace( R2GrandPDM[i].first, R2GrandPDM[i].second );
  }
}
