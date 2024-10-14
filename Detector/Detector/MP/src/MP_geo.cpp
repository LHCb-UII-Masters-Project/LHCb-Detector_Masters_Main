//==========================================================================
//  AIDA Detector description implementation
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
// Author     : M.Frank, Tai-Hua Lin
//
//==========================================================================
//
// Specialized generic detector constructor
//
//==========================================================================
#include "Core/UpgradeTags.h"
#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/Printout.h"
#include "DD4hep/detail/DetectorInterna.h"
#include "TClass.h"
#include "XML/Utilities.h"

using namespace std;
using namespace dd4hep;
using namespace dd4hep::detail;

#define HELPER_CLASS_DEFAULTS( x )                                                                                     \
  x()           = default;                                                                                             \
  x( x&& )      = default;                                                                                             \
  x( const x& ) = default;                                                                                             \
  x& operator=( const x& ) = default;                                                                                  \
  ~x()                     = default

namespace {

  /// Helper class to build the MP detector of LHCb
  struct MPBuild : public xml::tools::VolumeBuilder {
    // Debug flags: To be enables/disbaled by the <debug/> section in the detector.xml file.
    bool        m_build_passive  = true;
    bool        m_build_frames   = true;
    bool        m_build_halves   = true;
    bool        m_build_layers   = true;
    bool        m_build_stations = true;
    bool        m_local_debug    = false;
    bool        m_build_MT       = false; // MP:includeMT
    bool        m_halfLayer      = false; // MP:halfLayers
    double      m_geo_option     = 0;     // MP:geometryOptions
    std::string m_attach_volume;

    // General constants used by several member functions to build the MP geometry
    double mod_size_x       = 0.0; //  MP:ModuleSizeX
    double mod_size_y       = 0.0; //  MP:ModuleSizeY
    double short_mod_size_y = 0.0; //  MP:ShortModuleSizeY
    double mod_size_z       = 0.0; //  MP:ModuleSizeZ

    double sheet_size_z = 0.0; //  MP:ModuleSizeZ

    double mat_pitch_x       = 0.0; //  MP:MatPitchX
    double mat_pitch_y       = 0.0; //  MP:MatPitchY
    double short_mat_pitch_y = 0.0; //  MP:ShortMatPitchY

    double mat_size_x       = 0.0; //  MP:MatSizeX
    double mat_size_y       = 0.0; //  MP:MatSizeY
    double short_mat_size_y = 0.0; //  MP:ShortMatSizeY
    double mat_size_z       = 0.0; //  MP:MatSizeZ

    double station_size_x = 0.0; //  MP:StationSizeX
    double station_size_y = 0.0; //  MP:StationSizeY
    double station_size_z = 0.0; //  MP:StationSizeZ

    double eps               = 0.0; //  MP:eps
    double mod_pitch_x       = 0.0; //  MP:ModulePitchX
    double layer_pitch_z     = 0.0; //  MP:LayerPitchZ
    double cframe_pitch_z    = 0.0; //  MP:CFramePitchZ
    double cframe_shift_y    = 0.0; //  MP:CFrameShiftY
    double hole_size_x       = 0.0; //  MP:HoleSizeX
    double hole_leftx_size_y = 0.0; //  MP:HoleLeftXSizeY"
    double beam_hole_radius  = 0.0; //  MP:BeamHoleRadius

    double mat_gap_y = 0.0; // MP:MatGapY used to prevent beam hole overlap 5*mm

    struct Module {
      double      sign = 1.0, holeSizeX = 0;
      std::string mat, matShort;
      Module( double sgn, double hs, const std::string& m, const std::string& ms )
          : sign( sgn ), holeSizeX( hs ), mat( m ), matShort( ms ) {}
      HELPER_CLASS_DEFAULTS( Module );
    };
    map<string, Module>     m_modules;
    map<string, DetElement> m_detElements;

    /// Utility functions to register detector elements for cloning. Will all be deleted!
    void       registerDetElement( const std::string& nam, DetElement de );
    DetElement detElement( const std::string& nam ) const;

    /// Initializing constructor
    MPBuild( Detector& description, xml_elt_t e, SensitiveDetector sens );
    MPBuild()                 = delete;
    MPBuild( MPBuild&& )      = delete;
    MPBuild( const MPBuild& ) = delete;
    MPBuild& operator=( const MPBuild& ) = delete;

    /// Default destructor
    virtual ~MPBuild();
    void build_mats();
    void build_modules();
    void build_halves();
    void build_layers();
    void build_stations();
    void build_detector();
  };

  void       MPBuild::registerDetElement( const std::string& nam, DetElement de ) { m_detElements[nam] = de; }
  DetElement MPBuild::detElement( const std::string& nam ) const {
    auto i = m_detElements.find( nam );
    if ( i == m_detElements.end() ) {
      printout( ERROR, "MP-geo", "Attempt to access non-existing detector element %s", nam.c_str() );
    }
    return ( *i ).second;
  }

