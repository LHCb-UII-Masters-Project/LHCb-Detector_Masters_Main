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
/**  DeRich1PhDetPanel detector element implementation in DD4HEP    SE 2022-02-10    ***/
/************************************************************************************/

#include "Detector/Rich1/DetElemAccess/DeRich1PhDetPanel.h"
#include "Detector/Rich/Utilities.h"
#include "Detector/Rich1/RichDD4HepAccessUtil.h"
#include "Detector/Rich1/RichGeoUtil.h"
#include "Detector/Rich1/RichPmtGeoAux.h"

using namespace LHCb::Detector::detail;

DeRich1PhDetPanelObject::DeRich1PhDetPanelObject( const dd4hep::DetElement&             de,
                                                  dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeRichPhDetPanelObject( de, ctxt ) {

  detail::init_param( m_NumMapmtEdgeModulesInAPanel, "Rh1NumPMTEdgeNonOuterCornerModulesInPanel" );
  detail::init_param( m_NumMapmtOuterCornerModulesInAPanel, "Rh1NumPMTOuterCornerModulesInPanel" );
  detail::init_param( m_PhDetPanelCopyIdRef, "pvRich1PhDetSupFrameH0CopyNumber" );
  detail::init_param( m_NumMapmtModulesInAPanel, "Rh1TotalNumPMTModulesInPanel" );
  detail::init_param( m_NumMapmtStdModulesInAPanel, "Rh1TotalNumPMTStdModulesInPanel" );

  m_PhDetPanelN = de.id() - m_PhDetPanelCopyIdRef;

  const auto pmtAux           = RichPmtGeoAux::getRichPmtGeoAuxInstance();
  const auto R1StdPDM         = pmtAux->Rich1PDModuleListDE( de, ctxt, m_PhDetPanelN, 0 );
  const auto R1EdgePDM        = pmtAux->Rich1PDModuleListDE( de, ctxt, m_PhDetPanelN, 1 );
  const auto R1OuterCornerPDM = pmtAux->Rich1PDModuleListDE( de, ctxt, m_PhDetPanelN, 2 );

  detail::print( *this );

  assert( m_Rich1StdPDModuleDetElem.size() == R1StdPDM.size() );
  for ( std::size_t i = 0; i < R1StdPDM.size(); ++i ) {
    m_Rich1StdPDModuleDetElem[i].emplace( R1StdPDM[i].first, R1StdPDM[i].second );
  }

  assert( m_Rich1EdgePDModuleDetElem.size() == R1EdgePDM.size() );
  for ( std::size_t i = 0; i < R1EdgePDM.size(); ++i ) {
    m_Rich1EdgePDModuleDetElem[i].emplace( R1EdgePDM[i].first, R1EdgePDM[i].second );
  }

  assert( m_Rich1OuterCornerPDModuleDetElem.size() == R1OuterCornerPDM.size() );
  for ( std::size_t i = 0; i < R1OuterCornerPDM.size(); ++i ) {
    m_Rich1OuterCornerPDModuleDetElem[i].emplace( R1OuterCornerPDM[i].first, R1OuterCornerPDM[i].second );
  }
}
