//==========================================================================
//  AIDA Detector description implementation
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
// Author     : M.Frank
//
//==========================================================================
//
// Specialized generic detector constructor
//
//==========================================================================

#include "Core/UpgradeTags.h"
#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/Printout.h"
#include "Math/Polar2D.h"
#include "XML/Utilities.h"

static dd4hep::Ref_t create_element( dd4hep::Detector& description, xml_h e, dd4hep::Ref_t sens_det ) {
  xml_det_t                         x_det = e;
  dd4hep::xml::tools::VolumeBuilder builder( description, x_det, sens_det );
  dd4hep::DetElement                bcm_det = builder.detector;
  std::string                       name    = bcm_det.name();
  dd4hep::Assembly                  bcm_vol( "lv" + name );
  dd4hep::PlacedVolume              pv;

  // Diamond sensor
  xml_comp_t       x_sens     = x_det.child( _U( sensor ) );
  xml_dim_t        x_sens_pos = x_sens.position();
  dd4hep::Material sens_mat   = description.material( x_sens.materialStr() );
  dd4hep::Box      sens_box( x_sens.dx(), x_sens.dy(), x_sens.dz() );
  dd4hep::Volume   sens_vol( "lv" + name + "Sensor", sens_box, sens_mat );

  sens_vol.setVisAttributes( description, x_sens.visStr() );
  sens_vol.setSensitiveDetector( sens_det );

  // Place the sensors in the main frame of the detector:
  double              phi    = x_sens_pos.phi();
  double              radius = x_sens_pos.r();
  double              z_pos  = x_sens_pos.z();
  std::string         fmt    = name + "Sensor%d";
  dd4hep::RotationZYX rotZ( M_PI / 4e0, 0, 0 );
  for ( int isens = 0; isens < 8; ++isens ) {
    dd4hep::DetElement          de( bcm_det, dd4hep::_toString( isens, fmt.c_str() ), x_det.id() );
    ROOT::Math::Polar2D<double> dim( radius, phi );
    dd4hep::Transform3D         tr( dd4hep::RotationZYX( phi, 0, 0 ), dd4hep::Position( dim.X(), dim.Y(), z_pos ) );
    pv = bcm_vol.placeVolume( sens_vol, tr );
    pv.addPhysVolID( "sensor", isens );
    de.setPlacement( pv );
    phi += M_PI / 4e0; // Advance by 45 degree
  }
  builder.buildVolumes( e );
  builder.placeDaughters( bcm_det, bcm_vol, e );
  pv = builder.placeDetector( bcm_vol );
  pv.addPhysVolID( "station", x_det.station() );
  pv.addPhysVolID( "system", x_det.id() );
  return bcm_det;
}

// first argument is the type from the xml file
DECLARE_DETELEMENT( LHCb_Bcm_v1_0, create_element )
