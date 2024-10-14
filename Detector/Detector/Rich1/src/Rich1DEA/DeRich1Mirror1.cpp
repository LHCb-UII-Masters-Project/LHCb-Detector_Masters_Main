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
/**  DeRich1Mirror1 detector element implementation in DD4HEP    SE 2021-07-03    ***/
/*****************************************************************************/

#include "Detector/Rich1/DetElemAccess/DeRich1Mirror1.h"
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
    return aRichDD4HepAccessUtil->GetAnOpticalSurface( aRichSurfaceUtil->Rich1Mirror1SurfaceName( n ) );
  }
} // namespace

DeRich1Mirror1SegObject::DeRich1Mirror1SegObject( const dd4hep::DetElement&             de,
                                                  dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeRichMirrorSegObject( de, ctxt ) {

  // mirror number
  m_mirrorNumber = de.parent().id();

  // XY sizes
  detail::init_param( m_SegmentXSize, "Rh1Mirror1SizeAlongXAxis" );
  detail::init_param( m_SegmentYSize, "Rh1Mirror1SizeAlongYAxis" );

  // ROC
  // The following is done through the looping over the boolean operations. May be improved in the future.
  const dd4hep::BooleanSolid& aBoolShape = detector().solid();
  const dd4hep::BooleanSolid& s1         = aBoolShape.leftShape();
  const dd4hep::BooleanSolid& s2         = s1.leftShape();
  const dd4hep::BooleanSolid& s3         = s2.leftShape();
  const dd4hep::BooleanSolid& s4         = s3.leftShape();
  const dd4hep::Sphere&       s          = s4.leftShape();
  m_SegmentROC                           = s.rMin();

  // reflectivity
  auto aRichSurfaceUtil = RichSurfaceUtil::getRichSurfaceUtilInstance();
  m_Reflectivity = aRichSurfaceUtil->RetrieveRichSurfProperty( getSurface( m_mirrorNumber ), RichSurfPropType::Refl );

  // initialise the common data in DeRichMirror.
  commonMirrorInit();

  // mirror side. Must be done after common.
  m_mirrorSide = ( m_MirrorSurfCentrePtn.y() > 0 ? Rich::top : Rich::bottom );
}

DeRich1Mirror1EnvObject::DeRich1Mirror1EnvObject( const dd4hep::DetElement&             de,
                                                  dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeIOVObject( de, ctxt )
    , m_Rich1Mirror1QN( de.id() )
    , m_Rich1Mirror1SegDetElem{de.child( RichGeoUtil::getRichGeoUtilInstance()->Rich1Mirror1DetName( 1, de.id() ) ),
                               ctxt} {
  // nothing to initialise here yet ...
}

void DeRich1Mirror1SegObject::PrintR1M1ReflectivityFromDBInObj() {
  auto aRichSurfaceUtil = RichSurfaceUtil::getRichSurfaceUtilInstance();
  aRichSurfaceUtil->PrintRichSurfProperty( getSurface( m_mirrorNumber ), RichSurfPropType::Refl );
}
