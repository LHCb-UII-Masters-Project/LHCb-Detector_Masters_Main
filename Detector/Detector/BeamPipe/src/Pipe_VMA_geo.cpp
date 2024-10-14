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
#include "XML/Utilities.h"

namespace {

  /// Helper class to build the VMA detector of LHCb
  struct VMABuild : public dd4hep::xml::tools::VolumeBuilder {
    std::string lvName;
    // Local parameters
    double         FullZ            = 0;
    double         transTubeRad     = 0;
    double         transTubeCylZ    = 0;
    double         transTubeConsRad = 0;
    double         transTubeConsZ   = 0;
    double         entryUpCylZ      = 0;
    double         rfContactRad     = 0; // RF contact mount parameters
    int            rfFingers        = 0;
    dd4hep::Volume lvVMA;
    // Global parameters:
    double          FlangeUpPosZ   = 0;
    double          FlangeDownPosZ = 0;
    double          EntryUpPosZ    = 0;
    double          EntryDownPosZ  = 0;
    double          BellowPosZ     = 0;
    double          PipePosZ       = 0;
    double          RfContactPosZ  = 0;
    dd4hep::VisAttr visAttr;

    /// Initializing constructor
    VMABuild( dd4hep::Detector& description, xml_elt_t e, dd4hep::SensitiveDetector sens );
    void build_rf_contact();
    void build_trans_tube();
    void build_pipe();
    void build_vacuum();
    void build_junction();
    void build();
  };

  /// Initializing constructor
  VMABuild::VMABuild( dd4hep::Detector& dsc, xml_elt_t e, dd4hep::SensitiveDetector sens )
      : dd4hep::xml::tools::VolumeBuilder( dsc, e, sens ) {
    // Process debug flags
    xml_comp_t x_dbg = x_det.child( _U( debug ), false );
    if ( x_dbg ) {
      for ( xml_coll_t i( x_dbg, _U( item ) ); i; ++i ) {
        xml_comp_t  c( i );
        std::string n = c.nameStr();
        if ( n == "debug" ) debug = c.attr<int>( _U( value ) ) != 0;
      }
    }
    xml_comp_t x_para = x_det.child( _U( params ) );
    for ( xml_coll_t i( x_para, _U( param ) ); i; ++i ) {
      xml_comp_t  c( i );
      std::string n = c.nameStr();
      // Rf contact
      if ( n == "RfContactRad" )
        rfContactRad = c.attr<double>( _U( value ) );
      else if ( n == "RfFingers" )
        rfFingers = c.attr<int>( _U( value ) );
      // Trans-tube
      else if ( n == "TransTubeRad" )
        transTubeRad = c.attr<double>( _U( value ) );
      else if ( n == "TransTubeCylZ" )
        transTubeCylZ = c.attr<double>( _U( value ) );
      else if ( n == "TransTubeConsRad" )
        transTubeConsRad = c.attr<double>( _U( value ) );
      else if ( n == "TransTubeConsZ" )
        transTubeConsZ = c.attr<double>( _U( value ) );
      ///
      else if ( n == "EntryUpCylZ" )
        entryUpCylZ = c.attr<double>( _U( value ) );
      else if ( n == "FullZ" )
        FullZ = c.attr<double>( _U( value ) );
    }
    visAttr = description.visAttributes( x_det.visStr() );
  }

  void VMABuild::build() {
    lvVMA = dd4hep::Assembly( lvName = "lv" + name );
    lvVMA.setVisAttributes( visAttr );
    build_rf_contact();
    build_trans_tube();
    build_pipe();
    build_vacuum();
    if ( x_det.hasChild( _Unicode( Junction ) ) ) { build_junction(); }
    placeDetector( lvVMA );
  }

