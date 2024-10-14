//==========================================================================
//  AIDA Detector description implementation
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
// Author     : M.Frank, modified by B.Dey
// Modified by Zehua XU & Jessy DANIEL, half layer implemented
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

#define HELPER_CLASS_DEFAULTS( x )                                                                                     \
  x()           = default;                                                                                             \
  x( x&& )      = default;                                                                                             \
  x( const x& ) = default;                                                                                             \
  x& operator=( const x& ) = default;                                                                                  \
  ~x()                     = default

namespace {

  /// Helper class to build the FT detector of LHCb
  struct FTBuild : public dd4hep::xml::tools::VolumeBuilder {
    // Debug flags: To be enables/disbaled by the <debug/> section in the detector.xml file.
    bool        m_build_passive     = true;
    bool        m_build_frames      = true;
    bool        m_build_quarters    = true;
    bool        m_build_half_layers = true;
    bool        m_build_layers      = true;
    bool        m_build_stations    = true;
    bool        m_local_debug       = false;
    std::string m_attach_volume;
    bool        m_build_MT = false; // MightyT:includeMT

    // General constants used by several member functions to build the FT geometry
    double mod_size_x        = 0.0; //  FT:ModuleSizeX
    double mod_size_y        = 0.0; //  FT:ModuleSizeY
    double mod_size_z        = 0.0; //  FT:ModuleSizeZ
    double wall_thick        = 0.0; //  FT:WallThickness
    double mat_pitch_x       = 0.0; //  FT:MatPitchX
    double mat_size_x        = 0.0; //  FT:MatSizeX
    double mat_size_y        = 0.0; //  FT:MatSizeY
    double mat_size_z        = 0.0; //  FT:MatSizeZ
    double skin_size_x       = 0.0; //  FT:SkinSizeX
    double skin_size_y       = 0.0; //  FT:SkinSizeY
    double skin_size_z       = 0.0; //  FT:SkinSizeZ
    double core_size_x       = 0.0; //  FT:CoreSizeX
    double core_size_y       = 0.0; //  FT:CoreSizeY
    double core_size_z       = 0.0; //  FT:CoreSizeZ
    double station_size_x    = 0.0; //  FT:StationSizeX
    double station_size_y    = 0.0; //  FT:StationSizeY
    double station_size_z    = 0.0; //  FT:StationSizeZ
    double endpiece_size_x   = 0.0; //  FT:EndpieceSizeX
    double endpiece_size_y   = 0.0; //  FT:EndpieceSizeY
    double endpiece_size_z   = 0.0; //  FT:EndpieceSizeZ
    double eps               = 0.0; //  FT:eps
    double mod_pitch_x       = 0.0; //  FT:ModulePitchX
    double layer_pitch_z     = 0.0; //  FT:LayerPitchZ
    double layer_uv_pitch_z  = 0.0; //  FT:LayerUVPitchZ
    double layer_x_pitch_z   = 0.0; //  FT:LayerXPitchZ
    double cframe_pitch_z    = 0.0; //  FT:CFramePitchZ
    double cframe_shift_y    = 0.0; //  FT:CFrameShiftY
    double hole_size_x       = 0.0; //  FT:HoleSizeX
    double dead_size_y       = 0.0; //  FT:DeadSizeY
    double endplug_size_y    = 0.0; //  FT:EndplugSizeY"
    double hole_leftx_size_y = 0.0; //  FT:HoleLeftXSizeY"
    double beam_hole_radius  = 0.0; //  FT:BeamHoleRadius

    double mighty_mat_pitch_x = 0.0; //  MightyT:MatPitchX
    double mighty_mat_pitch_y = 0.0; //  MightyT:MatPitchY
    double mighty_pitch_x     = 0.0; //  MightyT:TotalPitchX
    double mighty_pitch_y     = 0.0; //  MightyT:TotalPitchY
    double overlap            = 0.0; //  FT:overlap

    struct Module {
      double      sign = 1.0, leftHoleSizeY = 0, rightHoleSizeY = 0;
      std::string left, right;
      Module( double sgn, double hl, double hr, const std::string& l, const std::string& r )
          : sign( sgn ), leftHoleSizeY( hl ), rightHoleSizeY( hr ), left( l ), right( r ) {}
      HELPER_CLASS_DEFAULTS( Module );
    };
    std::map<std::string, Module>     m_modules;
    std::map<std::string, DetElement> m_detElements;
    std::map<std::string, double>     HoleType;

    /// Utility function to register detector elements for cloning. Will all be deleted!
    void registerDetElement( const std::string& nam, DetElement de );
    /// Utility function to access detector elements for cloning. Will all be deleted!
    dd4hep::DetElement detElement( const std::string& nam ) const;

    /// Initializing constructor
    FTBuild( dd4hep::Detector& description, xml_elt_t e, dd4hep::SensitiveDetector sens );
    FTBuild()                 = delete;
    FTBuild( FTBuild&& )      = delete;
    FTBuild( const FTBuild& ) = delete;
    FTBuild& operator=( const FTBuild& ) = delete;

    /// Default destructor
    virtual ~FTBuild();
    void build_mats();
    void build_modules();
    void build_quarters();
    void build_half_layers();
    void build_layers();
    void build_stations();
    void build_detector();
  };

  void FTBuild::registerDetElement( const std::string& nam, dd4hep::DetElement de ) { m_detElements[nam] = de; }
  dd4hep::DetElement FTBuild::detElement( const std::string& nam ) const {
    auto i = m_detElements.find( nam );
    if ( i == m_detElements.end() ) {
      dd4hep::printout( dd4hep::ERROR, "FT-geo", "Attempt to access non-existing detector element %s", nam.c_str() );
    }
    return ( *i ).second;
  }

  /// Initializing constructor
  FTBuild::FTBuild( dd4hep::Detector& dsc, xml_elt_t e, dd4hep::SensitiveDetector sens )
      : dd4hep::xml::tools::VolumeBuilder( dsc, e, sens ) {
    // Process debug flags
    xml_comp_t x_dbg = x_det.child( _U( debug ), false );
    if ( x_dbg ) {
      for ( xml_coll_t i( x_dbg, _U( item ) ); i; ++i ) {
        xml_comp_t  c( i );
        std::string n = c.nameStr();
        if ( n == "build_passive" ) m_build_passive = c.attr<bool>( _U( value ) );
        if ( n == "build_frames" ) m_build_frames = c.attr<bool>( _U( value ) );
        if ( n == "build_quarters" ) m_build_quarters = c.attr<bool>( _U( value ) );
        if ( n == "build_half_layers" ) m_build_half_layers = c.attr<bool>( _U( value ) );
        if ( n == "build_layers" ) m_build_layers = c.attr<bool>( _U( value ) );
        if ( n == "build_stations" ) m_build_stations = c.attr<bool>( _U( value ) );
        if ( n == "attach_volume" ) m_attach_volume = c.attr<std::string>( _U( value ) );
        if ( n == "local_debug" ) m_local_debug = c.attr<bool>( _U( value ) );
        if ( n == "debug" ) debug = c.attr<bool>( _U( value ) );
      }
    }
    eps               = dd4hep::_toDouble( "FT:eps" );
    mod_pitch_x       = dd4hep::_toDouble( "FT:ModulePitchX" );
    mod_size_x        = dd4hep::_toDouble( "FT:ModuleSizeX" );
    mod_size_y        = dd4hep::_toDouble( "FT:ModuleSizeY" );
    mod_size_z        = dd4hep::_toDouble( "FT:ModuleSizeZ" );
    wall_thick        = dd4hep::_toDouble( "FT:WallThickness" );
    mat_pitch_x       = dd4hep::_toDouble( "FT:MatPitchX" );
    mat_size_x        = dd4hep::_toDouble( "FT:MatSizeX" );
    mat_size_y        = dd4hep::_toDouble( "FT:MatSizeY" );
    mat_size_z        = dd4hep::_toDouble( "FT:MatSizeZ" );
    skin_size_x       = dd4hep::_toDouble( "FT:SkinSizeX" );
    skin_size_y       = dd4hep::_toDouble( "FT:SkinSizeY" );
    skin_size_z       = dd4hep::_toDouble( "FT:SkinSizeZ" );
    core_size_x       = dd4hep::_toDouble( "FT:CoreSizeX" );
    core_size_y       = dd4hep::_toDouble( "FT:CoreSizeY" );
    core_size_z       = dd4hep::_toDouble( "FT:CoreSizeZ" );
    station_size_x    = dd4hep::_toDouble( "FT:StationSizeX" );
    station_size_y    = dd4hep::_toDouble( "FT:StationSizeY" );
    station_size_z    = dd4hep::_toDouble( "FT:StationSizeZ" );
    endpiece_size_x   = dd4hep::_toDouble( "FT:EndpieceSizeX" );
    endpiece_size_y   = dd4hep::_toDouble( "FT:EndpieceSizeY" );
    endpiece_size_z   = dd4hep::_toDouble( "FT:EndpieceSizeZ" );
    layer_pitch_z     = dd4hep::_toDouble( "FT:LayerPitchZ" );
    cframe_pitch_z    = dd4hep::_toDouble( "FT:CFramePitchZ" );
    cframe_shift_y    = dd4hep::_toDouble( "FT:CFrameShiftY" );
    hole_size_x       = dd4hep::_toDouble( "FT:HoleSizeX" );
    dead_size_y       = dd4hep::_toDouble( "FT:DeadSizeY" );
    endplug_size_y    = dd4hep::_toDouble( "FT:EndplugSizeY" );
    hole_leftx_size_y = dd4hep::_toDouble( "FT:HoleLeftXSizeY" );
    beam_hole_radius  = dd4hep::_toDouble( "Regions:TBeamHoleRadius" );

    try {
      m_build_MT         = dd4hep::_toDouble( "MP:includeMT" );
      mighty_mat_pitch_x = dd4hep::_toDouble( "MP:MatPitchX" );
      mighty_mat_pitch_y = dd4hep::_toDouble( "MP:MatPitchY" );
      mighty_pitch_x     = dd4hep::_toDouble( "MP:TotalPitchX" );
      mighty_pitch_y     = dd4hep::_toDouble( "MP:TotalPitchY" );
      overlap            = dd4hep::_toDouble( "FT:overlap" );

      layer_uv_pitch_z = dd4hep::_toDouble( "FT:LayerUVPitchZ" );
      layer_x_pitch_z  = dd4hep::_toDouble( "FT:LayerXPitchZ" );
    } catch ( std::runtime_error& e ) {}

    m_modules["LeftV"] =
        Module( -1., dd4hep::_toDouble( "FT:HoleLeftVSizeY" ), 0, "lvFibreShortMatLeftV", "lvFibreMat" );
    m_modules["RightV"] =
        Module( 1., 0, dd4hep::_toDouble( "FT:HoleRightVSizeY" ), "lvFibreMat", "lvFibreShortMatRightV" );

    m_modules["LeftU"] =
        Module( -1., dd4hep::_toDouble( "FT:HoleLeftUSizeY" ), 0, "lvFibreShortMatLeftU", "lvFibreMat" );
    m_modules["RightU"] =
        Module( 1., 0, dd4hep::_toDouble( "FT:HoleRightUSizeY" ), "lvFibreMat", "lvFibreShortMatRightU" );

    m_modules["LeftX1"] =
        Module( -1., dd4hep::_toDouble( "FT:HoleLeftXSizeY" ), 0, "lvFibreShortMatLeftX1", "lvFibreMat" );
    m_modules["RightX1"] =
        Module( 1., 0, dd4hep::_toDouble( "FT:HoleRightXSizeY" ), "lvFibreMat", "lvFibreShortMatRightX1" );

    m_modules["LeftX2"] =
        Module( -1., dd4hep::_toDouble( "FT:HoleLeftXSizeY" ), 0, "lvFibreShortMatLeftX2", "lvFibreMat" );
    m_modules["RightX2"] =
        Module( 1., 0, dd4hep::_toDouble( "FT:HoleRightXSizeY" ), "lvFibreMat", "lvFibreShortMatRightX2" );

    if ( m_build_MT != 0 ) {
      HoleType["I"]   = 3;
      HoleType["II"]  = 3;
      HoleType["III"] = 2;
      HoleType["IV"]  = 0;
    } else {
      HoleType["I"] = 0;
    }
  }

