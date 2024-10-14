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
#include "XML/Utilities.h"

namespace {
  struct Builder : public dd4hep::xml::tools::VolumeBuilder {
    double                        coil_x;
    double                        coil_y;
    double                        coil_z;
    double                        yoke_x;
    double                        yoke_y;
    double                        yoke_z;
    double                        coil_hole_x;
    double                        coil_hole_y;
    double                        coil_hole_pos_x;
    std::map<std::string, double> params;

    Builder( dd4hep::Detector& description, xml_h e );
    double             param( const std::string& nam ) const;
    dd4hep::DetElement makeMagnet();
    dd4hep::Volume     makeYoke( const char* nam );
    dd4hep::Volume     makeSpacer( const char* nam );
    dd4hep::Volume     makeCoil( const char* nam, double curve_rad, double curve_thick );
  };
} // namespace

Builder::Builder( dd4hep::Detector& d, xml_h e ) : VolumeBuilder( d, e ) {
  for ( xml_coll_t c( e, _U( parameter ) ); c; ++c ) {
    xml_dim_t v = c;
    params.insert( make_pair( v.nameStr(), dd4hep::_toDouble( v.valueStr() ) ) );
  }
  coil_x          = param( "MBXW:CoilX" );
  coil_y          = param( "MBXW:CoilY" );
  coil_z          = param( "MBXW:CoilZ" );
  yoke_x          = param( "MBXW:YokeX" );
  yoke_y          = param( "MBXW:YokeY" );
  yoke_z          = coil_z * 2.0;
  coil_hole_x     = coil_x + 1.0 * dd4hep::mm;
  coil_hole_y     = param( "MBXW:CoilHoleY" );
  coil_hole_pos_x = param( "MBXW:CoilHolePosX" );
}

double Builder::param( const std::string& nam ) const {
  auto i = params.find( nam );
  if ( i != params.end() ) return ( *i ).second;
  return dd4hep::_toDouble( nam );
}

dd4hep::Volume Builder::makeSpacer( const char* nam ) {
  dd4hep::Material mat      = materials["spacer"];
  double           spacer_x = coil_x / 2.0;
  double           spacer_y = param( "MBXW:SpacerY" ) / 2.0;
  double           spacer_z = yoke_z / 2.0;
  dd4hep::Box      solid( spacer_x, spacer_y, spacer_z );
  dd4hep::Volume   vol( nam, solid, mat );
  vol.setVisAttributes( description, "MBXW:SpacerVis" );
  return vol;
}

dd4hep::Volume Builder::makeYoke( const char* nam ) {
  dd4hep::Material mat         = materials["yoke"];
  double           hole_x      = param( "MBXW:CenterHoleX" ) / 2.0;
  double           hole_y      = param( "MBXW:CenterHoleY" ) / 2.0;
  double           hole_z      = ( yoke_z + 1.0 * dd4hep::mm ) / 2.0;
  double           coil_hole_z = hole_z;

  dd4hep::Box   yoke( yoke_x / 2.0, yoke_y / 2.0, yoke_z / 2.0 );
  dd4hep::Box   hole( hole_x, hole_y, hole_z );
  dd4hep::Box   coil_hole( coil_hole_x / 2.0, coil_hole_y / 2.0, coil_hole_z );
  dd4hep::Solid solid = dd4hep::SubtractionSolid( yoke, hole );
  solid               = dd4hep::SubtractionSolid( solid, coil_hole, dd4hep::Position( -coil_hole_pos_x, 0, 0 ) );
  solid               = dd4hep::SubtractionSolid( solid, coil_hole, dd4hep::Position( coil_hole_pos_x, 0, 0 ) );
  dd4hep::Volume vol( nam, solid, mat );
  vol.setVisAttributes( description, "MBXW:YokeVis" );
  return vol;
}