  void VMABuild::build_rf_contact() {
    double rfContactThick       = dd4hep::_toDouble( "VMA:RfContactThick" );
    double rfContactCylZ        = dd4hep::_toDouble( "VMA:RfContactCylZ" );
    double rfContactFlangeZ     = dd4hep::_toDouble( "VMA:RfContactFlangeZ" );
    double rfContactFlangeThick = dd4hep::_toDouble( "VMA:RfContactFlangeThickRaw" ) - rfContactRad - rfContactThick;
    double rfContSingleThick    = dd4hep::_toDouble( "VMA:RfContSingleThick" );
    double rfContSinglePlateZ   = dd4hep::_toDouble( "VMA:RfContSinglePlateZ" );
    double rfContSingleX        = dd4hep::_toDouble( "VMA:RfContSingleX" );
    double rfContSingleArmZ     = dd4hep::_toDouble( "VMA:RfContSingleArmZ" );
    double rfContSingleArmAng   = dd4hep::_toDouble( "VMA:RfContSingleArmAng" );
    double rfContSingleEndZ     = dd4hep::_toDouble( "VMA:RfContSingleEndZ" );
    double rfContSingleEndAng   = dd4hep::_toDouble( "VMA:RfContSingleEndAng" );

    double rfContSingleArmAngCos = std::cos( rfContSingleArmAng );
    double rfContSingleArmAngSin = std::sin( rfContSingleArmAng );
    double rfContSingleArmPosY =
        rfContSingleArmZ / 2.0 * rfContSingleArmAngSin + rfContactRad - rfContSingleThick / 2.0;
    double rfContSingleArmPosZ =
        -rfContSingleArmZ / 2.0 - rfContSinglePlateZ / 2.0 + rfContSingleArmZ / 2.0 * ( 1.0 - rfContSingleArmAngCos );
    double rfContSingleEndAngCos = std::cos( rfContSingleEndAng );
    double rfContSingleEndAngSin = std::sin( rfContSingleEndAng );
    double rfContSingleEndPosY   = rfContSingleArmPosY + rfContSingleArmZ / 2.0 * rfContSingleArmAngSin +
                                 rfContSingleEndZ / 2.0 * rfContSingleEndAngSin;
    double rfContSingleEndPosZ = rfContSingleArmPosZ - rfContSingleArmZ / 2.0 * rfContSingleArmAngCos -
                                 rfContSingleEndZ / 2.0 + rfContSingleEndZ / 2.0 * ( 1 - rfContSingleEndAngCos ) -
                                 0.05 * dd4hep::mm;

    xml_det_t       x_rfContact = x_det.child( _Unicode( RfContact ) );
    dd4hep::Volume  vol_mount, vol_single, vol_ring;
    dd4hep::VisAttr contactVis = description.visAttributes( x_rfContact.visStr() );

    if ( contactVis.isValid() ) contactVis = visAttr;

    { // RF contact mount
      xml_det_t    x_rfContMount = x_rfContact.child( _Unicode( RfContactMount ) );
      dd4hep::Tube cyl( rfContactRad, rfContactRad + rfContactThick, rfContactCylZ / 2.0 );
      dd4hep::Tube flange( rfContactRad + rfContactThick, rfContactRad + rfContactThick + rfContactFlangeThick,
                           rfContactFlangeZ / 2.0 );
      dd4hep::SubtractionSolid solid( cyl, flange,
                                      dd4hep::Position( 0, 0, ( rfContactCylZ - rfContactFlangeZ ) / 2.0 ) );
      dd4hep::Material         mat = description.material( x_rfContMount.attr<std::string>( _U( material ) ) );
      vol_mount                    = dd4hep::Volume( lvName + "RfContMount", solid, mat );
      vol_mount.setVisAttributes( contactVis );
    }
    { // RF single contact
      xml_det_t                x_rfContSingle = x_rfContact.child( _Unicode( RfContactSingle ) );
      dd4hep::Tube             plate( rfContactRad, rfContactRad + rfContactThick, rfContSinglePlateZ / 2.0,
                          -M_PI / double( rfFingers ) + M_PI / 2.0, 2 * M_PI / double( rfFingers ) );
      dd4hep::Box              arm( rfContSingleX / 2.0, rfContSingleThick / 2.0, rfContSingleArmZ / 2.0 );
      dd4hep::Box              end( rfContSingleX / 2.0, rfContSingleThick / 2.0, rfContSingleEndZ / 2.0 );
      dd4hep::Position         posa( 0, rfContSingleArmPosY, rfContSingleArmPosZ );
      dd4hep::RotationZYX      rota( rfContSingleArmAng, 0, 0 );
      dd4hep::SubtractionSolid sub( plate, arm, dd4hep::Transform3D( rota, posa ) );
      dd4hep::Position         pose( 0, rfContSingleEndPosY, rfContSingleEndPosZ );
      dd4hep::RotationZYX      rote( rfContSingleEndAng, 0, 0 );
      dd4hep::SubtractionSolid solid( sub, end, dd4hep::Transform3D( rote, pose ) );
      dd4hep::Material         mat = description.material( x_rfContSingle.attr<std::string>( _U( material ) ) );
      vol_single                   = dd4hep::Volume( lvName + "RfContSingle", solid, mat );
      vol_single.setVisAttributes( contactVis );
    }
    { // RF Contact ring, 48 finger version
      double           delta_phi = 2.0 * M_PI / 48.0;
      double           r = 0, z = 0;
      dd4hep::Assembly vol( lvName + "ContactRing" );
      for ( int i = 0; i < rfFingers; ++i ) {
        dd4hep::PositionRhoZPhi pos( r, z, delta_phi * double( i ) - M_PI / 2.0 );
        dd4hep::RotationZYX     rot( 0, 0, double( i ) * delta_phi );
        vol.placeVolume( vol_single, dd4hep::Transform3D( rot, pos ) );
      }
      vol.setVisAttributes( contactVis );
      vol_ring = vol;
    }
    { // RF Contact
      dd4hep::Position pos_ring( 0, 0, ( rfContactCylZ - rfContSinglePlateZ ) / 2.0 - 1.0 * dd4hep::mm );
      dd4hep::Assembly vol( name + "RfContact" );
      vol.placeVolume( vol_mount );
      vol.placeVolume( vol_ring, pos_ring );
      vol.setVisAttributes( contactVis );
      RfContactPosZ = -rfContactCylZ / 2.0 + FullZ / 2.0;
      lvVMA.placeVolume( vol, dd4hep::Position( 0, 0, RfContactPosZ ) );
    }
  }

