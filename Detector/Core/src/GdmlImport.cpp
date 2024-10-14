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
#include "DD4hep/Printout.h"
#include "TGDMLParse.h"
#include "TGeoManager.h"
#include "TUri.h"
#include "XML/DocumentHandler.h"
#include "XML/Utilities.h"

static dd4hep::Ref_t create_element( dd4hep::Detector& description, xml_h e, dd4hep::Ref_t /* sens_det */ ) {
  xml_det_t          x_det = e;
  int                id    = x_det.hasAttr( _U( id ) ) ? x_det.id() : 0;
  xml_dim_t          x_pos( x_det.child( _U( position ), false ) );
  xml_dim_t          x_rot( x_det.child( _U( rotation ), false ) );
  xml_dim_t          x_gdml( x_det.child( _U( gdmlFile ) ) );
  xml_dim_t          x_reg( x_det.child( _U( region ) ) );
  std::string        name       = x_det.nameStr();
  std::string        region     = x_reg.nameStr();
  std::string        input      = x_gdml.attr<std::string>( _U( ref ) );
  dd4hep::DetElement det_region = description.detector( region );
  TGDMLParse         parser;
  if ( !input.empty() && input[0] == '/' ) {
    TUri uri( input.c_str() );
    input = uri.GetRelativePart();
  } else {
    std::string path = dd4hep::xml::DocumentHandler::system_path( e, input );
    TUri        uri( path.c_str() );
    input = uri.GetRelativePart();
  }
  if ( !det_region.isValid() ) { dd4hep::except( name, "+++ Cannot access detector region: %s", region.c_str() ); }
  dd4hep::DetElement sdet( name, id );
  dd4hep::Volume     volume = parser.GDMLReadFile( input.c_str() );
  if ( !volume.isValid() ) { dd4hep::except( "ROOTGDMLParse", "+++ Failed to parse GDML file:%s", input.c_str() ); }
  volume.import(); // We require the extensions in dd4hep.
  dd4hep::printout( dd4hep::INFO, "ROOTGDMLParse", "+++ Attach GDML volume %s", volume.name() );
  dd4hep::Volume       mother = det_region.volume();
  dd4hep::PlacedVolume pv;

  if ( x_pos && x_rot ) {
    dd4hep::Rotation3D  rot( dd4hep::RotationZYX( x_rot.z(), x_rot.y(), x_rot.x() ) );
    dd4hep::Transform3D transform( rot, dd4hep::Position( x_pos.x(), x_pos.y(), x_pos.z() ) );
    pv = mother.placeVolume( volume, transform );
  } else if ( x_rot ) {
    dd4hep::Rotation3D  rot( dd4hep::RotationZYX( x_rot.z(), x_rot.y(), x_rot.x() ) );
    dd4hep::Transform3D transform( rot, dd4hep::Position( 0, 0, 0 ) );
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
  sdet.setPlacement( pv );
  return sdet;
}

// first argument is the type from the xml file
DECLARE_DETELEMENT( DD4hep_GdmlImport, create_element )