dd4hep::Volume Builder::makeCoil( const char* nam, double curve_rad, double curve_thick ) {
  dd4hep::Material mat         = materials["coil"];
  double           curve_dz    = coil_y / 2.0;
  double           curve_rmin  = curve_rad;
  double           curve_rmax  = curve_rad + curve_thick;
  double           curve_pos_x = curve_rad + curve_thick / 2.0;
  double           curve_pos_z = coil_z / 2.0 + 0.01 * dd4hep::mm;
  double           line_x      = coil_hole_pos_x + coil_x / 2.0 - curve_thick - curve_rad - 0.01 * dd4hep::mm;
  double           line_y      = coil_y;
  double           line_z      = coil_x;
  double           line_pos_x  = line_x / 2.0 - coil_x / 2.0 + curve_thick + curve_rad;
  double           line_pos_z  = coil_z / 2.0 - coil_x / 2.0 + curve_thick + curve_rad;

  dd4hep::Box         straight( coil_x / 2.0, coil_y / 2.0, coil_z / 2.0 );
  dd4hep::Tube        curve( curve_rmin, curve_rmax, curve_dz, M_PI / 2.0 );
  dd4hep::Box         line( line_x / 2.0, line_y / 2.0, line_z / 2.0 );
  dd4hep::Position    pos( -curve_pos_x, 0, curve_pos_z );
  dd4hep::RotationZYX rot( 0, 0, M_PI / 2.0 );
  dd4hep::Transform3D trafo = dd4hep::Transform3D( pos ) * dd4hep::Transform3D( rot );
  dd4hep::Solid       solid = dd4hep::UnionSolid( straight, curve, trafo );
  solid                     = dd4hep::UnionSolid( solid, line, dd4hep::Position( -line_pos_x, 0, line_pos_z ) );
  dd4hep::Volume vol( nam, solid, mat );
  vol.setVisAttributes( description, "MBXW:CoilVis" );
  return vol;
}

dd4hep::DetElement Builder::makeMagnet() {
  dd4hep::Assembly    mbxw_vol( std::string( "lv" ) + detector.name() );
  dd4hep::Transform3D rotX( dd4hep::RotationZYX( 0, 0, M_PI ) );
  dd4hep::Transform3D rotZ( dd4hep::RotationZYX( M_PI, 0, 0 ) );
  dd4hep::Transform3D rotXZ( dd4hep::RotationZYX( M_PI, 0, M_PI ) );

  collectMaterials( x_det.child( _U( materials ) ) );

  double         coil_pos_y = ( coil_hole_y - coil_y - 1.0 * dd4hep::mm ) / 2.0;
  double         coil_pos_z = coil_z / 2.0 + 0.01 * dd4hep::mm;
  dd4hep::Volume yoke       = makeYoke( "lvMagnetYoke" );
  dd4hep::Volume coil_up    = makeCoil( "lvMagnetCoilUp", param( "MBXW:UpCurveRad" ), coil_x );
  dd4hep::Volume coil_dwn   = makeCoil( "lvMagnetCoilDown", param( "MBXW:DownCurveRad" ), coil_x );
  dd4hep::Volume spacer     = makeSpacer( "lvMagnetBetweenCoil" );

  mbxw_vol.placeVolume( yoke );
  mbxw_vol.placeVolume( spacer, dd4hep::Position( -coil_hole_pos_x, 0, 0 ) );
  mbxw_vol.placeVolume( spacer, dd4hep::Transform3D( dd4hep::Position( coil_hole_pos_x, 0, 0 ) ) * rotZ );

  mbxw_vol.placeVolume( coil_dwn, dd4hep::Position( coil_hole_pos_x, coil_pos_y, coil_pos_z ) );
  mbxw_vol.placeVolume( coil_up,
                        dd4hep::Transform3D( dd4hep::Position( coil_hole_pos_x, coil_pos_y, -coil_pos_z ) ) * rotX );
  mbxw_vol.placeVolume( coil_dwn,
                        dd4hep::Transform3D( dd4hep::Position( -coil_hole_pos_x, coil_pos_y, coil_pos_z ) ) * rotZ );
  mbxw_vol.placeVolume( coil_up,
                        dd4hep::Transform3D( dd4hep::Position( -coil_hole_pos_x, coil_pos_y, -coil_pos_z ) ) * rotXZ );

  mbxw_vol.placeVolume( coil_dwn, dd4hep::Position( coil_hole_pos_x, -coil_pos_y, coil_pos_z ) );
  mbxw_vol.placeVolume( coil_up,
                        dd4hep::Transform3D( dd4hep::Position( coil_hole_pos_x, -coil_pos_y, -coil_pos_z ) ) * rotX );
  mbxw_vol.placeVolume( coil_dwn,
                        dd4hep::Transform3D( dd4hep::Position( -coil_hole_pos_x, -coil_pos_y, coil_pos_z ) ) * rotZ );
  mbxw_vol.placeVolume( coil_up,
                        dd4hep::Transform3D( dd4hep::Position( -coil_hole_pos_x, -coil_pos_y, -coil_pos_z ) ) * rotXZ );
  placeDetector( mbxw_vol );
  return detector;
}

static dd4hep::Ref_t create_element( dd4hep::Detector& description, xml_h e, dd4hep::Ref_t /* sens_det */ ) {
  Builder b( description, e );
  return b.makeMagnet();
}

// first argument is the type from the xml file
DECLARE_DETELEMENT( LHCb_MBXW_v1_0, create_element )