  FTBuild::~FTBuild() {
    for ( auto& d : m_detElements ) dd4hep::detail::destroyHandle( d.second );
  }

  void FTBuild::build_mats() {
    dd4hep::Position pos;

    dd4hep::Box    mod_box( mod_size_x / 2.0, mod_size_y / 2.0, mod_size_z / 2.0 );
    dd4hep::Volume mod_vol( "lvFTModuleFull", mod_box, description.air() );
    mod_vol.setVisAttributes( description, "FT:Module" );
    registerVolume( mod_vol.name(), mod_vol );

    if ( m_build_MT ) {
      for ( const auto& h : HoleType ) {
        const auto& nMTMat = h.second;
        if ( nMTMat ) {
          overlap = 20;
        } else {
          overlap = 0;
        }

        double      mighty_mt_pitch = mighty_mat_pitch_y * nMTMat / 2.0;
        dd4hep::Box mod_tube( mod_pitch_x, ( mighty_mt_pitch + eps - 2 * overlap ) / 2.0, ( mod_size_z + eps ) / 2.0 );
        dd4hep::Material skin_mat = description.material( "FT:Skin" );
        dd4hep::Box      skin_box( skin_size_x / 2.0, ( skin_size_y + 2 * overlap ) / 2.0, skin_size_z / 2.0 );
        dd4hep::Box      skin_tube( mod_pitch_x, ( mighty_mt_pitch + eps ) / 2.0, skin_size_z / 2.0 + eps );
        dd4hep::Material core_mat = description.material( "FT:Honeycomb" );
        dd4hep::Box      core_box( core_size_x / 2.0, core_size_y / 2.0, core_size_z / 2.0 );
        dd4hep::Box      core_tube( mod_pitch_x, ( mighty_mt_pitch + eps - 2 * overlap ) / 2.0,
                               ( core_size_z + eps ) / 2.0 );
        double           delta_angle = dd4hep::_toDouble( "FT:HoleDeltaAngle" );
        dd4hep::Material fibre_mat   = description.material( "FT:SciFibre" );

        int counter = 5;
        for ( const auto& m : m_modules ) {
          const auto& p           = m.second;
          double      hole_size_y = mighty_mt_pitch;

          dd4hep::Solid mod_solid;
          if ( m_build_passive ) {
            double dy = -mod_size_y + ( nMTMat / ( 2.0 * 5.0 ) ) * mighty_pitch_y -
                        2 * overlap; // one factor of two for upper and lower,
            pos       = dd4hep::Position( p.sign * mod_pitch_x / 2.0, dy / 2.0, 0 );
            mod_solid = dd4hep::SubtractionSolid( mod_box, mod_tube, pos ); // here is the problem for overlap TH
            mod_vol   = dd4hep::Volume( "lvFTModuleHole" + m.first + h.first, mod_solid, description.air() );
          } else {
            mod_vol   = dd4hep::Assembly( "lvFTModuleHole" + m.first + h.first );
            mod_solid = mod_vol.solid();
          }
          mod_vol.setVisAttributes( description, "FT:Module" );
          registerVolume( mod_vol.name(), mod_vol );
          dd4hep::printout( m_local_debug ? dd4hep::ALWAYS : dd4hep::DEBUG, "FT-geo",
                            "+++ Building volume %-24s solid: %-24s material:%s", mod_vol.name(),
                            mod_solid->IsA()->GetName(), mod_vol.material().name() );

          if ( m_build_passive ) {
            pos.SetY( 0.5 * ( -skin_size_y + ( nMTMat / ( 2.0 * 5.0 ) ) * mighty_pitch_y ) - overlap );
            dd4hep::SubtractionSolid skin_solid( skin_box, skin_tube, pos );
            dd4hep::Volume           skin_vol( "lvSkinHole" + m.first + h.first, skin_solid, skin_mat );
            skin_vol.setVisAttributes( description, "FT:Support" );
            registerVolume( skin_vol.name(), skin_vol );
            dd4hep::printout( m_local_debug ? dd4hep::ALWAYS : dd4hep::DEBUG, "FT-geo",
                              "+++ Building volume %-24s solid: %-24s material:%s", skin_vol.name(),
                              skin_solid->IsA()->GetName(), skin_mat.name() );

            double core_dy = 0.5 * ( -core_size_y + ( nMTMat / ( 2.0 * 5.0 ) ) * mighty_pitch_y - 2 * overlap );
            pos.SetY( core_dy );
            dd4hep::SubtractionSolid core_front_solid( core_box, core_tube, pos );
            dd4hep::Volume core_front_vol( "lvCoreHoleFront" + m.first + h.first, core_front_solid, core_mat );
            core_front_vol.setVisAttributes( description, "FT:Core" );
            core_front_vol.placeVolume( volume( "lvEndplug" ),
                                        dd4hep::Position( 0, 0.5 * ( core_size_y - endplug_size_y ), 0 ) );

            pos = dd4hep::Position( -0.5 * p.sign * hole_size_x,
                                    -0.5 * ( core_size_y - endpiece_size_y - dead_size_y ) + hole_size_y - 2 * overlap,
                                    0.5 * ( core_size_z - endpiece_size_z ) );

            core_front_vol.placeVolume( volume( "lvEndpiece3" ), pos );

            pos = dd4hep::Position( 0.5 * p.sign * ( core_size_x - hole_size_x ),
                                    -0.5 * ( core_size_y - endpiece_size_y - dead_size_y ) + hole_size_y - 2 * overlap,
                                    0.5 * ( core_size_z - endpiece_size_z ) );

            core_front_vol.placeVolume( volume( "lvEndpiece1" ), pos );

            registerVolume( core_front_vol.name(), core_front_vol );
            dd4hep::printout( m_local_debug ? dd4hep::ALWAYS : dd4hep::DEBUG, "FT-geo",
                              "+++ Building volume %-24s solid: %-24s material:%s", core_front_vol.name(),
                              core_front_solid->IsA()->GetName(), core_mat.name() );

            pos = dd4hep::Position( -0.5 * p.sign * mod_pitch_x, core_dy, 0.0 );

            dd4hep::SubtractionSolid core_back_solid( "CoreHoleBack" + m.first + h.first + "_shape", core_box,
                                                      core_tube, pos );
            dd4hep::Volume           core_back_vol( "lvCoreHoleBack" + m.first + h.first, core_back_solid, core_mat );
            core_back_vol.setVisAttributes( description, "FT:Core" );
            registerVolume( core_back_vol.name(), core_back_vol );
            dd4hep::printout( m_local_debug ? dd4hep::ALWAYS : dd4hep::DEBUG, "FT-geo",
                              "+++ Module:%s Sign:%.0f LeftHoleSizeY:%5.2f RightHoleSizeY:%5.2f "
                              "[FT:Hole%sSizeY: %5.2f]  hole_size_y: %5.2f",
                              mod_vol.name(), p.sign, p.leftHoleSizeY, p.rightHoleSizeY, m.first.c_str(),
                              dd4hep::_toDouble( "FT:Hole" + m.first + "SizeY" ), hole_size_y );
            core_back_vol.placeVolume( volume( "lvEndplug" ),
                                       dd4hep::Position( 0, 0.5 * ( core_size_y - endplug_size_y ), 0 ) );

            pos = dd4hep::Position( +0.5 * p.sign * hole_size_x,
                                    -0.5 * ( core_size_y - endpiece_size_y - dead_size_y ) + hole_size_y - 2 * overlap,
                                    +0.5 * ( core_size_z - endpiece_size_z ) );

            core_back_vol.placeVolume( volume( "lvEndpiece3" ), pos );

            pos = dd4hep::Position( +0.5 * p.sign * ( hole_size_x - core_size_x ),
                                    -0.5 * ( core_size_y - endpiece_size_y - dead_size_y ) + hole_size_y - 2 * overlap,
                                    +0.5 * ( core_size_z - endpiece_size_z ) );
            core_back_vol.placeVolume( volume( "lvEndpiece1" ), pos );
            pos.SetY( core_dy );

            dd4hep::printout( m_local_debug ? dd4hep::ALWAYS : dd4hep::DEBUG, "FT-geo",
                              "+++ Building volume %-24s solid: %-24s material:%s", core_back_vol.name(),
                              core_back_solid->IsA()->GetName(), core_mat.name() );

            dd4hep::Box side_wall_solid(
                wall_thick / 2.0, ( skin_size_y - hole_size_y - wall_thick + 2 * overlap ) / 2.0, mod_size_z / 2.0 );
            dd4hep::Volume side_wall_vol( "lvSideWall" + m.first + h.first, side_wall_solid, skin_mat );
            side_wall_vol.setVisAttributes( description, "FT:Support" );
            registerVolume( side_wall_vol.name(), side_wall_vol );
            dd4hep::printout( m_local_debug ? dd4hep::ALWAYS : dd4hep::DEBUG, "FT-geo",
                              "+++ Building volume %-24s solid: %-24s material:%s Wall-thickness", side_wall_vol.name(),
                              side_wall_solid->IsA()->GetName(), skin_mat.name(), wall_thick );

            double angle    = std::asin( ( hole_size_y / nMTMat - hole_leftx_size_y ) /
                                      hole_leftx_size_y ); // TODO: verify if nMTMat is correct
            double startPhi = 0.5 * ( 1.0 + p.sign ) * ( M_PI / 2.0 + delta_angle + angle ) - angle;
            double endPhi   = startPhi + M_PI / 2.0 - delta_angle + angle;
            //
            // The "wall_thick" below is exactly the extrusion between: lvFTModuleHoleRightU/lvHoleWallRightU_9
            // Don't know how to handle this
            // Tube   hole_wall_solid(hole_leftx_size_y, hole_leftx_size_y + wall_thick, mod_size_z/2.0, startPhi,
            // endPhi);
            if ( endPhi > M_PI ) endPhi -= delta_angle / 10.;
            if ( startPhi < 0 ) startPhi += delta_angle / 10.;
            dd4hep::Tube hole_wall_solid( hole_leftx_size_y, hole_leftx_size_y + wall_thick, mod_size_z / 2.0, startPhi,
                                          endPhi );
            // Box    hole_wall_solid(skin_size_x/2.0, wall_thick/2.0, mod_size_z/2.0);
            dd4hep::Volume hole_wall_vol( "lvHoleWall" + m.first + h.first, hole_wall_solid, skin_mat );
            hole_wall_vol.setVisAttributes( description, "FT:Support" );
            registerVolume( hole_wall_vol.name(), hole_wall_vol );
            dd4hep::printout( m_local_debug ? dd4hep::ALWAYS : dd4hep::DEBUG, "FT-geo",
                              "+++ Building volume %-24s solid: %-24s material:%s", hole_wall_vol.name(),
                              hole_wall_solid->IsA()->GetName(), skin_mat.name() );
            dd4hep::printout( m_local_debug ? dd4hep::ALWAYS : dd4hep::DEBUG, "FT-geo",
                              "+++ Volume lvHoleWall%s Sign: %.0f Angle:%f rad", m.first.c_str(), p.sign, angle );
          }
          if ( p.left != "lvFibreMat" ) {
            dd4hep::Box    mat0_solid( mat_size_x / 2.0, ( mat_size_y - mighty_mt_pitch + 2 * overlap ) / 2.0,
                                    mat_size_z / 2.0 );
            dd4hep::Volume mat0_vol( p.left + h.first, mat0_solid, fibre_mat );
            mat0_vol.setVisAttributes( description, "FT:Fibre" );
            mat0_vol.setSensitiveDetector( sensitive );
            registerVolume( mat0_vol.name(), mat0_vol );
            dd4hep::printout( m_local_debug ? dd4hep::ALWAYS : dd4hep::DEBUG, "FT-geo",
                              "+++ Building volume %-24s solid: %-24s material:%s", mat0_vol.name(),
                              mat0_solid->IsA()->GetName(), fibre_mat.name() );
          }
          if ( p.right != "lvFibreMat" ) {
            dd4hep::Box    mat3_solid( mat_size_x / 2.0, ( mat_size_y - mighty_mt_pitch + 2 * overlap ) / 2.0,
                                    mat_size_z / 2.0 );
            dd4hep::Volume mat3_vol( p.right + h.first, mat3_solid, fibre_mat );
            mat3_vol.setVisAttributes( description, "FT:Fibre" );
            mat3_vol.setSensitiveDetector( sensitive );
            registerVolume( mat3_vol.name(), mat3_vol );
            dd4hep::printout( m_local_debug ? dd4hep::ALWAYS : dd4hep::DEBUG, "FT-geo",
                              "+++ Building volume %-24s solid: %-24s material:%s", mat3_vol.name(),
                              mat3_solid->IsA()->GetName(), fibre_mat.name() );
          } else {
            dd4hep::Box    mat_solid( mat_size_x / 2.0, ( mat_size_y - mighty_mt_pitch + 2 * overlap ) / 2.0,
                                   mat_size_z / 2.0 );
            dd4hep::Volume mat_vol( p.right + h.first + std::to_string( counter ), mat_solid, fibre_mat );
            mat_vol.setVisAttributes( description, "FT:Fibre" );
            mat_vol.setSensitiveDetector( sensitive );
            registerVolume( mat_vol.name(), mat_vol );
            dd4hep::printout( m_local_debug ? dd4hep::ALWAYS : dd4hep::DEBUG, "FT-geo",
                              "+++ Building volume %-24s solid: %-24s material:%s", mat_vol.name(),
                              mat_solid->IsA()->GetName(), fibre_mat.name() );
          }
          counter--;
        }
      }
    } else {
      // dd4hep::Position pos;

      // dd4hep::Box    mod_box( mod_size_x / 2.0, mod_size_y / 2.0, mod_size_z / 2.0 );
      // dd4hep::Volume mod_vol( "lvFTModuleFull", mod_box, description.air() );
      // mod_vol.setVisAttributes( description, "FT:Module" );
      // registerVolume( mod_vol.name(), mod_vol );

      dd4hep::Tube     mod_tube( 0, hole_leftx_size_y, ( mod_size_z + eps ) / 2.0 );
      dd4hep::Material skin_mat = description.material( "FT:Skin" );
      dd4hep::Box      skin_box( skin_size_x / 2.0, skin_size_y / 2.0, skin_size_z / 2.0 );
      dd4hep::Tube     skin_tube( 0, hole_leftx_size_y + wall_thick, ( skin_size_z + eps ) / 2.0 );
      dd4hep::Material core_mat = description.material( "FT:Honeycomb" );
      dd4hep::Box      core_box( core_size_x / 2.0, core_size_y / 2.0, core_size_z / 2.0 );
      dd4hep::Tube     core_tube( 0, hole_leftx_size_y + wall_thick, ( core_size_z ) / 2.0 );
      double           delta_angle = dd4hep::_toDouble( "FT:HoleDeltaAngle" );
      dd4hep::Material fibre_mat   = description.material( "FT:SciFibre" );

      for ( const auto& m : m_modules ) {
        const auto&   p           = m.second;
        double        hole_size_y = dd4hep::_toDouble( "FT:Hole" + m.first + "SizeY" );
        dd4hep::Solid mod_solid;
        if ( m_build_passive ) {
          double dy = -0.5 * mod_size_y + hole_size_y - hole_leftx_size_y;
          pos       = dd4hep::Position( p.sign * mod_pitch_x / 2.0, dy, 0 );
          mod_solid = dd4hep::SubtractionSolid( mod_box, mod_tube, pos );
          mod_vol   = dd4hep::Assembly( "lvFTModuleHole" + m.first );
        } else {
          mod_vol   = dd4hep::Assembly( "lvFTModuleHole" + m.first );
          mod_solid = mod_vol.solid();
        }
        mod_vol.setVisAttributes( description, "FT:Module" );
        registerVolume( mod_vol.name(), mod_vol );
        dd4hep::printout( m_local_debug ? dd4hep::ALWAYS : dd4hep::DEBUG, "FT-geo",
                          "+++ Building volume %-24s solid: %-24s material:%s", mod_vol.name(),
                          mod_solid->IsA()->GetName(), mod_vol.material().name() );

        if ( m_build_passive ) {
          pos.SetY( -0.5 * skin_size_y + hole_size_y - hole_leftx_size_y );
          dd4hep::SubtractionSolid skin_solid( skin_box, skin_tube, pos );
          dd4hep::Volume           skin_vol( "lvSkinHole" + m.first, skin_solid, skin_mat );
          skin_vol.setVisAttributes( description, "FT:Support" );
          registerVolume( skin_vol.name(), skin_vol );
          dd4hep::printout( m_local_debug ? dd4hep::ALWAYS : dd4hep::DEBUG, "FT-geo",
                            "+++ Building volume %-24s solid: %-24s material:%s", skin_vol.name(),
                            skin_solid->IsA()->GetName(), skin_mat.name() );

          pos.SetY( -0.5 * core_size_y + hole_size_y - hole_leftx_size_y );
          dd4hep::SubtractionSolid core_front_solid( core_box, core_tube, pos );
          dd4hep::Volume           core_front_vol( "lvCoreHoleFront" + m.first, core_front_solid, core_mat );
          core_front_vol.setVisAttributes( description, "FT:Core" );
          core_front_vol.placeVolume( volume( "lvEndplug" ),
                                      dd4hep::Position( 0, 0.5 * ( core_size_y - endplug_size_y ), 0 ) );
          pos = dd4hep::Position( -0.5 * p.sign * hole_size_x, -0.5 * ( core_size_y - endpiece_size_y ) + dead_size_y,
                                  0.5 * ( core_size_z - endpiece_size_z ) );
          core_front_vol.placeVolume( volume( "lvEndpiece3" ), pos );
          pos = dd4hep::Position( 0.5 * p.sign * ( core_size_x - hole_size_x ),
                                  -0.5 * ( core_size_y - endpiece_size_y ) + hole_size_y + dead_size_y,
                                  0.5 * ( core_size_z - endpiece_size_z ) );
          core_front_vol.placeVolume( volume( "lvEndpiece1" ), pos );
          registerVolume( core_front_vol.name(), core_front_vol );
          dd4hep::printout( m_local_debug ? dd4hep::ALWAYS : dd4hep::DEBUG, "FT-geo",
                            "+++ Building volume %-24s solid: %-24s material:%s", core_front_vol.name(),
                            core_front_solid->IsA()->GetName(), core_mat.name() );

          pos = dd4hep::Position( -0.5 * p.sign * mod_pitch_x, -0.5 * core_size_y + hole_size_y - hole_leftx_size_y,
                                  0.0 );
          dd4hep::SubtractionSolid core_back_solid( "CoreHoleBack" + m.first + "_shape", core_box, core_tube, pos );
          dd4hep::Volume           core_back_vol( "lvCoreHoleBack" + m.first, core_back_solid, core_mat );
          core_back_vol.setVisAttributes( description, "FT:Core" );
          registerVolume( core_back_vol.name(), core_back_vol );
          dd4hep::printout( m_local_debug ? dd4hep::ALWAYS : dd4hep::DEBUG, "FT-geo",
                            "+++ Module:%s Sign:%.0f LeftHoleSizeY:%5.2f RightHoleSizeY:%5.2f "
                            "[FT:Hole%sSizeY: %5.2f]  hole_size_y: %5.2f",
                            mod_vol.name(), p.sign, p.leftHoleSizeY, p.rightHoleSizeY, m.first.c_str(),
                            dd4hep::_toDouble( "FT:Hole" + m.first + "SizeY" ), hole_size_y );
          core_back_vol.placeVolume( volume( "lvEndplug" ),
                                     dd4hep::Position( 0, 0.5 * ( core_size_y - endplug_size_y ), 0 ) );
          pos = dd4hep::Position( +0.5 * p.sign * hole_size_x, -0.5 * ( core_size_y - endpiece_size_y ) + dead_size_y,
                                  +0.5 * ( core_size_z - endpiece_size_z ) );
          core_back_vol.placeVolume( volume( "lvEndpiece3" ), pos );
          pos = dd4hep::Position( +0.5 * p.sign * ( hole_size_x - core_size_x ),
                                  -0.5 * ( core_size_y - endpiece_size_y ) + hole_size_y + dead_size_y,
                                  +0.5 * ( core_size_z - endpiece_size_z ) );
          //-0.5 * ( core_size_z - endpiece_size_z ) // BD check this
          core_back_vol.placeVolume( volume( "lvEndpiece1" ), pos );

          dd4hep::printout( m_local_debug ? dd4hep::ALWAYS : dd4hep::DEBUG, "FT-geo",
                            "+++ Building volume %-24s solid: %-24s material:%s", core_back_vol.name(),
                            core_back_solid->IsA()->GetName(), core_mat.name() );

          dd4hep::Box    side_wall_solid( wall_thick / 2.0, ( skin_size_y - hole_size_y - wall_thick ) / 2.0,
                                       mod_size_z / 2.0 );
          dd4hep::Volume side_wall_vol( "lvSideWall" + m.first, side_wall_solid, skin_mat );
          side_wall_vol.setVisAttributes( description, "FT:Support" );
          registerVolume( side_wall_vol.name(), side_wall_vol );
          dd4hep::printout( m_local_debug ? dd4hep::ALWAYS : dd4hep::DEBUG, "FT-geo",
                            "+++ Building volume %-24s solid: %-24s material:%s Wall-thickness", side_wall_vol.name(),
                            side_wall_solid->IsA()->GetName(), skin_mat.name(), wall_thick );

          double angle    = std::asin( ( hole_size_y - hole_leftx_size_y ) / hole_leftx_size_y );
          double startPhi = 0.5 * ( 1.0 + p.sign ) * ( M_PI / 2.0 + delta_angle + angle ) - angle;
          double endPhi   = startPhi + M_PI / 2.0 - delta_angle + angle;
          //
          // The "wall_thick" below is exactly the extrusion between: lvFTModuleHoleRightU/lvHoleWallRightU_9
          // Don't know how to handle this
          // BD changed: *small* change to handle extrusions
          if ( endPhi > M_PI ) endPhi -= delta_angle / 10.;
          if ( startPhi < 0 ) startPhi += delta_angle / 10.;
          dd4hep::Tube   hole_wall_solid( hole_leftx_size_y, hole_leftx_size_y + wall_thick, mod_size_z / 2.0, startPhi,
                                        endPhi );
          dd4hep::Volume hole_wall_vol( "lvHoleWall" + m.first, hole_wall_solid, skin_mat );
          hole_wall_vol.setVisAttributes( description, "FT:Support" );
          registerVolume( hole_wall_vol.name(), hole_wall_vol );
          dd4hep::printout( m_local_debug ? dd4hep::ALWAYS : dd4hep::DEBUG, "FT-geo",
                            "+++ Building volume %-24s solid: %-24s material:%s", hole_wall_vol.name(),
                            hole_wall_solid->IsA()->GetName(), skin_mat.name() );
          dd4hep::printout( m_local_debug ? dd4hep::ALWAYS : dd4hep::DEBUG, "FT-geo",
                            "+++ Volume lvHoleWall%s Sign: %.0f Angle:%f rad", m.first.c_str(), p.sign, angle );
        }
        if ( p.left != "lvFibreMat" ) {
          dd4hep::Box    mat0_solid( mat_size_x / 2.0, ( mat_size_y - hole_size_y ) / 2.0, mat_size_z / 2.0 );
          dd4hep::Volume mat0_vol( p.left, mat0_solid, fibre_mat );
          mat0_vol.setVisAttributes( description, "FT:Fibre" );
          mat0_vol.setSensitiveDetector( sensitive );
          registerVolume( mat0_vol.name(), mat0_vol );
          dd4hep::printout( m_local_debug ? dd4hep::ALWAYS : dd4hep::DEBUG, "FT-geo",
                            "+++ Building volume %-24s solid: %-24s material:%s", mat0_vol.name(),
                            mat0_solid->IsA()->GetName(), fibre_mat.name() );
        }
        if ( p.right != "lvFibreMat" ) {
          dd4hep::Box    mat3_solid( mat_size_x / 2.0, ( mat_size_y - hole_size_y ) / 2.0, mat_size_z / 2.0 );
          dd4hep::Volume mat3_vol( p.right, mat3_solid, fibre_mat );
          mat3_vol.setVisAttributes( description, "FT:Fibre" );
          mat3_vol.setSensitiveDetector( sensitive );
          registerVolume( mat3_vol.name(), mat3_vol );
          dd4hep::printout( m_local_debug ? dd4hep::ALWAYS : dd4hep::DEBUG, "FT-geo",
                            "+++ Building volume %-24s solid: %-24s material:%s", mat3_vol.name(),
                            mat3_solid->IsA()->GetName(), fibre_mat.name() );
        }
      }
    }
  }

