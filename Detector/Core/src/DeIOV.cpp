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

// Framework include files
#include "Core/DeIOV.h"
#include "Core/Keys.h"
#include "Core/PrintHelpers.h"

#include "DD4hep/Alignments.h"
#include "DD4hep/AlignmentsPrinter.h"
#include "DD4hep/Conditions.h"
#include "DD4hep/ConditionsDebug.h"
#include "DD4hep/ConditionsMap.h"
#include "DD4hep/GrammarUnparsed.h"
#include "DD4hep/Printout.h"
#include "DD4hep/detail/AlignmentsInterna.h"

#include <map>
#include <sstream>
#include <vector>

using lhcb::geometrytools::toTransform3D;

/// Initialization of sub-classes
LHCb::Detector::detail::DeIOVObject::DeIOVObject( dd4hep::DetElement de, dd4hep::cond::ConditionUpdateContext& ctxt,
                                                  int classID, bool hasAlignment )
    :
#if !defined( DD4HEP_MINIMAL_CONDITIONS )
    ConditionObject( Keys::deKeyName )
    ,
#endif
    clsID( classID )
    , m_detector( de )
    , m_geometry( de.placement() ) {
  hash = LHCb::Detector::ConditionKey( de, Keys::deKey );
  data.bindExtern( this );

  if ( hasAlignment ) {
    auto kalign         = LHCb::Detector::ConditionKey( m_detector, Keys::alignmentKey );
    m_detectorAlignment = ctxt.condition( kalign );
  } else {
    m_detectorAlignment = de.nominal();
  }

  if ( m_detectorAlignment.isValid() ) {
    m_toGlobalMatrix = toTransform3D( m_detectorAlignment.worldTransformation() );
    m_toLocalMatrix  = m_toGlobalMatrix.Inverse();
  }

  m_toGlobalMatrixNominal = toTransform3D( m_detector.nominal().worldTransformation() );
  m_toLocalMatrixNominal  = m_toGlobalMatrixNominal.Inverse();

  TGeoHMatrix geoDelta{};
  m_detectorAlignment.delta().computeMatrix( geoDelta );
  m_delta = toTransform3D( geoDelta );
}

/// Printout method to stdout
void LHCb::Detector::detail::DeIOVObject::print( int indent, int flg ) const {
  std::string prefix = getIndentation( indent );
  dd4hep::printout( dd4hep::INFO, "DeIOV", "%s*========== Detector:%s", prefix.c_str(), m_detector.path().c_str() );
  dd4hep::printout( dd4hep::INFO, "DeIOV", "%s+ Name:%s Hash:%016lX Type:%s Flags:%08X IOV:%s", prefix.c_str(),
                    dd4hep::cond::cond_name( this ).c_str(), hash, is_bound() ? data.dataType().c_str() : "<UNBOUND>",
                    flags, iov ? iov->str().c_str() : "--" );
  if ( flg & BASICS ) {
    const dd4hep::DetElement::Children& c = m_detector.children();
    dd4hep::printout( dd4hep::INFO, "DeIOV", "%s+ Detector:%s #Dau:%d", prefix.c_str(), m_detector.name(),
                      int( c.size() ) );
  }
  if ( flg & DETAIL ) {
    dd4hep::printout( dd4hep::INFO, "DeIOV", "%s+  >> Alignment:%s", prefix.c_str(),
                      dd4hep::yes_no( m_detectorAlignment.isValid() ) );
    if ( m_detectorAlignment.isValid() ) {
      char                         txt1[64], txt2[64], txt3[64];
      std::stringstream            str;
      dd4hep::Alignment::Object*   ptr            = m_detectorAlignment.ptr();
      const dd4hep::AlignmentData& alignment_data = m_detectorAlignment.data();
      const dd4hep::Delta&         D              = alignment_data.delta;

      if ( D.hasTranslation() )
        ::snprintf( txt1, sizeof( txt1 ), "Tr: x:%g y:%g z:%g ", D.translation.x(), D.translation.Y(),
                    D.translation.Z() );
      else
        ::snprintf( txt1, sizeof( txt1 ), "Tr:    ------- " );
      if ( D.hasRotation() )
        ::snprintf( txt2, sizeof( txt2 ), "Rot: phi:%g psi:%g theta:%g ", D.rotation.Phi(), D.rotation.Psi(),
                    D.rotation.Theta() );
      else
        ::snprintf( txt2, sizeof( txt2 ), "Rot:   ------- " );
      if ( D.hasPivot() )
        ::snprintf( txt3, sizeof( txt3 ), "Rot: x:%g y:%g z:%g ", D.pivot.Vect().X(), D.pivot.Vect().Y(),
                    D.pivot.Vect().Z() );
      else
        ::snprintf( txt3, sizeof( txt3 ), "Pivot: ------- " );

      dd4hep::printout( dd4hep::INFO, "DeIOV", "%s+  >> Aligment [%p] Typ:%s \tData:(%11s-%8s-%5s)", prefix.c_str(),
                        m_detectorAlignment.ptr(), dd4hep::typeName( typeid( *ptr ) ).c_str(),
                        D.hasTranslation() ? "Translation" : "", D.hasRotation() ? "Rotation" : "",
                        D.hasPivot() ? "Pivot" : "" );
      if ( D.hasTranslation() || D.hasRotation() || D.hasPivot() ) {
        dd4hep::printout( dd4hep::INFO, "DeIOV", "%s+  >> Aligment-Delta %s %s %s", prefix.c_str(), txt1, txt2, txt3 );
      }
    }
  }
}

bool LHCb::Detector::detail::DeIOVObject::isInside( const ROOT::Math::XYZPoint& globalPoint ) const {
  auto     localPoint = toLocal( globalPoint );
  Double_t p[3]{toDD4hepUnits( localPoint.x() ), toDD4hepUnits( localPoint.y() ), toDD4hepUnits( localPoint.z() )};
  return detector().solid()->Contains( p );
}

LHCb::Detector::DeIOV
LHCb::Detector::detail::DeIOVObject::findChildForPoint( const ROOT::Math::XYZPoint& point ) const {
  DeIOV res{};
  DeIOV resChild{this};
  while ( resChild.isValid() ) {
    res      = resChild;
    resChild = DeIOV{};
    res->applyToAllChildren( [&resChild, &point]( DeIOV child ) {
      if ( !resChild.isValid() && child.isInside( point ) ) resChild = child;
    } );
  }
  return res;
}
