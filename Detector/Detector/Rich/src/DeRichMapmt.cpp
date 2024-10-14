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

#include "Detector/Rich/DeRichMapmt.h"
#include "Detector/Rich1/RichDD4HepAccessUtil.h"
#include "Detector/Rich1/RichMatOPD.h"

using namespace LHCb::Detector::detail;

DeRichMapmtObject::DeRichMapmtObject( const dd4hep::DetElement&             de, //
                                      dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeIOVObject( de, ctxt ) {
  // Set the PMT QE table. For now just set to the nominal table.
  // Sajan - This should be eventually be updated to set the appropriate
  //         (scaled) table for each PMT.
  auto       aRichDD4HepAccessUtil = RichDD4HepAccessUtil::getRichDD4HepAccessUtilInstance();
  auto       aRichMatOpd           = RichMatOPD::getRichMatOPDInstance();
  const auto pmtTableName = aRichMatOpd->GetQEGeneralTableName( RichMatPropType::pmtQE, RichMatQE::qePmtNominal );
  m_PmtQE = aRichMatOpd->GetARichGeneralGDMLMatrix( aRichDD4HepAccessUtil->GetGDMLTable( pmtTableName ) );
  // Pixel area for R-Type PMTs. Over-written later on for H-Type ones in specific constructor
  using FP       = decltype( m_effPixelArea );
  m_effPixelArea = detail::dd4hep_param<FP>( "RhPMTPixelXSize" ) * detail::dd4hep_param<FP>( "RhPMTPixelYSize" );
  detail::init_param( m_numPixels, "RichPmtTotalNumberofPixels" );
}
