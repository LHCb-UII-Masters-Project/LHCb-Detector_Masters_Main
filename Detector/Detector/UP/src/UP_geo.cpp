//==========================================================================
//  AIDA Detector description implementation
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
// Author     : Shuqi Sheng & Ji Peng
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

using namespace dd4hep;

namespace {

  /// Helper class to build the UP detector of LHCb
  struct UPBuild : public xml::tools::VolumeBuilder {
    // Reusable DetElements for cloning
    DetElement de_pixel_module_a;
    DetElement de_pixel_module_b;
    DetElement de_pixel_module_c;
    DetElement de_pixel_module_d;
    DetElement de_pixel_module_e;
    DetElement de_pixel_module_f;
    DetElement de_pixel_module_g;
    DetElement de_stave_e;
    DetElement de_stave_f;
    DetElement de_stave_g;
    DetElement de_stave_h;

    // Debug flags
    bool m_build_sides      = true;
    bool m_build_frame      = true;
    bool m_build_box        = true;
    bool m_build_boxplug    = true;
    bool m_build_jacket     = true;
    bool m_build_pipeheater = true;
    bool m_build_aside      = true;
    bool m_build_aside_upax = true;
    bool m_build_aside_upau = true;
    bool m_build_aside_upbv = true;
    bool m_build_aside_upbx = true;
    bool m_build_cside      = true;
    bool m_build_cside_upax = true;
    bool m_build_cside_upau = true;
    bool m_build_cside_upbv = true;
    bool m_build_cside_upbx = true;

    // Select a particular volume to build
    std::string select_volume{"lvUP"};

    /// Initializing constructor
    UPBuild( Detector& description, xml_elt_t e, SensitiveDetector sens );
    void build_modules();
    void build_staves();
    void build_detector();
    void build_groups();
  };

  /// Initializing constructor
  UPBuild::UPBuild( Detector& dsc, xml_elt_t e, SensitiveDetector sens ) : xml::tools::VolumeBuilder( dsc, e, sens ) {
    // Process debug flags
    xml_comp_t x_dbg = x_det.child( _U( debug ), false );
    if ( x_dbg ) {
      for ( xml_coll_t i( x_dbg, _U( item ) ); i; ++i ) {
        xml_comp_t  c( i );
        std::string n = c.nameStr();
        if ( n == "Build_Sides" )
          m_build_sides = c.attr<bool>( _U( value ) );
        else if ( n == "Build_Aside" )
          m_build_aside = c.attr<bool>( _U( value ) );
        else if ( n == "Build_Aside_UPaX" )
          m_build_aside_upax = c.attr<bool>( _U( value ) );
        else if ( n == "Build_Aside_UPaU" )
          m_build_aside_upau = c.attr<bool>( _U( value ) );
        else if ( n == "Build_Aside_UPbV" )
          m_build_aside_upbv = c.attr<bool>( _U( value ) );
        else if ( n == "Build_Aside_UPbX" )
          m_build_aside_upbx = c.attr<bool>( _U( value ) );
        else if ( n == "Build_Cside" )
          m_build_cside = c.attr<bool>( _U( value ) );
        else if ( n == "Build_Cside_UPaX" )
          m_build_cside_upax = c.attr<bool>( _U( value ) );
        else if ( n == "Build_Cside_UPaU" )
          m_build_cside_upau = c.attr<bool>( _U( value ) );
        else if ( n == "Build_Cside_UPbV" )
          m_build_cside_upbv = c.attr<bool>( _U( value ) );
        else if ( n == "Build_Cside_UPbX" )
          m_build_cside_upbx = c.attr<bool>( _U( value ) );
        else if ( n == "Build_Frame" )
          m_build_frame = c.attr<bool>( _U( value ) );
        else if ( n == "Build_Jacket" )
          m_build_jacket = c.attr<bool>( _U( value ) );
        else if ( n == "Build_PipeHeater" )
          m_build_pipeheater = c.attr<bool>( _U( value ) );
        else if ( n == "Build_Box" )
          m_build_box = c.attr<bool>( _U( value ) );
        else if ( n == "Build_BoxPlug" )
          m_build_boxplug = c.attr<bool>( _U( value ) );
        else if ( n == "select_volume" )
          select_volume = c.attr<std::string>( _U( value ) );
        else if ( n == "debug" ) {
          debug = c.attr<bool>( _U( value ) );
        }
      }
    }
  }

