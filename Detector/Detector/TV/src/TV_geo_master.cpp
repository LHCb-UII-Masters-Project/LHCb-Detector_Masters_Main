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
#include "TVector3.h"
#include "XML/Utilities.h"

namespace {

  /// Helper class to build the TV detector of LHCb
  struct TVBuild : public dd4hep::xml::tools::VolumeBuilder {
    /// Debug flags
    dd4hep::Material silicon;
    /// Reference to the module detector element
    dd4hep::DetElement de_module_with_support;
    /// Flag to disable building supports
    bool build_mod_support = true;
    /// Flag to disable building the TV half stations
    bool build_sides = true;
    /// Flag to disable building the wakefield cones
    bool build_wake_cones = true;
    /// Flag to disable building the detector support in the vacuum tank
    bool build_det_support = true;
    /// Flag to disable building the vakuum tank
    bool build_vacuum_tank = true;
    /// Flag to disable building the RF box
    bool build_rf_box = true;
    /// Flag to disable building the RF foil
    bool build_rf_foil = true;

    std::string select_volume{"lvTV"};
    /// Initializing constructor
    TVBuild( dd4hep::Detector& description, xml_elt_t e, dd4hep::SensitiveDetector sens );
    void                                          build_groups();
    void                                          build_module();
    void                                          build_detector();
    std::pair<dd4hep::Assembly, dd4hep::Assembly> generate_cooling_pipes();
    void swapCuts( const unsigned nPipe, TVector3& fV, TVector3& bV, int Direction );
  };

  /// Initializing constructor
  TVBuild::TVBuild( dd4hep::Detector& dsc, xml_elt_t e, dd4hep::SensitiveDetector sens )
      : dd4hep::xml::tools::VolumeBuilder( dsc, e, sens ) {
    // Process debug flags
    xml_comp_t x_dbg = x_det.child( _U( debug ), false );
    if ( x_dbg ) {
      for ( xml_coll_t i( x_dbg, _U( item ) ); i; ++i ) {
        xml_comp_t  c( i );
        std::string n = c.nameStr();
        if ( n == "debug" )
          debug = c.attr<int>( _U( value ) ) != 0;
        else if ( n == "build_mod_support" )
          build_mod_support = c.attr<int>( _U( value ) ) != 0;
        else if ( n == "build_sides" )
          build_sides = c.attr<int>( _U( value ) ) != 0;
        else if ( n == "build_det_support" )
          build_det_support = c.attr<int>( _U( value ) ) != 0;
        else if ( n == "build_vacuum_tank" )
          build_vacuum_tank = c.attr<int>( _U( value ) ) != 0;
        else if ( n == "build_wake_cones" )
          build_wake_cones = c.attr<int>( _U( value ) ) != 0;
        else if ( n == "build_rf_box" )
          build_rf_box = c.attr<int>( _U( value ) ) != 0;
        else if ( n == "build_rf_foil" )
          build_rf_foil = c.attr<int>( _U( value ) ) != 0;
        else if ( n == "select_volume" )
          select_volume = c.attr<std::string>( _U( value ) );
      }
    }
    silicon = description.material( "Silicon" );
  }