  /// Initializing constructor
  MPBuild::MPBuild( Detector& dsc, xml_elt_t e, SensitiveDetector sens ) : xml::tools::VolumeBuilder( dsc, e, sens ) {
    // Process debug flags
    xml_comp_t x_dbg = x_det.child( _U( debug ), false );
    if ( x_dbg ) {
      for ( xml_coll_t i( x_dbg, _U( item ) ); i; ++i ) {
        xml_comp_t c( i );
        string     n = c.nameStr();
        if ( n == "build_passive" ) m_build_passive = c.attr<bool>( _U( value ) );
        if ( n == "build_frames" ) m_build_frames = c.attr<bool>( _U( value ) );
        if ( n == "build_halves" ) m_build_halves = c.attr<bool>( _U( value ) );
        if ( n == "build_layers" ) m_build_layers = c.attr<bool>( _U( value ) );
        if ( n == "build_stations" ) m_build_stations = c.attr<bool>( _U( value ) );
        if ( n == "attach_volume" ) m_attach_volume = c.attr<string>( _U( value ) );
        if ( n == "local_debug" ) m_local_debug = c.attr<bool>( _U( value ) );
        if ( n == "debug" ) debug = c.attr<bool>( _U( value ) );
      }
    }
    m_build_MT   = _toDouble( "MP:includeMT" );
    m_geo_option = _toDouble( "MP:geometryOptions" );
    m_halfLayer  = _toDouble( "MP:halfLayers" );
    eps          = _toDouble( "MP:eps" );
    mod_pitch_x  = _toDouble( "MP:ModulePitchX" );
    mod_size_x   = _toDouble( "MP:ModuleSizeX" );
    if ( m_geo_option == 1 ) {
      mod_size_y  = _toDouble( "MP:ShortModuleSizeY" );
      mat_pitch_y = _toDouble( "MP:ShortMatPitchY" );
      mat_size_y  = _toDouble( "MP:ShortMatSizeY" );
    } else {
      mod_size_y  = _toDouble( "MP:ModuleSizeY" );
      mat_pitch_y = _toDouble( "MP:MatPitchY" );
      mat_size_y  = _toDouble( "MP:MatSizeY" );
    }
    short_mod_size_y = _toDouble( "MP:ShortModuleSizeY" );
    mod_size_z       = _toDouble( "MP:ModuleSizeZ" );

    sheet_size_z      = _toDouble( "MP:SheetSizeZ" );
    mat_pitch_x       = _toDouble( "MP:MatPitchX" );
    short_mat_pitch_y = _toDouble( "MP:ShortMatPitchY" );

    mat_size_x       = _toDouble( "MP:MatSizeX" );
    short_mat_size_y = _toDouble( "MP:ShortMatSizeY" );
    mat_size_z       = _toDouble( "MP:MatSizeZ" );

    station_size_x = _toDouble( "MP:StationSizeX" );
    station_size_y = _toDouble( "MP:StationSizeY" );
    station_size_z = _toDouble( "MP:StationSizeZ" );

    layer_pitch_z     = _toDouble( "MP:LayerPitchZ" );
    cframe_pitch_z    = _toDouble( "MP:CFramePitchZ" );
    cframe_shift_y    = _toDouble( "MP:CFrameShiftY" );
    hole_size_x       = _toDouble( "MP:HoleSizeX" );
    hole_leftx_size_y = _toDouble( "MP:HoleLeftXSizeY" );
    beam_hole_radius  = _toDouble( "Regions:TBeamHoleRadius" );

    mat_gap_y = _toDouble( "MP:MatGapY" );

    // m_modules["LeftX"]  = Module( 1., hole_size_x, "lvSiMat", "lvSiShortMatLeftX" );
    // if (m_geo_option == 1){
    //  m_modules["LeftX"]  = Module( 1., hole_size_x, "lvSiMatShort", "lvSiShortMatLeftX" );
    //  m_modules["RightX"] = Module( -1., hole_size_x, "lvSiMatShort", "lvSiShortMatRightX" );
    //} else {
    m_modules["LeftX"]  = Module( 1., hole_size_x, "lvSiMat", "lvSiShortMatLeftX" );
    m_modules["RightX"] = Module( -1., hole_size_x, "lvSiMat", "lvSiShortMatRightX" );
    //}

    std::cout << "mat_size_y  ; short_mat_size_y " << mat_size_y << "  ; " << short_mat_size_y << std::endl;
    std::cout << "mod_size_y  ; short_mod_size_y " << mod_size_y << "  ; " << short_mod_size_y << std::endl;
    std::cout << "mat_pitch_y  ; short_mat_pitch_y " << mat_pitch_y << "  ; " << short_mat_pitch_y << std::endl;
  }

  MPBuild::~MPBuild() {
    for ( auto& d : m_detElements ) destroyHandle( d.second );
  }

