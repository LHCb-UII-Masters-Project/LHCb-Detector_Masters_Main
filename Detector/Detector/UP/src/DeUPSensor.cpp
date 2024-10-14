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

#include "Detector/UP/DeUPSensor.h"
#include "Core/PrintHelpers.h"

#include "DD4hep/Printout.h"

LHCb::Detector::detail::DeUPSensorObject::DeUPSensorObject( const dd4hep::DetElement&             de,
                                                            dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeIOVObject( de, ctxt, 9330 ) {
  m_id        = (unsigned int)de.id();
  m_uMaxLocal = 0.5f * ( m_pitch * m_nPixel );
  m_uMinLocal = -m_uMaxLocal;

  {
    auto const g1 = toGlobal( ROOT::Math::XYZPoint{m_uMinLocal, m_vMinLocal, 0.} );
    auto const g2 = toGlobal( ROOT::Math::XYZPoint{m_uMaxLocal, m_vMinLocal, 0.} );
    auto const g3 = toGlobal( ROOT::Math::XYZPoint{m_uMinLocal, m_vMaxLocal, 0.} );
    m_xInverted   = ( g1.x() > g2.x() );
    m_yInverted   = ( g1.y() > g3.y() );
  }
  {
      /*    auto* mainBox = geometry->volume()->GetShape();
      m_vMaxLocal   = 0.5 * mainBox->GetDY() - m_deadWidth;
      m_vMinLocal   = -m_vMaxLocal;
      m_pixelLength = fabs( m_vMaxLocal - m_vMinLocal );
      m_thickness   = mainBox->GetDZ(); // Or can read from access()->thickness, where thickness = _toDouble...*/
  }

  {
    float yUpper = m_vMaxLocal;
    float yLower = m_vMinLocal;
    if ( m_yInverted ) std::swap( yUpper, yLower );

    float xUpper = m_uMaxLocal - 0.5f * m_pitch;
    float xLower = m_uMinLocal + 0.5f * m_pitch;
    if ( m_xInverted ) std::swap( xUpper, xLower );

    // direction
    auto const g1 = toGlobal( ROOT::Math::XYZPoint( {xLower, yLower, 0.} ) );
    auto const g2 = toGlobal( ROOT::Math::XYZPoint( {xLower, yUpper, 0.} ) );
    m_direction   = ( g2 - g1 ).Unit();

    // trajectory of middle
    auto const g3 = toGlobal( ROOT::Math::XYZPoint( {xLower, 0., 0.} ) );
    auto const g4 = toGlobal( ROOT::Math::XYZPoint( {xUpper, 0., 0.} ) );
    m_midTraj.emplace( g3, g4 );

    // range ---> pixel Length
    m_range = std::make_pair( -0.5f * m_pixelLength, 0.5f * m_pixelLength );

    // plane
    m_plane = ROOT::Math::Plane3D( g1, g2, g4 );

    m_entryPlane =
        ROOT::Math::Plane3D( m_plane.Normal(), toGlobal( ROOT::Math::XYZPoint( {0., 0., -0.5f * m_thickness} ) ) );
    m_exitPlane =
        ROOT::Math::Plane3D( m_plane.Normal(), toGlobal( ROOT::Math::XYZPoint( {0., 0., 0.5f * m_thickness} ) ) );
  }

  // fill caches
}
