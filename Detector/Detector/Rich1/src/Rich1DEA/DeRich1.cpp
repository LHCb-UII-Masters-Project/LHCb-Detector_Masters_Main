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
/**  DeRich1 detector element implementation in DD4HEP    SE 2020-07-03    ***/
/*****************************************************************************/

#include "Detector/Rich1/DetElemAccess/DeRich1.h"
#include "DD4hep/Printout.h"
#include "Detector/Rich1/RichGeoUtil.h"

using namespace LHCb::Detector::detail;

DeRich1Object::DeRich1Object( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeRichObject( Rich::Rich1, de, ctxt )
    , m_Rich1RadiatorGasDetElem{de.child( RichGeoUtil::getRichGeoUtilInstance()->Rich1SubMasterDetName() ), ctxt}
    , m_Rich1Mirror1EnvDetElem{{{de.child( RichGeoUtil::getRichGeoUtilInstance()->Rich1Mirror1DetName( 0, 0 ) ), ctxt},
                                {de.child( RichGeoUtil::getRichGeoUtilInstance()->Rich1Mirror1DetName( 0, 1 ) ), ctxt},
                                {de.child( RichGeoUtil::getRichGeoUtilInstance()->Rich1Mirror1DetName( 0, 2 ) ), ctxt},
                                {de.child( RichGeoUtil::getRichGeoUtilInstance()->Rich1Mirror1DetName( 0, 3 ) ), ctxt}}}
    , m_Rich1Mirror2MasterDetElem{{{de.child( RichGeoUtil::getRichGeoUtilInstance()->Rich1Mirror2MasterDetName( 0 ) ),
                                    ctxt},
                                   {de.child( RichGeoUtil::getRichGeoUtilInstance()->Rich1Mirror2MasterDetName( 1 ) ),
                                    ctxt}}}
    , m_Rich1PhDetPanel{{{de.child( RichGeoUtil::getRichGeoUtilInstance()->Rich1PhDetSupFrameDetName( 0 ) ), ctxt},
                         {de.child( RichGeoUtil::getRichGeoUtilInstance()->Rich1PhDetSupFrameDetName( 1 ) ), ctxt}}} {

  detail::init_param( m_Rich1Mirror1NumSeg, "Rh1NumberofMirror1Segments" );
  detail::init_param( m_Rich1Mirror2NumSets, "Rh1NumberOfMirror2Systems" );

  detail::init_param( m_NumSides, "Rh1NumSides" );
  detail::init_param( m_nSphMirrorSegRows, "Rh1Mirror1NumRows" );
  detail::init_param( m_nSphMirrorSegCols, "Rh1Mirror1NumColumns" );
  detail::init_param( m_nSecMirrorSegRows, "Rh1Mirror2NumRows" );
  detail::init_param( m_nSecMirrorSegCols, "Rh1Mirror2NumColumns" );

  detail::init_param( m_sphMirrorNominalROC, "Rh1Mirror1InnerR" );

  using F = decltype( m_NominalPanelCoC[Rich::top].X() );

  m_NominalPanelCoC[Rich::top] = {detail::dd4hep_param<F>( "Rh1Mirror1CCLHCbXTop" ),
                                  detail::dd4hep_param<F>( "Rh1Mirror1CCLHCbYTop" ),
                                  detail::dd4hep_param<F>( "Rh1Mirror1CCLHCbZTop" )};

  m_NominalPanelCoC[Rich::bottom] = {detail::dd4hep_param<F>( "Rh1Mirror1CCLHCbXBot" ),
                                     detail::dd4hep_param<F>( "Rh1Mirror1CCLHCbYBot" ),
                                     detail::dd4hep_param<F>( "Rh1Mirror1CCLHCbZBot" )};

  detail::print( *this );
}