  void MPBuild::build_mats() {
    Position pos;

    Box      mod_box( mod_size_x / 2.0, mod_size_y / 2.0, mod_size_z / 2.0 );
    Assembly mod_vol( "lvMPModuleColI" );

    mod_vol.setVisAttributes( description, "MP:Module" );
    registerVolume( mod_vol.name(), mod_vol );

    mod_vol = Assembly( "lvMPModuleColII" );
    mod_vol.setVisAttributes( description, "MP:Module" );
    registerVolume( mod_vol.name(), mod_vol );

    mod_vol = Assembly( "lvMPModuleColIII" );
    mod_vol.setVisAttributes( description, "MP:Module" );
    registerVolume( mod_vol.name(), mod_vol );

    // Tube mod_tube( 0, hole_leftx_size_y, ( mod_size_z + eps ) / 2.0 );

    Material mighty_material = description.material( "MP:SiliconTracker" );

    Material sheet_material = description.material( "MP:PassiveMaterial" );
    if ( m_build_passive ) {
      Box    sheet_box_colI( mod_size_x / 2.0, mod_size_y * 2, sheet_size_z / 2.0 );
      Volume sheet_volI( "lvMPSheetColI", sheet_box_colI, sheet_material );
      sheet_volI.setVisAttributes( description, "MP:Support" );
      registerVolume( sheet_volI.name(), sheet_volI );

      Box    sheet_box_colII( mod_size_x / 2.0, mod_size_y * 1.5, sheet_size_z / 2.0 );
      Volume sheet_volII( "lvMPSheetColII", sheet_box_colII, sheet_material );
      sheet_volII.setVisAttributes( description, "MP:Support" );
      registerVolume( sheet_volII.name(), sheet_volII );

      Box    sheet_box_colIII( mod_size_x / 2.0, mod_size_y, sheet_size_z / 2.0 );
      Volume sheet_volIII( "lvMPSheetColIII", sheet_box_colIII, sheet_material );
      sheet_volIII.setVisAttributes( description, "MP:Support" );
      registerVolume( sheet_volIII.name(), sheet_volIII );
    }

    for ( const auto& m : m_modules ) {
      const auto& p = m.second;

      mod_vol = Assembly( "lvMPModuleHole" + m.first );
      // mod_vol   = Volume( "lvMPModuleHole" + m.first, mod_vol.solid(), description.air());
      // mod_solid = mod_vol.solid();

      mod_vol.setVisAttributes( description, "MP:Module" );
      registerVolume( mod_vol.name(), mod_vol );
      // printout( m_local_debug ? ALWAYS : DEBUG, "MP-geo", "+++ Building volume %-24s solid: %-24s material:%s",
      //          mod_vol.name(), mod_solid->IsA()->GetName(), mod_vol.material().name() );
      std::cout << "material " << mod_vol.material().name() << std::endl;

      if ( m_build_passive ) {
        double n_sheet = 1;
        if ( m_geo_option == 0 ) { n_sheet = 2; }
        Box sheet_box( mod_size_x / 2.0, ( short_mod_size_y / 2.0 ) + ( mod_size_y * n_sheet ), sheet_size_z / 2.0 );
        Box hole_box( hole_size_x, short_mod_size_y / 2.0, ( sheet_size_z + eps ) / 2.0 );
        // pos = Position(mod_size_x/2.0,0,sheet_size_z/2.0+eps);
        pos = Position( -1.0 * p.sign * mod_size_x / 2.0, 0, 0 );
        SubtractionSolid sheet_solid( sheet_box, hole_box, pos );
        Volume           sheet_vol( "lvMPSheet" + m.first, sheet_solid, sheet_material );
        sheet_vol.setVisAttributes( description, "MP:Support" );
        registerVolume( sheet_vol.name(), sheet_vol );
      }

      // TODO what does this do in the original code? Shortens the Modules but how is it used?
      if ( p.mat != "lvSiMat" ) {
        // Box mat0_solid((mat_size_x-hole_size_x)/2.0, mat_size_y/2.0, mat_size_z/2.0);
        Box    mat0_solid( ( mod_size_x - hole_size_x ) / 2.0, short_mat_size_y / 2.0, mat_size_z / 2.0 );
        Volume mat0_vol( p.mat, mat0_solid, mighty_material );
        // mat0_vol.setVisAttributes(description, "MP:MT");
        mat0_vol.setVisAttributes( description, "MP:Silicon" );
        mat0_vol.setSensitiveDetector( sensitive );
        registerVolume( mat0_vol.name(), mat0_vol );
        printout( m_local_debug ? ALWAYS : DEBUG, "MP-geo", "+++ Building volume %-24s solid: %-24s material:%s",
                  mat0_vol.name(), mat0_solid->IsA()->GetName(), mighty_material.name() );
      }
      if ( p.matShort != "lvSiMat" ) {
        // Box mat3_solid((mat_size_x-hole_size_x)/2.0, mat_size_y/2.0, mat_size_z/2.0);
        // std::cout<<"mod_size_x/2.0 "<<mod_size_x/2.0<<std::endl;
        // std::cout<<"mat_size_x/2.0 "<<mat_size_x/2.0<<std::endl;
        Box    mat3_solid( ( mod_size_x - hole_size_x ) / 2.0, short_mat_size_y / 2.0, mat_size_z / 2.0 );
        Volume mat3_vol( p.matShort, mat3_solid, mighty_material );
        mat3_vol.setVisAttributes( description, "MP:Silicon" );
        mat3_vol.setSensitiveDetector( sensitive );
        registerVolume( mat3_vol.name(), mat3_vol );
        printout( m_local_debug ? ALWAYS : DEBUG, "MP-geo", "+++ Building volume %-24s solid: %-24s material:%s",
                  mat3_vol.name(), mat3_solid->IsA()->GetName(), mighty_material.name() );
      }
    }
  }

