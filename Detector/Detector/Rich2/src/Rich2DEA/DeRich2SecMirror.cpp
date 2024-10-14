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
/**  DeRich2 Sec mirror detector element implementation in DD4HEP  ****/
/**  SE 2022-02-02    ***/
/*****************************************************************************/

#include "Detector/Rich2/DetElemAccess/DeRich2SecMirror.h"
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
    return aRichDD4HepAccessUtil->GetAnOpticalSurface( aRichSurfaceUtil->Rich2Mirror2SurfaceName( n ) );
  }
} // namespace

DeRich2SecMirrorSegObject::DeRich2SecMirrorSegObject( const dd4hep::DetElement&             de,
                                                      dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeRichMirrorSegObject( de, ctxt ) {

  // mirror number
  m_mirrorNumber = de.id();

  // mirror side
  m_mirrorSide = Rich::Side( de.parent().id() - detail::dd4hep_param<int>( "pvRich2SecMirrorMaster0CopyNumber" ) );

  // XY sizes
  detail::init_param( m_SegmentXSize, "Rh2SecMirrorSegSizeX" );
  detail::init_param( m_SegmentYSize, "Rh2SecMirrorSegSizeY" );

  // RoC
  // For now the values are directly accessed from the
  // parameters in the GeomParam area. This may be modified in the future.
  const auto aR2M2SegS      = std::to_string( m_mirrorNumber );
  const auto aR2M2SegNumStr = ( m_mirrorNumber < 10 ) ? ( "0" + aR2M2SegS ) : aR2M2SegS;
  detail::init_param( m_SegmentROC, "Rh2SecMirror" + aR2M2SegNumStr + "InnerROC" );

  // reflectivity
  const auto aRichSurfaceUtil = RichSurfaceUtil::getRichSurfaceUtilInstance();
  m_Reflectivity = aRichSurfaceUtil->RetrieveRichSurfProperty( getSurface( m_mirrorNumber ), RichSurfPropType::Refl );

  // initialise the common data in DeRichMirror. Must be done last...
  commonMirrorInit();
}

DeRich2SecMirrorMasterObject::DeRich2SecMirrorMasterObject( const dd4hep::DetElement&             de,
                                                            dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeIOVObject( de, ctxt ) {
  detail::init_param( m_Rich2SecMirrorMasterCopyIdRef, "pvRich2SecMirrorMaster0CopyNumber" );
  detail::init_param( m_Rich2SecMirrorNumSegTotal, "Rh2NumSecMirrorSegments" );
  m_Rich2SecMirrorNumSegSide = m_Rich2SecMirrorNumSegTotal / 2;
  m_Rich2SecMirrorMasterN    = de.id() - m_Rich2SecMirrorMasterCopyIdRef;
  const auto geoAux          = Rich2MirrorGeoAux::getRich2MirrorGeoAuxInstance();
  const auto R2SecDL         = geoAux->Rich2SecMirrorListDE( de, ctxt, m_Rich2SecMirrorMasterN );
  assert( m_Rich2SecMirrorSegDetElem.size() == R2SecDL.size() );
  for ( std::size_t i = 0; i < m_Rich2SecMirrorSegDetElem.size(); ++i ) {
    m_Rich2SecMirrorSegDetElem[i].emplace( R2SecDL[i].first, R2SecDL[i].second );
  }
}

void LHCb::Detector::detail::DeRich2SecMirrorSegObject::PrintR2M2ReflectivityFromDBInObj() {
  const auto aRichSurfaceUtil = RichSurfaceUtil::getRichSurfaceUtilInstance();
  aRichSurfaceUtil->PrintRichSurfProperty( getSurface( m_mirrorNumber ), RichSurfPropType::Refl );
}
