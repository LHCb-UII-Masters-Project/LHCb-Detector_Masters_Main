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
/**  DeRich2Mapmt detector element implementation in DD4HEP    SE 2022-02-10    ***/
/************************************************************************************/

#include "Detector/Rich2/DetElemAccess/DeRich2Mapmt.h"
#include "DD4hep/Printout.h"
#include "Detector/Rich1/RichDD4HepAccessUtil.h"
#include "Detector/Rich1/RichPmtGeoAux.h"

using namespace LHCb::Detector::detail;

DeRich2StdMapmtObject::DeRich2StdMapmtObject( const dd4hep::DetElement&             de, //
                                              dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeRichMapmtObject( de, ctxt ) {
  m_MapmtCopyNum     = de.id();
  auto pmtAux        = RichPmtGeoAux::getRichPmtGeoAuxInstance();
  m_MapmtSide        = pmtAux->Rich2SideFromRich2PmtCopyNum( m_MapmtCopyNum );
  m_MapmtNumInModule = pmtAux->getRichPmtNumInStdModuleFromPmtCopyNum( m_MapmtCopyNum );
}

DeRich2GrandMapmtObject::DeRich2GrandMapmtObject( const dd4hep::DetElement&             de, //
                                                  dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeRichMapmtObject( de, ctxt ) {
  m_MapmtCopyNum     = de.id();
  auto pmtAux        = RichPmtGeoAux::getRichPmtGeoAuxInstance();
  m_MapmtSide        = pmtAux->Rich2SideFromRich2PmtCopyNum( m_MapmtCopyNum );
  m_MapmtNumInModule = pmtAux->getRichPmtNumInGrandModuleFromPmtCopyNum( m_MapmtCopyNum );
  // H-Type PMT specific settings, overriding the R-Type values set in the base constructor
  m_isHType = true;
  using F   = decltype( m_effPixelArea );
  m_effPixelArea =
      detail::dd4hep_param<F>( "RhGrandPMTPixelXSize" ) * detail::dd4hep_param<F>( "RhGrandPMTPixelYSize" );
  detail::init_param( m_numPixels, "RichGrandPmtTotalNumberofPixels" );
}