  void MPBuild::build_modules() {
    Position     pos;
    PlacedVolume pv;
    RotationZYX  rot_Y( 0, M_PI, 0 );
    Volume       mod_vol = volume( "lvMPModuleColI" );
    DetElement   de_mod( "module", id );
    DetElement   de_mat;

    printout( m_local_debug ? ALWAYS : DEBUG, "MP-geo", "+++ Building module %s", mod_vol.name() );
    registerDetElement( mod_vol.name(), de_mod );

    if ( m_build_MT != 0 ) {
      // MT - first column
      // pos.SetX(pos.X() + mod_pitch_x);
      pos = Position( 0, -2 * ( mat_size_y + mat_gap_y ) + mat_pitch_y / 2, 0 );
      pv  = mod_vol.placeVolume( volume( "lvSiMat" ), pos );
      pv.addPhysVolID( "mat", 6 );
      de_mat = DetElement( de_mod, "Mat6", 6 );
      de_mat.setPlacement( pv );

      pos.SetY( pos.Y() + mat_pitch_y );
      pv = mod_vol.placeVolume( volume( "lvSiMat" ), pos );
      pv.addPhysVolID( "mat", 7 );
      de_mat = DetElement( de_mod, "Mat7", 7 );
      de_mat.setPlacement( pv );

      pos.SetY( pos.Y() + mat_pitch_y );
      pv = mod_vol.placeVolume( volume( "lvSiMat" ), pos );
      pv.addPhysVolID( "mat", 8 );
      de_mat = DetElement( de_mod, "Mat8", 8 );
      de_mat.setPlacement( pv );

      pos.SetY( pos.Y() + mat_pitch_y );
      pv = mod_vol.placeVolume( volume( "lvSiMat" ), pos );
      pv.addPhysVolID( "mat", 9 );
      de_mat = DetElement( de_mod, "Mat9", 9 );
      de_mat.setPlacement( pv );
      std::cout << "mat9 materialI " << mod_vol.material().name() << std::endl;

      if ( m_build_passive ) {
        pos = Position( 0, -0.5 * ( mat_size_y + mat_gap_y ) + mat_pitch_y / 2, mat_size_z + ( sheet_size_z / 2 ) );
        if ( m_geo_option == 0 ) {
          pv = mod_vol.placeVolume( volume( "lvMPSheetColI" ), pos );
        } else {
          pv = mod_vol.placeVolume( volume( "lvMPSheetColII" ), pos );
        }
      }

      // MT - second column
      de_mod  = DetElement( "module", id );
      mod_vol = volume( "lvMPModuleColII" );
      registerDetElement( mod_vol.name(), de_mod );

      pos = Position( 0, -1 * ( mat_size_y + mat_gap_y ), 0 );
      pv  = mod_vol.placeVolume( volume( "lvSiMat" ), pos );
      pv.addPhysVolID( "mat", 10 );
      de_mat = DetElement( de_mod, "Mat10", 10 );
      de_mat.setPlacement( pv );

      pos.SetY( pos.Y() + mat_pitch_y );
      pv = mod_vol.placeVolume( volume( "lvSiMat" ), pos );
      pv.addPhysVolID( "mat", 11 );
      de_mat = DetElement( de_mod, "Mat11", 11 );
      de_mat.setPlacement( pv );

      pos.SetY( pos.Y() + mat_pitch_y );
      pv = mod_vol.placeVolume( volume( "lvSiMat" ), pos );
      pv.addPhysVolID( "mat", 12 );
      de_mat = DetElement( de_mod, "Mat12", 12 );
      de_mat.setPlacement( pv );
      std::cout << "mat12 materialII " << mod_vol.material().name() << std::endl;

      if ( m_build_passive ) {
        pos = Position( 0, -0.5 * ( mat_size_y + mat_gap_y ) + mat_pitch_y / 2, mat_size_z + ( sheet_size_z / 2 ) );
        pv  = mod_vol.placeVolume( volume( "lvMPSheetColII" ), pos );
      }

      // MT - third column
      de_mod  = DetElement( "module", id );
      mod_vol = volume( "lvMPModuleColIII" );
      registerDetElement( mod_vol.name(), de_mod );

      pos = Position( 0, -0.5 * ( mat_size_y + mat_gap_y ), 0 );
      pv  = mod_vol.placeVolume( volume( "lvSiMat" ), pos );
      pv.addPhysVolID( "mat", 13 );
      de_mat = DetElement( de_mod, "Mat13", 13 );
      de_mat.setPlacement( pv );

      pos.SetY( pos.Y() + mat_pitch_y );
      pv = mod_vol.placeVolume( volume( "lvSiMat" ), pos );
      pv.addPhysVolID( "mat", 14 );
      de_mat = DetElement( de_mod, "Mat14", 14 );
      de_mat.setPlacement( pv );
      std::cout << "mat 14 materialIII " << mod_vol.material().name() << std::endl;

      if ( m_build_passive ) {
        pos = Position( 0, -0.5 * ( mat_size_y + mat_gap_y ) + mat_pitch_y / 2, mat_size_z + ( sheet_size_z / 2 ) );
        pv  = mod_vol.placeVolume( volume( "lvMPSheetColIII" ), pos );
      }
    }

    // Inner modules that require shortening are built in this loop. IE build IT Module in here
    for ( const auto& m : m_modules ) {
      const auto& p = m.second;

      de_mod  = DetElement( "module", id );
      mod_vol = volume( "lvMPModuleHole" + m.first );
      registerDetElement( mod_vol.name(), de_mod );
      printout( m_local_debug ? ALWAYS : DEBUG, "MP-geo", "+++ Building module %s", mod_vol.name() );

      // middle shorter mat IT
      // std::cout<<"shorter mat position "<< p.sign*p.holeSizeX/2<<std::endl;
      pos = Position( p.sign * p.holeSizeX / 2, 0, 0 );
      pv  = mod_vol.placeVolume( volume( p.matShort ), pos );
      pv.addPhysVolID( "mat", 1 );
      de_mat = DetElement( de_mod, "Mat1", 1 );
      de_mat.setPlacement( pv );
      std::cout << "materialmIT " << mod_vol.material().name() << std::endl;

      // Top mat IT
      // changing x moves both L&R mats, ie. centre around which half is defined.
      pos = Position( 0, 1 * ( ( 0.5 * short_mat_size_y ) + ( 0.5 * mat_size_y ) + mat_gap_y ), 0 );
      pv  = mod_vol.placeVolume( volume( "lvSiMat" ), pos );
      pv.addPhysVolID( "mat", 2 );
      de_mat = DetElement( de_mod, "Mat2", 2 );
      de_mat.setPlacement( pv );

      // bottom mat IT
      pos = Position( 0, -1 * ( ( 0.5 * short_mat_size_y ) + ( 0.5 * mat_size_y ) + mat_gap_y ), 0 );
      pv  = mod_vol.placeVolume( volume( "lvSiMat" ), pos );
      pv.addPhysVolID( "mat", 3 );
      de_mat = DetElement( de_mod, "Mat3", 3 );
      de_mat.setPlacement( pv );
      std::cout << "materialbIT " << mod_vol.material().name() << std::endl;

      if ( m_geo_option == 0 ) {
        // if ( m_build_MT != 0 ) {
        // Top mat MT
        pos = Position( 0, ( ( 0.5 * short_mat_size_y ) + ( 0.5 * mat_size_y ) + mat_size_y + ( 2 * mat_gap_y ) ), 0 );
        pv  = mod_vol.placeVolume( volume( "lvSiMat" ), pos );
        pv.addPhysVolID( "mat", 4 );
        de_mat = DetElement( de_mod, "Mat4", 4 );
        de_mat.setPlacement( pv );
        std::cout << "materialtMT " << mod_vol.material().name() << std::endl;

        // bottom mat MT
        pos = Position( 0, -1 * ( ( 0.5 * short_mat_size_y ) + ( 0.5 * mat_size_y ) + mat_size_y + ( 2 * mat_gap_y ) ),
                        0 );
        pv  = mod_vol.placeVolume( volume( "lvSiMat" ), pos );
        pv.addPhysVolID( "mat", 5 );
        de_mat = DetElement( de_mod, "Mat5", 5 );
        de_mat.setPlacement( pv );
      }
      if ( m_build_passive ) {
        pos = Position( 0, 0, mat_size_z + ( sheet_size_z / 2 ) );
        pv  = mod_vol.placeVolume( volume( "lvMPSheet" + m.first ), pos );
      }
    }
  }