  void FTBuild::build_modules() {
    dd4hep::Position     pos;
    dd4hep::PlacedVolume pv;
    dd4hep::RotationZYX  rot_Y( 0, M_PI, 0 );
    dd4hep::Volume       mod_vol = volume( "lvFTModuleFull" );
    dd4hep::DetElement   de_mod( "module", id );
    dd4hep::DetElement   de_mat;

    dd4hep::printout( m_local_debug ? dd4hep::ALWAYS : dd4hep::DEBUG, "FT-geo", "+++ Building module %s",
                      mod_vol.name() );
    registerDetElement( mod_vol.name(), de_mod );

    if ( m_build_passive ) {
      pos =
          dd4hep::Position( 0, -0.5 * ( mod_size_y - skin_size_y ), -0.5 * ( mat_size_z + skin_size_z ) - core_size_z );
      pv  = mod_vol.placeVolume( volume( "lvSkin" ), pos );
      pos = dd4hep::Position( 0, -0.5 * ( mod_size_y - core_size_y ), -0.5 * ( mat_size_z + core_size_z ) );
      pv  = mod_vol.placeVolume( volume( "lvCore" ), pos );
    }
    pos = dd4hep::Position( -1.5 * mat_pitch_x, 0.5 * ( mod_size_y - mat_size_y ), 0.0 );
    pv  = mod_vol.placeVolume( volume( "lvFibreMat" ), pos );
    pv.addPhysVolID( "mat", 0 );
    de_mat = DetElement( de_mod, "Mat0", 0 );
    de_mat.setPlacement( pv );

    pos = dd4hep::Position( -0.5 * mat_pitch_x, 0.5 * ( mod_size_y - mat_size_y ), 0.0 );
    pv  = mod_vol.placeVolume( volume( "lvFibreMat" ), pos );
    pv.addPhysVolID( "mat", 1 );
    de_mat = DetElement( de_mod, "Mat1", 1 );
    de_mat.setPlacement( pv );

    pos = dd4hep::Position( 0.5 * mat_pitch_x, 0.5 * ( mod_size_y - mat_size_y ), 0.0 );
    pv  = mod_vol.placeVolume( volume( "lvFibreMat" ), pos );
    pv.addPhysVolID( "mat", 2 );
    de_mat = DetElement( de_mod, "Mat2", 2 );
    de_mat.setPlacement( pv );

    pos = dd4hep::Position( 1.5 * mat_pitch_x, 0.5 * ( mod_size_y - mat_size_y ), 0.0 );
    pv  = mod_vol.placeVolume( volume( "lvFibreMat" ), pos );
    pv.addPhysVolID( "mat", 3 );
    de_mat = DetElement( de_mod, "Mat3", 3 );
    de_mat.setPlacement( pv );

    if ( m_build_passive ) {
      pos = dd4hep::Position( 0, -0.5 * ( mod_size_y - core_size_y ), 0.5 * ( mat_size_z + core_size_z ) );
      pv  = mod_vol.placeVolume( volume( "lvCore" ), pos );
      pos =
          dd4hep::Position( 0, -0.5 * ( mod_size_y - skin_size_y ), 0.5 * ( mat_size_z + skin_size_z ) + core_size_z );
      pv = mod_vol.placeVolume( volume( "lvSkin" ), pos );

      pos = dd4hep::Position( -0.5 * ( mod_size_x - wall_thick ), -0.5 * ( mod_size_y - skin_size_y ), 0.0 );
      pv  = mod_vol.placeVolume( volume( "lvSideWall" ), pos );
      pos = dd4hep::Position( 0.5 * ( mod_size_x - wall_thick ), -0.5 * ( mod_size_y - skin_size_y ), 0.0 );
      pv  = mod_vol.placeVolume( volume( "lvSideWall" ), pos );
    }
    if ( m_build_MT ) {
      for ( const auto& h : HoleType ) {
        const auto& nMTMat = h.second;
        if ( nMTMat ) {
          overlap = 20;
        } else {
          overlap = 0;
        }
        /**
           This part of the mat construction code takes care of the inner parts
           of the layers, meaning the mats that are of a specific length to
           go in the U,V, X1 or X2 layers of the detector.
           There is a distinction between X1 and X2 solely because the mat itself
           is oriented differently.
        **/
        int counter = 5;
        for ( const auto& m : m_modules ) {
          const auto& p           = m.second;
          double      hole_size_y = mighty_mat_pitch_y * nMTMat / 2.0;

          de_mod  = dd4hep::DetElement( "module", id );
          mod_vol = volume( "lvFTModuleHole" + m.first + h.first );
          registerDetElement( mod_vol.name(), de_mod );
          dd4hep::printout( m_local_debug ? dd4hep::ALWAYS : dd4hep::DEBUG, "FT-geo", "+++ Building module %s",
                            mod_vol.name() );

          if ( m_build_passive ) {
            pos = dd4hep::Position( 0.0, -0.5 * ( mod_size_y - skin_size_y ) - overlap,
                                    -0.5 * ( mat_size_z + skin_size_z ) - core_size_z );
            pv  = mod_vol.placeVolume( volume( "lvSkinHole" + m.first + h.first ), pos );

            pos = dd4hep::Position( 0.0, -0.5 * ( mod_size_y - core_size_y ), -0.5 * ( mat_size_z + core_size_z ) );
            pv  = mod_vol.placeVolume( volume( "lvCoreHoleFront" + m.first + h.first ), pos );
          }
          double ypos = 0.5 * ( nMTMat / ( 2.0 * 5.0 ) ) * mighty_pitch_y - overlap;
          pos         = dd4hep::Position( -1.5 * mat_pitch_x, ypos, 0.0 );

          std::string vol_name_left, vol_name_right;
          if ( p.left != "lvFibreMat" ) {
            vol_name_left = p.left + h.first;
          } else {
            vol_name_left = p.left + h.first + std::to_string( counter );
          }
          pv = mod_vol.placeVolume( volume( vol_name_left ), pos );
          pv.addPhysVolID( "mat", 0 );
          de_mat = dd4hep::DetElement( de_mod, "Mat0", 0 );
          de_mat.setPlacement( pv );

          pos = dd4hep::Position( -0.5 * mat_pitch_x, ypos, 0.0 );
          pv  = mod_vol.placeVolume( volume( "lvFibreMat" + h.first + std::to_string( counter ) ), pos );
          pv.addPhysVolID( "mat", 1 );
          de_mat = dd4hep::DetElement( de_mod, "Mat1", 1 );
          de_mat.setPlacement( pv );

          pos = dd4hep::Position( 0.5 * mat_pitch_x, ypos, 0.0 );
          pv  = mod_vol.placeVolume( volume( "lvFibreMat" + h.first + std::to_string( counter ) ), pos );
          pv.addPhysVolID( "mat", 2 );
          de_mat = dd4hep::DetElement( de_mod, "Mat2", 2 );
          de_mat.setPlacement( pv );

          if ( p.right != "lvFibreMat" ) {
            vol_name_right = p.right + h.first;
          } else {
            vol_name_right = p.right + h.first + std::to_string( counter );
          }

          pos = dd4hep::Position( 1.5 * mat_pitch_x, ypos, 0.0 );
          pv  = mod_vol.placeVolume( volume( vol_name_right ), pos );
          pv.addPhysVolID( "mat", 3 );
          de_mat = dd4hep::DetElement( de_mod, "Mat3", 3 );
          de_mat.setPlacement( pv );

          if ( m_build_passive ) {
            pos = dd4hep::Position( 0.0, -0.5 * ( mod_size_y - core_size_y ), 0.5 * ( mat_size_z + core_size_z ) );
            pv  = mod_vol.placeVolume( volume( "lvCoreHoleBack" + m.first + h.first ),
                                      dd4hep::Transform3D( rot_Y, pos ) );

            pos = dd4hep::Position( 0.0, -0.5 * ( mod_size_y - skin_size_y ) - overlap,
                                    0.5 * ( mat_size_z + skin_size_z ) + core_size_z );
            pv  = mod_vol.placeVolume( volume( "lvSkinHole" + m.first + h.first ), pos );

            double ypos = -0.5 * ( mod_size_y - skin_size_y ) + 0.5 * hole_size_y - overlap;
            pos         = dd4hep::Position( -0.5 * p.sign * ( mod_size_x - wall_thick ), ypos, 0.0 );
            pv          = mod_vol.placeVolume( volume( "lvSideWall" + m.first + h.first ), pos );

            pos = dd4hep::Position( 0.5 * p.sign * ( mod_size_x - wall_thick ), ypos, 0.0 );
            pv  = mod_vol.placeVolume( volume( "lvSideWall" + m.first + h.first ), pos );

            pos = dd4hep::Position( 0.5 * p.sign * mod_pitch_x, -0.5 * mod_size_y + hole_size_y - hole_leftx_size_y,
                                    0.0 );
          }
        }
      }
    } else {
      for ( const auto& m : m_modules ) {
        const auto& p           = m.second;
        double      hole_size_y = dd4hep::_toDouble( "FT:Hole" + m.first + "SizeY" );

        de_mod  = DetElement( "module", id );
        mod_vol = volume( "lvFTModuleHole" + m.first );
        registerDetElement( mod_vol.name(), de_mod );
        dd4hep::printout( m_local_debug ? dd4hep::ALWAYS : dd4hep::DEBUG, "FT-geo", "+++ Building module %s",
                          mod_vol.name() );

        if ( m_build_passive ) {
          pos = dd4hep::Position( 0.0, -0.5 * ( mod_size_y - skin_size_y ),
                                  -0.5 * ( mat_size_z + skin_size_z ) - core_size_z );
          pv  = mod_vol.placeVolume( volume( "lvSkinHole" + m.first ), pos );

          pos = dd4hep::Position( 0.0, -0.5 * ( mod_size_y - core_size_y ), -0.5 * ( mat_size_z + core_size_z ) );
          pv  = mod_vol.placeVolume( volume( "lvCoreHoleFront" + m.first ), pos );
        }
        pos = dd4hep::Position( -1.5 * mat_pitch_x,
                                0.5 * ( mod_size_y - mat_size_y - p.leftHoleSizeY ) + p.leftHoleSizeY, 0.0 );
        pv  = mod_vol.placeVolume( volume( p.left ), pos );
        pv.addPhysVolID( "mat", 0 );
        de_mat = DetElement( de_mod, "Mat0", 0 );
        de_mat.setPlacement( pv );

        pos = dd4hep::Position( -0.5 * mat_pitch_x, 0.5 * ( mod_size_y - mat_size_y ), 0.0 );
        pv  = mod_vol.placeVolume( volume( "lvFibreMat" ), pos );
        pv.addPhysVolID( "mat", 1 );
        de_mat = DetElement( de_mod, "Mat1", 1 );
        de_mat.setPlacement( pv );

        pos = dd4hep::Position( 0.5 * mat_pitch_x, 0.5 * ( mod_size_y - mat_size_y ), 0.0 );
        pv  = mod_vol.placeVolume( volume( "lvFibreMat" ), pos );
        pv.addPhysVolID( "mat", 2 );
        de_mat = DetElement( de_mod, "Mat2", 2 );
        de_mat.setPlacement( pv );

        pos = dd4hep::Position( 1.5 * mat_pitch_x,
                                0.5 * ( mod_size_y - mat_size_y - p.rightHoleSizeY ) + p.rightHoleSizeY, 0.0 );
        pv  = mod_vol.placeVolume( volume( p.right ), pos );
        pv.addPhysVolID( "mat", 3 );
        de_mat = DetElement( de_mod, "Mat3", 3 );
        de_mat.setPlacement( pv );

        if ( m_build_passive ) {
          pos = dd4hep::Position( 0.0, -0.5 * ( mod_size_y - core_size_y ), 0.5 * ( mat_size_z + core_size_z ) );
          pv  = mod_vol.placeVolume( volume( "lvCoreHoleBack" + m.first ), dd4hep::Transform3D( rot_Y, pos ) );
          // pv = mod_vol.placeVolume(volume("lvCoreHoleBack"+m.first), pos); // BD check this
          pos = dd4hep::Position( 0.0, -0.5 * ( mod_size_y - skin_size_y ),
                                  0.5 * ( mat_size_z + skin_size_z ) + core_size_z );
          pv  = mod_vol.placeVolume( volume( "lvSkinHole" + m.first ), pos );

          pos =
              dd4hep::Position( -0.5 * p.sign * ( mod_size_x - wall_thick ), -0.5 * ( mod_size_y - skin_size_y ), 0.0 );
          pv = mod_vol.placeVolume( volume( "lvSideWall" ), pos );

          pos = dd4hep::Position( 0.5 * p.sign * ( mod_size_x - wall_thick ),
                                  -0.5 * ( mod_size_y - skin_size_y - hole_size_y - wall_thick ), 0.0 );
          pv  = mod_vol.placeVolume( volume( "lvSideWall" + m.first ), pos );

          pos =
              dd4hep::Position( 0.5 * p.sign * mod_pitch_x, -0.5 * mod_size_y + hole_size_y - hole_leftx_size_y, 0.0 );
          pv = mod_vol.placeVolume( volume( "lvHoleWall" + m.first ), pos );
        }
      }
    }
  }
  /**
    Do not need to construct V or X2 types here; be care for the hole position
   **/
  void FTBuild::build_quarters() {
    struct Quarter {
      double      sign            = 0;
      double      stereo          = 0;
      double      rotation_factor = 1;
      std::string special;
      Quarter( double sgn, double st, double reflection, const std::string& s )
          : sign( sgn ), stereo( st ), rotation_factor( reflection ), special( s ) {}
      HELPER_CLASS_DEFAULTS( Quarter );
    };
    dd4hep::Position               pos;
    dd4hep::PlacedVolume           pv;
    dd4hep::Volume                 vol_full = volume( "lvFTModuleFull" );
    dd4hep::DetElement             de_full  = detElement( "lvFTModuleFull" );
    std::map<std::string, Quarter> quarters;
    double                         stereo_u = dd4hep::_toDouble( "FT:StereoAngleU" );
    double                         stereo_x = dd4hep::_toDouble( "FT:StereoAngleX" );
    quarters["UNeg"]                        = Quarter( -1., stereo_u, 1, "RightU" );
    quarters["UPos"]                        = Quarter( 1., stereo_u, 1, "LeftU" );

    quarters["X1Neg"] = Quarter( -1., stereo_x, -1, "RightX1" );
    quarters["X1Pos"] = Quarter( 1., stereo_x, -1, "LeftX1" );

    for ( int stationType = 5, nmod = 6; stationType <= nmod; ++stationType ) {
      for ( const auto& q : quarters ) {
        const auto&         p = q.second;
        dd4hep::Assembly    vol_quarter( dd4hep::_toString( stationType, "lvQuarter%d" ) + q.first );
        double              sin_stereo = std::sin( p.rotation_factor * p.stereo );
        double              cos_stereo = std::cos( p.rotation_factor * p.stereo );
        dd4hep::RotationZYX rot( p.rotation_factor * p.stereo, 0, 0 );
        dd4hep::DetElement  de_quarter( "quarter", id );
        dd4hep::DetElement  de, de_module;

        registerVolume( vol_quarter.name(), vol_quarter );
        registerDetElement( vol_quarter.name(), de_quarter );
        vol_quarter.setVisAttributes( description, "FT:Quarter" );

        dd4hep::printout( m_local_debug ? dd4hep::ALWAYS : dd4hep::DEBUG, "FT-geo",
                          "+++ Building quarter: %s sign:%c stereo:%.3f special:%s", vol_quarter.name(),
                          p.sign > 0. ? '+' : '-', p.stereo, p.special.c_str() );

        int nMod = 0;
        pos      = dd4hep::Position( p.sign * 0.5 * mod_pitch_x / cos_stereo - 0.5 * mod_size_y * sin_stereo,
                                mod_size_y * cos_stereo / 2.0, 0.0 );

        if ( m_build_MT ) {
          for ( const auto& h : HoleType ) {

            std::stringstream ss;
            ss << nMod;
            std::string strMod = ss.str();

            if ( nMod > 0 ) { pos.SetX( pos.X() + p.sign * mod_pitch_x / cos_stereo ); }
            pv = vol_quarter.placeVolume( volume( "lvFTModuleHole" + p.special + h.first ),
                                          dd4hep::Transform3D( rot, pos ) );
            de = detElement( pv.volume().name() );
            pv.addPhysVolID( "module", nMod );
            de_module = de.clone( ( "M" + strMod ).c_str(), nMod );
            de_module.setPlacement( pv );
            de_quarter.add( de_module );

            nMod++;

            if ( m_build_MT == 0 ) {
              for ( unsigned int i = 1; i < 4; i++ ) {
                std::stringstream ss;
                ss << i;
                std::string strMod = ss.str();

                pos.SetX( pos.X() + p.sign * mod_pitch_x / cos_stereo );
                pv = vol_quarter.placeVolume( vol_full, dd4hep::Transform3D( rot, pos ) );
                pv.addPhysVolID( "module", i );
                de_module = de_full.clone( ( "M" + strMod ).c_str(), i );
                de_module.setPlacement( pv );
                de_quarter.add( de_module );
              }
            }
          }
        } else {
          pv = vol_quarter.placeVolume( volume( "lvFTModuleHole" + p.special ), dd4hep::Transform3D( rot, pos ) );
          de = detElement( pv.volume().name() );
          pv.addPhysVolID( "module", 0 );
          de_module = de.clone( "M0", 0 );
          de_module.setPlacement( pv );
          de_quarter.add( de_module );

          pos.SetX( pos.X() + p.sign * mod_pitch_x / cos_stereo );
          pv = vol_quarter.placeVolume( vol_full, dd4hep::Transform3D( rot, pos ) );
          pv.addPhysVolID( "module", 1 );
          de_module = de_full.clone( "M1", 1 );
          de_module.setPlacement( pv );
          de_quarter.add( de_module );

          pos.SetX( pos.X() + p.sign * mod_pitch_x / cos_stereo );
          pv = vol_quarter.placeVolume( vol_full, dd4hep::Transform3D( rot, pos ) );
          pv.addPhysVolID( "module", 2 );
          de_module = de_full.clone( "M2", 2 );
          de_module.setPlacement( pv );
          de_quarter.add( de_module );

          pos.SetX( pos.X() + p.sign * mod_pitch_x / cos_stereo );
          pv = vol_quarter.placeVolume( vol_full, dd4hep::Transform3D( rot, pos ) );
          pv.addPhysVolID( "module", 3 );
          de_module = de_full.clone( "M3", 3 );
          de_module.setPlacement( pv );
          de_quarter.add( de_module );
        }
        pos.SetX( pos.X() + p.sign * mod_pitch_x / cos_stereo );
        pv = vol_quarter.placeVolume( vol_full, dd4hep::Transform3D( rot, pos ) );
        pv.addPhysVolID( "module", 4 );
        de_module = de_full.clone( "M4", 4 );
        de_module.setPlacement( pv );
        de_quarter.add( de_module );

        if ( stationType == 5 ) continue; // Distinguish between the quarters with 5 and 6 modules
        pos.SetX( pos.X() + p.sign * mod_pitch_x / cos_stereo );
        pv = vol_quarter.placeVolume( vol_full, dd4hep::Transform3D( rot, pos ) );
        pv.addPhysVolID( "module", 5 );
        de_module = de_full.clone( "M5", 5 );
        de_module.setPlacement( pv );
        de_quarter.add( de_module );
      }
    }
  }