  void VMABuild::build_trans_tube() {
    xml_det_t x_trans               = x_det.child( _Unicode( TransitionTube ) );
    double    transTubeThick        = dd4hep::_toDouble( "VMA:TransTubeThick" );
    double    transTubeFlangeZ      = dd4hep::_toDouble( "VMA:TransTubeFlangeZ" );
    double    transTubeFlangeInnerZ = dd4hep::_toDouble( "VMA:TransTubeFlangeInnerZ" );
    double    transTubeFlangeInnerThick =
        dd4hep::_toDouble( "VMA:TransTubeFlangeInnerThickRaw" ) - transTubeRad - transTubeThick;
    double transTubeFlangeThick =
        dd4hep::_toDouble( "VMA:TransTubeFlangeThickRaw" ) - transTubeRad - transTubeThick - transTubeFlangeInnerThick;
    dd4hep::Material mat = description.material( x_trans.attr<std::string>( _U( material ) ) );
    dd4hep::VisAttr  vis = description.visAttributes( x_trans.visStr() );
    dd4hep::Volume   vol_tube, vol_flange;
    if ( vis.isValid() ) vis = visAttr;

    {
      dd4hep::Tube        tube( transTubeRad, transTubeRad + transTubeThick, transTubeCylZ / 2.0 );
      dd4hep::ConeSegment cons( transTubeConsZ / 2.0, transTubeRad, transTubeRad + transTubeThick, transTubeConsRad,
                                transTubeConsRad + transTubeThick );
      dd4hep::UnionSolid  solid( tube, cons, dd4hep::Position( 0, 0, ( transTubeCylZ + transTubeConsZ ) / 2.0 ) );
      vol_tube = dd4hep::Volume( lvName + "TransTube_Tube", solid, mat );
      vol_tube.setVisAttributes( vis );
    }
    {
      dd4hep::Tube       tub1( transTubeRad + transTubeThick + transTubeFlangeInnerThick + 0.01 * dd4hep::mm,
                         transTubeRad + transTubeThick + transTubeFlangeInnerThick + transTubeFlangeThick,
                         transTubeFlangeZ / 2.0 );
      dd4hep::Tube       tub2( transTubeRad + transTubeThick, transTubeRad + transTubeThick + transTubeFlangeInnerThick,
                         transTubeFlangeInnerZ / 2.0 );
      dd4hep::UnionSolid solid( tub1, tub2,
                                dd4hep::Position( 0, 0, ( transTubeFlangeInnerZ - transTubeFlangeZ ) / 2.0 ) );
      vol_flange = dd4hep::Volume( lvName + "TransTube_Flange", solid, mat );
      vol_flange.setVisAttributes( vis );
    }
    {
      dd4hep::Assembly vol( lvName + "TransTube" );
      dd4hep::Position pos( 0, 0, -transTubeCylZ / 2.0 + transTubeFlangeZ / 2.0 );
      double           transTubePosZ = transTubeCylZ / 2.0 - FullZ / 2.0;
      vol.placeVolume( vol_tube );
      vol.placeVolume( vol_flange, pos );
      vol.setVisAttributes( vis );
      lvVMA.placeVolume( vol, dd4hep::Position( 0, 0, transTubePosZ ) );
    }
  }

