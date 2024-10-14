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

#include "Detector/UP/DeUPLayer.h"
#include "Core/PrintHelpers.h"

#include "DD4hep/Printout.h"

LHCb::Detector::detail::DeUPLayerObject::DeUPLayerObject( const dd4hep::DetElement&             de,
                                                          dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeIOVObject( de, ctxt, 9310 ) {
  for ( auto iter : de.children() ) {
    // m_staves.emplace_back( iter.second, ctxt );
  }

  {
    ROOT::Math::XYZPoint p1 = toGlobal( ROOT::Math::XYZPoint( {0, 0, 0} ) );
    ROOT::Math::XYZPoint p2 = toGlobal( ROOT::Math::XYZPoint( {3 * dd4hep::meter, 0, 0} ) );
    ROOT::Math::XYZPoint p3 = toGlobal( ROOT::Math::XYZPoint( {0, 3 * dd4hep::meter, 0} ) );
    m_plane                 = ROOT::Math::Plane3D( p1, p2, p3 );
  }

  {
      /*ROOT::Math::XYZPoint  p1 = ROOT::Math::XYZPoint( toGlobal( {0, 1, 0} ) );
      ROOT::Math::XYZPoint  p2( 0, 1, 0 );
      //m_angle = ROOT::Math::gv_detail::dist( p1, p2 );
      m_sinAngle = sin( m_angle );
      m_cosAngle = cos( m_angle );*/
  };
}
