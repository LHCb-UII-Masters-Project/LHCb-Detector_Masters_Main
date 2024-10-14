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
/**  DeRich1Mapmt detector element implementation in DD4HEP    SE 2022-02-10    ***/
/************************************************************************************/

#include "Detector/Rich1/DetElemAccess/DeRich1Mapmt.h"
#include "DD4hep/Printout.h"
#include "Detector/Rich1/RichDD4HepAccessUtil.h"
#include "Detector/Rich1/RichPmtGeoAux.h"

using namespace LHCb::Detector::detail;

DeRich1MapmtObject::DeRich1MapmtObject( const dd4hep::DetElement&             de, //
                                        dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeRichMapmtObject( de, ctxt ) {
  m_MapmtCopyNum     = de.id();
  auto pmtAux        = RichPmtGeoAux::getRichPmtGeoAuxInstance();
  m_MapmtSide        = pmtAux->Rich1PmtSideFromCopyNum( m_MapmtCopyNum );
  m_MapmtNumInModule = pmtAux->getRichPmtNumInStdModuleFromPmtCopyNum( m_MapmtCopyNum );
}