  void TVBuild::build_module() {
    // double       eps    = dd4hep::_toDouble("TV:Epsilon");
    double               TV_eps   = dd4hep::_toDouble( "TV:TVEpsilon" );
    double               TV_rot_z = dd4hep::_toDouble( "TV:Rotation" );
    dd4hep::Position     pos;
    dd4hep::PlacedVolume pv;

    // Distance between two readout chips
    double Chip2ChipDist = dd4hep::_toDouble( "TV:Chip2ChipDist" );
    // Chip dimensions (total = active area plus inactive edges)
    double ChipThick      = dd4hep::_toDouble( "TV:ChipThick" );
    double ChipTotalSizeX = dd4hep::_toDouble( "TV:ChipTotalSizeX" );
    double ChipTotalSizeY = dd4hep::_toDouble( "TV:ChipTotalSizeY" );
    double ChipActiveSize = dd4hep::_toDouble( "TV:ChipActiveSize" );
    double ChipTopEdgeY   = dd4hep::_toDouble( "TV:ChipTopEdgeY" );
    // Thickness of glue layer
    double GlueThick = dd4hep::_toDouble( "TV:GlueThick" );
    // Thickness of bump bond layer
    double BumpBondsThick = dd4hep::_toDouble( "TV:BumpBondsThick" );
    // Sensor dimensions (active area)
    double SiThick = dd4hep::_toDouble( "TV:SiThick" );
    double SiSizeX = dd4hep::_toDouble( "TV:SiSizeX" );
    double SiSizeY = dd4hep::_toDouble( "TV:SiSizeY" );
    // Width of inactive edge
    double SiEdge = dd4hep::_toDouble( "TV:SiEdge" );

    // Description of single Chip
    dd4hep::Box    boxChip( ChipTotalSizeX / 2.0, ChipTotalSizeY / 2.0, ChipThick / 2.0 );
    dd4hep::Volume lvChip( "lvChip", boxChip, silicon );
    lvChip.setVisAttributes( description, "TV:Chip" );

    // Glue layer between substrate and chip
    dd4hep::Box    boxGlue( ChipTotalSizeX / 2.0, ChipTotalSizeY / 2.0, GlueThick / 2.0 );
    dd4hep::Volume lvGlue( "lvGlue", boxGlue, description.material( "TV:Stycast" ) );
    lvGlue.setVisAttributes( description, "TV:Glue" );

    // Bump bond layer between chip and sensor
    dd4hep::Box    boxBumpBonds( ChipActiveSize / 2.0, ChipActiveSize / 2.0, BumpBondsThick / 2.0 );
    dd4hep::Volume lvBumpBonds( "lvBumpBonds", boxBumpBonds, description.material( "TV:BumpBonds" ) );
    lvBumpBonds.setVisAttributes( description, "TV:BumpBonds" );

    // Chip array
    dd4hep::Assembly lvChips( "lvChips" );
    double           pos_X = SiSizeX / 2.0 - ChipActiveSize - Chip2ChipDist;
    double           pos_Y = SiSizeY + ChipTopEdgeY - ChipTotalSizeY / 2.0;
    pv                     = lvChips.placeVolume( lvChip, dd4hep::Position( pos_X, pos_Y, ChipThick / 2.0 ) );
    pv->SetName( "pvChip0" );
    pv = lvChips.placeVolume( lvGlue, dd4hep::Position( pos_X, pos_Y, -GlueThick / 2.0 ) );
    pv->SetName( "pvGlue0" );
    pv = lvChips.placeVolume( lvBumpBonds, dd4hep::Position( pos_X, SiSizeY / 2.0, ChipThick + BumpBondsThick / 2.0 ) );
    pv->SetName( "pvBumpBonds0" );

    pos_X = SiSizeX / 2.0;
    pv    = lvChips.placeVolume( lvChip, dd4hep::Position( pos_X, pos_Y, ChipThick / 2.0 ) );
    pv->SetName( "pvChip1" );
    pv = lvChips.placeVolume( lvGlue, dd4hep::Position( pos_X, pos_Y, -GlueThick / 2.0 ) );
    pv->SetName( "pvGlue1" );
    pv = lvChips.placeVolume( lvBumpBonds, dd4hep::Position( pos_X, SiSizeY / 2.0, ChipThick + BumpBondsThick / 2.0 ) );
    pv->SetName( "pvBumpBonds1" );

    pos_X = SiSizeX / 2.0 + ChipActiveSize + Chip2ChipDist;
    pv    = lvChips.placeVolume( lvChip, dd4hep::Position( pos_X, pos_Y, ChipThick / 2.0 ) );
    pv->SetName( "pvChip2" );
    pv = lvChips.placeVolume( lvGlue, dd4hep::Position( pos_X, pos_Y, -GlueThick / 2.0 ) );
    pv->SetName( "pvGlue2" );
    pv = lvChips.placeVolume( lvBumpBonds, dd4hep::Position( pos_X, SiSizeY / 2.0, ChipThick + BumpBondsThick / 2.0 ) );
    pv->SetName( "pvBumpBonds2" );

    // Silicon sensor
    dd4hep::Box              detBox( ( SiSizeX + 2 * SiEdge ) / 2.0, ( SiSizeY + 2 * SiEdge ) / 2.0, SiThick / 2.0 );
    dd4hep::Tube             detCutTopRight( SiEdge + TV_eps, 2.0 * SiEdge, 2.0 * SiThick, 0.0 * dd4hep::degree,
                                 90.0 * dd4hep::degree );
    dd4hep::Tube             detCutTopLeft( SiEdge + TV_eps, 2.0 * SiEdge, 2.0 * SiThick, 90.0 * dd4hep::degree,
                                180.0 * dd4hep::degree );
    dd4hep::Tube             detCutBottomLeft( SiEdge + TV_eps, 2.0 * SiEdge, 2.0 * SiThick, 180.0 * dd4hep::degree,
                                   270.0 * dd4hep::degree );
    dd4hep::Tube             detCutBottomRight( SiEdge + TV_eps, 2.0 * SiEdge, 2.0 * SiThick, 270.0 * dd4hep::degree,
                                    360.0 * dd4hep::degree );
    dd4hep::SubtractionSolid detSolid( detBox, detCutTopRight, dd4hep::Position( SiSizeX / 2.0, SiSizeY / 2.0, 0 ) );
    detSolid =
        dd4hep::SubtractionSolid( detSolid, detCutTopLeft, dd4hep::Position( -SiSizeX / 2.0, SiSizeY / 2.0, 0 ) );
    detSolid =
        dd4hep::SubtractionSolid( detSolid, detCutBottomLeft, dd4hep::Position( -SiSizeX / 2.0, -SiSizeY / 2.0, 0 ) );
    detSolid =
        dd4hep::SubtractionSolid( detSolid, detCutBottomRight, dd4hep::Position( SiSizeX / 2.0, -SiSizeY / 2.0, 0 ) );
    dd4hep::Volume lvDet( "lvDet", detSolid, silicon );
    lvDet.setVisAttributes( description, "TV:Sensor" );
    lvDet.setSensitiveDetector( sensitive );

    dd4hep::Assembly lvSensor( "lvSensor" );
    pv = lvSensor.placeVolume( lvDet, dd4hep::Position( SiSizeX / 2.0, SiSizeY / 2.0, 0 ) );
    pv->SetName( "pvDet" );
    registerVolume( lvSensor.name(), lvSensor );

    dd4hep::Assembly     lvLadder( "lvLadder" );
    dd4hep::PlacedVolume pvSensor =
        lvLadder.placeVolume( lvSensor, dd4hep::Position( 0, 0, ChipThick + BumpBondsThick + SiThick / 2.0 + TV_eps ) );
    pvSensor->SetName( "pvSensor" );
    pv = lvLadder.placeVolume( lvChips );
    pv->SetName( "pvChips" );
    registerVolume( lvLadder.name(), lvLadder );

    dd4hep::DetElement deLadder, deSensor;
    dd4hep::Assembly   lvModule( "lvModule" );
    registerVolume( lvModule.name(), lvModule );

    /// Module structural hierarchy
    de_module_with_support = DetElement( "module", id );

    // Thickness of silicon substrate
    double SubstrateThick = dd4hep::_toDouble( "TV:SubstrateThick" );

    // Size of a single pixel
    double PixelSize = dd4hep::_toDouble( "TV:PixelSize" );

    // Positions of first pixel rows and columns
    double ClosestInnerPixelRow    = dd4hep::_toDouble( "TV:ClosestInnerPixelRow" );
    double ClosestOuterPixelRow    = dd4hep::_toDouble( "TV:ClosestOuterPixelRow" );
    double ClosestShortPixelColumn = dd4hep::_toDouble( "TV:ClosestShortPixelColumn" );
    double ClosestNLOPixelColumn   = dd4hep::_toDouble( "TV:ClosestNLOPixelColumn" );
    double ClosestCLIPixelColumn   = dd4hep::_toDouble( "TV:ClosestCLIPixelColumn" );

    // Connector side Long Inner (CLI)
    pos = dd4hep::Position( SiSizeX - ClosestCLIPixelColumn - PixelSize / 2.0,
                            -( ClosestInnerPixelRow + SiSizeY - PixelSize / 2.0 ),
                            -( SubstrateThick / 2.0 + GlueThick ) );
    pv  = lvModule.placeVolume( lvLadder, dd4hep::Transform3D( dd4hep::RotationZ( -TV_rot_z ) ) *
                                             dd4hep::Transform3D( dd4hep::RotationY( M_PI ), pos ) );
    pv->SetName( "pvLadder_0" );
    pv.addPhysVolID( "ladder", 0 );
    deLadder = DetElement( de_module_with_support, "ladder_0", 0 );
    deLadder.setPlacement( pv );
    deSensor = DetElement( deLadder, "sensor", 0 );
    deSensor.setPlacement( pvSensor );

    // Non-connector side Long Outer (NLO)
    pos = dd4hep::Position( -( ClosestNLOPixelColumn + PixelSize / 2.0 ),
                            -( ClosestOuterPixelRow + SiSizeY - PixelSize / 2.0 ), SubstrateThick / 2.0 + GlueThick );
    pv  = lvModule.placeVolume( lvLadder,
                               dd4hep::Transform3D( dd4hep::RotationZ( -TV_rot_z ) ) * dd4hep::Transform3D( pos ) );
    pv->SetName( "pvLadder_1" );
    pv.addPhysVolID( "ladder", 1 );
    deLadder = DetElement( de_module_with_support, "ladder_1", 1 );
    deLadder.setPlacement( pv );
    deSensor = DetElement( deLadder, "sensor", 1 );
    deSensor.setPlacement( pvSensor );

    // Non-connector side Short Inner (NSI)
    pos = dd4hep::Position( -( ClosestInnerPixelRow + SiSizeY - PixelSize / 2.0 ),
                            ClosestShortPixelColumn + PixelSize / 2.0, SubstrateThick / 2.0 + GlueThick );
    pv  = lvModule.placeVolume( lvLadder, dd4hep::Transform3D( dd4hep::RotationZ( -TV_rot_z ) ) *
                                             dd4hep::Transform3D( dd4hep::RotationZ( -M_PI / 2.0 ), pos ) );
    pv->SetName( "pvLadder_2" );
    pv.addPhysVolID( "ladder", 2 );
    deLadder = DetElement( de_module_with_support, "ladder_2", 2 );
    deLadder.setPlacement( pv );
    deSensor = DetElement( deLadder, "sensor", 2 );
    deSensor.setPlacement( pvSensor );

    // Connector side Short Outer (CSO)
    pos = dd4hep::Position( -( ClosestOuterPixelRow + SiSizeY - PixelSize / 2.0 ),
                            -( SiSizeX - ClosestShortPixelColumn - PixelSize / 2.0 ),
                            -( SubstrateThick / 2.0 + GlueThick ) );
    pv  = lvModule.placeVolume( lvLadder, dd4hep::Transform3D( dd4hep::RotationZ( -TV_rot_z ) ) *
                                             dd4hep::Transform3D( dd4hep::RotationZYX( M_PI / 2.0, M_PI, 0.0 ), pos ) );
    pv->SetName( "pvLadder_3" );
    pv.addPhysVolID( "ladder", 3 );
    deLadder = DetElement( de_module_with_support, "ladder_3", 3 );
    deLadder.setPlacement( pv );
    deSensor = DetElement( deLadder, "sensor", 3 );
    deSensor.setPlacement( pvSensor );

    // Hybrid
    dd4hep::Assembly lvModuleWithSupport( "lvModuleWithSupport" );
    if ( build_mod_support ) {
      pv = lvModule.placeVolume( volume( "lvHybrid" ) );
      pv->SetName( "pvHybrid" );

      auto [pipeL, pipeR] = generate_cooling_pipes();

      registerVolume( pipeL.name(), pipeL );
      registerVolume( pipeR.name(), pipeR );

      dd4hep::Assembly lvSupport( volume( "lvSupport" ) );

      if ( pipeL->GetNdaughters() > 0 && pipeR->GetNdaughters() > 0 ) {
        dd4hep::Position    pipePosL( dd4hep::_toDouble( "TV:DeliveryPipeXpos" ),
                                   dd4hep::_toDouble( "TV:DeliveryPipeYpos" ),
                                   dd4hep::_toDouble( "TV:DeliveryPipeZpos" ) );
        dd4hep::Position    pipePosR( dd4hep::_toDouble( "TV:DeliveryPipeXpos" ),
                                   -dd4hep::_toDouble( "TV:DeliveryPipeYpos" ),
                                   dd4hep::_toDouble( "TV:DeliveryPipeZpos" ) );
        dd4hep::RotationZYX pipeRot( 0, -M_PI * 0.5, 0 );

        lvSupport.placeVolume( pipeL, dd4hep::Transform3D( pipeRot, pipePosL ) );
        lvSupport.placeVolume( pipeR, dd4hep::Transform3D( pipeRot, pipePosR ) );
      }

      pv = lvModuleWithSupport.placeVolume( volume( "lvSupport" ) );
      pv->SetName( "pvSupport" );
    }

    pv = lvModuleWithSupport.placeVolume( lvModule );
    pv->SetName( "pvModule" );

    registerVolume( lvModuleWithSupport.name(), lvModuleWithSupport );
  }

