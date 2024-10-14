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
/**  DeRich2 detector element implementation in DD4HEP    SE 2022-01-10    ***/
/*****************************************************************************/

#include "Detector/Rich2/DetElemAccess/DeRich2.h"
#include "DD4hep/Printout.h"
#include "Detector/Rich2/Rich2GeoUtil.h"

LHCb::Detector::detail::DeRich2Object::DeRich2Object( const dd4hep::DetElement&             de,
                                                      dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeRichObject( Rich::Rich2, de, ctxt )
    , m_Rich2RadiatorGasDetElem{de.child( Rich2GeoUtil::getRich2GeoUtilInstance()->Rich2GasEnclosureDetName() ), ctxt}
    , m_Rich2BeamPipeDetElem{de.child( Rich2GeoUtil::getRich2GeoUtilInstance()->Rich2_BeamTubeDetName() ), ctxt}
    , m_Rich2PhDetPanel{{{de.child( Rich2GeoUtil::getRich2GeoUtilInstance()->Rich2PhDetPanelDetName( 0 ) ), ctxt},
                         {de.child( Rich2GeoUtil::getRich2GeoUtilInstance()->Rich2PhDetPanelDetName( 1 ) ), ctxt}}} {

  detail::init_param( m_NumSides, "Rh2NumSides" );
  detail::init_param( m_nSphMirrorSegRows, "Rh2NumSphMirrorSegRows" );
  detail::init_param( m_nSphMirrorSegCols, "Rh2NumSphMirrorSegCols" );
  detail::init_param( m_nSecMirrorSegRows, "Rh2NumSecMirrorSegRows" );
  detail::init_param( m_nSecMirrorSegCols, "Rh2NumSecMirrorSegCols" );

  detail::init_param( m_sphMirrorNominalROC, "Rh2SphMirrorInnerR" );

  using F = decltype( m_NominalPanelCoC[Rich::left].X() );

  m_NominalPanelCoC[Rich::left] = {detail::dd4hep_param<F>( "Rh2SphMirrorLHCbCoCX" ),
                                   detail::dd4hep_param<F>( "Rh2SphMirrorLHCbCoCY" ),
                                   detail::dd4hep_param<F>( "Rh2SphMirrorLHCbCoCZ" )};

  m_NominalPanelCoC[Rich::right] = {detail::dd4hep_param<F>( "Rh2SphMirrorLHCbNegativeCoCX" ),
                                    detail::dd4hep_param<F>( "Rh2SphMirrorLHCbCoCY" ),
                                    detail::dd4hep_param<F>( "Rh2SphMirrorLHCbCoCZ" )};

  detail::print( *this );
}
