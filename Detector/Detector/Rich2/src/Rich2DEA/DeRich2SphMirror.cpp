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
/**  DeRich2 Spherical mirror detector element implementation in DD4HEP  ****/
/**  SE 2022-01-12    ***/
/*****************************************************************************/

#include "Detector/Rich2/DetElemAccess/DeRich2SphMirror.h"
#include "Detector/Rich1/RichDD4HepAccessUtil.h"
#include "Detector/Rich1/RichSurfaceUtil.h"
#include "Detector/Rich2/Rich2MirrorGeoAux.h"

#include "DD4hep/Printout.h"

using namespace LHCb::Detector::detail;

namespace {
  template <typename NUM>
  auto getSurface( const NUM n ) {
    auto aRichSurfaceUtil      = RichSurfaceUtil::getRichSurfaceUtilInstance();
    auto aRichDD4HepAccessUtil = RichDD4HepAccessUtil::getRichDD4HepAccessUtilInstance();
    return aRichDD4HepAccessUtil->GetAnOpticalSurface( aRichSurfaceUtil->Rich2Mirror1SurfaceName( n ) );
  }
} // namespace

DeRich2SphMirrorSegObject::DeRich2SphMirrorSegObject( const dd4hep::DetElement&             de,
                                                      dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeRichMirrorSegObject( de, ctxt ) {

  // mirror number
  m_mirrorNumber = de.id();

  // mirror side
  m_mirrorSide = Rich::Side( de.parent().id() - detail::dd4hep_param<int>( "pvRich2SphMirrorMaster0CopyNumber" ) );

  // XY sizes
  // m_SegmentXSize = ... ?
  // m_SegmentYSize = ... ?

  // ROC Value
  // The following is done through the looping over the boolean operations. May be improved in the future.
  // const dd4hep::BooleanSolid& aBoolShape = detector.solid();
  // As done in the Rich1 case, normally one may go through the reverse of the boolean operations which
  // used to create each segment, to reach the original spherical segment to find its ROC. For the hexagons and half
  // hexagons these are different list of operations, in this context. For now the values are directly accessed from the
  // parameters in the GeomParam area. This may be modified in the future.
  const auto aR2M1SegS      = std::to_string( m_mirrorNumber );
  const auto aR2M1SegNumStr = ( m_mirrorNumber < 10 ? "0" + aR2M1SegS : aR2M1SegS );
  detail::init_param( m_SegmentROC, "Rh2SphMirror" + aR2M1SegNumStr + "InnerROC" );

  // reflectivity
  auto aRichSurfaceUtil = RichSurfaceUtil::getRichSurfaceUtilInstance();
  m_Reflectivity = aRichSurfaceUtil->RetrieveRichSurfProperty( getSurface( m_mirrorNumber ), RichSurfPropType::Refl );

  // initialise the common data in DeRichMirror. Must be done last...
  commonMirrorInit();
}

DeRich2SphMirrorMasterObject::DeRich2SphMirrorMasterObject( const dd4hep::DetElement&             de,
                                                            dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeIOVObject( de, ctxt ) {
  detail::init_param( m_Rich2SphMMCopyIdRef, "pvRich2SphMirrorMaster0CopyNumber" );
  detail::init_param( m_Rich2NumSphMirrorSegTotal, "Rh2NumSphMirrorSegments" );
  m_Rich2NumSphMirrorSegSide = m_Rich2NumSphMirrorSegTotal / 2;
  m_Rich2SphMMSN             = de.id() - m_Rich2SphMMCopyIdRef;
  const auto geoAux          = Rich2MirrorGeoAux::getRich2MirrorGeoAuxInstance();
  const auto R2SphDL         = geoAux->Rich2SphMirrorListDE( de, ctxt, m_Rich2SphMMSN );
  assert( R2SphDL.size() == m_Rich2SphMirrorSegDetElem.size() );
  for ( std::size_t i = 0; i < m_Rich2SphMirrorSegDetElem.size(); ++i ) {
    m_Rich2SphMirrorSegDetElem[i].emplace( R2SphDL[i].first, R2SphDL[i].second );
  }
}

void DeRich2SphMirrorSegObject::PrintR2M1ReflectivityFromDBInObj() {
  auto aRichSurfaceUtil = RichSurfaceUtil::getRichSurfaceUtilInstance();
  aRichSurfaceUtil->PrintRichSurfProperty( getSurface( m_mirrorNumber ), RichSurfPropType::Refl );
}