  std::pair<dd4hep::Assembly, dd4hep::Assembly> TVBuild::generate_cooling_pipes() {
    // The actual pipes which will be returned
    dd4hep::Assembly pipeL( "lvDeliveryPipeTotalL" );
    dd4hep::Assembly pipeR( "lvDeliveryPipeTotalR" );

    /// Flag for the shape of the cooling pipes
    /// If true, use a torus/curved design
    /// If false, use a blocky design similar to that in DetDesc's description
    bool build_pipe_tori{true};
    try {
      build_pipe_tori = dd4hep::_toInt( "TV:DeliveryPipeBuildTori" );
    } catch ( std::runtime_error& e ) { return std::make_pair( pipeL, pipeR ); }

    // the radii of the pipes
    const double rMinLS = dd4hep::_toDouble( "TV:DeliveryPipeInnerRadiusL" );
    const double rMinRS = dd4hep::_toDouble( "TV:DeliveryPipeInnerRadiusR" );
    const double rMaxLS = dd4hep::_toDouble( "TV:DeliveryPipeOuterRadiusL" );
    const double rMaxRS = dd4hep::_toDouble( "TV:DeliveryPipeOuterRadiusR" );

    dd4hep::Material vacuum = description.material( "Vacuum" );
    dd4hep::Material sSteel = description.material( "TV:StainlessSteel" );
    dd4hep::Material CO2    = description.material( "TV:Coolant" );

    // function to act as a shorthand for getting a length
    auto getLength = []( const char* name ) {
      double var = 0.;
      try {
        var = dd4hep::_toDouble( name );
      } catch ( std::runtime_error& e ) {}
      return var;
    };

    // vector to hold the parent volumes
    std::map<unsigned, dd4hep::Volume> parentVolsL;
    std::map<unsigned, dd4hep::Volume> parentVolsR;

    // Do a loop, make the parent volumes, then the steel pieces, then the CO2
    for ( unsigned i = 0; i < 3; ++i ) {
      // dx dy and dz are the rolling positions which will take you to the end of the pipes that have been constructed
      // so far
      double dz = 0.;
      double dy = 0.;
      double dx = 0.;

      const double TV_eps{dd4hep::_toDouble( "TV:TVEpsilon" )};

      dd4hep::Material material;
      std::string      matName;

      double rMinL{-1.0}, rMaxL{-1.0}, rMinR{-1.0}, rMaxR{-1.0};

      switch ( i ) {
      case 0:
        material = vacuum;
        matName  = "";
        rMinL    = 0.0;
        rMinR    = 0.0;
        rMaxL    = rMaxLS;
        rMaxR    = rMaxRS;
        break;
      case 1:
        material = sSteel;
        matName  = "Metal";
        rMinL    = rMinLS;
        rMinR    = rMinRS;
        rMaxL    = rMaxLS;
        rMaxR    = rMaxRS;
        break;
      case 2:
        material = CO2;
        matName  = "CO2";
        rMinL    = 0.0;
        rMinR    = 0.0;
        rMaxL    = rMinLS;
        rMaxR    = rMinRS;
        break;
      }

      int direction = 1; // direction 1 = going forward -1 = going backwards
      // we need to know the previous angles and next angles so the lengths can be truncated, avoiding overlaps
      double prevAngleX = 0., prevAngleY = 0.;
      double nextAngleX = 0., nextAngleY = 0.;

      // the front Vector fV and back vector bV will show where each piece of the pipe needs to be cut
      TVector3 fV, bV;

      for ( unsigned nPipe = 1;; ++nPipe ) {
        // The pipe lengths
        double pipeZ = getLength( Form( "TV:DeliveryPipeLength%uZ", nPipe ) );
        double pipeY = getLength( Form( "TV:DeliveryPipeLength%uY", nPipe ) );
        double pipeX = getLength( Form( "TV:DeliveryPipeLength%uX", nPipe ) );

        // set the direction, if none is specified, assume it's the same as the last piece
        try {
          direction = dd4hep::_toInt( Form( "TV:DeliveryPipeDirection%u", nPipe ) );
        } catch ( std::runtime_error& e ) {}

        // If there's no X, Y or Z length, we're done
        if ( not( pipeX or pipeY or pipeZ ) ) { break; }

        // Special case, if the pipes have a big torus in them
        if ( build_pipe_tori and ( nPipe == 3 or nPipe == 7 ) ) {
          double        rTorus = getLength( "TV:DeliveryPipeLength4Y" ) * 0.5;
          dd4hep::Torus torusL( rTorus, rMinL, rMaxL, 0.5 * M_PI, M_PI );
          dd4hep::Torus torusR( rTorus, rMinR, rMaxR, 0.5 * M_PI, M_PI );

          dd4hep::Volume torusLvol( Form( "lvDeliveryPipe%sS%uL", matName.c_str(), nPipe ), torusL, material );
          dd4hep::Volume torusRvol( Form( "lvDeliveryPipe%sS%uR", matName.c_str(), nPipe ), torusR, material );

          dd4hep::Position posL( -dx, dy + rTorus, dz );
          dd4hep::Position posR( -dx, -dy - rTorus, dz );

          if ( i == 0 ) {
            parentVolsL[nPipe] = torusLvol;
            parentVolsR[nPipe] = torusRvol;
            pipeL.placeVolume( parentVolsL[nPipe],
                               dd4hep::Transform3D( dd4hep::RotationY( 0.5 * M_PI * direction ), posL ) );
            pipeR.placeVolume( parentVolsR[nPipe],
                               dd4hep::Transform3D( dd4hep::RotationY( 0.5 * M_PI * direction ), posR ) );
          } else {
            parentVolsL[nPipe].placeVolume( torusLvol );
            parentVolsR[nPipe].placeVolume( torusRvol );
          }

          dy += 2 * rTorus;

          direction *= -1;
          nPipe += 2;
          prevAngleX = prevAngleY = 0.;
          continue;
        }

        // calculate pipe length with pythagoras
        double tubeLenL, tubeLenR;
        tubeLenL = tubeLenR = std::sqrt( pipeX * pipeX + pipeY * pipeY + pipeZ * pipeZ );

        // calculate angles and next angles
        double angleX = std::atan2( pipeY, pipeZ ) * direction;
        double angleY = std::atan2( pipeX, pipeZ );

        double nextZ = getLength( Form( "TV:DeliveryPipeLength%uZ", nPipe + 1 ) );
        double nextY = getLength( Form( "TV:DeliveryPipeLength%uY", nPipe + 1 ) );
        double nextX = getLength( Form( "TV:DeliveryPipeLength%uX", nPipe + 1 ) );

        if ( not( nextX or nextZ ) ) {
          nextAngleY = 0.;
        } else {
          nextAngleY = std::atan2( nextX, nextZ );
        }
        if ( not( nextY or nextZ ) ) {
          nextAngleX = 0.;
        } else {
          nextAngleX = std::atan2( nextY, nextZ );
        }

        // if the pipe is parallel to the Z axis, get its vectors from the next and previous angles
        if ( angleX == 0. and angleY == 0. ) {
          // half nextX and nextY
          Double_t nXa = nextAngleX * 0.5, nYa = nextAngleY * 0.5;
          // half prevX and prevY
          Double_t pXa = prevAngleX * 0.5, pYa = prevAngleY * 0.5;

          fV.SetXYZ( std::sin( nYa ), -std::sin( nXa ), std::cos( nXa ) * std::cos( nYa ) * direction );
          bV.SetXYZ( std::sin( pYa ), -std::sin( pXa ), -std::cos( pXa ) * std::cos( pYa ) * direction );
        }
        // otherwise get the vectors from the angles themselves
        else {
          // half angleX and angleY
          Double_t nXa = angleX * 0.5, nYa = angleY * 0.5;
          Double_t pXa = nXa, pYa = nYa;

          if ( build_pipe_tori ) {
            if ( nPipe == 6 ) {
              nXa *= 2;
              nYa *= 2;
              pXa *= 2;
              pYa *= 2;
            }
            if ( nPipe == 2 ) {
              nXa *= 2;
              nYa *= 2;
            }
            if ( nPipe == 10 ) {
              pXa *= 2;
              pYa *= 2;
            }
          }

          fV.SetXYZ( std::sin( nYa ), -std::sin( nXa ), std::cos( nXa ) * std::cos( nYa ) * direction );
          bV.SetXYZ( std::sin( pYa ), -std::sin( pXa ), -std::cos( pXa ) * std::cos( pYa ) * direction );
        }

        // Sometimes we need to invert the cut direction, do that here
        swapCuts( nPipe, fV, bV, direction );

        // creating the tubes
        //        dd4hep::Tube tubeL( rMinL, rMaxL, tubeLenL*0.5 );
        //        dd4hep::Tube tubeR( rMinR, rMaxR, tubeLenR*0.5 );
        dd4hep::CutTube tubeL( rMinL, rMaxL, tubeLenL * 0.5 - TV_eps, 0., 2 * M_PI, bV.X(), bV.Y(), bV.Z(), fV.X(),
                               fV.Y(), fV.Z() );
        dd4hep::CutTube tubeR( rMinR, rMaxR, tubeLenR * 0.5 - TV_eps, 0., 2 * M_PI, bV.X(), bV.Y(), bV.Z(),
                               nPipe == 3 ? -fV.X() : fV.X(), nPipe == 3 ? -fV.Y() : fV.Y(), fV.Z() );

        // making the volumes
        dd4hep::Volume pipeLvol( Form( "lvDeliveryPipe%sS%uL", matName.c_str(), nPipe ), tubeL, material );
        dd4hep::Volume pipeRvol( Form( "lvDeliveryPipe%sS%uR", matName.c_str(), nPipe ), tubeR, material );

        dx += pipeX * 0.5;
        dy += pipeY * 0.5;
        dz += pipeZ * 0.5 * direction;

        dd4hep::Position    posL( -dx, dy, dz );
        dd4hep::Position    posR( -dx, -dy, dz );
        dd4hep::RotationZYX rotL( 0., -angleY, -angleX );
        dd4hep::RotationZYX rotR( direction == -1 or nPipe == 9 ? M_PI : 0, -angleY, angleX );

        dx += pipeX * 0.5;
        dy += pipeY * 0.5;
        dz += pipeZ * 0.5 * direction;

        if ( i == 0 ) {
          parentVolsL[nPipe] = pipeLvol;
          parentVolsR[nPipe] = pipeRvol;
          pipeL.placeVolume( parentVolsL[nPipe], dd4hep::Transform3D( rotL, posL ) );
          pipeR.placeVolume( parentVolsR[nPipe], dd4hep::Transform3D( rotR, posR ) );
        } else {
          parentVolsL[nPipe].placeVolume( pipeLvol );
          parentVolsR[nPipe].placeVolume( pipeRvol );
        }

        prevAngleX = angleX;
        prevAngleY = angleY;
      }
    }

    return std::make_pair( pipeL, pipeR );
  }

