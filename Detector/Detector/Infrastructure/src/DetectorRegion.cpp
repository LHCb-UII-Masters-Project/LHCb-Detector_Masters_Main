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

#include "DD4hep/DetFactoryHelper.h"
#include "XML/Utilities.h"

static dd4hep::Ref_t create_element( dd4hep::Detector& description, xml_h e, dd4hep::Ref_t /* sens_det */ ) {
  xml_det_t            x_det = e;
  int                  id    = x_det.hasAttr( _U( id ) ) ? x_det.id() : 0;
  std::string          name  = x_det.nameStr();
  xml_comp_t           x_vol( x_det.child( _U( volume ) ) );
  xml_dim_t            x_pos( x_det.child( _U( position ), false ) );
  xml_dim_t            x_rot( x_det.child( _U( rotation ), false ) );
  dd4hep::Material     mat( description.material( x_vol.materialStr() ) );
  dd4hep::DetElement   det( name, id );
  dd4hep::Volume       mother = description.pickMotherVolume( det );
  dd4hep::Solid        solid  = dd4hep::xml::createShape( description, x_vol.typeStr(), x_vol );
  dd4hep::Volume       volume( name + "_vol", solid, mat );
  dd4hep::PlacedVolume pv;

  if ( x_pos && x_rot ) {
    dd4hep::Transform3D transform( dd4hep::Rotation3D( dd4hep::RotationZYX( x_rot.z(), x_rot.y(), x_rot.x() ) ),
                                   dd4hep::Position( x_pos.x(), x_pos.y(), x_pos.z() ) );
    pv = mother.placeVolume( volume, transform );
  } else if ( x_rot ) {
    dd4hep::Transform3D transform( dd4hep::Rotation3D( dd4hep::RotationZYX( x_rot.z(), x_rot.y(), x_rot.x() ) ),
                                   dd4hep::Position( 0, 0, 0 ) );
    pv = mother.placeVolume( volume, transform );
  } else if ( x_pos ) {
    pv = mother.placeVolume( volume, dd4hep::Position( x_pos.x(), x_pos.y(), x_pos.z() ) );
  } else {
    pv = mother.placeVolume( volume );
  }
  volume.setVisAttributes( description, x_det.visStr() );
  volume.setLimitSet( description, x_det.limitsStr() );
  volume.setRegion( description, x_det.regionStr() );
  if ( id != 0 ) { pv.addPhysVolID( "system", id ); }
  det.setPlacement( pv );
  return det;
}

// first argument is the type from the xml file
DECLARE_DETELEMENT( DD4hep_DetectorRegion_v1_0, create_element )