  void MPBuild::build_halves() {
    struct Half {
      double      sign = 0;
      std::string special;
      Half( double sgn, const std::string& s ) : sign( sgn ), special( s ) {}
      HELPER_CLASS_DEFAULTS( Half );
    };
    Position          pos;
    PlacedVolume      pv;
    Volume            vol_colI   = volume( "lvMPModuleColI" );
    DetElement        de_colI    = detElement( "lvMPModuleColI" );
    Volume            vol_colII  = volume( "lvMPModuleColII" );
    DetElement        de_colII   = detElement( "lvMPModuleColII" );
    Volume            vol_colIII = volume( "lvMPModuleColIII" );
    DetElement        de_colIII  = detElement( "lvMPModuleColIII" );
    RotationZYX       rot( 0, 0, 0 );
    map<string, Half> halves;
    halves["XNeg"] = Half( -1., "RightX" );
    halves["XPos"] = Half( 1., "LeftX" );

    for ( const auto& q : halves ) {
      const auto& p = q.second;
      Assembly    vol_half( "lvHalf5" + q.first );
      DetElement  de_half( "half", id );
      DetElement  de, de_module;

      registerVolume( vol_half.name(), vol_half );
      registerDetElement( vol_half.name(), de_half );
      vol_half.setVisAttributes( description.invisible() );

      printout( m_local_debug ? ALWAYS : DEBUG, "MP-geo", "+++ Building half: %s sign:%c special:%s", vol_half.name(),
                p.sign ? '+' : '-', p.special.c_str() );

      // position and add modules to each half
      pos = Position( p.sign * 0.5 * mod_pitch_x, 0, 0.0 );
      // std::cout << "hole pos.X: " << pos.X() << std::endl;
      // std::cout << "mod_pitch_x: " << mod_pitch_x << std::endl;
      pv = vol_half.placeVolume( volume( "lvMPModuleHole" + p.special ), Transform3D( rot, pos ) );
      de = detElement( pv.volume().name() );
      pv.addPhysVolID( "module", 0 );
      de_module = de.clone( "M0", 0 );
      de_module.setPlacement( pv );
      de_half.add( de_module );

      std::cout << "build geo option : " << m_geo_option << std::endl;
      if ( m_geo_option == 0 ) {
        // if ( m_build_MT != 0 ) {
        // For MT Modules
        // TODO use pos.SetX() to position the modules (I, II, III, IV) relative to each other.
        // Need to find way to add dissimilar modules in this method.
        pos.SetX( pos.X() + p.sign * mod_pitch_x );
        // std::cout << "colI pos.X: " << pos.X() << std::endl;
        // TODO positioning of Right hand MT modules incorrect. Needs Fixing in reflection build half.s
        pv = vol_half.placeVolume( vol_colI, Transform3D( rot, pos ) );
        pv.addPhysVolID( "module", 1 );
        de_module = de_colI.clone( "M1", 1 );
        de_module.setPlacement( pv );
        de_half.add( de_module );
      } else {

        // pos.SetX(pos.X() + p.sign*2*mod_pitch_x);
        pos.SetX( pos.X() + p.sign * mod_pitch_x );
        // std::cout << "colII pos.X: " << pos.X() << std::endl;
        pv = vol_half.placeVolume( vol_colII, Transform3D( rot, pos ) );
        pv.addPhysVolID( "module", 2 );
        de_module = de_colII.clone( "M2", 2 );
        de_module.setPlacement( pv );
        de_half.add( de_module );

        pos.SetX( pos.X() + p.sign * mod_pitch_x );
        // std::cout << "colIII pos.X: " << pos.X() << std::endl;
        pv = vol_half.placeVolume( vol_colIII, Transform3D( rot, pos ) );
        pv.addPhysVolID( "module", 3 );
        de_module = de_colIII.clone( "M3", 3 );
        de_module.setPlacement( pv );
        de_half.add( de_module );

        // pos.SetX(pos.X() + p.sign*mod_pitch_x);
        // pv = vol_half.placeVolume(vol_colI, Transform3D(rot,pos));
        // pv.addPhysVolID("module",3);
        // de_module = de_colI.clone("M3",3);
        // de_module.setPlacement(pv);
        // de_half.add(de_module);
      }
    }
  }