  // A helper function as part of generate_cooling_pipes that works out if the cuts need to have their direction swapped
  // (as they sometimes do)
  void TVBuild::swapCuts( const unsigned nPipe, TVector3& fV, TVector3& bV, int direction ) {
    // If we're going backwards, the FE and BE swap
    if ( direction < 0 ) {
      TVector3 temp = fV;
      fV            = bV;
      bV            = temp;
    }

    // Sometimes we also swap direction (dictated by the XML)
    bool swapFE = false, swapBE = false;
    try {
      swapFE = dd4hep::_toInt( Form( "TV:DeliveryPipeSwapFE%u", nPipe ) );
    } catch ( std::runtime_error& e ) {}
    try {
      swapBE = dd4hep::_toInt( Form( "TV:DeliveryPipeSwapBE%u", nPipe ) );
    } catch ( std::runtime_error& e ) {}

    if ( swapFE ) {
      fV.SetY( fV.Y() * -1 );
      fV.SetX( fV.X() * -1 );
    }
    if ( swapBE ) {
      bV.SetY( bV.Y() * -1 );
      bV.SetX( bV.X() * -1 );
    }
    return;
  }

  void TVBuild::build_detector() {
    dd4hep::PlacedVolume pv;
    double               Right2LeftDist = dd4hep::_toDouble( "TV:Right2LeftDist" );
    dd4hep::Assembly     lvLeft, lvRight, lvMotionLeft, lvMotionRight;
    dd4hep::DetElement   deOpt, deLeft, deRight, deMotionLeft, deMotionRight;
    if ( build_sides ) {
      dd4hep::RotationZYX rot_left( M_PI, 0, 0 );
      dd4hep::Volume      lvModule = volume( "lvModuleWithSupport" );
      lvLeft                       = dd4hep::Assembly( "lvTVLeft" );
      lvRight                      = dd4hep::Assembly( "lvTVRight" );
      lvMotionLeft                 = dd4hep::Assembly( "lvMotionTVLeft" );
      lvMotionRight                = dd4hep::Assembly( "lvMotionTVRight" );
      deMotionLeft                 = dd4hep::DetElement( detector, "MotionTVLeft", 0 );
      deMotionRight                = dd4hep::DetElement( detector, "MotionTVRight", 1 );
      deLeft                       = dd4hep::DetElement( deMotionLeft, "TVLeft", 0 );
      deRight                      = dd4hep::DetElement( deMotionRight, "TVRight", 1 );

      const int NoOfStations = dd4hep::_toInt( "TV:NoOfStations" );

      for ( int i = 0; i < NoOfStations; ++i ) { // One or two loops -- what would be more efficient in tracking?
        double station_z = dd4hep::_toDouble( dd4hep::_toString( i, "TV:Station%02dZ" ) );
        pv               = lvRight.placeVolume( lvModule, dd4hep::Position( 0, 0, station_z - Right2LeftDist / 2.0 ) );
        pv->SetName( dd4hep::_toString( i * 2, "pvModule%02dWithSupport" ).c_str() ); // Optional
        pv.addPhysVolID( "module", i * 2 );
        deOpt = de_module_with_support.clone( dd4hep::_toString( i * 2, "Module%02d" ), i * 2 );
        deOpt.setPlacement( pv );
        deRight.add( deOpt );
        pv = lvLeft.placeVolume(
            lvModule, dd4hep::Transform3D( rot_left, dd4hep::Position( 0, 0, station_z + Right2LeftDist / 2.0 ) ) );
        pv->SetName( dd4hep::_toString( 1 + i * 2, "pvModule%02dWithSupport" ).c_str() ); // Optional
        pv.addPhysVolID( "module", i * 2 + 1 );
        deOpt = de_module_with_support.clone( dd4hep::_toString( i * 2 + 1, "Module%02d" ), i * 2 + 1 );
        deOpt.setPlacement( pv );
        deLeft.add( deOpt );
      }

      if ( build_rf_box ) {
        pv = lvLeft.placeVolume( volume( "lvRFBoxLeft" ) );
        pv->SetName( "pvRFBoxLeft" );
        deOpt = DetElement( deLeft, "RFBoxLeft", deLeft.id() );
        deOpt.setPlacement( pv );
        pv = lvRight.placeVolume( volume( "lvRFBoxRight" ) );
        pv->SetName( "pvRFBoxRight" );
        deOpt = DetElement( deRight, "RFBoxRight", deRight.id() );
        deOpt.setPlacement( pv );
      }
      if ( build_rf_foil ) {
        dd4hep::Volume   vol = volume( "lvRFFoil" );
        dd4hep::Position pos_left( 0, 0, dd4hep::_toDouble( "TV:RFFoilLeftPosZ" ) );
        dd4hep::Position pos_right( 0, 0, dd4hep::_toDouble( "TV:RFFoilRightPosZ" ) );
        pv = lvLeft.placeVolume( vol, pos_left );
        pv->SetName( "pvRFFoilLeft" );
        deOpt = DetElement( deLeft, "RFFoilLeft", deLeft.id() );
        deOpt.setPlacement( pv );
        pv = lvRight.placeVolume( vol, dd4hep::Transform3D( dd4hep::RotationZYX( M_PI, 0, 0 ), pos_right ) );
        pv->SetName( "pvRFFoilRight" );
        deOpt = DetElement( deRight, "RFFoilRight", deRight.id() );
        deOpt.setPlacement( pv );
      }
    }
    if ( build_vacuum_tank && build_det_support ) {
      dd4hep::Volume   vol = volume( "lvVeloDetSup" );
      dd4hep::Position pos_left( dd4hep::_toDouble( "TV:DetSupLeftPosX" ), 0,
                                 dd4hep::_toDouble( "TV:DetSupLeftPosZ" ) );
      dd4hep::Position pos_right( dd4hep::_toDouble( "TV:DetSupRightPosX" ), 0,
                                  dd4hep::_toDouble( "TV:DetSupRightPosZ" ) );
      pv = lvLeft.placeVolume( vol, dd4hep::Transform3D( dd4hep::RotationZYX( M_PI, 0, 0 ), pos_left ) );
      pv->SetName( "pvDetSupLeft" );
      deOpt = DetElement( deLeft, "DetSupportLeft", deLeft.id() );
      deOpt.setPlacement( pv );
      pv = lvRight.placeVolume( vol, pos_right );
      pv->SetName( "pvDetSupRight" );
      deOpt = DetElement( deRight, "DetSupportRight", deRight.id() );
      deOpt.setPlacement( pv );
    }

    // Now we build the main velo shape
    double             zpos            = dd4hep::_toDouble( "TV:VeloZ" );
    double             radius          = dd4hep::_toDouble( "TV:VeloRad" );
    double             DSEndStartZ     = dd4hep::_toDouble( "TV:VeloDSEndStartZ" );
    double             DSEndDeltaZ     = dd4hep::_toDouble( "TV:VeloDSEndDeltaZ" );
    double             EWFlangeZExcess = dd4hep::_toDouble( "TV:VeloEWFlangeZExcess" );
    dd4hep::UnionSolid TV_shape( dd4hep::Tube( 0, radius, zpos / 2.0 ), dd4hep::Tube( 0, radius, DSEndDeltaZ / 2.0 ),
                                 dd4hep::Position( 0, 0, DSEndStartZ + DSEndDeltaZ / 2.0 ) );
    TV_shape = dd4hep::UnionSolid(
        TV_shape,
        dd4hep::Tube( "TV:vTankDownEWFlangeIR-2*mm", "TV:vTankDownEWFlangeOR+2*mm",
                      EWFlangeZExcess / 2.0 + 1. * dd4hep::mm ),
        dd4hep::Position( 0, 0, DSEndStartZ + DSEndDeltaZ + ( EWFlangeZExcess + 2.0 * dd4hep::mm ) / 2.0 ) );

    dd4hep::Volume lvTV( "lvTV", TV_shape, description.vacuum() );
    lvTV.setAttributes( description, x_det.regionStr(), x_det.limitsStr(), x_det.visStr() );
    if ( build_sides ) {
      pv = lvMotionLeft.placeVolume( lvLeft );
      pv->SetName( "pvTVLeft" );
      pv.addPhysVolID( "side", 0 );
      deLeft.setPlacement( pv );
      pv = lvMotionRight.placeVolume( lvRight );
      pv->SetName( "pvTVRight" );
      pv.addPhysVolID( "side", 1 );
      deRight.setPlacement( pv );

      // The Motion DetElements are there to be able to easily combine the transformation of the TV position
      // and the TV alignment
      pv = lvTV.placeVolume( lvMotionLeft );
      pv->SetName( "pvTVMotionLeft" );
      pv.addPhysVolID( "motionside", 0 );
      deMotionLeft.setPlacement( pv );
      pv = lvTV.placeVolume( lvMotionRight );
      pv->SetName( "pvTVMotionRight" );
      pv.addPhysVolID( "motionside", 1 );
      deMotionRight.setPlacement( pv );
    }
    if ( build_vacuum_tank ) {
      DetElement       de( detector, "VacuumTank", id );
      dd4hep::Position pos( 0, 0, dd4hep::_toDouble( "TV:vTankPosZ" ) );
      pv = lvTV.placeVolume( volume( "lvVacTank" ), pos );
      pv->SetName( "pvVacTank" );
      de.setPlacement( pv );
    }
    if ( build_wake_cones ) {
      // We have them as volumes, but we dop not enter them into the hierarchy.
      dd4hep::Position pos_up( 0, 0, dd4hep::_toDouble( "TV:UpStrWakeFieldConePosZ" ) );
      pv = lvTV.placeVolume( volume( "lvUpstreamWakeFieldCone" ), pos_up );
      pv->SetName( "pvUpstreamWakeFieldCone" );
      dd4hep::Position pos_down( 0, 0, dd4hep::_toDouble( "TV:DownStrWakeFieldConePosZ" ) );
      pv = lvTV.placeVolume( volume( "lvDownstreamWakeFieldCone" ), pos_down );
      pv->SetName( "pvDownstreamWakeFieldCone" );
    }
    registerVolume( lvRight.name(), lvRight );
    registerVolume( lvLeft.name(), lvLeft );
    registerVolume( lvMotionRight.name(), lvMotionRight );
    registerVolume( lvMotionLeft.name(), lvMotionLeft );
    registerVolume( lvTV.name(), lvTV );
  }

  void TVBuild::build_groups() {
    sensitive.setType( "tracker" );
    load( x_det, "include" );
    buildVolumes( x_det );
    placeDaughters( detector, dd4hep::Volume(), x_det );
    if ( build_sides ) { build_module(); }
    build_detector();
    dd4hep::Volume       vol = volume( select_volume );
    dd4hep::PlacedVolume pv  = placeDetector( vol );
    pv.addPhysVolID( "system", id );
    dd4hep::detail::destroyHandle( de_module_with_support );
  }

} // namespace

static dd4hep::Ref_t create_element( dd4hep::Detector& description, xml_h e, dd4hep::SensitiveDetector sens_det ) {
  TVBuild builder( description, e, sens_det );
  builder.build_groups();
  return builder.detector;
}
DECLARE_DETELEMENT( LHCb_TV_v1_0, create_element )
