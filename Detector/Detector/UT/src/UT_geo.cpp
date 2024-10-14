//==========================================================================
//  AIDA Detector description implementation
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
// Author     : H.Wu
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

  /// Helper class to build the UT detector of LHCb
  struct UTBuild : public xml::tools::VolumeBuilder {
    // Reusable DetElements for cloning
    DetElement de_module_a;
    DetElement de_module_b;
    DetElement de_module_c;
    DetElement de_module_d;
    DetElement de_stave_a;
    DetElement de_stave_b;
    DetElement de_stave_c;

    // Debug flags
    bool m_build_sides      = true;
    bool m_build_frame      = true;
    bool m_build_box        = true;
    bool m_build_boxplug    = true;
    bool m_build_jacket     = true;
    bool m_build_pipeheater = true;
    bool m_build_aside      = true;
    bool m_build_aside_utax = true;
    bool m_build_aside_utau = true;
    bool m_build_aside_utbv = true;
    bool m_build_aside_utbx = true;
    bool m_build_cside      = true;
    bool m_build_cside_utax = true;
    bool m_build_cside_utau = true;
    bool m_build_cside_utbv = true;
    bool m_build_cside_utbx = true;

    // Select a particular volume to build
    std::string select_volume{"lvUT"};

    /// Initializing constructor
    UTBuild( Detector& description, xml_elt_t e, SensitiveDetector sens );
    void build_modules();
    void build_staves();
    void build_detector();
    void build_groups();
  };

  /// Initializing constructor
  UTBuild::UTBuild( Detector& dsc, xml_elt_t e, SensitiveDetector sens ) : xml::tools::VolumeBuilder( dsc, e, sens ) {
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
        else if ( n == "Build_Aside_UTaX" )
          m_build_aside_utax = c.attr<bool>( _U( value ) );
        else if ( n == "Build_Aside_UTaU" )
          m_build_aside_utau = c.attr<bool>( _U( value ) );
        else if ( n == "Build_Aside_UTbV" )
          m_build_aside_utbv = c.attr<bool>( _U( value ) );
        else if ( n == "Build_Aside_UTbX" )
          m_build_aside_utbx = c.attr<bool>( _U( value ) );
        else if ( n == "Build_Cside" )
          m_build_cside = c.attr<bool>( _U( value ) );
        else if ( n == "Build_Cside_UTaX" )
          m_build_cside_utax = c.attr<bool>( _U( value ) );
        else if ( n == "Build_Cside_UTaU" )
          m_build_cside_utau = c.attr<bool>( _U( value ) );
        else if ( n == "Build_Cside_UTbV" )
          m_build_cside_utbv = c.attr<bool>( _U( value ) );
        else if ( n == "Build_Cside_UTbX" )
          m_build_cside_utbx = c.attr<bool>( _U( value ) );
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

  void UTBuild::build_modules() {
    PlacedVolume pv;
    Volume       lvModule, lvGroup, lvSector, lvSensor;
    DetElement   deGroup, deSector, deSensor;
    DetElement   de_sector_dual, de_sector_quad;

    // Module A
    de_module_a = DetElement( "module", id );
    deGroup     = DetElement( de_module_a, "GroupA", 0 );
    deSector    = DetElement( deGroup, "Sector0", 0 );
    deSensor    = DetElement( deSector, "Sensor0", 0 );

    lvModule = volume( "lvModuleA" );
    lvGroup  = volume( "lvGroupA" );
    lvSector = volume( "lvSectorNorm" );

    pv = lvSector.placeVolume( volume( "lvSensorNorm" ) );
    pv->SetName( "pvSensorNorm" );
    pv.addPhysVolID( "sensor", 0 );
    deSensor.setPlacement( pv );

    pv = lvGroup.placeVolume( lvSector, Position( 0, _toDouble( "UTSensorSiOffsetY" ), _toDouble( "UTSensorSiZ" ) ) );
    pv->SetName( "pvSectorA1" );
    pv.addPhysVolID( "sector", 0 );
    deSector.setPlacement( pv );

    pv = lvModule.placeVolume( lvGroup, Position( 0, _toDouble( "UTSensorYA" ), _toDouble( "UTSensorZ" ) ) );
    pv->SetName( "pvGroupA" );
    deGroup.setPlacement( pv );

    // Module B
    // deSectorDual
    de_sector_dual = DetElement( "sector", id );
    lvSector       = volume( "lvSectorDual" );
    pv             = lvSector.placeVolume( volume( "lvSensorDual" ) );
    pv->SetName( "pvSensorDual" );
    pv.addPhysVolID( "sensor", 0 );
    deSensor = DetElement( de_sector_dual, "Sensor0", 0 );
    deSensor.setPlacement( pv );

    de_module_b = DetElement( "module", id );
    deGroup     = DetElement( de_module_b, "GroupB", 0 );
    lvGroup     = volume( "lvGroupB" );

    pv = lvGroup.placeVolume( lvSector, Position( _toDouble( "-UTSensorSiWide/4" ), _toDouble( "UTSensorSiOffsetY" ),
                                                  _toDouble( "UTSensorSiZ" ) ) );
    pv->SetName( "pvSectorB1" );
    pv.addPhysVolID( "sector", 0 );
    deSector = de_sector_dual.clone( "Sector0", 0 );
    deSector.setPlacement( pv );
    deGroup.add( deSector );

    pv = lvGroup.placeVolume( lvSector, Position( _toDouble( "UTSensorSiWide/4" ), _toDouble( "UTSensorSiOffsetY" ),
                                                  _toDouble( "UTSensorSiZ" ) ) );
    pv->SetName( "pvSectorB2" );
    pv.addPhysVolID( "sector", 1 );
    deSector = de_sector_dual.clone( "Sector1", 1 );
    deSector.setPlacement( pv );
    deGroup.add( deSector );

    lvModule = volume( "lvModuleB" );
    pv       = lvModule.placeVolume( lvGroup, Position( 0, _toDouble( "UTSensorYB" ), _toDouble( "UTSensorZ" ) ) );
    pv->SetName( "pvGroupB" );
    deGroup.setPlacement( pv );

    // Module C
    // deSectorQuad
    de_sector_quad = DetElement( "sector", id );
    lvSector       = volume( "lvSectorQuad" );
    pv             = lvSector.placeVolume( volume( "lvSensorQuad" ) );
    pv->SetName( "pvSensorQuad" );
    pv.addPhysVolID( "sensor", 0 );
    deSensor = DetElement( de_sector_quad, "Sensor0", 0 );
    deSensor.setPlacement( pv );

    de_module_c = DetElement( "module", id );
    deGroup     = DetElement( de_module_c, "GroupC", 0 );
    lvGroup     = volume( "lvGroupC" );

    pv = lvGroup.placeVolume( lvSector, Position( _toDouble( "-UTSensorSiWide/4" ), _toDouble( "UTSensorSiOffsetY" ),
                                                  _toDouble( "UTSensorSiZ" ) ) );
    pv->SetName( "pvSectorC1" );
    pv.addPhysVolID( "sector", 0 );
    deSector = de_sector_quad.clone( "Sector0", 0 );
    deSector.setPlacement( pv );
    deGroup.add( deSector );

    pv = lvGroup.placeVolume( lvSector, Position( _toDouble( "UTSensorSiWide/4" ), _toDouble( "UTSensorSiOffsetY" ),
                                                  _toDouble( "UTSensorSiZ" ) ) );
    pv->SetName( "pvSectorC2" );
    pv.addPhysVolID( "sector", 1 );
    deSector = de_sector_quad.clone( "Sector1", 1 );
    deSector.setPlacement( pv );
    deGroup.add( deSector );

    lvModule = volume( "lvModuleC" );
    pv       = lvModule.placeVolume( lvGroup, Position( 0, _toDouble( "UTSensorYC" ), _toDouble( "UTSensorZ" ) ) );
    pv->SetName( "pvGroupC" );
    deGroup.setPlacement( pv );

    // Module D
    de_module_d = DetElement( "module", id );
    deGroup     = DetElement( de_module_d, "GroupD", 0 );
    deSector    = de_sector_quad.clone( "Sector0", 0 );
    deGroup.add( deSector );

    lvSector = volume( "lvSectorQuad" );
    lvGroup  = volume( "lvGroupD" );
    pv = lvGroup.placeVolume( lvSector, Position( _toDouble( "-UTSensorSiWide/4" ), _toDouble( "UTSensorSiOffsetY" ),
                                                  _toDouble( "UTSensorSiZ" ) ) );
    pv->SetName( "pvSectorD1" );
    pv.addPhysVolID( "sector", 0 );
    deSector.setPlacement( pv );

    deSector = DetElement( deGroup, "Sector1", 1 );
    deSensor = DetElement( deSector, "Sensor0", 0 );
    lvSector = volume( "lvSectorHole" );
    pv = lvGroup.placeVolume( lvSector, Position( _toDouble( "UTSensorSiWide/4" ), _toDouble( "UTSensorSiOffsetY" ),
                                                  _toDouble( "UTSensorSiZ" ) ) );
    pv->SetName( "pvSectorD2" );
    pv.addPhysVolID( "sector", 1 );
    deSector.setPlacement( pv );
    lvSensor = volume( "lvSensorHole" );
    pv       = lvSector.placeVolume( lvSensor );
    pv->SetName( "pvSensorHole" );
    pv.addPhysVolID( "sensor", 0 );
    deSensor.setPlacement( pv );

    lvModule = volume( "lvModuleD" );
    pv       = lvModule.placeVolume( lvGroup, Position( 0, _toDouble( "UTSensorYD" ), _toDouble( "UTSensorZ" ) ) );
    pv->SetName( "pvGroupD" );
    deGroup.setPlacement( pv );

    detail::destroyHandle( de_sector_dual );
    detail::destroyHandle( de_sector_quad );
  }

  void UTBuild::build_staves() {

    PlacedVolume pv;
    Volume       lvStave, lvStaveFace;
    DetElement   deStaveFace, deModule;

    build_modules();

    // Stave A
    de_stave_a = DetElement( "stave", id );
    lvStave    = volume( "lvStaveA" );

    // Stave A Front
    deStaveFace = DetElement( de_stave_a, "Face1", 1 );
    lvStaveFace = volume( "lvStaveAFront" );

    // Module 0
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleA" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "-13/2*UTStaveHybridAStepY-UTStaveAHybridY0" ), 0 ) ) );
    pv->SetName( "pvModuleA1" );
    pv.addPhysVolID( "module", 0 );
    deModule = de_module_a.clone( "Module0", 0 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 1
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleA" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "-9/2*UTStaveHybridAStepY-UTStaveAHybridY0" ), 0 ) ) );
    pv->SetName( "pvModuleA3" );
    pv.addPhysVolID( "module", 1 );
    deModule = de_module_a.clone( "Module1", 1 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 2
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleA" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "-5/2*UTStaveHybridAStepY-UTStaveAHybridY0" ), 0 ) ) );
    pv->SetName( "pvModuleA5" );
    pv.addPhysVolID( "module", 2 );
    deModule = de_module_a.clone( "Module2", 2 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 3
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleA" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "-1/2*UTStaveHybridAStepY-UTStaveAHybridY0" ), 0 ) ) );
    pv->SetName( "pvModuleA7" );
    pv.addPhysVolID( "module", 3 );
    deModule = de_module_a.clone( "Module3", 3 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 5
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleA" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ),
                     Position( 0, _toDouble( "3/2*UTStaveHybridAStepY+UTStaveAHybridY0" ), 0 ) ) );
    pv->SetName( "pvModuleA9" );
    pv.addPhysVolID( "module", 5 );
    deModule = de_module_a.clone( "Module5", 5 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 6
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleA" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ),
                     Position( 0, _toDouble( "7/2*UTStaveHybridAStepY+UTStaveAHybridY0" ), 0 ) ) );
    pv->SetName( "pvModuleA11" );
    pv.addPhysVolID( "module", 6 );
    deModule = de_module_a.clone( "Module6", 6 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 7
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleA" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ),
                     Position( 0, _toDouble( "11/2*UTStaveHybridAStepY+UTStaveAHybridY0" ), 0 ) ) );
    pv->SetName( "pvModuleA13" );
    pv.addPhysVolID( "module", 7 );
    deModule = de_module_a.clone( "Module7", 7 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    pv = lvStave.placeVolume( lvStaveFace, Position( 0, 0, _toDouble( "UTStaveHybridZ" ) ) );
    pv->SetName( "pvStaveAFront" );
    pv.addPhysVolID( "face", 1 );
    deStaveFace.setPlacement( pv );

    // Stave A Back
    deStaveFace = DetElement( de_stave_a, "Face0", 0 );
    lvStaveFace = volume( "lvStaveABack" );

    // Module 0
    pv = lvStaveFace.placeVolume( volume( "lvModuleA" ),
                                  Position( 0, _toDouble( "-11/2*UTStaveHybridAStepY-UTStaveAHybridY0" ), 0 ) );
    pv->SetName( "pvModuleA2" );
    pv.addPhysVolID( "module", 0 );
    deModule = de_module_a.clone( "Module0", 0 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 1
    pv = lvStaveFace.placeVolume( volume( "lvModuleA" ),
                                  Position( 0, _toDouble( "-7/2*UTStaveHybridAStepY-UTStaveAHybridY0" ), 0 ) );
    pv->SetName( "pvModuleA4" );
    pv.addPhysVolID( "module", 1 );
    deModule = de_module_a.clone( "Module1", 1 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 2
    pv = lvStaveFace.placeVolume( volume( "lvModuleA" ),
                                  Position( 0, _toDouble( "-3/2*UTStaveHybridAStepY-UTStaveAHybridY0" ), 0 ) );
    pv->SetName( "pvModuleA6" );
    pv.addPhysVolID( "module", 2 );
    deModule = de_module_a.clone( "Module2", 2 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 4
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleA" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "1/2*UTStaveHybridAStepY+UTStaveAHybridY0" ), 0 ) ) );
    pv->SetName( "pvModuleA8" );
    pv.addPhysVolID( "module", 4 );
    deModule = de_module_a.clone( "Module4", 4 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 5
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleA" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "5/2*UTStaveHybridAStepY+UTStaveAHybridY0" ), 0 ) ) );
    pv->SetName( "pvModuleA10" );
    pv.addPhysVolID( "module", 5 );
    deModule = de_module_a.clone( "Module5", 5 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 6
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleA" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "9/2*UTStaveHybridAStepY+UTStaveAHybridY0" ), 0 ) ) );
    pv->SetName( "pvModuleA12" );
    pv.addPhysVolID( "module", 6 );
    deModule = de_module_a.clone( "Module6", 6 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 7
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleA" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "13/2*UTStaveHybridAStepY+UTStaveAHybridY0" ), 0 ) ) );
    pv->SetName( "pvModuleA14" );
    pv.addPhysVolID( "module", 7 );
    deModule = de_module_a.clone( "Module7", 7 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    pv = lvStave.placeVolume( lvStaveFace, Position( 0, 0, _toDouble( "-UTStaveHybridZ" ) ) );
    pv->SetName( "pvStaveABack" );
    pv.addPhysVolID( "face", 0 );
    deStaveFace.setPlacement( pv );

    // Stave B
    de_stave_b = DetElement( "stave", id );
    lvStave    = volume( "lvStaveB" );

    // Stave B Front
    deStaveFace = DetElement( de_stave_b, "Face1", 1 );
    lvStaveFace = volume( "lvStaveBFront" );

    // Module 0
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleA" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "-13/2*UTStaveHybridAStepY-UTStaveBHybridY0" ), 0 ) ) );
    pv->SetName( "pvModuleB1" );
    pv.addPhysVolID( "module", 0 );
    deModule = de_module_a.clone( "Module0", 0 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 1
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleA" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "-9/2*UTStaveHybridAStepY-UTStaveBHybridY0" ), 0 ) ) );
    pv->SetName( "pvModuleB3" );
    pv.addPhysVolID( "module", 1 );
    deModule = de_module_a.clone( "Module1", 1 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 2
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleA" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "-5/2*UTStaveHybridAStepY-UTStaveBHybridY0" ), 0 ) ) );
    pv->SetName( "pvModuleB5" );
    pv.addPhysVolID( "module", 2 );
    deModule = de_module_a.clone( "Module2", 2 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 3
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleB" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "-1/2*UTStaveHybridAStepY-UTStaveBHybridY0" ), 0 ) ) );
    pv->SetName( "pvModuleB7" );
    pv.addPhysVolID( "module", 3 );
    deModule = de_module_b.clone( "Module3", 3 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 5
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleB" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ),
                     Position( 0, _toDouble( "3/2*UTStaveHybridAStepY+UTStaveBHybridY0" ), 0 ) ) );
    pv->SetName( "pvModuleB9" );
    pv.addPhysVolID( "module", 5 );
    deModule = de_module_b.clone( "Module5", 5 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 6
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleA" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ),
                     Position( 0, _toDouble( "7/2*UTStaveHybridAStepY+UTStaveBHybridY0" ), 0 ) ) );
    pv->SetName( "pvModuleB11" );
    pv.addPhysVolID( "module", 6 );
    deModule = de_module_a.clone( "Module6", 6 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 7
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleA" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ),
                     Position( 0, _toDouble( "11/2*UTStaveHybridAStepY+UTStaveBHybridY0" ), 0 ) ) );
    pv->SetName( "pvModuleB13" );
    pv.addPhysVolID( "module", 7 );
    deModule = de_module_a.clone( "Module7", 7 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    pv = lvStave.placeVolume( lvStaveFace, Position( 0, 0, _toDouble( "UTStaveHybridZ" ) ) );
    pv->SetName( "pvStaveBFront" );
    pv.addPhysVolID( "face", 1 );
    deStaveFace.setPlacement( pv );

    // Stave B Back
    deStaveFace = DetElement( de_stave_b, "Face0", 0 );
    lvStaveFace = volume( "lvStaveBBack" );

    // Module 0
    pv = lvStaveFace.placeVolume( volume( "lvModuleA" ),
                                  Position( 0, _toDouble( "-11/2*UTStaveHybridAStepY-UTStaveBHybridY0" ), 0 ) );
    pv->SetName( "pvModuleB2" );
    pv.addPhysVolID( "module", 0 );
    deModule = de_module_a.clone( "Module0" );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 1
    pv = lvStaveFace.placeVolume( volume( "lvModuleA" ),
                                  Position( 0, _toDouble( "-7/2*UTStaveHybridAStepY-UTStaveBHybridY0" ), 0 ) );
    pv->SetName( "pvModuleB4" );
    pv.addPhysVolID( "module", 1 );
    deModule = de_module_a.clone( "Module1" );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 2
    pv = lvStaveFace.placeVolume( volume( "lvModuleB" ),
                                  Position( 0, _toDouble( "-3/2*UTStaveHybridAStepY-UTStaveBHybridY0" ), 0 ) );
    pv->SetName( "pvModuleB6" );
    pv.addPhysVolID( "module", 2 );
    deModule = de_module_b.clone( "Module2", 2 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 4
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleB" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "1/2*UTStaveHybridAStepY+UTStaveBHybridY0" ), 0 ) ) );
    pv->SetName( "pvModuleB8" );
    pv.addPhysVolID( "module", 4 );
    deModule = de_module_b.clone( "Module4", 4 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 5
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleA" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "5/2*UTStaveHybridAStepY+UTStaveBHybridY0" ), 0 ) ) );
    pv->SetName( "pvModuleB10" );
    pv.addPhysVolID( "module", 5 );
    deModule = de_module_a.clone( "Module5", 5 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 6
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleA" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "9/2*UTStaveHybridAStepY+UTStaveBHybridY0" ), 0 ) ) );
    pv->SetName( "pvModuleB12" );
    pv.addPhysVolID( "module", 6 );
    deModule = de_module_a.clone( "Module6", 6 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 7
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleA" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "13/2*UTStaveHybridAStepY+UTStaveBHybridY0" ), 0 ) ) );
    pv->SetName( "pvModuleB14" );
    pv.addPhysVolID( "module", 7 );
    deModule = de_module_a.clone( "Module7", 7 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    pv = lvStave.placeVolume( lvStaveFace, Position( 0, 0, _toDouble( "-UTStaveHybridZ" ) ) );
    pv->SetName( "pvStaveBBack" );
    pv.addPhysVolID( "face", 0 );
    deStaveFace.setPlacement( pv );

    // Stave C
    de_stave_c = DetElement( "stave", id );
    lvStave    = volume( "lvStaveC" );

    // Stave C Front
    deStaveFace = DetElement( de_stave_c, "Face1", 1 );
    lvStaveFace = volume( "lvStaveCFront" );

    // Module 0
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleA" ),
        Transform3D(
            RotationY( M_PI ),
            Position( 0, _toDouble( "-11/2*UTStaveHybridAStepY-UTStaveCHybridY0-2*UTStaveHybridCStepY" ), 0 ) ) );
    pv->SetName( "pvModuleC1" );
    pv.addPhysVolID( "module", 0 );
    deModule = de_module_a.clone( "Module0", 0 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 1
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleA" ),
        Transform3D(
            RotationY( M_PI ),
            Position( 0, _toDouble( "-7/2*UTStaveHybridAStepY-UTStaveCHybridY0-2*UTStaveHybridCStepY" ), 0 ) ) );
    pv->SetName( "pvModuleC3" );
    pv.addPhysVolID( "module", 1 );
    deModule = de_module_a.clone( "Module1", 1 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 2
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleA" ),
        Transform3D(
            RotationY( M_PI ),
            Position( 0, _toDouble( "-3/2*UTStaveHybridAStepY-UTStaveCHybridY0-2*UTStaveHybridCStepY" ), 0 ) ) );
    pv->SetName( "pvModuleC5" );
    pv.addPhysVolID( "module", 2 );
    deModule = de_module_a.clone( "Module2", 2 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 3
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleC" ),
        Transform3D( RotationY( M_PI ), Position( 0, _toDouble( "-3/2*UTStaveHybridCStepY-UTStaveCHybridY0" ), 0 ) ) );
    pv->SetName( "pvModuleC7" );
    pv.addPhysVolID( "module", 3 );
    deModule = de_module_c.clone( "Module3", 3 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 4
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleD" ),
        Transform3D( RotationZYX( M_PI, M_PI, 0 ),
                     Position( 0, _toDouble( "1/2*UTStaveHybridCStepY+UTStaveCHybridY0" ), 0 ) ) );
    pv->SetName( "pvModuleC9" );
    pv.addPhysVolID( "module", 4 );
    deModule = de_module_d.clone( "Module4", 4 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 5
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleB" ),
        Transform3D(
            RotationZYX( M_PI, M_PI, 0 ),
            Position( 0, _toDouble( "1/2*UTStaveHybridAStepY+UTStaveCHybridY0+2*UTStaveHybridCStepY" ), 0 ) ) );
    pv->SetName( "pvModuleC11" );
    pv.addPhysVolID( "module", 5 );
    deModule = de_module_b.clone( "Module5", 5 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Moduel 6
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleA" ),
        Transform3D(
            RotationZYX( M_PI, M_PI, 0 ),
            Position( 0, _toDouble( "5/2*UTStaveHybridAStepY+UTStaveCHybridY0+2*UTStaveHybridCStepY" ), 0 ) ) );
    pv->SetName( "pvModuleC13" );
    pv.addPhysVolID( "module", 6 );
    deModule = de_module_a.clone( "Module6", 6 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Moduel 7
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleA" ),
        Transform3D(
            RotationZYX( M_PI, M_PI, 0 ),
            Position( 0, _toDouble( "9/2*UTStaveHybridAStepY+UTStaveCHybridY0+2*UTStaveHybridCStepY" ), 0 ) ) );
    pv->SetName( "pvModuleC15" );
    pv.addPhysVolID( "module", 7 );
    deModule = de_module_a.clone( "Module7", 7 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    pv = lvStave.placeVolume( lvStaveFace, Position( 0, 0, _toDouble( "UTStaveHybridZ" ) ) );
    pv->SetName( "pvStaveCFront" );
    pv.addPhysVolID( "face", 1 );
    deStaveFace.setPlacement( pv );

    // Stave C Back
    deStaveFace = DetElement( de_stave_c, "Face0", 0 );
    lvStaveFace = volume( "lvStaveCBack" );

    // Module 0
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleA" ),
        Position( 0, _toDouble( "-9/2*UTStaveHybridAStepY-UTStaveCHybridY0-2*UTStaveHybridCStepY" ), 0 ) );
    pv->SetName( "pvModuleC2" );
    pv.addPhysVolID( "module", 0 );
    deModule = de_module_a.clone( "Module0", 0 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 1
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleA" ),
        Position( 0, _toDouble( "-5/2*UTStaveHybridAStepY-UTStaveCHybridY0-2*UTStaveHybridCStepY" ), 0 ) );
    pv->SetName( "pvModuleC4" );
    pv.addPhysVolID( "module", 1 );
    deModule = de_module_a.clone( "Module1", 1 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 2
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleB" ),
        Position( 0, _toDouble( "-1/2*UTStaveHybridAStepY-UTStaveCHybridY0-2*UTStaveHybridCStepY" ), 0 ) );
    pv->SetName( "pvModuleC6" );
    pv.addPhysVolID( "module", 2 );
    deModule = de_module_b.clone( "Module2", 2 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 3
    pv = lvStaveFace.placeVolume( volume( "lvModuleD" ),
                                  Position( 0, _toDouble( "-1/2*UTStaveHybridCStepY-UTStaveCHybridY0" ), 0 ) );
    pv->SetName( "pvModuleC8" );
    pv.addPhysVolID( "module", 3 );
    deModule = de_module_d.clone( "Module3", 3 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Module 4
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleC" ),
        Transform3D( RotationZ( M_PI ), Position( 0, _toDouble( "3/2*UTStaveHybridCStepY+UTStaveCHybridY0" ), 0 ) ) );
    pv->SetName( "pvModuleC10" );
    pv.addPhysVolID( "module", 4 );
    deModule = de_module_c.clone( "Module4", 4 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Moduel 5
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleA" ),
        Transform3D(
            RotationZ( M_PI ),
            Position( 0, _toDouble( "3/2*UTStaveHybridAStepY+UTStaveCHybridY0+2*UTStaveHybridCStepY" ), 0 ) ) );
    pv->SetName( "pvModuleC12" );
    pv.addPhysVolID( "module", 5 );
    deModule = de_module_a.clone( "Module5", 5 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Moduel 6
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleA" ),
        Transform3D(
            RotationZ( M_PI ),
            Position( 0, _toDouble( "7/2*UTStaveHybridAStepY+UTStaveCHybridY0+2*UTStaveHybridCStepY" ), 0 ) ) );
    pv->SetName( "pvModuleC14" );
    pv.addPhysVolID( "module", 6 );
    deModule = de_module_a.clone( "Module6", 6 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    // Moduel 7
    pv = lvStaveFace.placeVolume(
        volume( "lvModuleA" ),
        Transform3D(
            RotationZ( M_PI ),
            Position( 0, _toDouble( "11/2*UTStaveHybridAStepY+UTStaveCHybridY0+2*UTStaveHybridCStepY" ), 0 ) ) );
    pv->SetName( "pvModuleC16" );
    pv.addPhysVolID( "module", 7 );
    deModule = de_module_a.clone( "Module7", 7 );
    deModule.setPlacement( pv );
    deStaveFace.add( deModule );

    pv = lvStave.placeVolume( lvStaveFace, Position( 0, 0, _toDouble( "-UTStaveHybridZ" ) ) );
    pv->SetName( "pvStaveCBack" );
    pv.addPhysVolID( "face", 0 );
    deStaveFace.setPlacement( pv );
  }

  void UTBuild::build_detector() {
    PlacedVolume pv;
    Volume       lvUTC, lvUTA, lvLayer;
    DetElement   deUTC, deUTA, deLayer;
    DetElement   deStave;

    // Construct sides
    if ( m_build_sides ) {

      lvUTC = volume( "lvUTC" );
      lvUTA = volume( "lvUTA" );
      deUTC = DetElement( detector, "Cside", 0 );
      deUTA = DetElement( detector, "Aside", 1 );

      if ( m_build_cside ) {
        if ( m_build_cside_utax ) {
          lvLayer = volume( "lvUTaXCsideLayer" );
          deLayer = DetElement( deUTC, "UTaX", 0 );
          pv      = lvUTC.placeVolume( lvLayer );
          pv->SetName( "pvUTaXCsideLayer" );
          pv.addPhysVolID( "layer", 0 );
          deLayer.setPlacement( pv );

          // Stave 1 (UTaX_1C), index starts from 0
          pv = lvLayer.placeVolume(
              volume( "lvStaveC" ),
              Position(
                  _toDouble( "1/2*UTLayerSideCorrectedWidth-1/2*UTStaveStep+UTStaveCCorrect+UTaXR2S2CorrectionX" ), 0,
                  _toDouble( "-UTStaveDz/2" ) ) );
          pv->SetName( "pvCsideUTaXStave0" );
          pv.addPhysVolID( "stave", 0 );
          deStave = de_stave_c.clone( "Stave0", 0 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 2 (UTaX_2C)
          pv = lvLayer.placeVolume(
              volume( "lvStaveB" ),
              Position(
                  _toDouble( "1/2*UTLayerSideCorrectedWidth-3/2*UTStaveStep+UTStaveBCorrect+UTaXR2S1CorrectionX" ), 0,
                  _toDouble( "UTStaveDz/2" ) ) );
          pv->SetName( "pvCsideUTaXStave1" );
          pv.addPhysVolID( "stave", 1 );
          deStave = de_stave_b.clone( "Stave1", 1 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 3-8 (UTaX_3C-8C)
          for ( int i = 2; i < 8; i++ ) {
            pv = lvLayer.placeVolume(
                volume( "lvStaveA" ),
                Position( _toDouble( "1/2*UTLayerSideCorrectedWidth" ) -
                              ( 5.0 / 2 + ( i - 2 ) ) * _toDouble( "UTStaveStep" ) +
                              _toDouble( "UTStaveACorrect+UTaXR1S" + std::to_string( 8 - i ) + "CorrectionX" ),
                          0, pow( -1, i + 1 ) * _toDouble( "UTStaveDz/2" ) ) );
            pv->SetName( _toString( i, "pvCsideUTaXStave%d" ).c_str() );
            pv.addPhysVolID( "stave", i );
            deStave = de_stave_a.clone( _toString( i, "Stave%d" ), i );
            deStave.setPlacement( pv );
            deLayer.add( deStave );
          }

        } // lvUTaXCsideLayer

        if ( m_build_cside_utau ) {
          lvLayer = volume( "lvUTaUCsideLayer" );
          deLayer = DetElement( deUTC, "UTaU", 1 );
          pv      = lvUTC.placeVolume( lvLayer, Position( 0, 0, _toDouble( "UTLayerDz" ) ) );
          pv->SetName( "pvUTaUCsideLayer" );
          pv.addPhysVolID( "layer", 1 );
          deLayer.setPlacement( pv );

          // Stave 1 (UTaU_1C), index starts from 0
          pv = lvLayer.placeVolume(
              volume( "lvStaveC" ),
              Transform3D(
                  RotationZ( _toDouble( "-UTAngle" ) ),
                  Position( _toDouble(
                                "1/2*UTLayerSideCorrectedWidth-1/2*UTStaveStep+UTStaveCCorrectUV+UTaUR2S2CorrectionX" ),
                            _toDouble( "UTStaveCUVPosY" ), _toDouble( "-UTStaveDz/2" ) ) ) );
          pv->SetName( "pvCsideUTaUStave0" );
          pv.addPhysVolID( "stave", 0 );
          deStave = de_stave_c.clone( "Stave0", 0 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 2 (UTaU_2C)
          pv = lvLayer.placeVolume(
              volume( "lvStaveB" ),
              Transform3D(
                  RotationZ( _toDouble( "-UTAngle" ) ),
                  Position( _toDouble(
                                "1/2*UTLayerSideCorrectedWidth-3/2*UTStaveStep+UTStaveBCorrectUV+UTaUR2S1CorrectionX" ),
                            0, _toDouble( "UTStaveDz/2" ) ) ) );
          pv->SetName( "pvCsideUTaUStave1" );
          pv.addPhysVolID( "stave", 1 );
          deStave = de_stave_b.clone( "Stave1", 1 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 3-8 (UTaU_3C-8C)
          for ( int i = 2; i < 8; i++ ) {
            pv = lvLayer.placeVolume( volume( "lvStaveA" ),
                                      Transform3D( RotationZ( _toDouble( "-UTAngle" ) ),
                                                   Position( _toDouble( "1/2*UTLayerSideCorrectedWidth" ) -
                                                                 ( 5.0 / 2 + ( i - 2 ) ) * _toDouble( "UTStaveStep" ) +
                                                                 _toDouble( "UTStaveACorrectUV+UTaUR1S" +
                                                                            std::to_string( 8 - i ) + "CorrectionX" ),
                                                             0, pow( -1, i + 1 ) * _toDouble( "UTStaveDz/2" ) ) ) );
            pv->SetName( _toString( i, "pvCsideUTaUStave%d" ).c_str() );
            pv.addPhysVolID( "stave", i );
            deStave = de_stave_a.clone( _toString( i, "Stave%d" ), i );
            deStave.setPlacement( pv );
            deLayer.add( deStave );
          }
        } // lvUTaUCsideLayer

        if ( m_build_cside_utbv ) {
          lvLayer = volume( "lvUTbVCsideLayer" );
          deLayer = DetElement( deUTC, "UTbV", 2 );
          pv      = lvUTC.placeVolume( lvLayer, Position( 0, 0, _toDouble( "UTStationDz" ) ) );
          pv->SetName( "pvUTbVCsideLayer" );
          pv.addPhysVolID( "layer", 2 );
          deLayer.setPlacement( pv );

          // Stave 1 (UTbV_1C), index starts from 0
          pv = lvLayer.placeVolume(
              volume( "lvStaveC" ),
              Transform3D(
                  RotationZ( _toDouble( "UTAngle" ) ),
                  Position( _toDouble(
                                "1/2*UTLayerSideCorrectedWidth-1/2*UTStaveStep+UTStaveCCorrectUV+UTbVR2S2CorrectionX" ),
                            _toDouble( "-UTStaveCUVPosY" ), _toDouble( "-UTStaveDz/2" ) ) ) );
          pv->SetName( "pvCsideUTbVStave0" );
          pv.addPhysVolID( "stave", 0 );
          deStave = de_stave_c.clone( "Stave0", 0 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 2 (UTbV_2C)
          pv = lvLayer.placeVolume(
              volume( "lvStaveB" ),
              Transform3D(
                  RotationZ( _toDouble( "UTAngle" ) ),
                  Position( _toDouble(
                                "1/2*UTLayerSideCorrectedWidth-3/2*UTStaveStep+UTStaveBCorrectUV+UTbVR2S1CorrectionX" ),
                            0, _toDouble( "UTStaveDz/2" ) ) ) );
          pv->SetName( "pvCsideUTbVStave1" );
          pv.addPhysVolID( "stave", 1 );
          deStave = de_stave_b.clone( "Stave1", 1 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 3-9 (UTbV_3C-9C)
          for ( int i = 2; i < 9; i++ ) {
            pv = lvLayer.placeVolume( volume( "lvStaveA" ),
                                      Transform3D( RotationZ( _toDouble( "UTAngle" ) ),
                                                   Position( _toDouble( "1/2*UTLayerSideCorrectedWidth" ) -
                                                                 ( 5.0 / 2 + ( i - 2 ) ) * _toDouble( "UTStaveStep" ) +
                                                                 _toDouble( "UTStaveACorrectUV+UTbVR1S" +
                                                                            std::to_string( 9 - i ) + "CorrectionX" ),
                                                             0, pow( -1, i + 1 ) * _toDouble( "UTStaveDz/2" ) ) ) );
            pv->SetName( _toString( i, "pvCsideUTbVStave%d" ).c_str() );
            pv.addPhysVolID( "stave", i );
            deStave = de_stave_a.clone( _toString( i, "Stave%d" ), i );
            deStave.setPlacement( pv );
            deLayer.add( deStave );
          }
        } // lvUTbVCsideLayer

        if ( m_build_cside_utbx ) {
          lvLayer = volume( "lvUTbXCsideLayer" );
          deLayer = DetElement( deUTC, "UTbX", 3 );
          pv      = lvUTC.placeVolume( lvLayer, Position( 0, 0, _toDouble( "UTStationDz+UTLayerDz" ) ) );
          pv->SetName( "pvUTbXCsideLayer" );
          pv.addPhysVolID( "layer", 3 );
          deLayer.setPlacement( pv );

          // Stave 1 (UTbX_1C), index starts from 0
          pv = lvLayer.placeVolume(
              volume( "lvStaveC" ),
              Position(
                  _toDouble( "1/2*UTLayerSideCorrectedWidth-1/2*UTStaveStep+UTStaveCCorrect+UTbXR2S2CorrectionX" ), 0,
                  _toDouble( "-UTStaveDz/2" ) ) );
          pv->SetName( "pvCsideUTbXStave0" );
          pv.addPhysVolID( "stave", 0 );
          deStave = de_stave_c.clone( "Stave0", 0 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 2 (UTbX_2C)
          pv = lvLayer.placeVolume(
              volume( "lvStaveB" ),
              Position(
                  _toDouble( "1/2*UTLayerSideCorrectedWidth-3/2*UTStaveStep+UTStaveBCorrect+UTbXR2S1CorrectionX" ), 0,
                  _toDouble( "UTStaveDz/2" ) ) );
          pv->SetName( "pvCsideUTbXStave1" );
          pv.addPhysVolID( "stave", 1 );
          deStave = de_stave_b.clone( "Stave1", 1 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 3-9 (UTbX_3C-9C)
          for ( int i = 2; i < 9; i++ ) {
            pv = lvLayer.placeVolume(
                volume( "lvStaveA" ),
                Position( _toDouble( "1/2*UTLayerSideCorrectedWidth" ) -
                              ( 5.0 / 2 + ( i - 2 ) ) * _toDouble( "UTStaveStep" ) +
                              _toDouble( "UTStaveACorrect+UTbXR1S" + std::to_string( 9 - i ) + "CorrectionX" ),
                          0, pow( -1, i + 1 ) * _toDouble( "UTStaveDz/2" ) ) );
            pv->SetName( _toString( i, "pvCsideUTbXStave%d" ).c_str() );
            pv.addPhysVolID( "stave", i );
            deStave = de_stave_a.clone( _toString( i, "Stave%d" ), i );
            deStave.setPlacement( pv );
            deLayer.add( deStave );
          }
        } // lvUTbXCsideLayer
      }   // Cside

      if ( m_build_aside ) {
        if ( m_build_aside_utax ) {
          lvLayer = volume( "lvUTaXAsideLayer" );
          deLayer = DetElement( deUTA, "UTaX", 0 );
          pv      = lvUTA.placeVolume( lvLayer );
          pv->SetName( "pvUTaXAsideLayer" );
          pv.addPhysVolID( "layer", 0 );
          deLayer.setPlacement( pv );

          // Stave 1 (UTaX_1A), index starts from 0
          pv = lvLayer.placeVolume(
              volume( "lvStaveC" ),
              Transform3D(
                  RotationY( M_PI ),
                  Position(
                      _toDouble( "-1/2*UTLayerSideCorrectedWidth+1/2*UTStaveStep-UTStaveCCorrect+UTaXR2S3CorrectionX" ),
                      0, _toDouble( "UTStaveDz/2" ) ) ) );
          pv->SetName( "pvAsideUTaXStave0" );
          pv.addPhysVolID( "stave", 0 );
          deStave = de_stave_c.clone( "Stave0", 0 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 2 (UTaX_2A)
          pv = lvLayer.placeVolume(
              volume( "lvStaveB" ),
              Transform3D(
                  RotationY( M_PI ),
                  Position(
                      _toDouble( "-1/2*UTLayerSideCorrectedWidth+3/2*UTStaveStep-UTStaveBCorrect+UTaXR2S4CorrectionX" ),
                      0, _toDouble( "-UTStaveDz/2" ) ) ) );
          pv->SetName( "pvAsideUTaXStave1" );
          pv.addPhysVolID( "stave", 1 );
          deStave = de_stave_b.clone( "Stave1", 1 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 3-8 (UTaX_3A-8A)
          for ( int i = 2; i < 8; i++ ) {
            pv = lvLayer.placeVolume(
                volume( "lvStaveA" ),
                Transform3D( RotationY( M_PI ), Position( _toDouble( "-1/2*UTLayerSideCorrectedWidth" ) +
                                                              ( 5.0 / 2 + ( i - 2 ) ) * _toDouble( "UTStaveStep" ) +
                                                              _toDouble( "-UTStaveACorrect+UTaXR3S" +
                                                                         std::to_string( i - 1 ) + "CorrectionX" ),
                                                          0, pow( -1, i ) * _toDouble( "UTStaveDz/2" ) ) ) );
            pv->SetName( _toString( i, "pvAsideUTaXStave%d" ).c_str() );
            pv.addPhysVolID( "stave", i );
            deStave = de_stave_a.clone( _toString( i, "Stave%d" ), i );
            deStave.setPlacement( pv );
            deLayer.add( deStave );
          }
        } // UTaXAsideLayer
        if ( m_build_aside_utau ) {
          lvLayer = volume( "lvUTaUAsideLayer" );
          deLayer = DetElement( deUTA, "UTaU", 1 );
          pv      = lvUTA.placeVolume( lvLayer, Position( 0, 0, _toDouble( "UTLayerDz" ) ) );
          pv->SetName( "pvUTaUAsideLayer" );
          pv.addPhysVolID( "layer", 1 );
          deLayer.setPlacement( pv );

          // Stave 1 (UTaU_1A), index starts from 0
          pv = lvLayer.placeVolume(
              volume( "lvStaveC" ),
              Transform3D(
                  RotationZYX( _toDouble( "UTAngle" ), M_PI, 0 ),
                  Position(
                      _toDouble(
                          "-1/2*UTLayerSideCorrectedWidth+1/2*UTStaveStep-UTStaveCCorrectUV+UTaUR2S3CorrectionX" ),
                      _toDouble( "-UTStaveCUVPosY" ), _toDouble( "UTStaveDz/2" ) ) ) );
          pv->SetName( "pvAsideUTaUStave0" );
          pv.addPhysVolID( "stave", 0 );
          deStave = de_stave_c.clone( "Stave0", 0 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 2 (UTaU_2A)
          pv = lvLayer.placeVolume(
              volume( "lvStaveB" ),
              Transform3D(
                  RotationZYX( _toDouble( "UTAngle" ), M_PI, 0 ),
                  Position(
                      _toDouble(
                          "-1/2*UTLayerSideCorrectedWidth+3/2*UTStaveStep-UTStaveBCorrectUV+UTaUR2S4CorrectionX" ),
                      0, _toDouble( "-UTStaveDz/2" ) ) ) );
          pv->SetName( "pvAsideUTaUStave1" );
          pv.addPhysVolID( "stave", 1 );
          deStave = de_stave_b.clone( "Stave1", 1 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 3-8 (UTaU_3A-8A)
          for ( int i = 2; i < 8; i++ ) {
            pv = lvLayer.placeVolume( volume( "lvStaveA" ),
                                      Transform3D( RotationZYX( _toDouble( "UTAngle" ), M_PI, 0 ),
                                                   Position( _toDouble( "-1/2*UTLayerSideCorrectedWidth" ) +
                                                                 ( 5.0 / 2 + ( i - 2 ) ) * _toDouble( "UTStaveStep" ) +
                                                                 _toDouble( "-UTStaveACorrectUV+UTaUR3S" +
                                                                            std::to_string( i - 1 ) + "CorrectionX" ),
                                                             0, pow( -1, i ) * _toDouble( "UTStaveDz/2" ) ) ) );
            pv->SetName( _toString( i, "pvAsideUTaUStave%d" ).c_str() );
            pv.addPhysVolID( "stave", i );
            deStave = de_stave_a.clone( _toString( i, "Stave%d" ), i );
            deStave.setPlacement( pv );
            deLayer.add( deStave );
          }
        } // lvUTaUAsideLayer
        if ( m_build_aside_utbv ) {
          lvLayer = volume( "lvUTbVAsideLayer" );
          deLayer = DetElement( deUTA, "UTbV", 2 );
          pv      = lvUTA.placeVolume( lvLayer, Position( 0, 0, _toDouble( "UTStationDz" ) ) );
          pv->SetName( "pvUTbVAsideLayer" );
          pv.addPhysVolID( "layer", 2 );
          deLayer.setPlacement( pv );

          // Stave 1 (UTbV_1A), index starts from 0
          pv = lvLayer.placeVolume(
              volume( "lvStaveC" ),
              Transform3D(
                  RotationZYX( _toDouble( "-UTAngle" ), M_PI, 0 ),
                  Position(
                      _toDouble(
                          "-1/2*UTLayerSideCorrectedWidth+1/2*UTStaveStep-UTStaveCCorrectUV+UTbVR2S3CorrectionX" ),
                      _toDouble( "UTStaveCUVPosY" ), _toDouble( "UTStaveDz/2" ) ) ) );
          pv->SetName( "pvAsideUTbVStave0" );
          pv.addPhysVolID( "stave", 0 );
          deStave = de_stave_c.clone( "Stave0", 0 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 2 (UTbV_2A)
          pv = lvLayer.placeVolume(
              volume( "lvStaveB" ),
              Transform3D(
                  RotationZYX( _toDouble( "-UTAngle" ), M_PI, 0 ),
                  Position(
                      _toDouble(
                          "-1/2*UTLayerSideCorrectedWidth+3/2*UTStaveStep-UTStaveBCorrectUV+UTbVR2S4CorrectionX" ),
                      0, _toDouble( "-UTStaveDz/2" ) ) ) );
          pv->SetName( "pvAsideUTbVStave1" );
          pv.addPhysVolID( "stave", 1 );
          deStave = de_stave_b.clone( "Stave1", 1 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 3-9 (UTbV_3A-9A)
          for ( int i = 2; i < 9; i++ ) {
            pv = lvLayer.placeVolume( volume( "lvStaveA" ),
                                      Transform3D( RotationZYX( _toDouble( "-UTAngle" ), M_PI, 0 ),
                                                   Position( _toDouble( "-1/2*UTLayerSideCorrectedWidth" ) +
                                                                 ( 5.0 / 2 + ( i - 2 ) ) * _toDouble( "UTStaveStep" ) +
                                                                 _toDouble( "-UTStaveACorrectUV+UTbVR3S" +
                                                                            std::to_string( i - 1 ) + "CorrectionX" ),
                                                             0, pow( -1, i ) * _toDouble( "UTStaveDz/2" ) ) ) );
            pv->SetName( _toString( i, "pvAsideUTbVStave%d" ).c_str() );
            pv.addPhysVolID( "stave", i );
            deStave = de_stave_a.clone( _toString( i, "Stave%d" ), i );
            deStave.setPlacement( pv );
            deLayer.add( deStave );
          }
        } // lvUTbVAsideLayer
        if ( m_build_aside_utbx ) {
          lvLayer = volume( "lvUTbXAsideLayer" );
          deLayer = DetElement( deUTA, "UTbX", 3 );
          pv      = lvUTA.placeVolume( lvLayer, Position( 0, 0, _toDouble( "UTStationDz+UTLayerDz" ) ) );
          pv->SetName( "pvUTbXAsideLayer" );
          pv.addPhysVolID( "layer", 3 );
          deLayer.setPlacement( pv );

          // Stave 1 (UTbX_1A), index starts from 0
          pv = lvLayer.placeVolume(
              volume( "lvStaveC" ),
              Transform3D(
                  RotationY( M_PI ),
                  Position(
                      _toDouble( "-1/2*UTLayerSideCorrectedWidth+1/2*UTStaveStep-UTStaveCCorrect+UTbXR2S3CorrectionX" ),
                      0, _toDouble( "UTStaveDz/2" ) ) ) );
          pv->SetName( "pvAsideUTbXStave0" );
          pv.addPhysVolID( "stave", 0 );
          deStave = de_stave_c.clone( "Stave0", 0 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 2 (UTbX_2A)
          pv = lvLayer.placeVolume(
              volume( "lvStaveB" ),
              Transform3D(
                  RotationY( M_PI ),
                  Position(
                      _toDouble( "-1/2*UTLayerSideCorrectedWidth+3/2*UTStaveStep-UTStaveBCorrect+UTbXR2S4CorrectionX" ),
                      0, _toDouble( "-UTStaveDz/2" ) ) ) );
          pv->SetName( "pvAsideUTbXStave1" );
          pv.addPhysVolID( "stave", 1 );
          deStave = de_stave_b.clone( "Stave1", 1 );
          deStave.setPlacement( pv );
          deLayer.add( deStave );

          // Stave 3-9 (UTbX_3A-9A)
          for ( int i = 2; i < 9; i++ ) {
            pv = lvLayer.placeVolume(
                volume( "lvStaveA" ),
                Transform3D( RotationY( M_PI ), Position( _toDouble( "-1/2*UTLayerSideCorrectedWidth" ) +
                                                              ( 5.0 / 2 + ( i - 2 ) ) * _toDouble( "UTStaveStep" ) +
                                                              _toDouble( "-UTStaveACorrect+UTbXR3S" +
                                                                         std::to_string( i - 1 ) + "CorrectionX" ),
                                                          0, pow( -1, i ) * _toDouble( "UTStaveDz/2" ) ) ) );
            pv->SetName( _toString( i, "pvAsideUTbXStave%d" ).c_str() );
            pv.addPhysVolID( "stave", i );
            deStave = de_stave_a.clone( _toString( i, "Stave%d" ), i );
            deStave.setPlacement( pv );
            deLayer.add( deStave );
          }
        } // lvUTbXAsideLayer
      }   // Aside
    }

    // lvUT
    Volume lvUT = volume( "lvUT" );
    if ( m_build_frame ) { pv = lvUT.placeVolume( volume( "lvUTFrame" ) ); }
    if ( m_build_box ) { pv = lvUT.placeVolume( volume( "lvUTBox" ) ); }
    if ( m_build_boxplug ) { pv = lvUT.placeVolume( volume( "lvUTBoxPlug" ) ); }
    if ( m_build_sides ) {
      if ( m_build_cside ) {
        pv = lvUT.placeVolume( lvUTC, Position( _toDouble( "-1/2*UTLayerSideCorrectedWidth" ), 0,
                                                _toDouble( "-1/2*UTStationDz-1/2*UTLayerDz" ) ) );
        pv.addPhysVolID( "side", 0 );
        deUTC.setPlacement( pv );
      }
      if ( m_build_aside ) {
        pv = lvUT.placeVolume( lvUTA, Position( _toDouble( "1/2*UTLayerSideCorrectedWidth" ), 0,
                                                _toDouble( "-1/2*UTStationDz-1/2*UTLayerDz" ) ) );
        pv.addPhysVolID( "side", 1 );
        deUTA.setPlacement( pv );
      }
    }
    if ( m_build_jacket ) {
      pv = lvUT.placeVolume( volume( "lvUTJacket" ),
                             Position( 0, 0, _toDouble( "UX851Rich1TTSplitZposIP-UTSystemZ" ) ) );
    }
    if ( m_build_pipeheater ) { pv = lvUT.placeVolume( volume( "lvUTPipeHeater" ) ); }
    // registerVolume( lvUT.name(), lvUT );
  } // build_detector

  void UTBuild::build_groups() {
    sensitive.setType( "tracker" );
    load( x_det, "include" );
    buildVolumes( x_det );
    placeDaughters( detector, Volume(), x_det );
    if ( m_build_sides ) { build_staves(); }
    build_detector();
    Volume       vol = volume( select_volume );
    PlacedVolume pv  = placeDetector( vol );
    pv.addPhysVolID( "system", id );
    detail::destroyHandle( de_module_a );
    detail::destroyHandle( de_module_b );
    detail::destroyHandle( de_module_c );
    detail::destroyHandle( de_module_d );
    detail::destroyHandle( de_stave_a );
    detail::destroyHandle( de_stave_b );
    detail::destroyHandle( de_stave_c );
  }

} // namespace

static Ref_t create_element( Detector& description, xml_h e, SensitiveDetector sens_det ) {
  UTBuild builder( description, e, sens_det );
  builder.build_groups();
  return builder.detector;
}
DECLARE_DETELEMENT( LHCb_UT_v1_0, create_element )
