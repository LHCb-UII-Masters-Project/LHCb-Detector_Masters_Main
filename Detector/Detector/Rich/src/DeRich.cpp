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

#include "Detector/Rich/DeRich.h"
#include "Detector/Rich1/RichDD4HepAccessUtil.h"
#include "Detector/Rich1/RichMatOPD.h"
#include "Detector/Rich1/RichSurfaceUtil.h"

using namespace LHCb::Detector::detail;

DeRichObject::DeRichObject( const Rich::DetectorType              rich, //
                            const dd4hep::DetElement&             de,   //
                            dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeIOVObject( de, ctxt ), m_rich( rich ) {

  assert( Rich::InvalidDetector != m_rich );

  auto aRichDD4HepAccessUtil = RichDD4HepAccessUtil::getRichDD4HepAccessUtilInstance();
  auto aRichMatOpd           = RichMatOPD::getRichMatOPDInstance();
  auto aRichSurfaceUtil      = RichSurfaceUtil::getRichSurfaceUtilInstance();

  const auto pmtTableName = aRichMatOpd->GetQEGeneralTableName( RichMatPropType::pmtQE, RichMatQE::qePmtNominal );
  m_NominalPmtQE = aRichMatOpd->GetARichGeneralGDMLMatrix( aRichDD4HepAccessUtil->GetGDMLTable( pmtTableName ) );

  const auto gasTableName = aRichMatOpd->GetTableName( RichMatNameWOP::R1GasQw, RichMatPropType::AbLe );
  m_QWNominalAbsLen = aRichMatOpd->GetARichGeneralGDMLMatrix( aRichDD4HepAccessUtil->GetGDMLTable( gasTableName ) );

  m_SphMNominalRefl = aRichSurfaceUtil->RetrieveRichSurfProperty(
      aRichDD4HepAccessUtil->GetAnOpticalSurface( Rich::Rich1 == m_rich
                                                      ? aRichSurfaceUtil->Rich1Mirror1NominalSurfaceName()
                                                      : aRichSurfaceUtil->Rich2Mirror1NominalSurfaceName() ),
      RichSurfPropType::Refl );

  m_SecMNominalRefl = aRichSurfaceUtil->RetrieveRichSurfProperty(
      aRichDD4HepAccessUtil->GetAnOpticalSurface( Rich::Rich1 == m_rich
                                                      ? aRichSurfaceUtil->Rich1Mirror2NominalSurfaceName()
                                                      : aRichSurfaceUtil->Rich2Mirror2NominalSurfaceName() ),
      RichSurfPropType::Refl );
}