  void MPBuild::build_layers() {
    struct LayerType {
      double      rotY = 0;
      std::string A, B;
      LayerType( double rot, const std::string& a, const std::string& b ) : rotY( rot ), A( a ), B( b ) {}
      HELPER_CLASS_DEFAULTS( LayerType );
    };
    PlacedVolume     pv;
    SubtractionSolid layer_solid;
    {
      // Box  box(station_size_x/2.0, station_size_y/2.0, station_size_z/2.0);
      // Tube tub(0, beam_hole_radius, station_size_z/2.0+eps);
      Box  box( station_size_x / 2.0, station_size_y / 2.0, mod_size_z / 2.0 + eps );
      Tube tub( 0, beam_hole_radius, mod_size_z + eps );
      layer_solid = SubtractionSolid( box, tub );
    }
    map<string, LayerType> layers;

    layers["X1"] = LayerType( M_PI, "lvHalf5XNeg", "lvHalf5XPos" );
    layers["X2"] = LayerType( 0, "lvHalf5XPos", "lvHalf5XNeg" );
    Volume vol;
    for ( const auto& l : layers ) {
      const auto& p = l.second;
      // Assembly    vol_layer( "lvLayer" + l.first );
      Volume     vol_layer( "lvLayer" + l.first, layer_solid, description.air() );
      DetElement de_layer( l.first, id );
      DetElement de_half;
      registerVolume( vol_layer.name(), vol_layer );
      registerDetElement( vol_layer.name(), de_layer );
      vol_layer.setVisAttributes( description.invisible() );
      printout( m_local_debug ? ALWAYS : DEBUG, "MP-geo", "+++ Building layer:   %-24s   A-type: %s   B-type: %s",
                vol_layer.name(), p.A.c_str(), p.B.c_str() );

      // Right
      pv = vol_layer.placeVolume( vol = volume( p.A ), RotationZYX( 0, p.rotY, 0 ) );
      pv.addPhysVolID( "half", 0 );
      de_half = detElement( vol.name() ).clone( "Q0", 0 );
      de_half.setPlacement( pv );
      de_layer.add( de_half );

      // Left
      pv = vol_layer.placeVolume( vol = volume( p.B ), RotationZYX( 0, p.rotY, 0 ) );
      pv.addPhysVolID( "half", 1 );
      de_half = detElement( vol.name() ).clone( "Q1", 1 );
      de_half.setPlacement( pv );
      de_layer.add( de_half );
    }
  }