  void UPBuild::build_modules() {
    PlacedVolume                pv;
    [[maybe_unused]] Volume     lvModule, lvGroup, lvSector, lvSensor;
    Volume                      lvonechip;
    DetElement                  deGroup, deSector, deSensor;
    [[maybe_unused]] DetElement de_onesector, de_onegroup;

    de_onesector = DetElement( "chip", id );
    lvSector     = volume( "lvSectorFourteen" );
    pv           = lvSector.placeVolume( volume( "lvSensorFourteen" ) );
    pv->SetName( "pvSensorFourteen" );
    deSensor = DetElement( de_onesector, "sensor0", 0 );
    deSensor.setPlacement( pv );

    // de_onegroup     = DetElement( "Group" );
    lvonechip = volume( "lvOnePixelGroup" );
    pv        = lvonechip.placeVolume(
        lvSector, Position( 0, _toDouble( "-UPPixelLong/2+UPPixelBufferLong+UPPixelSideLong+UPPixelTrueLong/2" ), 0 ) );
    pv->SetName( "pvSectorOnePixelGroup" );
    // pv.addPhysVolID( "sector", 0 );
    deSector = de_onesector.clone( "Sector0", 0 );
    deSector.setPlacement( pv );

    // Pixel Module A-F
    lvGroup = volume( "lvPixelGroupA" );

    de_pixel_module_a = DetElement( "module", id );
    de_pixel_module_b = DetElement( "module", id );
    de_pixel_module_c = DetElement( "module", id );
    de_pixel_module_d = DetElement( "module", id );
    de_pixel_module_e = DetElement( "module", id );
    de_pixel_module_f = DetElement( "module", id );

    DetElement deGroup_a;
    DetElement deGroup_b;
    DetElement deGroup_c;
    DetElement deGroup_d;
    DetElement deGroup_e;
    DetElement deGroup_f;

    deGroup_a = DetElement( de_pixel_module_a, "PixelGroupA", 0 );
    deGroup_b = DetElement( de_pixel_module_b, "PixelGroupA", 0 );
    deGroup_c = DetElement( de_pixel_module_c, "PixelGroupA", 0 );
    deGroup_d = DetElement( de_pixel_module_d, "PixelGroupA", 0 );
    deGroup_e = DetElement( de_pixel_module_e, "PixelGroupA", 0 );
    deGroup_f = DetElement( de_pixel_module_f, "PixelGroupA", 0 );

    pv = lvGroup.placeVolume(
        lvonechip, dd4hep::Transform3D( RotationZYX( M_PI, 0, 0 ),
                                        dd4hep::Position( _toDouble( "SetpvChipA1X" ), _toDouble( "SetpvChipA1Y" ),
                                                          _toDouble( "UPSensorSiZ" ) ) ) );
    pv->SetName( "pvChipA1" );
    pv.addPhysVolID( "chip", 0 );
    deSector = de_onesector.clone( "chip0", 0 );
    deSector.setPlacement( pv );
    deGroup_a.add( deSector );
    deGroup_b.add( deSector );
    deGroup_c.add( deSector );
    deGroup_d.add( deSector );
    deGroup_e.add( deSector );
    deGroup_f.add( deSector );

    pv = lvGroup.placeVolume(
        lvonechip, Position( _toDouble( "SetpvChipA2X" ), _toDouble( "SetpvChipA2Y" ), _toDouble( "UPSensorSiZ" ) ) );
    pv->SetName( "pvChipA2" );
    pv.addPhysVolID( "chip", 1 );
    deSector = de_onesector.clone( "chip01", 1 );
    deSector.setPlacement( pv );
    deGroup_a.add( deSector );
    deGroup_b.add( deSector );
    deGroup_c.add( deSector );
    deGroup_d.add( deSector );
    deGroup_e.add( deSector );
    deGroup_f.add( deSector );

    pv = lvGroup.placeVolume(
        lvonechip, dd4hep::Transform3D( RotationZYX( M_PI, 0, 0 ),
                                        dd4hep::Position( _toDouble( "SetpvChipA3X" ), _toDouble( "SetpvChipA3Y" ),
                                                          _toDouble( "UPSensorSiZ" ) ) ) );
    pv->SetName( "pvChipA3" );
    pv.addPhysVolID( "chip", 2 );
    deSector = de_onesector.clone( "chip02", 2 );
    deSector.setPlacement( pv );
    deGroup_a.add( deSector );
    deGroup_b.add( deSector );
    deGroup_c.add( deSector );
    deGroup_d.add( deSector );
    deGroup_e.add( deSector );
    deGroup_f.add( deSector );

    pv = lvGroup.placeVolume(
        lvonechip, Position( _toDouble( "SetpvChipA4X" ), _toDouble( "SetpvChipA4Y" ), _toDouble( "UPSensorSiZ" ) ) );
    pv->SetName( "pvChipA4" );
    pv.addPhysVolID( "chip", 3 );
    deSector = de_onesector.clone( "chip03", 3 );
    deSector.setPlacement( pv );
    deGroup_a.add( deSector );
    deGroup_b.add( deSector );
    deGroup_c.add( deSector );
    deGroup_d.add( deSector );
    deGroup_e.add( deSector );
    deGroup_f.add( deSector );

    pv = lvGroup.placeVolume(
        lvonechip, dd4hep::Transform3D( RotationZYX( M_PI, 0, 0 ),
                                        dd4hep::Position( _toDouble( "SetpvChipA5X" ), _toDouble( "SetpvChipA5Y" ),
                                                          _toDouble( "UPSensorSiZ" ) ) ) );
    pv->SetName( "pvChipA5" );
    pv.addPhysVolID( "chip", 4 );
    deSector = de_onesector.clone( "chip04", 4 );
    deSector.setPlacement( pv );
    deGroup_a.add( deSector );
    deGroup_b.add( deSector );
    deGroup_c.add( deSector );
    deGroup_d.add( deSector );
    deGroup_e.add( deSector );
    deGroup_f.add( deSector );

    pv = lvGroup.placeVolume(
        lvonechip, Position( _toDouble( "SetpvChipA6X" ), _toDouble( "SetpvChipA6Y" ), _toDouble( "UPSensorSiZ" ) ) );
    pv->SetName( "pvChipA6" );
    pv.addPhysVolID( "chip", 5 );
    deSector = de_onesector.clone( "chip05", 5 );
    deSector.setPlacement( pv );
    deGroup_a.add( deSector );
    deGroup_b.add( deSector );
    deGroup_c.add( deSector );
    deGroup_d.add( deSector );
    deGroup_e.add( deSector );
    deGroup_f.add( deSector );

    pv = lvGroup.placeVolume(
        lvonechip, dd4hep::Transform3D( RotationZYX( M_PI, 0, 0 ), dd4hep::Position( 0, _toDouble( "SetpvChipA7Y" ),
                                                                                     _toDouble( "UPSensorSiZ" ) ) ) );
    pv->SetName( "pvChipA7" );
    pv.addPhysVolID( "chip", 6 );
    deSector = de_onesector.clone( "chip06", 6 );
    deSector.setPlacement( pv );
    deGroup_a.add( deSector );
    deGroup_b.add( deSector );
    deGroup_c.add( deSector );
    deGroup_d.add( deSector );
    deGroup_e.add( deSector );
    deGroup_f.add( deSector );

    pv = lvGroup.placeVolume( lvonechip, Position( 0, _toDouble( "SetpvChipA8Y" ), _toDouble( "UPSensorSiZ" ) ) );
    pv->SetName( "pvChipA8" );
    pv.addPhysVolID( "chip", 7 );
    deSector = de_onesector.clone( "chip07", 7 );
    deSector.setPlacement( pv );
    deGroup_a.add( deSector );
    deGroup_b.add( deSector );
    deGroup_c.add( deSector );
    deGroup_d.add( deSector );
    deGroup_e.add( deSector );
    deGroup_f.add( deSector );

    pv = lvGroup.placeVolume(
        lvonechip, dd4hep::Transform3D( RotationZYX( M_PI, 0, 0 ),
                                        dd4hep::Position( _toDouble( "SetpvChipA9X" ), _toDouble( "SetpvChipA9Y" ),
                                                          _toDouble( "UPSensorSiZ" ) ) ) );
    pv->SetName( "pvChipA9" );
    pv.addPhysVolID( "chip", 8 );
    deSector = de_onesector.clone( "chip08", 8 );
    deSector.setPlacement( pv );
    deGroup_a.add( deSector );
    deGroup_b.add( deSector );
    deGroup_c.add( deSector );
    deGroup_d.add( deSector );
    deGroup_e.add( deSector );
    deGroup_f.add( deSector );

    pv = lvGroup.placeVolume(
        lvonechip, Position( _toDouble( "SetpvChipA10X" ), _toDouble( "SetpvChipA10Y" ), _toDouble( "UPSensorSiZ" ) ) );
    pv->SetName( "pvChipA10" );
    pv.addPhysVolID( "chip", 9 );
    deSector = de_onesector.clone( "chip09", 9 );
    deSector.setPlacement( pv );
    deGroup_a.add( deSector );
    deGroup_b.add( deSector );
    deGroup_c.add( deSector );
    deGroup_d.add( deSector );
    deGroup_e.add( deSector );
    deGroup_f.add( deSector );

    pv = lvGroup.placeVolume(
        lvonechip, dd4hep::Transform3D( RotationZYX( M_PI, 0, 0 ),
                                        dd4hep::Position( _toDouble( "SetpvChipA11X" ), _toDouble( "SetpvChipA11Y" ),
                                                          _toDouble( "UPSensorSiZ" ) ) ) );
    pv->SetName( "pvChipA11" );
    pv.addPhysVolID( "chip", 10 );
    deSector = de_onesector.clone( "chip10", 10 );
    deSector.setPlacement( pv );
    deGroup_a.add( deSector );
    deGroup_b.add( deSector );
    deGroup_c.add( deSector );
    deGroup_d.add( deSector );
    deGroup_e.add( deSector );
    deGroup_f.add( deSector );

    pv = lvGroup.placeVolume(
        lvonechip, Position( _toDouble( "SetpvChipA12X" ), _toDouble( "SetpvChipA12Y" ), _toDouble( "UPSensorSiZ" ) ) );
    pv->SetName( "pvChipA12" );
    pv.addPhysVolID( "chip", 11 );
    deSector = de_onesector.clone( "chip11", 11 );
    deSector.setPlacement( pv );
    deGroup_a.add( deSector );
    deGroup_b.add( deSector );
    deGroup_c.add( deSector );
    deGroup_d.add( deSector );
    deGroup_e.add( deSector );
    deGroup_f.add( deSector );

    pv = lvGroup.placeVolume(
        lvonechip, dd4hep::Transform3D( RotationZYX( M_PI, 0, 0 ),
                                        dd4hep::Position( _toDouble( "SetpvChipA13X" ), _toDouble( "SetpvChipA13Y" ),
                                                          _toDouble( "UPSensorSiZ" ) ) ) );
    pv->SetName( "pvChipA13" );
    pv.addPhysVolID( "chip", 12 );
    deSector = de_onesector.clone( "chip12", 12 );
    deSector.setPlacement( pv );
    deGroup_a.add( deSector );
    deGroup_b.add( deSector );
    deGroup_c.add( deSector );
    deGroup_d.add( deSector );
    deGroup_e.add( deSector );
    deGroup_f.add( deSector );

    pv = lvGroup.placeVolume(
        lvonechip, Position( _toDouble( "SetpvChipA14X" ), _toDouble( "SetpvChipA14Y" ), _toDouble( "UPSensorSiZ" ) ) );
    pv->SetName( "pvChipA14" );
    pv.addPhysVolID( "chip", 13 );
    deSector = de_onesector.clone( "chip13", 13 );
    deSector.setPlacement( pv );
    deGroup_a.add( deSector );
    deGroup_b.add( deSector );
    deGroup_c.add( deSector );
    deGroup_d.add( deSector );
    deGroup_e.add( deSector );
    deGroup_f.add( deSector );

    lvModule = volume( "lvPixelModuleA" );
    pv       = lvModule.placeVolume( lvGroup, Position( 0, _toDouble( "UPPixelYD" ), _toDouble( "UPPixelZ" ) ) );
    pv->SetName( "pvGroup2E1" );
    deGroup_a.setPlacement( pv );

    lvModule = volume( "lvPixelModuleB" );
    pv       = lvModule.placeVolume( lvGroup, Position( 0, _toDouble( "UPPixelYD" ), _toDouble( "UPPixelZ" ) ) );
    pv->SetName( "pvGroup2E2" );
    deGroup_b.setPlacement( pv );

    lvModule = volume( "lvPixelModuleC" );
    pv       = lvModule.placeVolume( lvGroup, Position( 0, _toDouble( "UPPixelYEL" ), _toDouble( "UPPixelZ" ) ) );
    pv->SetName( "pvPixelModuleC" );
    deGroup_c.setPlacement( pv );

    lvModule = volume( "lvPixelModuleD" );
    pv       = lvModule.placeVolume( lvGroup, Position( 0, _toDouble( "UPPixelYEL" ), _toDouble( "UPPixelZ" ) ) );
    pv->SetName( "pvPixelModuleD" );
    deGroup_d.setPlacement( pv );

    lvModule = volume( "lvPixelModuleE" );
    pv       = lvModule.placeVolume( lvGroup, Position( 0, _toDouble( "UPPixelYEL" ), _toDouble( "UPPixelZ" ) ) );
    pv->SetName( "pvPixelModuleE" );
    deGroup_e.setPlacement( pv );

    lvModule = volume( "lvPixelModuleF" );
    pv       = lvModule.placeVolume( lvGroup, Position( 0, _toDouble( "UPPixelYELL" ), _toDouble( "UPPixelZ" ) ) );
    pv->SetName( "pvPixelModuleF" );
    deGroup_f.setPlacement( pv );

    // Pixel Module G
    de_pixel_module_g = DetElement( "module", id );
    deGroup           = DetElement( de_pixel_module_g, "PixelGroupB", 0 );
    lvGroup           = volume( "lvPixelGroupB" );

    pv = lvGroup.placeVolume(
        lvonechip, dd4hep::Transform3D( RotationZYX( M_PI, 0, 0 ),
                                        dd4hep::Position( _toDouble( "SetMGpvChipA1X" ), _toDouble( "SetMGpvChipA1Y" ),
                                                          _toDouble( "UPSensorSiZ" ) ) ) );
    pv->SetName( "pvChipA1" );
    pv.addPhysVolID( "chip", 0 );
    deSector = de_onesector.clone( "chip0", 0 );
    deSector.setPlacement( pv );
    deGroup.add( deSector );

    pv = lvGroup.placeVolume( lvonechip, Position( _toDouble( "SetMGpvChipA2X" ), _toDouble( "SetMGpvChipA2Y" ),
                                                   _toDouble( "UPSensorSiZ" ) ) );
    pv->SetName( "pvChipA2" );
    pv.addPhysVolID( "chip", 1 );
    deSector = de_onesector.clone( "chip01", 1 );
    deSector.setPlacement( pv );
    deGroup.add( deSector );

    pv = lvGroup.placeVolume(
        lvonechip, dd4hep::Transform3D( RotationZYX( M_PI, 0, 0 ),
                                        dd4hep::Position( _toDouble( "SetMGpvChipA3X" ), _toDouble( "SetMGpvChipA3Y" ),
                                                          _toDouble( "UPSensorSiZ" ) ) ) );
    pv->SetName( "pvChipA3" );
    pv.addPhysVolID( "chip", 2 );
    deSector = de_onesector.clone( "chip02", 2 );
    deSector.setPlacement( pv );
    deGroup.add( deSector );

    pv = lvGroup.placeVolume( lvonechip, Position( _toDouble( "SetMGpvChipA4X" ), _toDouble( "SetMGpvChipA4Y" ),
                                                   _toDouble( "UPSensorSiZ" ) ) );
    pv->SetName( "pvChipA4" );
    pv.addPhysVolID( "chip", 3 );
    deSector = de_onesector.clone( "chip03", 3 );
    deSector.setPlacement( pv );
    deGroup.add( deSector );

    pv = lvGroup.placeVolume(
        lvonechip, dd4hep::Transform3D( RotationZYX( M_PI, 0, 0 ),
                                        dd4hep::Position( _toDouble( "SetMGpvChipA5X" ), _toDouble( "SetMGpvChipA5Y" ),
                                                          _toDouble( "UPSensorSiZ" ) ) ) );
    pv->SetName( "pvChipA5" );
    pv.addPhysVolID( "chip", 4 );
    deSector = de_onesector.clone( "chip04", 4 );
    deSector.setPlacement( pv );
    deGroup.add( deSector );

    pv = lvGroup.placeVolume( lvonechip, Position( _toDouble( "SetMGpvChipA6X" ), _toDouble( "SetMGpvChipA6Y" ),
                                                   _toDouble( "UPSensorSiZ" ) ) );
    pv->SetName( "pvChipA6" );
    pv.addPhysVolID( "chip", 5 );
    deSector = de_onesector.clone( "chip05", 5 );
    deSector.setPlacement( pv );
    deGroup.add( deSector );

    pv = lvGroup.placeVolume(
        lvonechip, dd4hep::Transform3D( RotationZYX( M_PI, 0, 0 ), dd4hep::Position( 0, _toDouble( "SetMGpvChipA7Y" ),
                                                                                     _toDouble( "UPSensorSiZ" ) ) ) );
    pv->SetName( "pvChipA7" );
    pv.addPhysVolID( "chip", 6 );
    deSector = de_onesector.clone( "chip06", 6 );
    deSector.setPlacement( pv );
    deGroup.add( deSector );

    pv = lvGroup.placeVolume( lvonechip, Position( 0, _toDouble( "SetMGpvChipA8Y" ), _toDouble( "UPSensorSiZ" ) ) );
    pv->SetName( "pvChipA8" );
    pv.addPhysVolID( "chip", 7 );
    deSector = de_onesector.clone( "chip07", 7 );
    deSector.setPlacement( pv );
    deGroup.add( deSector );

    pv = lvGroup.placeVolume(
        lvonechip, dd4hep::Transform3D( RotationZYX( M_PI, 0, 0 ),
                                        dd4hep::Position( _toDouble( "SetMGpvChipA9X" ), _toDouble( "SetMGpvChipA9Y" ),
                                                          _toDouble( "UPSensorSiZ" ) ) ) );
    pv->SetName( "pvChipA9" );
    pv.addPhysVolID( "chip", 8 );
    deSector = de_onesector.clone( "chip08", 8 );
    deSector.setPlacement( pv );
    deGroup.add( deSector );

    pv = lvGroup.placeVolume( lvonechip, Position( _toDouble( "SetMGpvChipA10X" ), _toDouble( "SetMGpvChipA10Y" ),
                                                   _toDouble( "UPSensorSiZ" ) ) );
    pv->SetName( "pvChipA10" );
    pv.addPhysVolID( "chip", 9 );
    deSector = de_onesector.clone( "chip09", 9 );
    deSector.setPlacement( pv );
    deGroup.add( deSector );

    lvModule = volume( "lvPixelModuleG" );
    pv       = lvModule.placeVolume( lvGroup, Position( 0, _toDouble( "UPPixelYELL" ), _toDouble( "UPPixelZ" ) ) );
    pv->SetName( "pvPixelModuleG" );
    deGroup.setPlacement( pv );
  }

