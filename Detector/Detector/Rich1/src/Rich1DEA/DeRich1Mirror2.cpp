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
/**  DeRich1Mirror2 detector element implementation in DD4HEP    SE 2022-02-01    ***/
/************************************************************************************/

#include "Detector/Rich1/DetElemAccess/DeRich1Mirror2.h"
#include "Detector/Rich1/RichDD4HepAccessUtil.h"
#include "Detector/Rich1/RichGeoUtil.h"
#include "Detector/Rich1/RichSurfaceUtil.h"

#include "DD4hep/Printout.h"

using namespace LHCb::Detector::detail;

namespace {
  template <typename NUM>
  auto getSurface( const NUM n ) {
    auto aRichSurfaceUtil      = RichSurfaceUtil::getRichSurfaceUtilInstance();
    auto aRichDD4HepAccessUtil = RichDD4HepAccessUtil::getRichDD4HepAccessUtilInstance();
    return aRichDD4HepAccessUtil->GetAnOpticalSurface( aRichSurfaceUtil->Rich1Mirror2SurfaceName( n ) );
  }
} // namespace

DeRich1Mirror2SegObject::DeRich1Mirror2SegObject( const dd4hep::DetElement&             de,
                                                  dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeRichMirrorSegObject( de, ctxt ) {

  // mirror number
  m_mirrorNumber = de.id();

  // mirror side
  m_mirrorSide = Rich::Side( de.parent().id() - detail::dd4hep_param<int>( "pvRich1Mirror2MasterTopCopyNumber" ) );

  // XY sizes
  detail::init_param( m_SegmentXSize, "Rh1Mirror2SingleXSize" );
  detail::init_param( m_SegmentYSize, "Rh1Mirror2SingleYSize" );

  // RoC value.
  // For now the values are directly accessed from the
  // parameters in the GeomParam area. This may be modified in the future.
  const auto aR1M2SegS      = std::to_string( m_mirrorNumber );
  const auto aR1M2SegNumStr = ( m_mirrorNumber < 10 ) ? ( "0" + aR1M2SegS ) : aR1M2SegS;
  detail::init_param( m_SegmentROC, "Rh1Mirror2InnerR" + aR1M2SegNumStr );

  // reflectivity
  auto aRichSurfaceUtil = RichSurfaceUtil::getRichSurfaceUtilInstance();
  m_Reflectivity = aRichSurfaceUtil->RetrieveRichSurfProperty( getSurface( m_mirrorNumber ), RichSurfPropType::Refl );

  // initialise the common data in DeRichMirror. Must be done last...
  commonMirrorInit();
}

DeRich1Mirror2MasterObject::DeRich1Mirror2MasterObject( const dd4hep::DetElement&             de,
                                                        dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeIOVObject( de, ctxt ) {
  detail::init_param( m_Rich1Mirror2MasterCopyIdRef, "pvRich1Mirror2MasterTopCopyNumber" );
  detail::init_param( m_Rich1Mirror2NumSegTotal, "Rh1NumberOfMirror2Segments" );
  detail::init_param( m_Rich1Mirror2NumSegSide, "Rh1NumberOfMirror2SegmentsInaHalf" );
  m_Rich1Mirror2MasterN = ( de.id() - m_Rich1Mirror2MasterCopyIdRef );
  auto R1M2DL           = RichGeoUtil::getRichGeoUtilInstance()->Rich1Mirror2ListDE( de, ctxt, m_Rich1Mirror2MasterN );
  assert( R1M2DL.size() == m_Rich1Mirror2SegDetElem.size() );
  for ( std::size_t i = 0; i < m_Rich1Mirror2SegDetElem.size(); ++i ) {
    m_Rich1Mirror2SegDetElem[i].emplace( R1M2DL[i].first, R1M2DL[i].second );
  }
}

void LHCb::Detector::detail::DeRich1Mirror2SegObject::PrintR1M2ReflectivityFromDBInObj() {
  auto aRichSurfaceUtil = RichSurfaceUtil::getRichSurfaceUtilInstance();
  aRichSurfaceUtil->PrintRichSurfProperty( getSurface( m_mirrorNumber ), RichSurfPropType::Refl );
}