  void MPBuild::build_stations() {
    Box              box( station_size_x / 2.0, station_size_y / 2.0, station_size_z / 2.0 );
    Tube             tub( 0.0, beam_hole_radius, station_size_z / 2.0 + eps );
    SubtractionSolid solid( box, tub );
    PlacedVolume     pv;
    Position         pos;
    Volume           vol;

    // Assembly vol_station( "lvStation" );
    Volume     vol_station( "lvStation", solid, description.air() );
    DetElement de_station( "station", id );
    registerDetElement( vol_station.name(), de_station );
    registerVolume( vol_station.name(), vol_station );

    printout( m_local_debug ? ALWAYS : DEBUG, "MP-geo", "+++ Building station type: %s", vol_station.name() );

    if ( m_halfLayer == false ) {
      pos = Position( 0, 0, -0.5 * ( cframe_pitch_z + layer_pitch_z ) );
      pv  = vol_station.placeVolume( vol = volume( "lvLayerX1" ), pos );
      pv.addPhysVolID( "layer", 0 );
      DetElement de_layer0 = detElement( vol.name() ).clone( "X1", 0 );
      de_layer0.setPlacement( pv );
      de_station.add( de_layer0 );
      printout( m_local_debug ? ALWAYS : DEBUG, "MP-geo", "Layer 0 z=%.3f -z=%.3f +z=%.3f [cm]", pos.Z(),
                pos.Z() - pv.volume().boundingBox().z(), pos.Z() + pv.volume().boundingBox().z() );

      // 2nd Layer X
      pos = Position( 0, 0, +0.5 * ( cframe_pitch_z + layer_pitch_z ) );
      pv  = vol_station.placeVolume( vol = volume( "lvLayerX2" ), pos );
      pv.addPhysVolID( "layer", 1 );
      DetElement de_layer3 = detElement( vol.name() ).clone( "X2", 1 );
      de_layer3.setPlacement( pv );
      de_station.add( de_layer3 );
      printout( m_local_debug ? ALWAYS : DEBUG, "MP-geo", "Layer 3 z=%.3f -z=%.3f +z=%.3f [cm]", pos.Z(),
                pos.Z() - pv.volume().boundingBox().z(), pos.Z() + pv.volume().boundingBox().z() );
    } else {
      // pos = Position( 0, 0, +0.5 * ( cframe_pitch_z + layer_pitch_z ) );
      pos = Position( 0, 0, 0 );
      pv  = vol_station.placeVolume( vol = volume( "lvLayerX1" ), pos );
      pv.addPhysVolID( "layer", 0 );
      DetElement de_layer0 = detElement( vol.name() ).clone( "X1", 0 );
      de_layer0.setPlacement( pv );
      de_station.add( de_layer0 );
      printout( m_local_debug ? ALWAYS : DEBUG, "MP-geo", "Layer 0 z=%.3f -z=%.3f +z=%.3f [cm]", pos.Z(),
                pos.Z() - pv.volume().boundingBox().z(), pos.Z() + pv.volume().boundingBox().z() );
    }
  }

