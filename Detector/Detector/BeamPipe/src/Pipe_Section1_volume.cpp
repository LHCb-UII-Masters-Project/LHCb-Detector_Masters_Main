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
#include "DD4hep/detail/DetectorInterna.h"
#include "TClass.h"
#include "XML/Utilities.h"

#include <optional>

static dd4hep::Handle<TObject> create_section1( dd4hep::Detector& description, xml_h element ) {
  xml_dim_t             e( element );
  double                invalid          = std::numeric_limits<double>::max();
  double                pipe_length      = invalid;
  double                pipe_radius      = invalid;
  double                pipe_thick       = invalid;
  double                flange_length    = invalid;
  double                flange_thick     = invalid;
  double                flange_up_length = invalid;
  double                flange_up_thick  = invalid;
  std::optional<double> flange_up_posZ;
  double                flange_dwn_length = invalid;
  double                flange_dwn_thick  = invalid;
  std::optional<double> flange_dwn_posZ;
  std::string           vis, body_material, flange_material, flange_up_material, flange_dwn_material;
  std::string           name = e.attr<std::string>( _U( name ) );
  int                   cnt = 0, flup = 0, fldwn = 0, fl = 0;

  for ( xml_coll_t i( e, _U( param ) ); i; ++i ) {
    xml_comp_t  c( i );
    std::string n = c.nameStr();
    if ( n == "Length" )
      ++cnt, pipe_length = c.attr<double>( _U( value ) );
    else if ( n == "Radius" )
      ++cnt, pipe_radius = c.attr<double>( _U( value ) );
    else if ( n == "Thick" )
      ++cnt, pipe_thick = c.attr<double>( _U( value ) );
    else if ( n == "vis" )
      ++cnt, vis = c.attr<std::string>( _U( value ) );
    else if ( n == "Material" )
      ++cnt, body_material = c.attr<std::string>( _U( value ) );
    else if ( n == "FlangeLength" )
      ++fl, flange_length = c.attr<double>( _U( value ) );
    else if ( n == "FlangeThick" )
      ++fl, flange_thick = c.attr<double>( _U( value ) );
    else if ( n == "FlangeMaterial" )
      ++fl, flange_material = c.attr<std::string>( _U( value ) );
    else if ( n == "FlangeUpLength" )
      ++flup, flange_up_length = c.attr<double>( _U( value ) );
    else if ( n == "FlangeUpThick" )
      ++flup, flange_up_thick = c.attr<double>( _U( value ) );
    else if ( n == "FlangeUpPosZ" )
      ++flup, flange_up_posZ = c.attr<double>( _U( value ) );
    else if ( n == "FlangeUpMaterial" )
      ++flup, flange_up_material = c.attr<std::string>( _U( value ) );
    else if ( n == "FlangeDownLength" )
      ++fldwn, flange_dwn_length = c.attr<double>( _U( value ) );
    else if ( n == "FlangeDownThick" )
      ++fldwn, flange_dwn_thick = c.attr<double>( _U( value ) );
    else if ( n == "FlangeDownPosZ" )
      ++flup, flange_dwn_posZ = c.attr<double>( _U( value ) );
    else if ( n == "FlangeDownMaterial" )
      ++fldwn, flange_dwn_material = c.attr<std::string>( _U( value ) );
  }
  if ( !( cnt == 5 && ( ( ( fl >= 2 ) || ( flup >= 2 ) || ( fldwn >= 2 ) ) ||
                        ( ( fl == 0 ) && ( flup == 0 ) && ( fldwn == 0 ) ) ) ) ) {
    dd4hep::except( "LHCb_Pipe_Section1", "+++ Invalid number of parameters: %d [fl:%d flup:%d fldwn:%d]", cnt, fl,
                    flup, fldwn );
  }
  dd4hep::Assembly volume( name );
  dd4hep::VisAttr  visAttr  = description.visAttributes( vis );
  dd4hep::Material mat_body = description.material( body_material );
  dd4hep::Tube     tub_body( pipe_radius, pipe_radius + pipe_thick, pipe_length / 2.0 );
  dd4hep::Volume   vol_body( name + "Body", tub_body, mat_body );
  dd4hep::Tube     tub_vac( 0.0, pipe_radius, pipe_length / 2.0 );
  dd4hep::Volume   vol_vac( name + "Vacuum", tub_vac, description.vacuum() );
  volume.setVisAttributes( visAttr );
  vol_body.setVisAttributes( visAttr );
  vol_vac.setVisAttributes( description, "Pipe:Vacuum" );
  volume.placeVolume( vol_body );
  volume.placeVolume( vol_vac );
  if ( cnt == 5 && ( ( fl == 0 ) && ( flup == 0 ) && ( fldwn == 0 ) ) ) {
    // Do nothing. This is a  simple tube filled with vacuum. No flanges at all
  } else if ( cnt == 5 && ( ( flup >= 2 ) || ( fldwn >= 2 ) ) ) {
    // Up side flange. If no Z position is supplied, the default is: (flange_up_length-pipe_length)/2.0
    if ( flup >= 2 ) {
      double           flange_rad = pipe_radius + pipe_thick + 0.01 * dd4hep::mm;
      double           z          = flange_up_posZ.value_or( ( flange_up_length - pipe_length ) / 2.0 );
      dd4hep::Material mat        = flange_up_material.empty() ? mat_body : description.material( flange_up_material );
      dd4hep::Tube     tub( flange_rad, flange_rad + flange_up_thick, flange_up_length / 2.0 );
      dd4hep::Volume   vol( name + "FlangeUp", tub, mat );
      vol.setVisAttributes( visAttr );
      volume.placeVolume( vol, dd4hep::Position( 0, 0, z ) );
    }
    // Up side flange. If no Z position is supplied, the default is: (pipe_length - flange_dwn_length)/2.0
    if ( fldwn >= 2 ) {
      double           flange_rad = pipe_radius + pipe_thick + 0.01 * dd4hep::mm;
      double           z          = flange_dwn_posZ.value_or( ( pipe_length - flange_dwn_length ) / 2.0 );
      dd4hep::Material mat = flange_dwn_material.empty() ? mat_body : description.material( flange_dwn_material );
      dd4hep::Tube     tub( flange_rad, flange_rad + flange_dwn_thick, flange_dwn_length / 2.0 );
      dd4hep::Volume   vol( name + "FlangeDwn", tub, mat );
      vol.setVisAttributes( visAttr );
      volume.placeVolume( vol, dd4hep::Position( 0, 0, z ) );
    }
  } else if ( cnt == 5 && ( fl >= 2 ) ) {
    /// Simpler pipe: tube with 2 identical flanges at the end
    double           flange_rad = pipe_radius + pipe_thick + 0.01 * dd4hep::mm;
    double           zup        = flange_up_posZ.value_or( ( flange_length - pipe_length ) / 2.0 );
    double           zdwn       = flange_dwn_posZ.value_or( ( pipe_length - flange_length ) / 2.0 );
    dd4hep::Material mat        = flange_material.empty() ? mat_body : description.material( flange_material );
    dd4hep::Tube     tub_fl( flange_rad, flange_rad + flange_thick, flange_length / 2.0 );
    dd4hep::Volume   vol_fl( name + "Flange", tub_fl, mat );
    vol_fl.setVisAttributes( visAttr );
    volume.placeVolume( vol_fl, dd4hep::Position( 0, 0, zup ) );
    volume.placeVolume( vol_fl, dd4hep::Position( 0, 0, zdwn ) );
  }
  return dd4hep::Handle<TObject>( volume.ptr() );
}
DECLARE_XML_VOLUME( LHCb_Pipe_Section_Type_1__volume_constructor, create_section1 )