  void FTBuild::build_half_layers() {

    struct HalfLayerType {
      double      rotY = 0;
      std::string A, B;
      std::string Side;
      HalfLayerType( double rotY, const std::string& a, const std::string& b, const std::string& side )
          : rotY( rotY ), A( a ), B( b ), Side( side ) {}
      HELPER_CLASS_DEFAULTS( HalfLayerType );
    };

    std::map<std::string, HalfLayerType> halflayers;
    halflayers["5UR"] = HalfLayerType( 0, "lvQuarter5UPos", "lvQuarter5UNeg", "R" );
    halflayers["5UL"] = HalfLayerType( 0, "lvQuarter5UPos", "lvQuarter5UNeg", "L" );
    halflayers["5VR"] = HalfLayerType( M_PI, "lvQuarter5UNeg", "lvQuarter5UPos", "R" );
    halflayers["5VL"] = HalfLayerType( M_PI, "lvQuarter5UNeg", "lvQuarter5UPos", "L" );

    halflayers["5X1R"] = HalfLayerType( M_PI, "lvQuarter5X1Neg", "lvQuarter5X1Pos", "R" );
    halflayers["5X1L"] = HalfLayerType( M_PI, "lvQuarter5X1Neg", "lvQuarter5X1Pos", "L" );
    halflayers["5X2R"] = HalfLayerType( 0, "lvQuarter5X1Pos", "lvQuarter5X1Neg", "R" );
    halflayers["5X2L"] = HalfLayerType( 0, "lvQuarter5X1Pos", "lvQuarter5X1Neg", "L" );

    halflayers["6UR"] = HalfLayerType( 0, "lvQuarter6UPos", "lvQuarter6UNeg", "R" );
    halflayers["6UL"] = HalfLayerType( 0, "lvQuarter6UPos", "lvQuarter6UNeg", "L" );
    halflayers["6VR"] = HalfLayerType( M_PI, "lvQuarter6UNeg", "lvQuarter6UPos", "R" );
    halflayers["6VL"] = HalfLayerType( M_PI, "lvQuarter6UNeg", "lvQuarter6UPos", "L" );

    halflayers["6X1R"] = HalfLayerType( M_PI, "lvQuarter6X1Neg", "lvQuarter6X1Pos", "R" );
    halflayers["6X1L"] = HalfLayerType( M_PI, "lvQuarter6X1Neg", "lvQuarter6X1Pos", "L" );
    halflayers["6X2R"] = HalfLayerType( 0, "lvQuarter6X1Pos", "lvQuarter6X1Neg", "R" );
    halflayers["6X2L"] = HalfLayerType( 0, "lvQuarter6X1Pos", "lvQuarter6X1Neg", "L" );

    dd4hep::Position    pos( 0, 0, 0 );
    dd4hep::RotationZYX rotQ01;
    dd4hep::RotationZYX rotQ23;

    dd4hep::PlacedVolume pv;
    dd4hep::Volume       vol;
    for ( const auto& hl : halflayers ) {
      const auto&         p = hl.second;
      dd4hep::Assembly    vol_halflayer( hl.first );
      dd4hep::RotationZYX rot( 0, 0, 0 );
      dd4hep::DetElement  de_halflayer( "halflayer", id );
      dd4hep::DetElement  de_quarter;

      registerVolume( vol_halflayer.name(), vol_halflayer );
      registerDetElement( vol_halflayer.name(), de_halflayer );
      vol_halflayer.setVisAttributes( description, "FT:HalfLayer" );
      dd4hep::printout( m_local_debug ? dd4hep::ALWAYS : dd4hep::DEBUG, "FT-geo",
                        "+++ Building half layer:   %-24s   A-type: %s   B-type: %s", vol_halflayer.name(), p.A.c_str(),
                        p.B.c_str() );

      if ( p.Side == "R" ) {

        // Rotation for Q0 and Q2 using geo quarter with Y>0
        rotQ01 = dd4hep::RotationZYX( M_PI, p.rotY, 0 );
        rotQ23 = dd4hep::RotationZYX( 0, p.rotY, 0 );

        pv = vol_halflayer.placeVolume( vol = volume( p.A ), dd4hep::Transform3D( rotQ01, pos ) );
        pv.addPhysVolID( "quarter", 0 );
        de_quarter = detElement( vol.name() ).clone( "Q0", 0 );
        de_quarter.setPlacement( pv );
        de_halflayer.add( de_quarter );

        pv = vol_halflayer.placeVolume( vol = volume( p.B ), dd4hep::Transform3D( rotQ23, pos ) );
        pv.addPhysVolID( "quarter", 2 );
        de_quarter = detElement( vol.name() ).clone( "Q2", 2 );
        de_quarter.setPlacement( pv );
        de_halflayer.add( de_quarter );
      }

      if ( p.Side == "L" ) {

        // Rotation for Q1 and Q3 using geo quarter with Y>0
        rotQ01 = dd4hep::RotationZYX( M_PI, p.rotY, 0 );
        rotQ23 = dd4hep::RotationZYX( 0, p.rotY, 0 );

        pv = vol_halflayer.placeVolume( vol = volume( p.B ), dd4hep::Transform3D( rotQ01, pos ) );
        pv.addPhysVolID( "quarter", 1 );
        de_quarter = detElement( vol.name() ).clone( "Q1", 1 );
        de_quarter.setPlacement( pv );
        de_halflayer.add( de_quarter );

        pv = vol_halflayer.placeVolume( vol = volume( p.A ), dd4hep::Transform3D( rotQ23, pos ) );
        pv.addPhysVolID( "quarter", 3 );
        de_quarter = detElement( vol.name() ).clone( "Q3", 3 );
        de_quarter.setPlacement( pv );
        de_halflayer.add( de_quarter );
      }
    }
  }