  void MPBuild::build_detector() {
    PlacedVolume     pv;
    double           tm_size_x  = _toDouble( "Regions:TTMSizeX" );
    double           tm_size_y  = _toDouble( "Regions:TTMSizeY" );
    double           tm_size_z  = _toDouble( "Regions:TTMSizeZ" );
    double           beam_angle = _toDouble( "Regions:TBeamAngle" );
    Box              ft_box1( tm_size_x / 2.0, tm_size_y / 2.0, tm_size_z / 2.0 );
    Box              ft_box2( ( tm_size_x + eps ) / 2.0, ( tm_size_y + eps ) / 2.0, "10*mm" );
    Tube             ft_tube( 0, beam_hole_radius, ( tm_size_z + eps ) / 2.0, 2.0 * M_PI );
    SubtractionSolid ft_solid( ft_box1, ft_tube );
    RotationZYX      rot( 0, 0, beam_angle );
    Position         pos( 0, -beam_hole_radius, ( tm_size_z + 20.0 * mm ) / 2.0 );
    // solution? to mighty overlap?
    Assembly mp_vol( "lvMP" );
    // Volume           mp_vol("lvMP", SubtractionSolid(ft_solid,ft_box2,Transform3D(rot,pos)), description.air());
    Volume     vol;
    DetElement de;

    mp_vol.setVisAttributes( description.invisible() ); //, xml_det_t( x_det ).visStr() );
    sensitive.setType( "tracker" );
    buildVolumes( x_det );
    placeDaughters( detector, mp_vol, x_det );
    build_mats();
    build_modules();
    if ( m_build_halves ) {
      build_halves();
      if ( m_build_layers ) {
        build_layers();
        if ( m_build_stations ) { build_stations(); }
      }
    }
    if ( m_attach_volume.empty() && m_build_stations ) {
      Volume vol_station = volume( "lvStation" );
      // Assembly   vol_station = Assembly( "lvStation" );
      DetElement de_station;

      pos = Position( 0, 0, _toDouble( "MP:L1ZPos" ) );
      rot = RotationZYX( 0, 0, beam_angle );
      pv  = mp_vol.placeVolume( vol_station, Transform3D( rot, pos ) );
      pv.addPhysVolID( "station", 0 );
      de_station = detElement( vol_station.name() ).clone( "T1", 0 );
      de_station.setPlacement( pv );
      detector.add( de_station );
      printout( m_local_debug ? ALWAYS : DEBUG, "MP-geo", "+++ Position station T1 of type %s", vol_station.name() );

      pos = Position( 0, 0, _toDouble( "MP:L2ZPos" ) );
      pv  = mp_vol.placeVolume( vol_station, Transform3D( rot, pos ) );
      pv.addPhysVolID( "station", 1 );
      de_station = detElement( vol_station.name() ).clone( "T2", 1 );
      de_station.setPlacement( pv );
      detector.add( de_station );
      printout( m_local_debug ? ALWAYS : DEBUG, "MP-geo", "+++ Position station T2 of type %s", vol_station.name() );

      pos = Position( 0, 0, _toDouble( "MP:L3ZPos" ) );
      pv  = mp_vol.placeVolume( vol_station, Transform3D( rot, pos ) );
      pv.addPhysVolID( "station", 2 );
      de_station = detElement( vol_station.name() ).clone( "T3", 2 );
      de_station.setPlacement( pv );
      detector.add( de_station );
      printout( m_local_debug ? ALWAYS : DEBUG, "MP-geo", "+++ Position station T3 of type %s", vol_station.name() );

      pos = Position( 0, 0, _toDouble( "MP:L4ZPos" ) );
      pv  = mp_vol.placeVolume( vol_station, Transform3D( rot, pos ) );
      pv.addPhysVolID( "station", 3 );
      de_station = detElement( vol_station.name() ).clone( "T4", 3 );
      de_station.setPlacement( pv );
      detector.add( de_station );
      printout( m_local_debug ? ALWAYS : DEBUG, "MP-geo", "+++ Position station T4 of type %s", vol_station.name() );

      pos = Position( 0, 0, _toDouble( "MP:L5ZPos" ) );
      pv  = mp_vol.placeVolume( vol_station, Transform3D( rot, pos ) );
      pv.addPhysVolID( "station", 4 );
      de_station = detElement( vol_station.name() ).clone( "T5", 4 );
      de_station.setPlacement( pv );
      detector.add( de_station );
      printout( m_local_debug ? ALWAYS : DEBUG, "MP-geo", "+++ Position station T5 of type %s", vol_station.name() );

      pos = Position( 0, 0, _toDouble( "MP:L6ZPos" ) );
      pv  = mp_vol.placeVolume( vol_station, Transform3D( rot, pos ) );
      pv.addPhysVolID( "station", 5 );
      de_station = detElement( vol_station.name() ).clone( "T6", 5 );
      de_station.setPlacement( pv );
      detector.add( de_station );
      printout( m_local_debug ? ALWAYS : DEBUG, "MP-geo", "+++ Position station T6 of type %s", vol_station.name() );
    } else if ( !m_attach_volume.empty() ) {
      // Test display of individual entities
      mp_vol.placeVolume( vol = volume( m_attach_volume ) );
      de = detElement( vol.name() );
      detector.add( de.clone( m_attach_volume ) );
    } else {
      except( "MP-geo", "+++ Inconsistent build flags. "
                        "If no specific volume should be ejected, the stations must be built!" );
    }
    xml_h x_tr = x_det.child( _U( transformation ) );
    pv         = placeDetector( mp_vol, x_tr );
    pv.addPhysVolID( "system", id );
  }
} // namespace

static Ref_t create_element( dd4hep::Detector& description, xml_h e, dd4hep::SensitiveDetector sens_det ) {
  MPBuild builder( description, e, sens_det );
  builder.build_detector();
  return builder.detector;
}
DECLARE_DETELEMENT( LHCb_MP_geo, create_element )