  void VMABuild::build_pipe() {
    xml_det_t x_pipe          = x_det.child( _Unicode( Pipe ) );
    double    EntryThick      = dd4hep::_toDouble( "VMA:EntryThick" );
    double    EntryRad        = dd4hep::_toDouble( "VMA:EntryRad" );
    double    EntryConsZ      = dd4hep::_toDouble( "VMA:EntryConsZ" );
    double    EntryConsRadEnd = dd4hep::_toDouble( "VMA:EntryConsRadEnd" );
    double    EntryUpCylZ     = entryUpCylZ;
    double    EntryDownCylZ   = dd4hep::_toDouble( "VMA:EntryDownCylZ" );

    double BellowZ        = dd4hep::_toDouble( "VMA:EntryDownCylZ" );
    double BellowPitch    = dd4hep::_toDouble( "VMA:BellowPitch" );
    double BellowThick    = dd4hep::_toDouble( "VMA:BellowThick" );
    double BellowInnerRad = dd4hep::_toDouble( "VMA:BellowInnerRad" );
    double BellowOuterRad = dd4hep::_toDouble( "VMA:BellowOuterRad" );
    double BellowCyl1Z    = dd4hep::_toDouble( "VMA:BellowCyl1Z" );
    double BellowCyl2Z    = dd4hep::_toDouble( "VMA:BellowCyl2Z" );

    double FlangeUpThick     = dd4hep::_toDouble( "VMA:FlangeUpThick" );
    double FlangeUpCylZ      = dd4hep::_toDouble( "VMA:FlangeUpCylZ" );
    double FlangeUpCylRad    = dd4hep::_toDouble( "VMA:FlangeUpCylRad" );
    double FlangeUpCylSubZ   = dd4hep::_toDouble( "VMA:FlangeUpCylSubZ" );
    double FlangeUpCylSubRad = dd4hep::_toDouble( "VMA:FlangeUpCylSubRad" );

    double           FlangeDownThick     = dd4hep::_toDouble( "VMA:FlangeDownThick" );
    double           FlangeDownCylZ      = dd4hep::_toDouble( "VMA:FlangeDownCylZ" );
    double           FlangeDownCylRad    = dd4hep::_toDouble( "VMA:FlangeDownCylRad" );
    double           FlangeDownCylSubZ   = dd4hep::_toDouble( "VMA:FlangeDownCylSubZ" );
    double           FlangeDownCylSubRad = dd4hep::_toDouble( "VMA:FlangeDownCylSubRad" );
    double           PipeCylZ            = dd4hep::_toDouble( "VMA:PipeCylZ" );
    dd4hep::VisAttr  vis                 = description.visAttributes( x_pipe.visStr() );
    dd4hep::Material mat                 = description.material( x_pipe.attr<std::string>( _U( material ) ) );

    if ( vis.isValid() ) vis = visAttr;

    FlangeUpPosZ   = FlangeUpCylZ / 2.0 - FullZ / 2.0;
    FlangeDownPosZ = FullZ / 2.0 - FlangeDownCylZ / 2.0;
    EntryUpPosZ    = BellowPosZ - BellowZ / 2.0 - EntryConsZ - EntryUpCylZ / 2.0;
    EntryDownPosZ  = FlangeDownPosZ - EntryDownCylZ / 2.0;
    BellowPosZ     = EntryDownPosZ - EntryDownCylZ / 2.0 - EntryConsZ - BellowZ / 2.0;
    PipePosZ       = EntryUpPosZ - EntryUpCylZ / 2.0 - PipeCylZ / 2.0;

    { // End pieces of bellows
      dd4hep::Tube        tube( EntryRad, EntryRad + EntryThick, EntryUpCylZ / 2.0 );
      dd4hep::ConeSegment cons( EntryConsZ / 2.0, EntryRad, EntryRad + EntryThick, EntryConsRadEnd,
                                EntryConsRadEnd + EntryThick );
      dd4hep::UnionSolid  solid( tube, cons, dd4hep::Position( 0, 0, ( EntryUpCylZ + EntryConsZ ) / 2.0 ) );
      dd4hep::Volume      vol( lvName + "EntryUp", solid, mat );
      vol.setVisAttributes( vis );
      lvVMA.placeVolume( vol, dd4hep::Position( 0, 0, EntryUpPosZ ) );
    }
    {
      dd4hep::Tube        tube( EntryRad, EntryRad + EntryThick, EntryDownCylZ / 2.0 );
      dd4hep::ConeSegment cons( EntryConsZ / 2.0, EntryRad, EntryRad + EntryThick, EntryConsRadEnd,
                                EntryConsRadEnd + EntryThick );
      dd4hep::UnionSolid  solid( tube, cons, dd4hep::Position( 0, 0, ( EntryUpCylZ + EntryConsZ ) / 2.0 ) );
      dd4hep::Volume      vol( lvName + "EntryDown", solid, mat );
      vol.setVisAttributes( vis );
      lvVMA.placeVolume( vol, dd4hep::Position( 0, 0, EntryDownPosZ ) );
    }
    { // Bellows
      double              dz, r1, r2;
      std::vector<double> rmin, rmax, z;
      // First 2 z-planes go by hand:
      z.push_back( -BellowZ / 2.0 );
      rmin.push_back( EntryConsRadEnd );
      rmax.push_back( EntryConsRadEnd + BellowThick );
      z.push_back( -BellowZ / 2.0 + BellowThick );
      rmin.push_back( BellowInnerRad );
      rmax.push_back( BellowInnerRad + BellowThick );
      for ( int i = 1; i < 13; ++i ) {
        if ( i <= 5 )
          dz = double( i ) * BellowPitch / 2.0 - BellowZ / 2.0;
        else if ( i <= 9 )
          dz = double( i - 1 ) * BellowPitch / 2.0 - BellowZ / 2.0 + BellowCyl1Z;
        else
          dz = double( i - 2 ) * BellowPitch / 2.0 - BellowZ / 2.0 + BellowCyl1Z + BellowCyl2Z;
        r1 = i % 2 == 1 ? BellowInnerRad : BellowOuterRad;
        r2 = i % 2 == 0 ? BellowInnerRad : BellowOuterRad;
        rmin.push_back( r1 );
        rmax.push_back( r1 + BellowThick );
        z.push_back( dz - BellowThick );
        rmin.push_back( r2 );
        rmax.push_back( r2 + BellowThick );
        z.push_back( dz + BellowThick );
#if 0
        dd4hep::printout(debug ? dd4hep::ALWAYS : dd4hep::INFO,
                         "VMA","+++ Adding zplane %-3d: z=%6.3f  rmin=%6.3f rmax=%6.3f [mm]",
                         2*i, dz-BellowThick, r1, r2);
        dd4hep::printout(debug ? dd4hep::ALWAYS : dd4hep::INFO,
                         "VMA","+++ Adding zplane %-3d: z=%6.3f  rmin=%6.3f rmax=%6.3f [mm]",
                         2*i+1, dz+BellowThick, r2, r1);
#endif
      }
      dd4hep::Polycone cone( 0, 2 * M_PI, rmin, rmax, z );
      dd4hep::Volume   vol( lvName + "Bellow", cone, mat );
      vol.setVisAttributes( vis );
      lvVMA.placeVolume( vol, dd4hep::Position( 0, 0, BellowPosZ ) );
    }
    { // Upstream Flange
      dd4hep::Tube             tub1( FlangeUpCylRad, FlangeUpCylRad + FlangeUpThick, FlangeUpCylZ / 2.0 );
      dd4hep::Tube             tub2( 0.0, FlangeUpCylSubRad, ( FlangeUpCylSubZ + 0.01 * dd4hep::mm ) / 2.0 );
      dd4hep::SubtractionSolid solid(
          tub1, tub2, dd4hep::Position( 0, 0, -FlangeUpCylSubZ / 2.0 + FlangeUpCylZ / 2.0 + 0.01 * dd4hep::mm ) );
      dd4hep::Volume vol( lvName + "FlangeUp", solid, mat );
      vol.setVisAttributes( vis );
      lvVMA.placeVolume( vol, dd4hep::Position( 0, 0, FlangeUpPosZ ) );
    }
    { // Downstream Flange
      dd4hep::Tube             tub1( FlangeDownCylRad, FlangeDownCylRad + FlangeDownThick, FlangeDownCylZ / 2.0 );
      dd4hep::Tube             tub2( 0.0, FlangeDownCylSubRad, ( FlangeDownCylSubZ + 0.01 * dd4hep::mm ) / 2.0 );
      dd4hep::SubtractionSolid solid(
          tub1, tub2, dd4hep::Position( 0, 0, -FlangeDownCylSubZ / 2.0 + FlangeDownCylZ / 2.0 + 0.01 * dd4hep::mm ) );
      dd4hep::Volume vol( lvName + "FlangeDown", solid, mat );
      vol.setVisAttributes( vis );
      lvVMA.placeVolume( vol, dd4hep::Position( 0, 0, FlangeDownPosZ ) );
    }
  }