  void FTBuild::build_layers() {
    struct LayerType {
      double      rotY = 0;
      std::string A, B;
      LayerType( double rot, const std::string& a, const std::string& b ) : rotY( rot ), A( a ), B( b ) {}
      HELPER_CLASS_DEFAULTS( LayerType );
    };
    double                   beam_angle = dd4hep::_toDouble( "Regions:TBeamAngle" );
    dd4hep::PlacedVolume     pv;
    dd4hep::SubtractionSolid layer_solid;
    {
      dd4hep::Box         box( station_size_x / 2.0, station_size_y / 2.0, mod_size_z / 2.0 + eps );
      dd4hep::Tube        tub( 0, beam_hole_radius + 0.5 * std::tan( beam_angle ) * layer_pitch_z + eps,
                        mod_size_z / cos( beam_angle ) + std::tan( beam_angle ) * beam_hole_radius + eps );
      dd4hep::RotationZYX rot( 0, 0, -beam_angle );
      dd4hep::Position    pos0( 0, 0, 0 );

      layer_solid = dd4hep::SubtractionSolid( box, tub, dd4hep::Transform3D( rot, pos0 ) );
    }

    std::map<std::string, LayerType> layers;
    layers["5U"]  = LayerType( 0, "5UR", "5UL" );
    layers["5V"]  = LayerType( 0, "5VR", "5VL" );
    layers["5X1"] = LayerType( 0, "5X1R", "5X1L" );
    layers["5X2"] = LayerType( 0, "5X2R", "5X2L" );
    layers["6U"]  = LayerType( 0, "6UR", "6UL" );
    layers["6V"]  = LayerType( 0, "6VR", "6VL" );
    layers["6X1"] = LayerType( 0, "6X1R", "6X1L" );
    layers["6X2"] = LayerType( 0, "6X2R", "6X2L" );

    dd4hep::Position pos;
    dd4hep::Volume   vol;
    for ( const auto& l : layers ) {
      const auto& p = l.second;
      // This volume declaration makes Geant4 extremely unhappy!!!!
      // Assembly   vol_layer("lvLayer"+l.first);
      dd4hep::Volume     vol_layer( "lvLayer" + l.first, layer_solid, description.air() );
      dd4hep::DetElement de_layer( l.first, id );
      dd4hep::DetElement de_halflayer;

      registerVolume( vol_layer.name(), vol_layer );
      registerDetElement( vol_layer.name(), de_layer );
      vol_layer.setVisAttributes( description, "FT:Layer" );
      dd4hep::printout( m_local_debug ? dd4hep::ALWAYS : dd4hep::DEBUG, "FT-geo",
                        "+++ Building layer:   %-24s   A-type: %s   B-type: %s", vol_layer.name(), p.A.c_str(),
                        p.B.c_str() );
      pv = vol_layer.placeVolume( vol = volume( p.A ), dd4hep::RotationZYX( 0, 0, 0 ) );
      pv.addPhysVolID( "halflayer", 0 );
      de_halflayer = detElement( vol.name() ).clone( "HL0", 0 );
      de_halflayer.setPlacement( pv );
      de_layer.add( de_halflayer );

      pv = vol_layer.placeVolume( vol = volume( p.B ), dd4hep::RotationZYX( 0, 0, 0 ) );
      pv.addPhysVolID( "halflayer", 1 );
      de_halflayer = detElement( vol.name() ).clone( "HL1", 1 );
      de_halflayer.setPlacement( pv );
      de_layer.add( de_halflayer );
    }
  }