  void UPBuild::build_staves() {

    PlacedVolume pv;
    Volume       lvStave, lvStaveFace;
    DetElement   deStaveFace, deModule;

    build_modules();
    // Stave E
    de_stave_e = DetElement( "stave", id );
    lvStave    = volume( "lvStaveE" );

    // Stave E Front
    deStaveFace = DetElement( de_stave_e, "Face1", 1 );
    lvStaveFace = volume( "lvStaveEFront" );

    // Module 3
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleA" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "SetUPStavEFrontModule3" ), 0 ) ) );
    pv->SetName( "pvModuleE0" );
    pv.addPhysVolID( "module", 0 );
    deModule = de_pixel_module_a.clone( "Module0", 0 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 4
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ), Position( 0, _toDouble( "SetUPStavEFrontModule4" ), 0 ) ) );
    pv->SetName( "pvModuleE1" );
    pv.addPhysVolID( "module", 1 );
    deModule = de_pixel_module_b.clone( "Module1", 1 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 7
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleA" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "SetUPStavEFrontModule7" ), 0 ) ) );
    pv->SetName( "pvModuleE2" );
    pv.addPhysVolID( "module", 2 );
    deModule = de_pixel_module_a.clone( "Module2", 2 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 8
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ), Position( 0, _toDouble( "SetUPStavEFrontModule8" ), 0 ) ) );
    pv->SetName( "pvModuleE3" );
    pv.addPhysVolID( "module", 3 );
    deModule = de_pixel_module_b.clone( "Module3", 3 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 11
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleA" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "SetUPStavEFrontModule11" ), 0 ) ) );
    pv->SetName( "pvModuleE4" );
    pv.addPhysVolID( "module", 4 );
    deModule = de_pixel_module_a.clone( "Module4", 4 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 12
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ), Position( 0, _toDouble( "SetUPStavEFrontModule12" ), 0 ) ) );
    pv->SetName( "pvModuleE5" );
    pv.addPhysVolID( "module", 5 );
    deModule = de_pixel_module_b.clone( "Module5", 5 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 15
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleA" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "SetUPStavEFrontModule15" ), 0 ) ) );
    pv->SetName( "pvModuleE6" );
    pv.addPhysVolID( "module", 6 );
    deModule = de_pixel_module_a.clone( "Module6", 6 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 16
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ), Position( 0, _toDouble( "SetUPStavEFrontModule16" ), 0 ) ) );
    pv->SetName( "pvModuleE7" );
    pv.addPhysVolID( "module", 7 );
    deModule = de_pixel_module_b.clone( "Module7", 7 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 20
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ), Position( 0, _toDouble( "SetUPStavEFrontModule20" ), 0 ) ) );
    pv->SetName( "pvModuleE9" );
    pv.addPhysVolID( "module", 9 );
    deModule = de_pixel_module_b.clone( "Module9", 9 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 21
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleA" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "SetUPStavEFrontModule21" ), 0 ) ) );
    pv->SetName( "pvModuleE10" );
    pv.addPhysVolID( "module", 10 );
    deModule = de_pixel_module_a.clone( "Module10", 10 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 22
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ), Position( 0, _toDouble( "SetUPStavEFrontModule22" ), 0 ) ) );
    pv->SetName( "pvModuleE11" );
    pv.addPhysVolID( "module", 11 );
    deModule = de_pixel_module_b.clone( "Module11", 11 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 25
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleA" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "SetUPStavEFrontModule25" ), 0 ) ) );
    pv->SetName( "pvModuleE12" );
    pv.addPhysVolID( "module", 12 );
    deModule = de_pixel_module_a.clone( "Module12", 12 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 26
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ), Position( 0, _toDouble( "SetUPStavEFrontModule26" ), 0 ) ) );
    pv->SetName( "pvModuleE13" );
    pv.addPhysVolID( "module", 13 );
    deModule = de_pixel_module_b.clone( "Module13", 13 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 29
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleA" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "SetUPStavEFrontModule29" ), 0 ) ) );
    pv->SetName( "pvModuleE14" );
    pv.addPhysVolID( "module", 14 );
    deModule = de_pixel_module_a.clone( "Module14", 14 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 30
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ), Position( 0, _toDouble( "SetUPStavEFrontModule30" ), 0 ) ) );
    pv->SetName( "pvModuleE15" );
    pv.addPhysVolID( "module", 15 );
    deModule = de_pixel_module_b.clone( "Module15", 15 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 33
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleA" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "SetUPStavEFrontModule33" ), 0 ) ) );
    pv->SetName( "pvModuleE16" );
    pv.addPhysVolID( "module", 16 );
    deModule = de_pixel_module_a.clone( "Module16", 16 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );
    pv = lvStave.placeVolume( lvStaveFace, Position( 0, 0, _toDouble( "UPPixelStaveHybridZ" ) ) );
    pv->SetName( "pvStaveEFront" );
    pv.addPhysVolID( "face", 1 );
    deStaveFace.setPlacement( pv );

    // Stave E Back
    deStaveFace = DetElement( de_stave_e, "Face0", 0 );
    lvStaveFace = volume( "lvStaveEBack" );

    // Module 1
    pv = lvStaveFace.placeVolume( volume( "lvPixelModuleA" ), Position( 0, _toDouble( "SetUPStavEBackModule1" ), 0 ) );
    pv->SetName( "pvModuleE0" );
    pv.addPhysVolID( "module", 0 );
    deModule = de_pixel_module_a.clone( "Module0", 0 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 2
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "SetUPStavEBackModule2" ), 0 ) ) );
    pv->SetName( "pvModuleE1" );
    pv.addPhysVolID( "module", 1 );
    deModule = de_pixel_module_b.clone( "Module1", 1 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 5
    pv = lvStaveFace.placeVolume( volume( "lvPixelModuleA" ), Position( 0, _toDouble( "SetUPStavEBackModule5" ), 0 ) );
    pv->SetName( "pvModuleE2" );
    pv.addPhysVolID( "module", 2 );
    deModule = de_pixel_module_a.clone( "Module2", 2 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 6
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "SetUPStavEBackModule6" ), 0 ) ) );
    pv->SetName( "pvModuleE3" );
    pv.addPhysVolID( "module", 3 );
    deModule = de_pixel_module_b.clone( "Module3", 3 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 9
    pv = lvStaveFace.placeVolume( volume( "lvPixelModuleA" ), Position( 0, _toDouble( "SetUPStavEBackModule9" ), 0 ) );
    pv->SetName( "pvModuleE4" );
    pv.addPhysVolID( "module", 4 );
    deModule = de_pixel_module_a.clone( "Module4", 4 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 10
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "SetUPStavEBackModule10" ), 0 ) ) );
    pv->SetName( "pvModuleE5" );
    pv.addPhysVolID( "module", 5 );
    deModule = de_pixel_module_b.clone( "Module5", 5 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 13
    pv = lvStaveFace.placeVolume( volume( "lvPixelModuleA" ), Position( 0, _toDouble( "SetUPStavEBackModule13" ), 0 ) );
    pv->SetName( "pvModuleE6" );
    pv.addPhysVolID( "module", 6 );
    deModule = de_pixel_module_a.clone( "Module6", 6 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 14
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "SetUPStavEBackModule14" ), 0 ) ) );
    pv->SetName( "pvModuleE7" );
    pv.addPhysVolID( "module", 7 );
    deModule = de_pixel_module_b.clone( "Module7", 7 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 18
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "SetUPStavEBackModule18" ), 0 ) ) );
    pv->SetName( "pvModuleE9" );
    pv.addPhysVolID( "module", 9 );
    deModule = de_pixel_module_b.clone( "Module9", 9 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 23
    pv = lvStaveFace.placeVolume( volume( "lvPixelModuleA" ), Position( 0, _toDouble( "SetUPStavEBackModule23" ), 0 ) );
    pv->SetName( "pvModuleE10" );
    pv.addPhysVolID( "module", 10 );
    deModule = de_pixel_module_a.clone( "Module10", 10 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 24
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "SetUPStavEBackModule24" ), 0 ) ) );
    pv->SetName( "pvModuleE11" );
    pv.addPhysVolID( "module", 11 );
    deModule = de_pixel_module_b.clone( "Module11", 11 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 27
    pv = lvStaveFace.placeVolume( volume( "lvPixelModuleA" ), Position( 0, _toDouble( "SetUPStavEBackModule27" ), 0 ) );
    pv->SetName( "pvModuleE12" );
    pv.addPhysVolID( "module", 12 );
    deModule = de_pixel_module_a.clone( "Module12", 12 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 28
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "SetUPStavEBackModule28" ), 0 ) ) );
    pv->SetName( "pvModuleE13" );
    pv.addPhysVolID( "module", 13 );
    deModule = de_pixel_module_b.clone( "Module13", 13 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 31
    pv = lvStaveFace.placeVolume( volume( "lvPixelModuleA" ), Position( 0, _toDouble( "SetUPStavEBackModule31" ), 0 ) );
    pv->SetName( "pvModuleE14" );
    pv.addPhysVolID( "module", 14 );
    deModule = de_pixel_module_a.clone( "Module14", 14 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 32
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "SetUPStavEBackModule32" ), 0 ) ) );
    pv->SetName( "pvModuleE15" );
    pv.addPhysVolID( "module", 15 );
    deModule = de_pixel_module_b.clone( "Module15", 15 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 35
    pv = lvStaveFace.placeVolume( volume( "lvPixelModuleA" ), Position( 0, _toDouble( "SetUPStavEBackModule35" ), 0 ) );
    pv->SetName( "pvModuleE16" );
    pv.addPhysVolID( "module", 16 );
    deModule = de_pixel_module_a.clone( "Module16", 16 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    pv = lvStave.placeVolume( lvStaveFace, Position( 0, 0, _toDouble( "-UPPixelStaveHybridZ" ) ) );
    pv->SetName( "pvStaveEBack" );
    pv.addPhysVolID( "face", 0 );
    deStaveFace.setPlacement( pv );

    // Stave F
    de_stave_f = DetElement( "stave", id );
    lvStave    = volume( "lvStaveF" );

    // Stave F Front
    deStaveFace = DetElement( de_stave_f, "Face1", 1 );
    lvStaveFace = volume( "lvStaveFFront" );

    // Module 2
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleC" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ), Position( 0, _toDouble( "SetUPStavFFrontModule2" ), 0 ) ) );
    pv->SetName( "pvModuleF0" );
    pv.addPhysVolID( "module", 0 );
    deModule = de_pixel_module_c.clone( "Module0", 0 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 4
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleC" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ), Position( 0, _toDouble( "SetUPStavFFrontModule4" ), 0 ) ) );
    pv->SetName( "pvModuleF1" );
    pv.addPhysVolID( "module", 1 );
    deModule = de_pixel_module_c.clone( "Module1", 1 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 6
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleC" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ), Position( 0, _toDouble( "SetUPStavFFrontModule6" ), 0 ) ) );
    pv->SetName( "pvModuleF2" );
    pv.addPhysVolID( "module", 2 );
    deModule = de_pixel_module_c.clone( "Module2", 2 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 9
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleA" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "SetUPStavFFrontModule9" ), 0 ) ) );
    pv->SetName( "pvModuleF3" );
    pv.addPhysVolID( "module", 3 );
    deModule = de_pixel_module_a.clone( "Module3", 3 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 10
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ), Position( 0, _toDouble( "SetUPStavFFrontModule10" ), 0 ) ) );
    pv->SetName( "pvModuleF4" );
    pv.addPhysVolID( "module", 4 );
    deModule = de_pixel_module_b.clone( "Module4", 4 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 13
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleA" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "SetUPStavFFrontModule13" ), 0 ) ) );
    pv->SetName( "pvModuleF5" );
    pv.addPhysVolID( "module", 5 );
    deModule = de_pixel_module_a.clone( "Module5", 5 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 14
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ), Position( 0, _toDouble( "SetUPStavFFrontModule14" ), 0 ) ) );
    pv->SetName( "pvModuleF6" );
    pv.addPhysVolID( "module", 6 );
    deModule = de_pixel_module_b.clone( "Module6", 6 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 18
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ), Position( 0, _toDouble( "SetUPStavFFrontModule18" ), 0 ) ) );
    pv->SetName( "pvModuleF8" );
    pv.addPhysVolID( "module", 8 );
    deModule = de_pixel_module_b.clone( "Module8", 8 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 19
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleC" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "SetUPStavFFrontModule19" ), 0 ) ) );
    pv->SetName( "pvModuleF9" );
    pv.addPhysVolID( "module", 9 );
    deModule = de_pixel_module_c.clone( "Module9", 9 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 21
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleC" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "SetUPStavFFrontModule21" ), 0 ) ) );
    pv->SetName( "pvModuleF10" );
    pv.addPhysVolID( "module", 10 );
    deModule = de_pixel_module_c.clone( "Module10", 10 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 23
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleC" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "SetUPStavFFrontModule23" ), 0 ) ) );
    pv->SetName( "pvModuleF11" );
    pv.addPhysVolID( "module", 11 );
    deModule = de_pixel_module_c.clone( "Module11", 11 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 25
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleA" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "SetUPStavFFrontModule25" ), 0 ) ) );
    pv->SetName( "pvModuleF12" );
    pv.addPhysVolID( "module", 12 );
    deModule = de_pixel_module_a.clone( "Module12", 12 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 26
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ), Position( 0, _toDouble( "SetUPStavFFrontModule26" ), 0 ) ) );
    pv->SetName( "pvModuleF13" );
    pv.addPhysVolID( "module", 13 );
    deModule = de_pixel_module_b.clone( "Module13", 13 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 29
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleA" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "SetUPStavFFrontModule29" ), 0 ) ) );
    pv->SetName( "pvModuleF14" );
    pv.addPhysVolID( "module", 14 );
    deModule = de_pixel_module_a.clone( "Module14", 14 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 30
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ), Position( 0, _toDouble( "SetUPStavFFrontModule30" ), 0 ) ) );
    pv->SetName( "pvModuleF15" );
    pv.addPhysVolID( "module", 15 );
    deModule = de_pixel_module_b.clone( "Module15", 15 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 33
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleA" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "SetUPStavFFrontModule33" ), 0 ) ) );
    pv->SetName( "pvModuleF16" );
    pv.addPhysVolID( "module", 16 );
    deModule = de_pixel_module_a.clone( "Module16", 16 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    pv = lvStave.placeVolume( lvStaveFace, Position( 0, 0, _toDouble( "UPPixelStaveHybridZ" ) ) );
    pv->SetName( "pvStaveFFront" );
    pv.addPhysVolID( "face", 1 );
    deStaveFace.setPlacement( pv );

    // Stave F Back
    deStaveFace = DetElement( de_stave_f, "Face0", 0 );
    lvStaveFace = volume( "lvStaveFBack" );

    // Module 1
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleC" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "SetUPStavFBackModule1" ), 0 ) ) );
    pv->SetName( "pvModuleF0" );
    pv.addPhysVolID( "module", 0 );
    deModule = de_pixel_module_c.clone( "Module0", 0 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 3
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleC" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "SetUPStavFBackModule3" ), 0 ) ) );
    pv->SetName( "pvModuleF1" );
    pv.addPhysVolID( "module", 1 );
    deModule = de_pixel_module_c.clone( "Module1", 1 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 5
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleC" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "SetUPStavFBackModule5" ), 0 ) ) );
    pv->SetName( "pvModuleF2" );
    pv.addPhysVolID( "module", 2 );
    deModule = de_pixel_module_c.clone( "Module2", 2 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 7
    pv = lvStaveFace.placeVolume( volume( "lvPixelModuleA" ), Position( 0, _toDouble( "SetUPStavFBackModule7" ), 0 ) );
    pv->SetName( "pvModuleF3" );
    pv.addPhysVolID( "module", 3 );
    deModule = de_pixel_module_a.clone( "Module3", 3 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 8
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "SetUPStavFBackModule8" ), 0 ) ) );
    pv->SetName( "pvModuleF4" );
    pv.addPhysVolID( "module", 4 );
    deModule = de_pixel_module_b.clone( "Module4", 4 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 11
    pv = lvStaveFace.placeVolume( volume( "lvPixelModuleA" ), Position( 0, _toDouble( "SetUPStavFBackModule11" ), 0 ) );
    pv->SetName( "pvModuleF5" );
    pv.addPhysVolID( "module", 5 );
    deModule = de_pixel_module_a.clone( "Module5", 5 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 12
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "SetUPStavFBackModule12" ), 0 ) ) );
    pv->SetName( "pvModuleF6" );
    pv.addPhysVolID( "module", 6 );
    deModule = de_pixel_module_b.clone( "Module6", 6 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 16
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "SetUPStavFBackModule16" ), 0 ) ) );
    pv->SetName( "pvModuleF8" );
    pv.addPhysVolID( "module", 8 );
    deModule = de_pixel_module_b.clone( "Module8", 8 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 20
    pv = lvStaveFace.placeVolume( volume( "lvPixelModuleC" ), Position( 0, _toDouble( "SetUPStavFBackModule20" ), 0 ) );
    pv->SetName( "pvModuleF9" );
    pv.addPhysVolID( "module", 9 );
    deModule = de_pixel_module_c.clone( "Module9", 9 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 22
    pv = lvStaveFace.placeVolume( volume( "lvPixelModuleC" ), Position( 0, _toDouble( "SetUPStavFBackModule22" ), 0 ) );
    pv->SetName( "pvModuleF10" );
    pv.addPhysVolID( "module", 10 );
    deModule = de_pixel_module_c.clone( "Module10", 10 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 24
    pv = lvStaveFace.placeVolume( volume( "lvPixelModuleC" ), Position( 0, _toDouble( "SetUPStavFBackModule24" ), 0 ) );
    pv->SetName( "pvModuleF11" );
    pv.addPhysVolID( "module", 11 );
    deModule = de_pixel_module_c.clone( "Module11", 11 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 27
    pv = lvStaveFace.placeVolume( volume( "lvPixelModuleA" ), Position( 0, _toDouble( "SetUPStavFBackModule27" ), 0 ) );
    pv->SetName( "pvModuleF12" );
    pv.addPhysVolID( "module", 12 );
    deModule = de_pixel_module_a.clone( "Module12", 12 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 28
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "SetUPStavFBackModule28" ), 0 ) ) );
    pv->SetName( "pvModuleF13" );
    pv.addPhysVolID( "module", 13 );
    deModule = de_pixel_module_b.clone( "Module13", 13 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 31
    pv = lvStaveFace.placeVolume( volume( "lvPixelModuleA" ), Position( 0, _toDouble( "SetUPStavFBackModule31" ), 0 ) );
    pv->SetName( "pvModuleF14" );
    pv.addPhysVolID( "module", 14 );
    deModule = de_pixel_module_a.clone( "Module14", 14 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 32
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "SetUPStavFBackModule32" ), 0 ) ) );
    pv->SetName( "pvModuleF15" );
    pv.addPhysVolID( "module", 15 );
    deModule = de_pixel_module_b.clone( "Module15", 15 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 35
    pv = lvStaveFace.placeVolume( volume( "lvPixelModuleA" ), Position( 0, _toDouble( "SetUPStavFBackModule35" ), 0 ) );
    pv->SetName( "pvModuleF16" );
    pv.addPhysVolID( "module", 16 );
    deModule = de_pixel_module_a.clone( "Module16", 16 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    pv = lvStave.placeVolume( lvStaveFace, Position( 0, 0, _toDouble( "-UPPixelStaveHybridZ" ) ) );
    pv->SetName( "pvStaveFBack" );
    pv.addPhysVolID( "face", 0 );
    deStaveFace.setPlacement( pv );

    // Stave G
    de_stave_g = DetElement( "stave", id );
    lvStave    = volume( "lvStaveG" );

    // Stave G Front
    deStaveFace = DetElement( de_stave_g, "Face1", 1 );
    lvStaveFace = volume( "lvStaveGFront" );

    // Module 2
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleE" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ), Position( 0, _toDouble( "SetUPStavGFrontModule2" ), 0 ) ) );
    pv->SetName( "pvModuleG0" );
    pv.addPhysVolID( "module", 0 );
    deModule = de_pixel_module_e.clone( "Module0", 0 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 4
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleD" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ), Position( 0, _toDouble( "SetUPStavGFrontModule4" ), 0 ) ) );
    pv->SetName( "pvModuleG1" );
    pv.addPhysVolID( "module", 1 );
    deModule = de_pixel_module_d.clone( "Module1", 1 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 6
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleC" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ), Position( 0, _toDouble( "SetUPStavGFrontModule6" ), 0 ) ) );
    pv->SetName( "pvModuleG2" );
    pv.addPhysVolID( "module", 2 );
    deModule = de_pixel_module_c.clone( "Module2", 2 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 8
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleC" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ), Position( 0, _toDouble( "SetUPStavGFrontModule8" ), 0 ) ) );
    pv->SetName( "pvModuleG3" );
    pv.addPhysVolID( "module", 3 );
    deModule = de_pixel_module_c.clone( "Module3", 3 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 10
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleC" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ), Position( 0, _toDouble( "SetUPStavGFrontModule10" ), 0 ) ) );
    pv->SetName( "pvModuleG4" );
    pv.addPhysVolID( "module", 4 );
    deModule = de_pixel_module_c.clone( "Module4", 4 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 13
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleA" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "SetUPStavGFrontModule13" ), 0 ) ) );
    pv->SetName( "pvModuleG5" );
    pv.addPhysVolID( "module", 5 );
    deModule = de_pixel_module_a.clone( "Module5", 5 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 14
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ), Position( 0, _toDouble( "SetUPStavGFrontModule14" ), 0 ) ) );
    pv->SetName( "pvModuleG6" );
    pv.addPhysVolID( "module", 6 );
    deModule = de_pixel_module_b.clone( "Module6", 6 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 18
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ), Position( 0, _toDouble( "SetUPStavGFrontModule18" ), 0 ) ) );
    pv->SetName( "pvModuleG8" );
    pv.addPhysVolID( "module", 8 );
    deModule = de_pixel_module_b.clone( "Module8", 8 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 19
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleE" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "SetUPStavGFrontModule19" ), 0 ) ) );
    pv->SetName( "pvModuleG9" );
    pv.addPhysVolID( "module", 9 );
    deModule = de_pixel_module_e.clone( "Module9", 9 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 21
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleD" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "SetUPStavGFrontModule21" ), 0 ) ) );
    pv->SetName( "pvModuleG10" );
    pv.addPhysVolID( "module", 10 );
    deModule = de_pixel_module_d.clone( "Module10", 10 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 23
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleD" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "SetUPStavGFrontModule23" ), 0 ) ) );
    pv->SetName( "pvModuleG11" );
    pv.addPhysVolID( "module", 11 );
    deModule = de_pixel_module_d.clone( "Module11", 11 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 25
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleC" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "SetUPStavGFrontModule25" ), 0 ) ) );
    pv->SetName( "pvModuleG12" );
    pv.addPhysVolID( "module", 12 );
    deModule = de_pixel_module_c.clone( "Module12", 12 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 27
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleC" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "SetUPStavGFrontModule27" ), 0 ) ) );
    pv->SetName( "pvModuleG13" );
    pv.addPhysVolID( "module", 13 );
    deModule = de_pixel_module_c.clone( "Module13", 13 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 29
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleA" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "SetUPStavGFrontModule29" ), 0 ) ) );
    pv->SetName( "pvModuleG14" );
    pv.addPhysVolID( "module", 14 );
    deModule = de_pixel_module_a.clone( "Module14", 14 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 30
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ), Position( 0, _toDouble( "SetUPStavGFrontModule30" ), 0 ) ) );
    pv->SetName( "pvModuleG15" );
    pv.addPhysVolID( "module", 15 );
    deModule = de_pixel_module_b.clone( "Module15", 15 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 33
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleA" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "SetUPStavGFrontModule33" ), 0 ) ) );
    pv->SetName( "pvModuleG16" );
    pv.addPhysVolID( "module", 16 );
    deModule = de_pixel_module_a.clone( "Module16", 16 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    pv = lvStave.placeVolume( lvStaveFace, Position( 0, 0, _toDouble( "UPPixelStaveHybridZ" ) ) );
    pv->SetName( "pvStaveGFront" );
    pv.addPhysVolID( "face", 1 );
    deStaveFace.setPlacement( pv );

    // Stave G Back
    deStaveFace = DetElement( de_stave_g, "Face0", 0 );
    lvStaveFace = volume( "lvStaveGBack" );

    // Module 1
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleE" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "SetUPStavGBackModule1" ), 0 ) ) );
    pv->SetName( "pvModuleG0" );
    pv.addPhysVolID( "module", 0 );
    deModule = de_pixel_module_e.clone( "Module0", 0 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 3
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleD" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "SetUPStavGBackModule3" ), 0 ) ) );
    pv->SetName( "pvModuleG1" );
    pv.addPhysVolID( "module", 1 );
    deModule = de_pixel_module_d.clone( "Module1", 1 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 5
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleD" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "SetUPStavGBackModule5" ), 0 ) ) );
    pv->SetName( "pvModuleG2" );
    pv.addPhysVolID( "module", 2 );
    deModule = de_pixel_module_d.clone( "Module2", 2 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 7
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleC" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "SetUPStavGBackModule7" ), 0 ) ) );
    pv->SetName( "pvModuleG3" );
    pv.addPhysVolID( "module", 3 );
    deModule = de_pixel_module_c.clone( "Module3", 3 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 9
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleC" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "SetUPStavGBackModule9" ), 0 ) ) );
    pv->SetName( "pvModuleG4" );
    pv.addPhysVolID( "module", 4 );
    deModule = de_pixel_module_c.clone( "Module4", 4 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 11
    pv = lvStaveFace.placeVolume( volume( "lvPixelModuleA" ), Position( 0, _toDouble( "SetUPStavGBackModule11" ), 0 ) );
    pv->SetName( "pvModuleG5" );
    pv.addPhysVolID( "module", 5 );
    deModule = de_pixel_module_a.clone( "Module5", 5 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 12
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "SetUPStavGBackModule12" ), 0 ) ) );
    pv->SetName( "pvModuleG6" );
    pv.addPhysVolID( "module", 6 );
    deModule = de_pixel_module_b.clone( "Module6", 6 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 16
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "SetUPStavGBackModule16" ), 0 ) ) );
    pv->SetName( "pvModuleG8" );
    pv.addPhysVolID( "module", 8 );
    deModule = de_pixel_module_b.clone( "Module8", 8 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 20
    pv = lvStaveFace.placeVolume( volume( "lvPixelModuleE" ), Position( 0, _toDouble( "SetUPStavGBackModule20" ), 0 ) );
    pv->SetName( "pvModuleG9" );
    pv.addPhysVolID( "module", 9 );
    deModule = de_pixel_module_e.clone( "Module9", 9 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 22
    pv = lvStaveFace.placeVolume( volume( "lvPixelModuleD" ), Position( 0, _toDouble( "SetUPStavGBackModule22" ), 0 ) );
    pv->SetName( "pvModuleG10" );
    pv.addPhysVolID( "module", 10 );
    deModule = de_pixel_module_d.clone( "Module10", 10 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 24
    pv = lvStaveFace.placeVolume( volume( "lvPixelModuleC" ), Position( 0, _toDouble( "SetUPStavGBackModule24" ), 0 ) );
    pv->SetName( "pvModuleG11" );
    pv.addPhysVolID( "module", 11 );
    deModule = de_pixel_module_c.clone( "Module11", 11 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 26
    pv = lvStaveFace.placeVolume( volume( "lvPixelModuleC" ), Position( 0, _toDouble( "SetUPStavGBackModule26" ), 0 ) );
    pv->SetName( "pvModuleG12" );
    pv.addPhysVolID( "module", 12 );
    deModule = de_pixel_module_c.clone( "Module12", 12 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 28
    pv = lvStaveFace.placeVolume( volume( "lvPixelModuleC" ), Position( 0, _toDouble( "SetUPStavGBackModule28" ), 0 ) );
    pv->SetName( "pvModuleG13" );
    pv.addPhysVolID( "module", 13 );
    deModule = de_pixel_module_c.clone( "Module13", 13 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 31
    pv = lvStaveFace.placeVolume( volume( "lvPixelModuleA" ), Position( 0, _toDouble( "SetUPStavGBackModule31" ), 0 ) );
    pv->SetName( "pvModuleG14" );
    pv.addPhysVolID( "module", 14 );
    deModule = de_pixel_module_a.clone( "Module14", 14 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 32
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "SetUPStavGBackModule32" ), 0 ) ) );
    pv->SetName( "pvModuleG15" );
    pv.addPhysVolID( "module", 15 );
    deModule = de_pixel_module_b.clone( "Module15", 15 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 35
    pv = lvStaveFace.placeVolume( volume( "lvPixelModuleA" ), Position( 0, _toDouble( "SetUPStavGBackModule35" ), 0 ) );
    pv->SetName( "pvModuleG16" );
    pv.addPhysVolID( "module", 16 );
    deModule = de_pixel_module_a.clone( "Module16", 16 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    pv = lvStave.placeVolume( lvStaveFace, Position( 0, 0, _toDouble( "-UPPixelStaveHybridZ" ) ) );
    pv->SetName( "pvStaveGBack" );
    pv.addPhysVolID( "face", 0 );
    deStaveFace.setPlacement( pv );

    // Stave H
    de_stave_h = DetElement( "stave", id );
    lvStave    = volume( "lvStaveH" );

    // Stave H Front
    deStaveFace = DetElement( de_stave_h, "Face1", 1 );
    lvStaveFace = volume( "lvStaveHFront" );

    // Module 2
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleF" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ), Position( 0, _toDouble( "SetUPStavHFrontModule2" ), 0 ) ) );
    pv->SetName( "pvModuleH0" );
    pv.addPhysVolID( "module", 0 );
    deModule = de_pixel_module_f.clone( "Module0", 0 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 4
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleE" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ), Position( 0, _toDouble( "SetUPStavHFrontModule4" ), 0 ) ) );
    pv->SetName( "pvModuleH1" );
    pv.addPhysVolID( "module", 1 );
    deModule = de_pixel_module_e.clone( "Module1", 1 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 6
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleD" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ), Position( 0, _toDouble( "SetUPStavHFrontModule6" ), 0 ) ) );
    pv->SetName( "pvModuleH2" );
    pv.addPhysVolID( "module", 2 );
    deModule = de_pixel_module_d.clone( "Module2", 2 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 8
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleC" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ), Position( 0, _toDouble( "SetUPStavHFrontModule8" ), 0 ) ) );
    pv->SetName( "pvModuleH3" );
    pv.addPhysVolID( "module", 3 );
    deModule = de_pixel_module_c.clone( "Module3", 3 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 10
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleC" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ), Position( 0, _toDouble( "SetUPStavHFrontModule10" ), 0 ) ) );
    pv->SetName( "pvModuleH4" );
    pv.addPhysVolID( "module", 4 );
    deModule = de_pixel_module_c.clone( "Module4", 4 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 13
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleA" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "SetUPStavHFrontModule13" ), 0 ) ) );
    pv->SetName( "pvModuleH5" );
    pv.addPhysVolID( "module", 5 );
    deModule = de_pixel_module_a.clone( "Module5", 5 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 14
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ), Position( 0, _toDouble( "SetUPStavHFrontModule14" ), 0 ) ) );
    pv->SetName( "pvModuleH6" );
    pv.addPhysVolID( "module", 6 );
    deModule = de_pixel_module_b.clone( "Module6", 6 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 18
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ), Position( 0, _toDouble( "SetUPStavHFrontModule18" ), 0 ) ) );
    pv->SetName( "pvModuleH8" );
    pv.addPhysVolID( "module", 8 );
    deModule = de_pixel_module_b.clone( "Module8", 8 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 19
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleG" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "SetUPStavHFrontModule19" ), 0 ) ) );
    pv->SetName( "pvModuleH9" );
    pv.addPhysVolID( "module", 9 );
    deModule = de_pixel_module_g.clone( "Module9", 9 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 21
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleE" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "SetUPStavHFrontModule21" ), 0 ) ) );
    pv->SetName( "pvModuleH10" );
    pv.addPhysVolID( "module", 10 );
    deModule = de_pixel_module_e.clone( "Module10", 10 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 23
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleD" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "SetUPStavHFrontModule23" ), 0 ) ) );
    pv->SetName( "pvModuleH11" );
    pv.addPhysVolID( "module", 11 );
    deModule = de_pixel_module_d.clone( "Module11", 11 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 25
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleC" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "SetUPStavHFrontModule25" ), 0 ) ) );
    pv->SetName( "pvModuleH12" );
    pv.addPhysVolID( "module", 12 );
    deModule = de_pixel_module_c.clone( "Module12", 12 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 27
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleC" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "SetUPStavHFrontModule27" ), 0 ) ) );
    pv->SetName( "pvModuleH13" );
    pv.addPhysVolID( "module", 13 );
    deModule = de_pixel_module_c.clone( "Module13", 13 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 29
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleA" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "SetUPStavHFrontModule29" ), 0 ) ) );
    pv->SetName( "pvModuleH14" );
    pv.addPhysVolID( "module", 14 );
    deModule = de_pixel_module_a.clone( "Module14", 14 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 30
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ), Position( 0, _toDouble( "SetUPStavHFrontModule30" ), 0 ) ) );
    pv->SetName( "pvModuleH15" );
    pv.addPhysVolID( "module", 15 );
    deModule = de_pixel_module_b.clone( "Module15", 15 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 33
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleA" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "SetUPStavHFrontModule33" ), 0 ) ) );
    pv->SetName( "pvModuleH16" );
    pv.addPhysVolID( "module", 16 );
    deModule = de_pixel_module_a.clone( "Module16", 16 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    pv = lvStave.placeVolume( lvStaveFace, Position( 0, 0, _toDouble( "UPPixelStaveHybridZ" ) ) );
    pv->SetName( "pvStaveHFront" );
    pv.addPhysVolID( "face", 1 );
    deStaveFace.setPlacement( pv );

    // Stave H Back
    deStaveFace = DetElement( de_stave_h, "Face0", 0 );
    lvStaveFace = volume( "lvStaveHBack" );

    // Module 1
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleG" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "SetUPStavHBackModule1" ), 0 ) ) );
    pv->SetName( "pvModuleH0" );
    pv.addPhysVolID( "module", 0 );
    deModule = de_pixel_module_g.clone( "Module0", 0 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 3
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleE" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "SetUPStavHBackModule3" ), 0 ) ) );
    pv->SetName( "pvModuleH1" );
    pv.addPhysVolID( "module", 1 );
    deModule = de_pixel_module_e.clone( "Module1", 1 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 5
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleD" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "SetUPStavHBackModule5" ), 0 ) ) );
    pv->SetName( "pvModuleH2" );
    pv.addPhysVolID( "module", 2 );
    deModule = de_pixel_module_d.clone( "Module2", 2 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 7
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleC" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "SetUPStavHBackModule7" ), 0 ) ) );
    pv->SetName( "pvModuleH3" );
    pv.addPhysVolID( "module", 3 );
    deModule = de_pixel_module_c.clone( "Module3", 3 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 9
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleC" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "SetUPStavHBackModule9" ), 0 ) ) );
    pv->SetName( "pvModuleH4" );
    pv.addPhysVolID( "module", 4 );
    deModule = de_pixel_module_c.clone( "Module4", 4 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 11
    pv = lvStaveFace.placeVolume( volume( "lvPixelModuleA" ), Position( 0, _toDouble( "SetUPStavHBackModule11" ), 0 ) );
    pv->SetName( "pvModuleH5" );
    pv.addPhysVolID( "module", 5 );
    deModule = de_pixel_module_a.clone( "Module5", 5 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 12
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "SetUPStavHBackModule12" ), 0 ) ) );
    pv->SetName( "pvModuleH6" );
    pv.addPhysVolID( "module", 6 );
    deModule = de_pixel_module_b.clone( "Module6", 6 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 16
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "SetUPStavHBackModule16" ), 0 ) ) );
    pv->SetName( "pvModuleH8" );
    pv.addPhysVolID( "module", 8 );
    deModule = de_pixel_module_b.clone( "Module8", 8 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 20
    pv = lvStaveFace.placeVolume( volume( "lvPixelModuleF" ), Position( 0, _toDouble( "SetUPStavHBackModule20" ), 0 ) );
    pv->SetName( "pvModuleH9" );
    pv.addPhysVolID( "module", 9 );
    deModule = de_pixel_module_f.clone( "Module9", 9 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 22
    pv = lvStaveFace.placeVolume( volume( "lvPixelModuleE" ), Position( 0, _toDouble( "SetUPStavHBackModule22" ), 0 ) );
    pv->SetName( "pvModuleH10" );
    pv.addPhysVolID( "module", 10 );
    deModule = de_pixel_module_e.clone( "Module10", 10 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 24
    pv = lvStaveFace.placeVolume( volume( "lvPixelModuleD" ), Position( 0, _toDouble( "SetUPStavHBackModule24" ), 0 ) );
    pv->SetName( "pvModuleH11" );
    pv.addPhysVolID( "module", 11 );
    deModule = de_pixel_module_d.clone( "Module11", 11 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 26
    pv = lvStaveFace.placeVolume( volume( "lvPixelModuleC" ), Position( 0, _toDouble( "SetUPStavHBackModule26" ), 0 ) );
    pv->SetName( "pvModuleH12" );
    pv.addPhysVolID( "module", 12 );
    deModule = de_pixel_module_c.clone( "Module12", 12 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 28
    pv = lvStaveFace.placeVolume( volume( "lvPixelModuleC" ), Position( 0, _toDouble( "SetUPStavHBackModule28" ), 0 ) );
    pv->SetName( "pvModuleH13" );
    pv.addPhysVolID( "module", 13 );
    deModule = de_pixel_module_c.clone( "Module13", 13 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 31
    pv = lvStaveFace.placeVolume( volume( "lvPixelModuleA" ), Position( 0, _toDouble( "SetUPStavHBackModule31" ), 0 ) );
    pv->SetName( "pvModuleH14" );
    pv.addPhysVolID( "module", 14 );
    deModule = de_pixel_module_a.clone( "Module14", 14 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 32
    pv = lvStaveFace.placeVolume(
        volume( "lvPixelModuleB" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "SetUPStavHBackModule32" ), 0 ) ) );
    pv->SetName( "pvModuleH15" );
    pv.addPhysVolID( "module", 15 );
    deModule = de_pixel_module_b.clone( "Module15", 15 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 35
    pv = lvStaveFace.placeVolume( volume( "lvPixelModuleA" ), Position( 0, _toDouble( "SetUPStavHBackModule35" ), 0 ) );
    pv->SetName( "pvModuleH16" );
    pv.addPhysVolID( "module", 16 );
    deModule = de_pixel_module_a.clone( "Module16", 16 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    pv = lvStave.placeVolume( lvStaveFace, Position( 0, 0, _toDouble( "-UPPixelStaveHybridZ" ) ) );
    pv->SetName( "pvStaveHBack" );
    pv.addPhysVolID( "face", 0 );
    deStaveFace.setPlacement( pv );
  }

  void UPBuild::build_detector() {
    PlacedVolume pv;
    Volume       lvUPC, lvUPA, lvLayer;
    DetElement   deUPC, deUPA, deLayer;
    DetElement   deStave;
    // Construct sides
    if ( m_build_sides ) {

      lvUPC = volume( "lvUPC" );
      lvUPA = volume( "lvUPA" );
      deUPC = DetElement( detector, "Cside", 0 );
      deUPA = DetElement( detector, "Aside", 1 );

      if ( m_build_cside ) {
        if ( m_build_cside_upax ) {
          lvLayer = volume( "lvUPaXCsideLayer" );
          deLayer = DetElement( deUPC, "UPaX", 0 );
          pv      = lvUPC.placeVolume( lvLayer );
          pv->SetName( "pvUPaXCsideLayer" );
          pv.addPhysVolID( "layer", 0 );
          deLayer.setPlacement( pv );

          // Stave 0
          pv = lvLayer.placeVolume( volume( "lvStaveH" ),
                                    Transform3D( RotationY( M_PI ), Position( _toDouble( "SetUPStave0HX" ), 0,
                                                                              _toDouble( "SetUPStave0HZ" ) ) ) );
          pv->SetName( "pvStave0" );
          pv.addPhysVolID( "stave", 0 );
          deStave = de_stave_h.clone( "Stave0", 0 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 1
          pv = lvLayer.placeVolume( volume( "lvStaveG" ),
                                    Transform3D( RotationY( M_PI ), Position( _toDouble( "SetUPStave1GX" ), 0,
                                                                              _toDouble( "SetUPStave1GZ" ) ) ) );
          pv->SetName( "pvStave1" );
          pv.addPhysVolID( "stave", 1 );
          deStave = de_stave_g.clone( "Stave1", 1 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 2
          pv = lvLayer.placeVolume( volume( "lvStaveF" ),
                                    Transform3D( RotationY( M_PI ), Position( _toDouble( "SetUPStave2FX" ), 0,
                                                                              _toDouble( "SetUPStave2FZ" ) ) ) );
          pv->SetName( "pvStave2" );
          pv.addPhysVolID( "stave", 2 );
          deStave = de_stave_f.clone( "Stave2", 2 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 3-4
          for ( int i = 3; i < 5; i++ ) {
            pv = lvLayer.placeVolume(
                volume( "lvStaveE" ),
                Transform3D( RotationY( M_PI ), Position( _toDouble( "1/2*UUPLayerSideCorrectedWidth" ) -
                                                              ( 7.0 / 2 + ( i - 3 ) ) * _toDouble( "UPPixelStaveStep" ),
                                                          0, pow( -1, i ) * _toDouble( "UPPixelStaveDz/2" ) ) ) );
            pv->SetName( _toString( i, "pvStave%d" ).c_str() );
            pv.addPhysVolID( "stave", i );
            deStave = de_stave_e.clone( _toString( i, "Stave%d" ), i );
            deStave.setPlacement( pv );
            deLayer.add( deStave );
          }

        } // lvUPaXCsideLayer

        if ( m_build_cside_upau ) {
          lvLayer = volume( "lvUPaUCsideLayer" );
          deLayer = DetElement( deUPC, "UPaU", 1 );
          pv      = lvUPC.placeVolume( lvLayer, Position( 0, 0, _toDouble( "UPLayerDz" ) ) );
          pv->SetName( "pvUPaUCsideLayer" );
          pv.addPhysVolID( "layer", 1 );
          deLayer.setPlacement( pv );

          // Stave 0
          pv = lvLayer.placeVolume( volume( "lvStaveH" ),
                                    Transform3D( RotationY( M_PI ), Position( _toDouble( "SetUPStave0HX" ), 0,
                                                                              _toDouble( "SetUPStave0HZ" ) ) ) );
          pv->SetName( "pvStave0" );
          pv.addPhysVolID( "stave", 0 );
          deStave = de_stave_h.clone( "Stave0", 0 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 1
          pv = lvLayer.placeVolume( volume( "lvStaveG" ),
                                    Transform3D( RotationY( M_PI ), Position( _toDouble( "SetUPStave1GX" ), 0,
                                                                              _toDouble( "SetUPStave1GZ" ) ) ) );
          pv->SetName( "pvStave1" );
          pv.addPhysVolID( "stave", 1 );
          deStave = de_stave_g.clone( "Stave1", 1 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 2
          pv = lvLayer.placeVolume( volume( "lvStaveF" ),
                                    Transform3D( RotationY( M_PI ), Position( _toDouble( "SetUPStave2FX" ), 0,
                                                                              _toDouble( "SetUPStave2FZ" ) ) ) );
          pv->SetName( "pvStave2" );
          pv.addPhysVolID( "stave", 2 );
          deStave = de_stave_f.clone( "Stave2", 2 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 3-4
          for ( int i = 3; i < 5; i++ ) {
            pv = lvLayer.placeVolume(
                volume( "lvStaveE" ),
                Transform3D( RotationY( M_PI ), Position( _toDouble( "1/2*UUPLayerSideCorrectedWidth" ) -
                                                              ( 7.0 / 2 + ( i - 3 ) ) * _toDouble( "UPPixelStaveStep" ),
                                                          0, pow( -1, i ) * _toDouble( "UPPixelStaveDz/2" ) ) ) );
            pv->SetName( _toString( i, "pvStave%d" ).c_str() );
            pv.addPhysVolID( "stave", i );
            deStave = de_stave_e.clone( _toString( i, "Stave%d" ), i );
            deStave.setPlacement( pv );
            deLayer.add( deStave );
          }
        } // lvUPaUCsideLayer

        if ( m_build_cside_upbv ) {
          lvLayer = volume( "lvUPbVCsideLayer" );
          deLayer = DetElement( deUPC, "UPbV", 2 );
          pv      = lvUPC.placeVolume( lvLayer, Position( 0, 0, _toDouble( "UPStationDz" ) ) );
          pv->SetName( "pvUPbVCsideLayer" );
          pv.addPhysVolID( "layer", 2 );
          deLayer.setPlacement( pv );

          // Stave 0
          pv = lvLayer.placeVolume( volume( "lvStaveH" ),
                                    Transform3D( RotationY( M_PI ), Position( _toDouble( "SetUPStave0HX" ), 0,
                                                                              _toDouble( "SetUPStave0HZ" ) ) ) );
          pv->SetName( "pvStave0" );
          pv.addPhysVolID( "stave", 0 );
          deStave = de_stave_h.clone( "Stave0", 0 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 1
          pv = lvLayer.placeVolume( volume( "lvStaveG" ),
                                    Transform3D( RotationY( M_PI ), Position( _toDouble( "SetUPStave1GX" ), 0,
                                                                              _toDouble( "SetUPStave1GZ" ) ) ) );
          pv->SetName( "pvStave1" );
          pv.addPhysVolID( "stave", 1 );
          deStave = de_stave_g.clone( "Stave1", 1 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 2
          pv = lvLayer.placeVolume( volume( "lvStaveF" ),
                                    Transform3D( RotationY( M_PI ), Position( _toDouble( "SetUPStave2FX" ), 0,
                                                                              _toDouble( "SetUPStave2FZ" ) ) ) );
          pv->SetName( "pvStave2" );
          pv.addPhysVolID( "stave", 2 );
          deStave = de_stave_f.clone( "Stave2", 2 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 3-4
          for ( int i = 3; i < 5; i++ ) {
            pv = lvLayer.placeVolume(
                volume( "lvStaveE" ),
                Transform3D( RotationY( M_PI ), Position( _toDouble( "1/2*UUPLayerSideCorrectedWidth" ) -
                                                              ( 7.0 / 2 + ( i - 3 ) ) * _toDouble( "UPPixelStaveStep" ),
                                                          0, pow( -1, i ) * _toDouble( "UPPixelStaveDz/2" ) ) ) );
            pv->SetName( _toString( i, "pvStave%d" ).c_str() );
            pv.addPhysVolID( "stave", i );
            deStave = de_stave_e.clone( _toString( i, "Stave%d" ), i );
            deStave.setPlacement( pv );
            deLayer.add( deStave );
          }
        } // lvUPbVCsideLayer

        if ( m_build_cside_upbx ) {
          lvLayer = volume( "lvUPbXCsideLayer" );
          deLayer = DetElement( deUPC, "UPbX", 3 );
          pv      = lvUPC.placeVolume( lvLayer, Position( 0, 0, _toDouble( "UPStationDz+UPLayerDz" ) ) );
          pv->SetName( "pvUPbXCsideLayer" );
          pv.addPhysVolID( "layer", 3 );
          deLayer.setPlacement( pv );

          // Stave 0
          pv = lvLayer.placeVolume( volume( "lvStaveH" ),
                                    Transform3D( RotationY( M_PI ), Position( _toDouble( "SetUPStave0HX" ), 0,
                                                                              _toDouble( "SetUPStave0HZ" ) ) ) );
          pv->SetName( "pvStave0" );
          pv.addPhysVolID( "stave", 0 );
          deStave = de_stave_h.clone( "Stave0", 0 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 1
          pv = lvLayer.placeVolume( volume( "lvStaveG" ),
                                    Transform3D( RotationY( M_PI ), Position( _toDouble( "SetUPStave1GX" ), 0,
                                                                              _toDouble( "SetUPStave1GZ" ) ) ) );
          pv->SetName( "pvStave1" );
          pv.addPhysVolID( "stave", 1 );
          deStave = de_stave_g.clone( "Stave1", 1 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 2
          pv = lvLayer.placeVolume( volume( "lvStaveF" ),
                                    Transform3D( RotationY( M_PI ), Position( _toDouble( "SetUPStave2FX" ), 0,
                                                                              _toDouble( "SetUPStave2FZ" ) ) ) );
          pv->SetName( "pvStave2" );
          pv.addPhysVolID( "stave", 2 );
          deStave = de_stave_f.clone( "Stave2", 2 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 3-4
          for ( int i = 3; i < 5; i++ ) {
            pv = lvLayer.placeVolume(
                volume( "lvStaveE" ),
                Transform3D( RotationY( M_PI ), Position( _toDouble( "1/2*UUPLayerSideCorrectedWidth" ) -
                                                              ( 7.0 / 2 + ( i - 3 ) ) * _toDouble( "UPPixelStaveStep" ),
                                                          0, pow( -1, i ) * _toDouble( "UPPixelStaveDz/2" ) ) ) );
            pv->SetName( _toString( i, "pvStave%d" ).c_str() );
            pv.addPhysVolID( "stave", i );
            deStave = de_stave_e.clone( _toString( i, "Stave%d" ), i );
            deStave.setPlacement( pv );
            deLayer.add( deStave );
          }
        } // lvUPbXCsideLayer
      }   // Cside

      if ( m_build_aside ) {
        if ( m_build_aside_upax ) {
          lvLayer = volume( "lvUPaXAsideLayer" );
          deLayer = DetElement( deUPA, "UPaX", 0 );
          pv      = lvUPA.placeVolume( lvLayer );
          pv->SetName( "pvUPaXAsideLayer" );
          pv.addPhysVolID( "layer", 0 );
          deLayer.setPlacement( pv );

          // Stave 0
          pv = lvLayer.placeVolume( volume( "lvStaveH" ),
                                    Position( _toDouble( "-1/2*UUPLayerSideCorrectedWidth+1/2*UPPixelStaveStep" ), 0,
                                              _toDouble( "-UPPixelStaveDz/2" ) ) );
          pv->SetName( "pvStave0" );
          pv.addPhysVolID( "stave", 0 );
          deStave = de_stave_h.clone( "Stave0", 0 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 1
          pv = lvLayer.placeVolume( volume( "lvStaveG" ),
                                    Position( _toDouble( "-1/2*UUPLayerSideCorrectedWidth+3/2*UPPixelStaveStep" ), 0,
                                              _toDouble( "UPPixelStaveDz/2" ) ) );
          pv->SetName( "pvStave1" );
          pv.addPhysVolID( "stave", 1 );
          deStave = de_stave_g.clone( "Stave1", 1 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 2
          pv = lvLayer.placeVolume( volume( "lvStaveF" ),
                                    Position( _toDouble( "-1/2*UUPLayerSideCorrectedWidth+5/2*UPPixelStaveStep" ), 0,
                                              _toDouble( "-UPPixelStaveDz/2" ) ) );
          pv->SetName( "pvStave2" );
          pv.addPhysVolID( "stave", 2 );
          deStave = de_stave_f.clone( "Stave2", 2 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 3-4
          for ( int i = 3; i < 5; i++ ) {
            pv = lvLayer.placeVolume( volume( "lvStaveE" ),
                                      Position( _toDouble( "-1/2*UUPLayerSideCorrectedWidth" ) +
                                                    ( 7.0 / 2 + ( i - 3 ) ) * _toDouble( "UPPixelStaveStep" ),
                                                0, pow( -1, i + 1 ) * _toDouble( "UPPixelStaveDz/2" ) ) );
            pv->SetName( _toString( i, "pvStave%d" ).c_str() );
            pv.addPhysVolID( "stave", i );
            deStave = de_stave_e.clone( _toString( i, "Stave%d" ), i );
            deStave.setPlacement( pv );
            deLayer.add( deStave );
          }
        } // UPaXAsideLayer
        if ( m_build_aside_upau ) {
          lvLayer = volume( "lvUPaUAsideLayer" );
          deLayer = DetElement( deUPA, "UPaU", 1 );
          pv      = lvUPA.placeVolume( lvLayer, Position( 0, 0, _toDouble( "UPLayerDz" ) ) );
          pv->SetName( "pvUPaUAsideLayer" );
          pv.addPhysVolID( "layer", 1 );
          deLayer.setPlacement( pv );

          // Stave 0
          pv = lvLayer.placeVolume( volume( "lvStaveH" ),
                                    Position( _toDouble( "-1/2*UUPLayerSideCorrectedWidth+1/2*UPPixelStaveStep" ), 0,
                                              _toDouble( "-UPPixelStaveDz/2" ) ) );
          pv->SetName( "pvStave0" );
          pv.addPhysVolID( "stave", 0 );
          deStave = de_stave_h.clone( "Stave0", 0 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 1
          pv = lvLayer.placeVolume( volume( "lvStaveG" ),
                                    Position( _toDouble( "-1/2*UUPLayerSideCorrectedWidth+3/2*UPPixelStaveStep" ), 0,
                                              _toDouble( "UPPixelStaveDz/2" ) ) );
          pv->SetName( "pvStave1" );
          pv.addPhysVolID( "stave", 1 );
          deStave = de_stave_g.clone( "Stave1", 1 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 2
          pv = lvLayer.placeVolume( volume( "lvStaveF" ),
                                    Position( _toDouble( "-1/2*UUPLayerSideCorrectedWidth+5/2*UPPixelStaveStep" ), 0,
                                              _toDouble( "-UPPixelStaveDz/2" ) ) );
          pv->SetName( "pvStave2" );
          pv.addPhysVolID( "stave", 2 );
          deStave = de_stave_f.clone( "Stave2", 2 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 3-4
          for ( int i = 3; i < 5; i++ ) {
            pv = lvLayer.placeVolume( volume( "lvStaveE" ),
                                      Position( _toDouble( "-1/2*UUPLayerSideCorrectedWidth" ) +
                                                    ( 7.0 / 2 + ( i - 3 ) ) * _toDouble( "UPPixelStaveStep" ),
                                                0, pow( -1, i + 1 ) * _toDouble( "UPPixelStaveDz/2" ) ) );
            pv->SetName( _toString( i, "pvStave%d" ).c_str() );
            pv.addPhysVolID( "stave", i );
            deStave = de_stave_e.clone( _toString( i, "Stave%d" ), i );
            deStave.setPlacement( pv );
            deLayer.add( deStave );
          }
        } // lvUPaUAsideLayer
        if ( m_build_aside_upbv ) {
          lvLayer = volume( "lvUPbVAsideLayer" );
          deLayer = DetElement( deUPA, "UPbV", 2 );
          pv      = lvUPA.placeVolume( lvLayer, Position( 0, 0, _toDouble( "UPStationDz" ) ) );
          pv->SetName( "pvUPbVAsideLayer" );
          pv.addPhysVolID( "layer", 2 );
          deLayer.setPlacement( pv );

          // Stave 0
          pv = lvLayer.placeVolume( volume( "lvStaveH" ),
                                    Position( _toDouble( "-1/2*UUPLayerSideCorrectedWidth+1/2*UPPixelStaveStep" ), 0,
                                              _toDouble( "-UPPixelStaveDz/2" ) ) );
          pv->SetName( "pvStave0" );
          pv.addPhysVolID( "stave", 0 );
          deStave = de_stave_h.clone( "Stave0", 0 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 1
          pv = lvLayer.placeVolume( volume( "lvStaveG" ),
                                    Position( _toDouble( "-1/2*UUPLayerSideCorrectedWidth+3/2*UPPixelStaveStep" ), 0,
                                              _toDouble( "UPPixelStaveDz/2" ) ) );
          pv->SetName( "pvStave1" );
          pv.addPhysVolID( "stave", 1 );
          deStave = de_stave_g.clone( "Stave1", 1 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 2
          pv = lvLayer.placeVolume( volume( "lvStaveF" ),
                                    Position( _toDouble( "-1/2*UUPLayerSideCorrectedWidth+5/2*UPPixelStaveStep" ), 0,
                                              _toDouble( "-UPPixelStaveDz/2" ) ) );
          pv->SetName( "pvStave2" );
          pv.addPhysVolID( "stave", 2 );
          deStave = de_stave_f.clone( "Stave2", 2 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 3-4
          for ( int i = 3; i < 5; i++ ) {
            pv = lvLayer.placeVolume( volume( "lvStaveE" ),
                                      Position( _toDouble( "-1/2*UUPLayerSideCorrectedWidth" ) +
                                                    ( 7.0 / 2 + ( i - 3 ) ) * _toDouble( "UPPixelStaveStep" ),
                                                0, pow( -1, i + 1 ) * _toDouble( "UPPixelStaveDz/2" ) ) );
            pv->SetName( _toString( i, "pvStave%d" ).c_str() );
            pv.addPhysVolID( "stave", i );
            deStave = de_stave_e.clone( _toString( i, "Stave%d" ), i );
            deStave.setPlacement( pv );
            deLayer.add( deStave );
          }
        } // lvUPbVAsideLayer
        if ( m_build_aside_upbx ) {
          lvLayer = volume( "lvUPbXAsideLayer" );
          deLayer = DetElement( deUPA, "UPbX", 3 );
          pv      = lvUPA.placeVolume( lvLayer, Position( 0, 0, _toDouble( "UPStationDz+UPLayerDz" ) ) );
          pv->SetName( "pvUPbXAsideLayer" );
          pv.addPhysVolID( "layer", 3 );
          deLayer.setPlacement( pv );

          // Stave 0
          pv = lvLayer.placeVolume( volume( "lvStaveH" ),
                                    Position( _toDouble( "-1/2*UUPLayerSideCorrectedWidth+1/2*UPPixelStaveStep" ), 0,
                                              _toDouble( "-UPPixelStaveDz/2" ) ) );
          pv->SetName( "pvStave0" );
          pv.addPhysVolID( "stave", 0 );
          deStave = de_stave_h.clone( "Stave0", 0 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 1
          pv = lvLayer.placeVolume( volume( "lvStaveG" ),
                                    Position( _toDouble( "-1/2*UUPLayerSideCorrectedWidth+3/2*UPPixelStaveStep" ), 0,
                                              _toDouble( "UPPixelStaveDz/2" ) ) );
          pv->SetName( "pvStave1" );
          pv.addPhysVolID( "stave", 1 );
          deStave = de_stave_g.clone( "Stave1", 1 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 2
          pv = lvLayer.placeVolume( volume( "lvStaveF" ),
                                    Position( _toDouble( "-1/2*UUPLayerSideCorrectedWidth+5/2*UPPixelStaveStep" ), 0,
                                              _toDouble( "-UPPixelStaveDz/2" ) ) );
          pv->SetName( "pvStave2" );
          pv.addPhysVolID( "stave", 2 );
          deStave = de_stave_f.clone( "Stave2", 2 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // stave 3-4
          for ( int i = 3; i < 5; i++ ) {
            pv = lvLayer.placeVolume( volume( "lvStaveE" ),
                                      Position( _toDouble( "-1/2*UUPLayerSideCorrectedWidth" ) +
                                                    ( 7.0 / 2 + ( i - 3 ) ) * _toDouble( "UPPixelStaveStep" ),
                                                0, pow( -1, i + 1 ) * _toDouble( "UPPixelStaveDz/2" ) ) );
            pv->SetName( _toString( i, "pvStave%d" ).c_str() );
            pv.addPhysVolID( "stave", i );
            deStave = de_stave_e.clone( _toString( i, "Stave%d" ), i );
            deStave.setPlacement( pv );
            deLayer.add( deStave );
          }
        } // lvUPbXAsideLayer

      } // Aside
    }

    // lvUP
    Box    up_shape( ( "UPVirtualBoxWide/2" ), "UPVirtualBoxLong/2", "UPBoxOuterThick/2" );
    Volume lvUP( "lvUP", up_shape, description.air() );
    if ( m_build_frame ) { pv = lvUP.placeVolume( volume( "lvUPFrame" ) ); }
    if ( m_build_box ) { pv = lvUP.placeVolume( volume( "lvUPBox" ) ); }
    if ( m_build_boxplug ) { pv = lvUP.placeVolume( volume( "lvUPBoxPlug" ) ); }

    if ( m_build_sides ) {
      if ( m_build_cside ) {
        pv = lvUP.placeVolume( lvUPC, Position( _toDouble( "-1/2*UUPLayerSideCorrectedWidth" ), 0,
                                                _toDouble( "-1/2*UPStationDz-1/2*UPLayerDz" ) ) );
        pv.addPhysVolID( "side", 0 );
        deUPC.setPlacement( pv );
      }
      if ( m_build_aside ) {
        pv = lvUP.placeVolume( lvUPA, Position( _toDouble( "1/2*UUPLayerSideCorrectedWidth" ), 0,
                                                _toDouble( "-1/2*UPStationDz-1/2*UPLayerDz" ) ) );
        pv.addPhysVolID( "side", 1 );
        deUPA.setPlacement( pv );
      }
    }
    registerVolume( lvUP.name(), lvUP );
  } // build_detector

  void UPBuild::build_groups() {
    sensitive.setType( "tracker" );
    load( x_det, "include" );
    buildVolumes( x_det );
    placeDaughters( detector, Volume(), x_det );
    if ( m_build_sides ) { build_staves(); }
    build_detector();
    Volume       vol = volume( select_volume );
    PlacedVolume pv  = placeDetector( vol );
    pv.addPhysVolID( "system", id );
  }

} // namespace

static Ref_t create_element( Detector& description, xml_h e, SensitiveDetector sens_det ) {
  UPBuild builder( description, e, sens_det );
  builder.build_groups();
  return builder.detector;
}
DECLARE_DETELEMENT( LHCb_UP_v1_0, create_element )