  void VMABuild::build_vacuum() {
    double         VacuumRad = transTubeRad - 0.01 * dd4hep::mm;
    dd4hep::Volume vol( lvName + "Vacuum", dd4hep::Tube( 0, VacuumRad, FullZ ), description.vacuum() );
    vol.setVisAttributes( "Pipe:Vacuum" );
    lvVMA.placeVolume( vol );
  }

  void VMABuild::build_junction() {
    xml_det_t x_junction   = x_det.child( _Unicode( Junction ) );
    double    PipeCylZ     = dd4hep::_toDouble( "VMA:PipeCylZ" );
    double    PipeCylRad   = dd4hep::_toDouble( "VMA:PipeCylRad" );
    double    PipeCylThick = dd4hep::_toDouble( "VMA:PipeCylThick" );

    double PipeTopCylZ     = dd4hep::_toDouble( "VMA:PipeTopCylZ" );
    double PipeTopCylRad   = dd4hep::_toDouble( "VMA:PipeTopCylRad" );
    double PipeTopCylThick = dd4hep::_toDouble( "VMA:PipeTopCylThick" );
    double PipeTopCylPosZ  = dd4hep::_toDouble( "VMA:PipeTopCylPosZ" );

    double PipeSideCylZ     = dd4hep::_toDouble( "VMA:PipeSideCylZ" );
    double PipeSideCylRad   = dd4hep::_toDouble( "VMA:PipeSideCylRad" );
    double PipeSideCylThick = PipeCylThick;
    double PipeSideCylPosZ  = PipeTopCylPosZ;

    double FlangeTopCylZ     = dd4hep::_toDouble( "VMA:FlangeTopCylZ" );
    double FlangeTopCylRad   = PipeTopCylRad + PipeTopCylThick + 0.01 * dd4hep::mm;
    double FlangeTopCylThick = dd4hep::_toDouble( "VMA:FlangeTopCylThickRaw" ) - FlangeTopCylRad;

    double           FlangeSideCylZ     = dd4hep::_toDouble( "VMA:FlangeSideCylZ" );
    double           FlangeSideCylRad   = PipeSideCylRad + PipeSideCylThick + 0.01 * dd4hep::mm;
    double           FlangeSideCylThick = dd4hep::_toDouble( "VMA:FlangeSideCylThickRaw" ) - FlangeSideCylRad;
    dd4hep::Material mat                = description.material( x_junction.attr<std::string>( _U( material ) ) );
    dd4hep::VisAttr  vis                = description.visAttributes( x_junction.visStr() );

    if ( vis.isValid() ) vis = visAttr;

    dd4hep::Transform3D tr, tr_rotX( dd4hep::RotationZYX( 0, 0, M_PI / 2.0 ), dd4hep::Position() );
    dd4hep::Volume      pipe_main, pipe_top, pipe_side, flange_top, flange_side, lid_top, lid_side;
    { // Main pipe
      dd4hep::Tube tub1( PipeCylRad, PipeCylRad + PipeCylThick, PipeCylZ / 2.0 );
      dd4hep::Tube tub2( 0.0, PipeTopCylRad, PipeTopCylZ / 2.0 );
      dd4hep::Tube tub3( 0.0, PipeSideCylRad, PipeSideCylZ / 2.0 );
      tr = ROOT::Math::Translation3D( 0, PipeTopCylZ / 2, PipeTopCylPosZ - PipeCylZ / 2 ) * tr_rotX;
      dd4hep::SubtractionSolid sub( tub1, tub2, tr );
      tr = ROOT::Math::Translation3D( 0, -PipeSideCylZ / 2, PipeSideCylPosZ - PipeCylZ / 2 ) * tr_rotX;
      dd4hep::SubtractionSolid solid( sub, tub3, tr );
      pipe_main = dd4hep::Volume( lvName + "PipeMain", solid, mat );
      pipe_main.setVisAttributes( vis );
    }
    { // Top pipe
      dd4hep::Tube tub1( PipeTopCylRad, PipeTopCylRad + PipeCylThick, PipeCylZ / 2.0 );
      dd4hep::Tube tub2( PipeCylRad, PipeCylRad + PipeCylThick + 0.01 * dd4hep::mm, PipeTopCylZ / 2.0 );
      tr = ROOT::Math::Translation3D( 0, PipeTopCylZ / 2, PipeTopCylPosZ - PipeCylZ / 2 ) * tr_rotX;
      dd4hep::SubtractionSolid solid( tub1, tub2, tr );
      pipe_top = dd4hep::Volume( lvName + "PipeTop", solid, mat );
      pipe_top.setVisAttributes( vis );
    }
    { // Top pipe flange
      dd4hep::Tube tube( FlangeTopCylRad, FlangeTopCylRad + FlangeTopCylThick, FlangeTopCylZ / 2.0 );
      flange_top = dd4hep::Volume( lvName + "FlangeTop", tube, mat );
      flange_top.setVisAttributes( vis );
    }
    { // Top pipe lid
      dd4hep::Tube tube( FlangeTopCylRad, FlangeTopCylRad + FlangeTopCylThick, FlangeTopCylZ / 2.0 );
      lid_top = dd4hep::Volume( lvName + "LidTop", tube, mat );
      lid_top.setVisAttributes( vis );
    }
    { // Side pipe
      dd4hep::Tube tub1( PipeSideCylRad, PipeSideCylRad + PipeCylThick, PipeSideCylZ / 2.0 );
      dd4hep::Tube tub2( PipeCylRad, PipeCylRad + PipeCylThick + 0.01 * dd4hep::mm, PipeCylZ / 2.0 );
      tr = ROOT::Math::Translation3D( 0, 0, PipeSideCylPosZ ) * tr_rotX;
      dd4hep::SubtractionSolid solid( tub1, tub2, tr );
      pipe_side = dd4hep::Volume( lvName + "PipeSide", solid, mat );
      pipe_side.setVisAttributes( vis );
    }
    { // Side pipe flange
      dd4hep::Tube tube( FlangeSideCylRad, FlangeSideCylRad + FlangeSideCylThick, FlangeSideCylZ / 2.0 );
      flange_side = dd4hep::Volume( lvName + "FlangeSide", tube, mat );
      flange_side.setVisAttributes( vis );
    }
    { // Side pipe lid
      dd4hep::Tube tube( FlangeSideCylRad, FlangeSideCylRad + FlangeSideCylThick, FlangeSideCylZ / 2.0 );
      lid_side = dd4hep::Volume( lvName + "LidSide", tube, mat );
      lid_side.setVisAttributes( vis );
    }
    {
      dd4hep::Assembly vol( lvName + "Pipe" );
      vol.placeVolume( pipe_main );
      tr = ROOT::Math::Translation3D( 0, PipeTopCylZ / 2.0, PipeTopCylPosZ - PipeCylZ / 2.0 ) * tr_rotX;
      vol.placeVolume( pipe_top, tr );
      tr = ROOT::Math::Translation3D( 0, PipeTopCylZ - FlangeTopCylZ / 2.0, PipeTopCylPosZ - PipeCylZ / 2.0 ) * tr_rotX;
      vol.placeVolume( flange_top, tr );
      tr = ROOT::Math::Translation3D( 0, PipeTopCylZ - FlangeTopCylZ / 2.0 + 0.01 * dd4hep::mm,
                                      PipeTopCylPosZ - PipeCylZ / 2.0 ) *
           tr_rotX;
      vol.placeVolume( lid_top, tr );
      tr = ROOT::Math::Translation3D( 0, -PipeSideCylZ / 2.0, PipeSideCylPosZ - PipeCylZ / 2.0 ) * tr_rotX;
      vol.placeVolume( pipe_side, tr );
      tr = ROOT::Math::Translation3D( 0, -PipeSideCylZ + FlangeSideCylZ / 2.0, PipeSideCylPosZ - PipeCylZ / 2.0 ) *
           tr_rotX;
      vol.placeVolume( flange_side, tr );
      tr = ROOT::Math::Translation3D( 0, -PipeSideCylZ - FlangeSideCylZ / 2.0 - 0.01 * dd4hep::mm,
                                      PipeSideCylPosZ - PipeCylZ / 2.0 ) *
           tr_rotX;
      vol.placeVolume( lid_side, tr );
      vol.setVisAttributes( vis );
      lvVMA.placeVolume( vol, dd4hep::Position( 0, 0, PipePosZ ) );
    }
  }
} // namespace

static dd4hep::Ref_t create_element( dd4hep::Detector& description, xml_h e, dd4hep::SensitiveDetector sens_det ) {
  VMABuild builder( description, e, sens_det );
  builder.build();
  return builder.detector;
}
DECLARE_DETELEMENT( LHCb_Pipe_VMA_v1_0, create_element )