  void FTBuild::build_stations() {
    double              beam_angle = dd4hep::_toDouble( "Regions:TBeamAngle" );
    dd4hep::Box         box( station_size_x / 2.0,
                     station_size_y / 2.0 + std::tan( beam_angle ) * ( 0.5 * ( cframe_pitch_z + layer_pitch_z ) ),
                     station_size_z / 2.0 );
    dd4hep::Tube        tub( 0.0, beam_hole_radius + eps,
                      station_size_z / ( 2.0 * cos( beam_angle ) ) + std::tan( beam_angle ) * beam_hole_radius + eps );
    dd4hep::RotationZYX rot( 0, 0, -beam_angle );
    dd4hep::Position    pos0( 0, 0, 0 );

    dd4hep::SubtractionSolid solid( box, tub, dd4hep::Transform3D( rot, pos0 ) );
    dd4hep::PlacedVolume     pv;
    dd4hep::Position         pos;
    dd4hep::Volume           vol;
    for ( int i = 5; i <= 6; ++i ) {
      dd4hep::Volume     vol_station( dd4hep::_toString( i, "lvStation%d" ), solid, description.air() );
      dd4hep::DetElement de_station( "station", id );
      registerDetElement( vol_station.name(), de_station );
      registerVolume( vol_station.name(), vol_station );

      dd4hep::printout( m_local_debug ? dd4hep::ALWAYS : dd4hep::DEBUG, "FT-geo", "+++ Building station type: %s",
                        vol_station.name() );

      if ( m_build_frames ) { /// Need to place CFrame here
        vol_station.placeVolume(
            volume( "lvCFramePair" ),
            dd4hep::Position( 0, 0.5 * cframe_shift_y + std::tan( beam_angle ) * ( -0.5 * cframe_pitch_z ),
                              -0.5 * cframe_pitch_z ) );
        vol_station.placeVolume(
            volume( "lvCFramePair" ),
            dd4hep::Position( 0, 0.5 * cframe_shift_y + std::tan( beam_angle ) * ( 0.5 * cframe_pitch_z ),
                              0.5 * cframe_pitch_z ) );
      }

      dd4hep::RotationZYX rotate_along_Y( 0, 0, 0 );

      pos = dd4hep::Position( 0, std::tan( beam_angle ) * ( -0.5 * ( cframe_pitch_z ) ),
                              -0.5 * ( cframe_pitch_z + ( m_build_MT ? layer_x_pitch_z : layer_pitch_z ) ) );

      pv = vol_station.placeVolume( vol = volume( dd4hep::_toString( i, "lvLayer%dX1" ) ),
                                    dd4hep::Transform3D( rotate_along_Y, pos ) );
      pv.addPhysVolID( "layer", 0 );
      dd4hep::DetElement de_layer0 = detElement( vol.name() ).clone( "X1", 0 );
      de_layer0.setPlacement( pv );
      de_station.add( de_layer0 );
      dd4hep::printout( m_local_debug ? dd4hep::ALWAYS : dd4hep::DEBUG, "FT-geo", "Layer 0 z=%.3f -z=%.3f +z=%.3f [cm]",
                        pos.Z(), pos.Z() - pv.volume().boundingBox().z(), pos.Z() + pv.volume().boundingBox().z() );

      pos = dd4hep::Position( 0, std::tan( beam_angle ) * ( -0.5 * ( cframe_pitch_z ) ),
                              -0.5 * ( cframe_pitch_z - ( m_build_MT ? layer_uv_pitch_z : layer_pitch_z ) ) );

      pv = vol_station.placeVolume( vol = volume( dd4hep::_toString( i, "lvLayer%dU" ) ), pos );
      pv.addPhysVolID( "layer", 1 );
      dd4hep::DetElement de_layer1 = detElement( vol.name() ).clone( "U", 1 );
      de_layer1.setPlacement( pv );
      de_station.add( de_layer1 );
      dd4hep::printout( m_local_debug ? dd4hep::ALWAYS : dd4hep::DEBUG, "FT-geo", "Layer 1 z=%.3f -z=%.3f +z=%.3f [cm]",
                        pos.Z(), pos.Z() - pv.volume().boundingBox().z(), pos.Z() + pv.volume().boundingBox().z() );

      pos = dd4hep::Position( 0, std::tan( beam_angle ) * ( 0.5 * ( cframe_pitch_z ) ),
                              +0.5 * ( cframe_pitch_z - ( m_build_MT ? layer_uv_pitch_z : layer_pitch_z ) ) );

      pv = vol_station.placeVolume( vol = volume( dd4hep::_toString( i, "lvLayer%dV" ) ),
                                    dd4hep::Transform3D( rotate_along_Y, pos ) );
      pv.addPhysVolID( "layer", 2 );
      dd4hep::DetElement de_layer2 = detElement( vol.name() ).clone( "V", 2 );
      de_layer2.setPlacement( pv );
      de_station.add( de_layer2 );
      dd4hep::printout( m_local_debug ? dd4hep::ALWAYS : dd4hep::DEBUG, "FT-geo", "Layer 2 z=%.3f -z=%.3f +z=%.3f [cm]",
                        pos.Z(), pos.Z() - pv.volume().boundingBox().z(), pos.Z() + pv.volume().boundingBox().z() );

      pos = dd4hep::Position( 0, std::tan( beam_angle ) * ( 0.5 * ( cframe_pitch_z ) ),
                              +0.5 * ( cframe_pitch_z + ( m_build_MT ? layer_x_pitch_z : layer_pitch_z ) ) );

      pv = vol_station.placeVolume( vol = volume( dd4hep::_toString( i, "lvLayer%dX2" ) ), pos );
      pv.addPhysVolID( "layer", 3 );
      dd4hep::DetElement de_layer3 = detElement( vol.name() ).clone( "X2", 3 );
      de_layer3.setPlacement( pv );
      de_station.add( de_layer3 );
      dd4hep::printout( m_local_debug ? dd4hep::ALWAYS : dd4hep::DEBUG, "FT-geo", "Layer 3 z=%.3f -z=%.3f +z=%.3f [cm]",
                        pos.Z(), pos.Z() - pv.volume().boundingBox().z(), pos.Z() + pv.volume().boundingBox().z() );
    }
  }
  void FTBuild::build_detector() {
    dd4hep::PlacedVolume     pv;
    double                   tm_size_x  = dd4hep::_toDouble( "Regions:TTMSizeX" );
    double                   tm_size_y  = dd4hep::_toDouble( "Regions:TTMSizeY" );
    double                   tm_size_z  = dd4hep::_toDouble( "Regions:TTMSizeZ" );
    double                   beam_angle = dd4hep::_toDouble( "Regions:TBeamAngle" );
    dd4hep::Box              ft_box1( tm_size_x / 2.0, tm_size_y / 2.0, tm_size_z / 2.0 );
    dd4hep::Box              ft_box2( ( tm_size_x + eps ) / 2.0, ( tm_size_y + eps ) / 2.0, "10*mm" );
    dd4hep::Tube             ft_tube( 0, beam_hole_radius, ( tm_size_z + eps ) / 2.0, 2.0 * M_PI );
    dd4hep::SubtractionSolid ft_solid( ft_box1, ft_tube );
    dd4hep::RotationZYX      rot( 0, 0, beam_angle );
    dd4hep::Position         pos( 0, -beam_hole_radius, ( tm_size_z + 20.0 * dd4hep::mm ) / 2.0 );
    dd4hep::Assembly         ft_vol( "lvFT" );
    dd4hep::Volume           vol;
    dd4hep::DetElement       de;

    ft_vol.setVisAttributes( description.invisible() );
    sensitive.setType( "tracker" );
    buildVolumes( x_det );
    placeDaughters( detector, ft_vol, x_det );
    build_mats();
    build_modules();
    if ( m_build_quarters ) {
      build_quarters();
      if ( m_build_half_layers ) {
        build_half_layers();
        if ( m_build_layers ) {
          build_layers();
          if ( m_build_stations ) { build_stations(); }
        }
      }
    }
    if ( m_attach_volume.empty() && m_build_stations ) {
      dd4hep::Volume     vol_station5 = volume( "lvStation5" );
      dd4hep::Volume     vol_station6 = volume( "lvStation6" );
      dd4hep::DetElement de_station;

      pos = dd4hep::Position( 0, 0, dd4hep::_toDouble( "FT:T1ZPos" ) );
      rot = dd4hep::RotationZYX( 0, 0, beam_angle );
      pv  = ft_vol.placeVolume( vol_station5, dd4hep::Transform3D( rot, pos ) );
      pv.addPhysVolID( "station", 0 );
      de_station = detElement( vol_station5.name() ).clone( "T1", 0 );
      de_station.setPlacement( pv );
      detector.add( de_station );
      dd4hep::printout( m_local_debug ? dd4hep::ALWAYS : dd4hep::DEBUG, "FT-geo", "+++ Position station T1 of type %s",
                        vol_station5.name() );

      pos = dd4hep::Position( 0, 0, dd4hep::_toDouble( "FT:T2ZPos" ) );
      pv  = ft_vol.placeVolume( vol_station5, dd4hep::Transform3D( rot, pos ) );
      pv.addPhysVolID( "station", 1 );
      de_station = detElement( vol_station5.name() ).clone( "T2", 1 );
      de_station.setPlacement( pv );
      detector.add( de_station );
      dd4hep::printout( m_local_debug ? dd4hep::ALWAYS : dd4hep::DEBUG, "FT-geo", "+++ Position station T2 of type %s",
                        vol_station5.name() );

      pos = dd4hep::Position( 0, 0, dd4hep::_toDouble( "FT:T3ZPos" ) );
      pv  = ft_vol.placeVolume( vol_station6, dd4hep::Transform3D( rot, pos ) );
      pv.addPhysVolID( "station", 2 );
      de_station = detElement( vol_station6.name() ).clone( "T3", 2 );
      de_station.setPlacement( pv );
      detector.add( de_station );
      dd4hep::printout( m_local_debug ? dd4hep::ALWAYS : dd4hep::DEBUG, "FT-geo", "+++ Position station T3 of type %s",
                        vol_station6.name() );
    } else if ( !m_attach_volume.empty() ) {
      // Test display of individual entities
      ft_vol.placeVolume( vol = volume( m_attach_volume ) );
      de = detElement( vol.name() );
      detector.add( de.clone( m_attach_volume ) );
    } else {
      dd4hep::except( "FT-geo", "+++ Inconsistent build flags. "
                                "If no specific volume should be ejected, the stations must be built!" );
    }
    xml_h x_tr = x_det.child( _U( transformation ) );
    pv         = placeDetector( ft_vol, x_tr );
    pv.addPhysVolID( "system", id );
  }
} // namespace

static dd4hep::Ref_t create_element( dd4hep::Detector& description, xml_h e, dd4hep::SensitiveDetector sens_det ) {
  FTBuild builder( description, e, sens_det );
  builder.build_detector();
  return builder.detector;
}
DECLARE_DETELEMENT( LHCb_FT_geo_v1_0, create_element )
