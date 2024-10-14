/*****************************************************************************\
* (c) Copyright 2020 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#include "Core/UpgradeTags.h"
#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/Printout.h"
#include "DD4hep/detail/DetectorInterna.h"
#include "TClass.h"
#include "XML/Utilities.h"
#include <DD4hep/DetFactoryHelper.h>
#include <XML/Layering.h>
#include <iostream>

using namespace std;
using namespace dd4hep;

namespace {

  struct HcalBuild : public dd4hep::xml::tools::VolumeBuilder {
    bool               m_build_inner_cell_workparts = true;
    bool               m_build_outer_cell_workparts = true;
    bool               m_build_inner_cells          = true;
    bool               m_build_outer_cells          = true;
    bool               m_build_inner_submodules     = true;
    bool               m_build_outer_submodules     = true;
    bool               m_build_inner_modules        = true;
    bool               m_build_outer_modules        = true;
    bool               m_build_HcalLeft             = true;
    bool               m_build_HcalRight            = true;
    bool               m_build_Hcal                 = true;
    bool               m_build_back_shield          = true;
    bool               m_build_BeamPlug             = true;
    bool               m_build_BeamPlugCarcass      = true;

    dd4hep::DetElement deScTile;
    int                count_ScTiles = 0;

    dd4hep::DetElement deLeft, deRight;
    dd4hep::DetElement deInnRegionLeft, deInnRegionRight;
    dd4hep::DetElement deInnModule, deInnMidlModuleLeft, deInnMidlModuleRight, deInnModuleTmp;
    dd4hep::DetElement deInnSubModule, deInnSubModuleTmp;
    dd4hep::DetElement deInnCell, deInnCellTmp;
    dd4hep::DetElement deOutRegionLeft, deOutRegionRight;
    dd4hep::DetElement deOutModule, deOutModuleTmp;
    dd4hep::DetElement deOutSubModule, deOutSubModuleTmp;
    dd4hep::DetElement deOutCell, deOutCellTmp;

    HcalBuild( dd4hep::Detector& description, xml_elt_t e, dd4hep::SensitiveDetector sens );
    dd4hep::Polycone get_PipeHole();
    void           build_beam_plug( std::string side_name );
    void           build_inner_cell();
    void           build_inner_section();
    void           build_outer_cell();
    void           build_outer_section();
    void           build_back_shield_bricks();
    std::string 	 build_back_shield( std::string side_name );
    dd4hep::Volume build_hcal();
  };

  // ******************************************************
  // Initializing constructor
  // ******************************************************

  HcalBuild::HcalBuild( dd4hep::Detector& dsc, xml_elt_t e, dd4hep::SensitiveDetector sens )
      : dd4hep::xml::tools::VolumeBuilder( dsc, e, sens ) {
    // Process debug flags
    xml_comp_t x_dbg = x_det.child( _U( debug ), false );
    if ( x_dbg ) {
      for ( xml_coll_t i( x_dbg, _U( item ) ); i; ++i ) {
        xml_comp_t  c( i );
        std::string n = c.nameStr();
        if ( n == "debug" )
          debug = c.attr<int>( _U( value ) ) != 0;
        else if ( n == "HcalBuildInnerCellWorkParts" )
          m_build_inner_cell_workparts = c.attr<int>( _U( value ) ) != 0;
        else if ( n == "HcalBuildOuterCellWorkParts" )
          m_build_outer_cell_workparts = c.attr<int>( _U( value ) ) != 0;
        else if ( n == "HcalBuildInnerCells" )
          m_build_inner_cells = c.attr<int>( _U( value ) ) != 0;
        else if ( n == "HcalBuildOuterCells" )
          m_build_outer_cells = c.attr<int>( _U( value ) ) != 0;
        else if ( n == "HcalBuildInnerSubModules" )
          m_build_inner_submodules = c.attr<int>( _U( value ) ) != 0;
        else if ( n == "HcalBuildOuterSubModules" )
          m_build_outer_submodules = c.attr<int>( _U( value ) ) != 0;
        else if ( n == "HcalBuildInnerModules" )
          m_build_inner_modules = c.attr<int>( _U( value ) ) != 0;
        else if ( n == "HcalBuildOuterModules" )
          m_build_outer_modules = c.attr<int>( _U( value ) ) != 0;
        else if ( n == "HcalBuildBackShield" )
          m_build_back_shield = c.attr<int>( _U( value ) ) != 0;
        else if ( n == "HcalBuildBeamPlug" )
          m_build_BeamPlug = c.attr<int>( _U( value ) ) != 0;
        else if ( n == "HcalBuildBeamPlugCarcass" )
          m_build_BeamPlugCarcass = c.attr<int>( _U( value ) ) != 0;

      }
    }
    if ( !m_build_inner_modules ) { m_build_inner_submodules = false; };
    if ( !m_build_inner_submodules ) { m_build_inner_cells = false; };
    if ( !m_build_inner_cells ) { m_build_inner_cell_workparts = false; };
    if ( !m_build_outer_modules ) { m_build_outer_submodules = false; };
    if ( !m_build_outer_submodules ) { m_build_outer_cells = false; };
    if ( !m_build_outer_cells ) { m_build_outer_cell_workparts = false; };
    if ( !m_build_BeamPlug ) { m_build_BeamPlugCarcass = false; }
  }

  dd4hep::Polycone HcalBuild::get_PipeHole(){
    std::vector<double> rmin, rmax, z;
    for ( int i = 1; i <= 12; ++i ) {
      double BeamPlugPipeHoleR_i = dd4hep::_toDouble( Form( "HcalBeamPlugPipeHoleR%d", i ) );
      double BeamPlugPipeHoleZ_i = dd4hep::_toDouble( Form( "HcalBeamPlugPipeHoleZ%d", i ) );
      rmin.push_back( 0.0 );
      rmax.push_back( BeamPlugPipeHoleR_i );
      z.push_back( BeamPlugPipeHoleZ_i );
    }
    dd4hep::Polycone PipeHole( 0, 2 * M_PI, rmin, rmax, z );
    return PipeHole;
  };

  void HcalBuild::build_beam_plug( std::string side_name ) {
    double      iside = 0;
    std::string name_PlugSupport, name_StSheetWithHole;
    if ( side_name == "left" ) {
      iside                = 1;
      name_PlugSupport     = "lvPlugSupportLeft";
      name_StSheetWithHole = "lvStSheetWithHoleL";
    } else if ( side_name == "right" ) {
      iside                = -1;
      name_PlugSupport     = "lvPlugSupportRight ";
      name_StSheetWithHole = "lvStSheetWithHoleR";
    } else {
      // TODO: throw error from here
    }

    double TiltAngle             = dd4hep::_toDouble( "HcalTiltAngle" );
    double BeamPlugXSize         = dd4hep::_toDouble( "HcalBeamPlugXSize" );
    double BeamPlugYSize         = dd4hep::_toDouble( "HcalBeamPlugYSize" );
    double BeamPlugZSize         = dd4hep::_toDouble( "HcalBeamPlugZSize" );
    double YShift                = dd4hep::_toDouble( "HcalYShift" );
    double SteelSheetThick       = dd4hep::_toDouble( "HcalSteelSheetThick" );
    double Delta                 = dd4hep::_toDouble( "HcalDelta" );
    double SteelSheetFrontRadius = dd4hep::_toDouble( "HcalSteelSheetFrontRadius" );
    double SteelSheetRibRadius   = dd4hep::_toDouble( "HcalSteelSheetRibRadius" );
    double SteelSheetBackRadius  = dd4hep::_toDouble( "HcalSteelSheetBackRadius" );
    double SteelSheetRibHoleZ1   = dd4hep::_toDouble( "HcalSteelSheetRibHoleZ1" );
    double SteelSheetRibHoleZ2   = dd4hep::_toDouble( "HcalSteelSheetRibHoleZ2" );
    double SteelSheetRibHoleZ3   = dd4hep::_toDouble( "HcalSteelSheetRibHoleZ3" );
    double SteelSheetRibHoleZ4   = dd4hep::_toDouble( "HcalSteelSheetRibHoleZ4" );

    // ******************************************************
    // Build plug support
    // ******************************************************

    dd4hep::Box              Plugbox( 0.5 * BeamPlugXSize / 2, BeamPlugYSize / 2, BeamPlugZSize / 2 );
    dd4hep::Polycone         PipeHole = get_PipeHole();
    dd4hep::RotationZYX      rot_Plug( 0., 0., -TiltAngle );
    dd4hep::Position         pos_Plug( -iside * BeamPlugXSize * 0.25, -YShift, 0. );
    dd4hep::SubtractionSolid PlugWithHole( Plugbox, PipeHole, dd4hep::Transform3D( rot_Plug, pos_Plug ) );
    dd4hep::Volume           lvPlugSupportVol( name_PlugSupport, PlugWithHole, description.material( "CaloPb" ) );
    lvPlugSupportVol.setVisAttributes( description, "vHcal:PbTiles" );

    // ******************************************************
    // Build steel sheets with hole (1) front, (2) rib, (3) back
    // ******************************************************
    dd4hep::Box              StSheet( 0.5 * ( BeamPlugXSize - 2 * SteelSheetThick) / 2,
                                            ( BeamPlugYSize - 2 * SteelSheetThick) / 2,
                                            SteelSheetThick / 2 );
    dd4hep::Position         pos_Hole( -iside * 0.25 * ( BeamPlugXSize - 2 * SteelSheetThick), 0, 0 );

    dd4hep::Tube             CylinderHoleFront( 0, SteelSheetFrontRadius, ( SteelSheetThick + Delta ) / 2 );
    dd4hep::SubtractionSolid StSheetWithHoleFront( StSheet, CylinderHoleFront, pos_Hole );
    dd4hep::Volume           lvSteelSheetWithHoleFrontVol( name_StSheetWithHole+"Front", StSheetWithHoleFront,
                                                           description.material( "CaloSteel" ) );
    lvSteelSheetWithHoleFrontVol.setVisAttributes( description, "vHcal:Steel" );

    dd4hep::Tube             CylinderHoleRib( 0, SteelSheetRibRadius, ( SteelSheetThick + Delta ) / 2 );
    dd4hep::SubtractionSolid StSheetWithHoleRib( StSheet, CylinderHoleRib, pos_Hole );
    dd4hep::Volume           lvSteelSheetWithHoleRibVol( name_StSheetWithHole + "Rib", StSheetWithHoleRib,
                                                         description.material( "CaloSteel" ) );
    lvSteelSheetWithHoleRibVol.setVisAttributes( description, "vHcal:Steel" );

    dd4hep::Tube             CylinderHoleBack( 0, SteelSheetBackRadius, ( SteelSheetThick + Delta ) / 2 );
    dd4hep::SubtractionSolid StSheetWithHoleBack( StSheet, CylinderHoleBack, pos_Hole );
    dd4hep::Volume           lvSteelSheetWithHoleBackVol( name_StSheetWithHole+"Back", StSheetWithHoleBack,
                                                          description.material( "CaloSteel" ) );
    lvSteelSheetWithHoleBackVol.setVisAttributes( description, "vHcal:Steel" );

    // ******************************************************
    // Build horizontal steel sheet
    // ******************************************************
    dd4hep::Box    HSteelSheetBox( 0.5 * BeamPlugXSize / 2, SteelSheetThick / 2, BeamPlugZSize / 2 );
    dd4hep::Volume lvHorizontalSteelSheetVol =
        dd4hep::Volume( "lvHorizontalSteelSheet", HSteelSheetBox, description.material( "CaloSteel" ) );
    lvHorizontalSteelSheetVol.setVisAttributes( description, "vHcal:Steel" );

    // ******************************************************
    // Build vertical steel sheet
    // ******************************************************
    dd4hep::Box VSteelSheetBox( SteelSheetThick / 2,
                                ( BeamPlugYSize - 2.0 * SteelSheetThick ) / 2,
                                ( BeamPlugZSize ) / 2 );
    dd4hep::Volume lvVerticalSteelSheetVol =
        dd4hep::Volume( "lvVerticalSteelSheet", VSteelSheetBox, description.material( "CaloSteel" ) );
    lvVerticalSteelSheetVol.setVisAttributes( description, "vHcal:Steel" );

    // ******************************************************
    // Insert steel sheets into plug support
    // ******************************************************
    if (m_build_BeamPlugCarcass){
      lvPlugSupportVol.placeVolume( lvHorizontalSteelSheetVol,
                                    dd4hep::Position( 0, -0.5 * ( BeamPlugYSize - SteelSheetThick ), 0 ) );
      lvPlugSupportVol.placeVolume( lvHorizontalSteelSheetVol,
                                    dd4hep::Position( 0, +0.5 * ( BeamPlugYSize - SteelSheetThick ), 0 ) );
      lvPlugSupportVol.placeVolume( lvVerticalSteelSheetVol,
                                    dd4hep::Position( iside * 0.25 * ( BeamPlugXSize - 2 * SteelSheetThick ), 0, 0 ) );
      lvPlugSupportVol.placeVolume( lvSteelSheetWithHoleFrontVol,
                                    dd4hep::Position( -iside * 0.5 * SteelSheetThick, 0, -0.5 * ( BeamPlugZSize - SteelSheetThick ) ) );
      lvPlugSupportVol.placeVolume( lvSteelSheetWithHoleRibVol,
                                    dd4hep::Position( -iside * 0.5 * SteelSheetThick, 0, SteelSheetRibHoleZ1 ) );
      lvPlugSupportVol.placeVolume( lvSteelSheetWithHoleRibVol,
                                    dd4hep::Position( -iside * 0.5 * SteelSheetThick, 0, SteelSheetRibHoleZ2 ) );
      lvPlugSupportVol.placeVolume( lvSteelSheetWithHoleRibVol,
                                    dd4hep::Position( -iside * 0.5 * SteelSheetThick, 0, SteelSheetRibHoleZ3 ) );
      lvPlugSupportVol.placeVolume( lvSteelSheetWithHoleRibVol,
                                    dd4hep::Position( -iside * 0.5 * SteelSheetThick, 0, SteelSheetRibHoleZ4 ) );
      lvPlugSupportVol.placeVolume( lvSteelSheetWithHoleBackVol,
                                    dd4hep::Position( -iside * 0.5 * SteelSheetThick, 0, +0.5 * ( BeamPlugZSize - SteelSheetThick ) ) );
    }
    registerVolume( lvPlugSupportVol.name(), lvPlugSupportVol );
  }

  void HcalBuild::build_inner_cell() {
    dd4hep::PlacedVolume pv;

    // ******************************************************
    // Loading parameters
    // ******************************************************

    double SubModXSize            = dd4hep::_toDouble( "HcalSubModXSize" );
    double SubModYSize            = dd4hep::_toDouble( "HcalSubModYSize" );
    double FaceWidth_for_CntStrip = dd4hep::_toDouble( "HcalFaceWidth_for_CntStrip" );
    double FwFaceShelfWidth       = dd4hep::_toDouble( "HcalFwFaceShelfWidth" );
    double FwFaceLength           = dd4hep::_toDouble( "HcalFwFaceLength" );
    double FwFaceShelfLength      = dd4hep::_toDouble( "HcalFwFaceShelfLength" );
    double CntStripWidth          = dd4hep::_toDouble( "HcalCntStripWidth" );
    double FwOutStripHeight       = dd4hep::_toDouble( "HcalFwOutStripHeight" );
    double FwOutStripWidth        = dd4hep::_toDouble( "HcalFwOutStripWidth" );
    double BkFaceTotLength        = dd4hep::_toDouble( "HcalBkFaceTotLength" );
    double BkFaceLength           = dd4hep::_toDouble( "HcalBkFaceLength" );
    double BkFaceShelfLength      = dd4hep::_toDouble( "HcalBkFaceShelfLength" );
    double BkFaceShelfWidth       = dd4hep::_toDouble( "HcalBkFaceShelfWidth" );
    double BkOutStripHeight       = dd4hep::_toDouble( "HcalBkOutStripHeight" );
    double BkOutStripWidth        = dd4hep::_toDouble( "HcalBkOutStripWidth" );
    double MsPlWidth              = dd4hep::_toDouble( "HcalMsPlWidth" );
    double MsPlHeight             = dd4hep::_toDouble( "HcalMsPlHeight" );
    double MsPlStepLength         = dd4hep::_toDouble( "HcalMsPlStepLength" );
    double MsPlCalHoleR           = dd4hep::_toDouble( "HcalMsPlCalHoleR" );
    double Rotation90             = dd4hep::_toDouble( "HcalRotation90" );
    double ScTileWidth            = dd4hep::_toDouble( "HcalScTileWidth" );
    double ScTileHeigth           = dd4hep::_toDouble( "HcalScTileHeigth" );
    double ScTileSlitWidth        = dd4hep::_toDouble( "HcalScTileSlitWidth" );
    double ScTileLength           = dd4hep::_toDouble( "HcalScTileLength" );
    double ScTileCalHoleR         = dd4hep::_toDouble( "HcalScTileCalHoleR" );
    double SpacerWidth            = dd4hep::_toDouble( "HcalSpacerWidth" );
    double SpacerHeigth           = dd4hep::_toDouble( "HcalSpacerHeigth" );
    double SpacerLength           = dd4hep::_toDouble( "HcalSpacerLength" );
    double CellPeriodWidth        = dd4hep::_toDouble( "HcalCellPeriodWidth" );
    double InnCellXPos1stLayer    = dd4hep::_toDouble( "HcalInnCellXPos1stLayer" );
    double InnCellXPos2ndLayer    = dd4hep::_toDouble( "HcalInnCellXPos2ndLayer" );
    double InnCellXSize           = dd4hep::_toDouble( "HcalInnCellXSize" );
    double Tolerance              = dd4hep::_toDouble( "HcalTolerance" );

    // ******************************************************
    // Building the forward face of submodule
    // ******************************************************

    dd4hep::Box    HcalFwFaceSubModuleBox( 0.5 * SubModXSize / 2, SubModYSize / 2, FwFaceLength / 2 );
    dd4hep::Volume lvHcalFwFaceSubModuleVol( "lvHcalFwFaceSubModule", HcalFwFaceSubModuleBox,
                                             description.material( "Air" ) );
    lvHcalFwFaceSubModuleVol.setVisAttributes( description, "vHcal:Air" );

    dd4hep::Volume lvHcalFwFaceShelfWorkStuffVol = volume( "lvHcalFwFaceShelfWorkStuff" );
    lvHcalFwFaceShelfWorkStuffVol.setVisAttributes( description, "vHcal:MsPlSteel" );
    dd4hep::Volume lvHcalFwFaceWorkStuffVol = volume( "lvHcalFwFaceWorkStuff" );
    lvHcalFwFaceWorkStuffVol.setVisAttributes( description, "vHcal:MsPlSteel" );
    dd4hep::Volume lvHcalCntStripVol = volume( "lvHcalCntStrip" );
    lvHcalCntStripVol.setVisAttributes( description, "vHcal:StripSteel" );
    dd4hep::Volume lvHcalFwOutStripVol = volume( "lvHcalFwOutStrip" );
    lvHcalFwOutStripVol.setVisAttributes( description, "vHcal:StripSteel" );

    lvHcalFwFaceSubModuleVol.placeVolume( lvHcalFwFaceShelfWorkStuffVol,
                                          dd4hep::Position( 0, 0.5 * FaceWidth_for_CntStrip + 0.5 * FwFaceShelfWidth,
                                                            -0.5 * FwFaceLength + 0.5 * FwFaceShelfLength ) );
    lvHcalFwFaceSubModuleVol.placeVolume( lvHcalFwFaceShelfWorkStuffVol,
                                          dd4hep::Position( 0, -0.5 * FaceWidth_for_CntStrip - 0.5 * FwFaceShelfWidth,
                                                            -0.5 * FwFaceLength + 0.5 * FwFaceShelfLength ) );
    lvHcalFwFaceSubModuleVol.placeVolume(
        lvHcalFwFaceWorkStuffVol,
        dd4hep::Position( 0, 0.25 * FaceWidth_for_CntStrip + 0.25 * MsPlHeight, 0.5 * FwFaceShelfLength ) );
    lvHcalFwFaceSubModuleVol.placeVolume(
        lvHcalFwFaceWorkStuffVol,
        dd4hep::Position( 0, -0.25 * FaceWidth_for_CntStrip - 0.25 * MsPlHeight, 0.5 * FwFaceShelfLength ) );
    lvHcalFwFaceSubModuleVol.placeVolume( lvHcalCntStripVol,
                                          dd4hep::Position( 0, 0, 0.5 * FwFaceLength - 0.5 * CntStripWidth ) );
    lvHcalFwFaceSubModuleVol.placeVolume(
        lvHcalFwOutStripVol,
        dd4hep::Position( 0, 0.5 * FaceWidth_for_CntStrip + FwFaceShelfWidth + 0.5 * FwOutStripHeight,
                          -0.5 * FwFaceLength + FwFaceShelfLength - 0.5 * FwOutStripWidth ) );
    lvHcalFwFaceSubModuleVol.placeVolume(
        lvHcalFwOutStripVol,
        dd4hep::Position( 0, -0.5 * FaceWidth_for_CntStrip - FwFaceShelfWidth - 0.5 * FwOutStripHeight,
                          -0.5 * FwFaceLength + FwFaceShelfLength - 0.5 * FwOutStripWidth ) );

    registerVolume( lvHcalFwFaceSubModuleVol.name(), lvHcalFwFaceSubModuleVol );

    // ******************************************************
    // Building the backward face of submodule
    // lvHcalCntStrip has been imported above
    // ******************************************************

    dd4hep::Box    HcalBkFaceSubModuleBox( 0.5 * SubModXSize / 2, SubModYSize / 2, BkFaceTotLength / 2 );
    dd4hep::Volume lvHcalBkFaceSubModuleVol( "lvHcalBkFaceSubModule", HcalBkFaceSubModuleBox,
                                             description.material( "Air" ) );
    lvHcalBkFaceSubModuleVol.setVisAttributes( description, "vHcal:Air" );

    dd4hep::Volume lvHcalBkFaceShelfWorkStuffVol = volume( "lvHcalBkFaceShelfWorkStuff" );
    lvHcalBkFaceShelfWorkStuffVol.setVisAttributes( description, "vHcal:MsPlSteel" );
    dd4hep::Volume lvHcalBkFaceWorkStuffVol = volume( "lvHcalBkFaceWorkStuff" );
    lvHcalBkFaceWorkStuffVol.setVisAttributes( description, "vHcal:MsPlSteel" );
    dd4hep::Volume lvHcalBkOutStripVol = volume( "lvHcalBkOutStrip" );
    lvHcalBkOutStripVol.setVisAttributes( description, "vHcal:StripSteel" );

    lvHcalBkFaceSubModuleVol.placeVolume(
        lvHcalBkFaceShelfWorkStuffVol,
        dd4hep::Position( 0, 0.5 * FaceWidth_for_CntStrip + 0.5 * BkFaceShelfWidth,
                          -0.5 * BkFaceTotLength + BkFaceLength - 0.5 * BkFaceShelfLength ) );
    lvHcalBkFaceSubModuleVol.placeVolume(
        lvHcalBkFaceShelfWorkStuffVol,
        dd4hep::Position( 0, -0.5 * FaceWidth_for_CntStrip - 0.5 * BkFaceShelfWidth,
                          -0.5 * BkFaceTotLength + BkFaceLength - 0.5 * BkFaceShelfLength ) );
    lvHcalBkFaceSubModuleVol.placeVolume(
        lvHcalBkFaceWorkStuffVol,
        dd4hep::Position( 0, 0, -0.5 * BkFaceTotLength + 0.5 * BkFaceLength - 0.5 * BkFaceShelfLength ) );
    lvHcalBkFaceSubModuleVol.placeVolume(
        lvHcalCntStripVol,
        dd4hep::Position( 0, 0, -0.5 * BkFaceTotLength + BkFaceLength - BkFaceShelfLength + 0.5 * CntStripWidth ) );
    lvHcalBkFaceSubModuleVol.placeVolume(
        lvHcalBkOutStripVol,
        dd4hep::Position( 0, 0.5 * FaceWidth_for_CntStrip + BkFaceShelfWidth + 0.5 * BkOutStripHeight,
                          -0.5 * BkFaceTotLength + BkFaceLength - BkFaceShelfLength + 0.5 * BkOutStripWidth ) );
    lvHcalBkFaceSubModuleVol.placeVolume(
        lvHcalBkOutStripVol,
        dd4hep::Position( 0, -0.5 * FaceWidth_for_CntStrip - BkFaceShelfWidth - 0.5 * BkOutStripHeight,
                          -0.5 * BkFaceTotLength + BkFaceLength - BkFaceShelfLength + 0.5 * BkOutStripWidth ) );

    registerVolume( lvHcalBkFaceSubModuleVol.name(), lvHcalBkFaceSubModuleVol );

    // ******************************************************
    // Building the upper Master Plate (UpMsPl)
    // ******************************************************

    dd4hep::Box  HcalInnCellUpMsPlBox( MsPlWidth / 2, 0.5 * MsPlHeight / 2, 2.0 * MsPlStepLength / 2 );
    dd4hep::Tube HcalInnCellUpMsPlCalHole( 0, MsPlCalHoleR, ( MsPlWidth + Tolerance ) / 2 );

    dd4hep::RotationZYX rot_CalHole( 0, Rotation90, 0 );
    dd4hep::Position    pos_1stCalHole( 0, -0.25 * MsPlHeight, -0.5 * MsPlStepLength );
    dd4hep::Position    pos_2ndCalHole( 0, -0.25 * MsPlHeight, 0.5 * MsPlStepLength );

    dd4hep::SubtractionSolid HcalInnCellUpMsPlTmp( HcalInnCellUpMsPlBox, HcalInnCellUpMsPlCalHole,
                                                   dd4hep::Transform3D( rot_CalHole, pos_1stCalHole ) );
    dd4hep::SubtractionSolid HcalInnCellUpMsPl( HcalInnCellUpMsPlTmp, HcalInnCellUpMsPlCalHole,
                                                dd4hep::Transform3D( rot_CalHole, pos_2ndCalHole ) );

    dd4hep::Volume lvHcalInnCellUpMsPlVol( "lvHcalInnCellUpMsPl", HcalInnCellUpMsPl,
                                           description.material( "CaloSteel" ) );
    lvHcalInnCellUpMsPlVol.setVisAttributes( description, "vHcal:MsPlSteel" );

    dd4hep::Box    HcalInnCellUpMsPlRootBox( MsPlWidth / 2, 0.5 * MsPlHeight / 2, 2.0 * MsPlStepLength / 2 );
    dd4hep::Volume lvHcalInnCellUpMsPlBoxVol( "lvHcalInnCellUpMsPlBox", HcalInnCellUpMsPlRootBox,
                                              description.material( "Air" ) );
    lvHcalInnCellUpMsPlBoxVol.setVisAttributes( description, "vHcal:Air" );
    lvHcalInnCellUpMsPlBoxVol.placeVolume( lvHcalInnCellUpMsPlVol, dd4hep::Position( 0, 0, 0 ) );

    registerVolume( lvHcalInnCellUpMsPlBoxVol.name(), lvHcalInnCellUpMsPlBoxVol );

    // ******************************************************
    // Building the lower Master Plate (LowMsPl)
    // ******************************************************

    dd4hep::Box  HcalInnCellLowMsPlBox( MsPlWidth / 2, 0.5 * MsPlHeight / 2, 2.0 * MsPlStepLength / 2 );
    dd4hep::Tube HcalInnCellLowMsPlCalHole( 0, MsPlCalHoleR, ( MsPlWidth + Tolerance ) / 2 );

    dd4hep::Position pos_Low1stCalHole( 0, 0.25 * MsPlHeight, -0.5 * MsPlStepLength );
    dd4hep::Position pos_Low2ndCalHole( 0, 0.25 * MsPlHeight, 0.5 * MsPlStepLength );

    dd4hep::SubtractionSolid HcalInnCellLowMsPlTmp( HcalInnCellLowMsPlBox, HcalInnCellLowMsPlCalHole,
                                                    dd4hep::Transform3D( rot_CalHole, pos_Low1stCalHole ) );
    dd4hep::SubtractionSolid HcalInnCellLowMsPl( HcalInnCellLowMsPlTmp, HcalInnCellLowMsPlCalHole,
                                                 dd4hep::Transform3D( rot_CalHole, pos_Low2ndCalHole ) );

    dd4hep::Volume lvHcalInnCellLowMsPlVol( "lvHcalInnCellLowMsPl", HcalInnCellLowMsPl,
                                            description.material( "CaloSteel" ) );
    lvHcalInnCellLowMsPlVol.setVisAttributes( description, "vHcal:MsPlSteel" );

    dd4hep::Box    HcalInnCellLowMsPlRootBox( MsPlWidth / 2, 0.5 * MsPlHeight / 2, 2.0 * MsPlStepLength / 2 );
    dd4hep::Volume lvHcalInnCellLowMsPlBoxVol( "lvHcalInnCellLowMsPlBox", HcalInnCellLowMsPlRootBox,
                                               description.material( "Air" ) );
    lvHcalInnCellLowMsPlBoxVol.setVisAttributes( description, "vHcal:Air" );
    lvHcalInnCellLowMsPlBoxVol.placeVolume( lvHcalInnCellLowMsPlVol, dd4hep::Position( 0, 0, 0 ) );

    registerVolume( lvHcalInnCellLowMsPlBoxVol.name(), lvHcalInnCellLowMsPlBoxVol );

    // ******************************************************
    // Building the upper half scintillating tile
    // ******************************************************

    dd4hep::Box  HcalInnCellUpScTileBox( ScTileWidth / 2, ( 0.5 * ScTileHeigth - 0.5 * ScTileSlitWidth ) / 2,
                                        ScTileLength / 2 );
    dd4hep::Tube HcalInnCellUpScTileCalHole( 0, ScTileCalHoleR, ( ScTileWidth + Tolerance ) / 2 );

    dd4hep::Position pos_UpScCalHole( 0, -0.25 * ScTileHeigth - 0.25 * ScTileSlitWidth, 0 );

    dd4hep::SubtractionSolid HcalInnCellUpScTile( HcalInnCellUpScTileBox, HcalInnCellUpScTileCalHole,
                                                  dd4hep::Transform3D( rot_CalHole, pos_UpScCalHole ) );

    dd4hep::Volume lvHcalInnCellUpScTileVol;
    lvHcalInnCellUpScTileVol =
        dd4hep::Volume( "lvHcalInnCellUpScTile", HcalInnCellUpScTile, description.material( "CaloSc" ) );
    lvHcalInnCellUpScTileVol.setVisAttributes( description, "vHcal:Sc" );
    lvHcalInnCellUpScTileVol.setSensitiveDetector( sensitive );

    dd4hep::Box    HcalInnCellUpScTileRootBox( ScTileWidth / 2, ( 0.5 * ScTileHeigth - 0.5 * ScTileSlitWidth ) / 2,
                                            ScTileLength / 2 );
    dd4hep::Volume lvHcalInnCellUpScTileBoxVol;
    lvHcalInnCellUpScTileBoxVol =
        dd4hep::Volume( "lvHcalInnCellUpScTileBox", HcalInnCellUpScTileRootBox, description.material( "Air" ) );
    lvHcalInnCellUpScTileBoxVol.setVisAttributes( description, "vHcal:Air" );
    pv = lvHcalInnCellUpScTileBoxVol.placeVolume( lvHcalInnCellUpScTileVol, dd4hep::Position( 0, 0, 0 ) );
    pv->SetName( "pvInnCellUpScTile" );
    pv.addPhysVolID( "sctile", 0 );
    registerVolume( lvHcalInnCellUpScTileBoxVol.name(), lvHcalInnCellUpScTileBoxVol );

    // ******************************************************
    // Building the lower half scintillating tile
    // ******************************************************

    dd4hep::Box  HcalInnCellLowScTileBox( ScTileWidth / 2, ( 0.5 * ScTileHeigth - 0.5 * ScTileSlitWidth ) / 2,
                                         ScTileLength / 2 );
    dd4hep::Tube HcalInnCellLowScTileCalHole( 0, ScTileCalHoleR, ( ScTileWidth + Tolerance ) / 2 );

    dd4hep::Position pos_LowScCalHole( 0, 0.25 * ScTileHeigth + 0.25 * ScTileSlitWidth, 0 );

    dd4hep::SubtractionSolid HcalInnCellLowScTile( HcalInnCellLowScTileBox, HcalInnCellLowScTileCalHole,
                                                   dd4hep::Transform3D( rot_CalHole, pos_LowScCalHole ) );

    dd4hep::Volume lvHcalInnCellLowScTileVol;
    lvHcalInnCellLowScTileVol =
        dd4hep::Volume( "lvHcalInnCellLowScTile", HcalInnCellLowScTile, description.material( "CaloSc" ) );
    lvHcalInnCellLowScTileVol.setVisAttributes( description, "vHcal:Sc" );
    lvHcalInnCellLowScTileVol.setSensitiveDetector( sensitive );

    dd4hep::Box    HcalInnCellLowScTileRootBox( ScTileWidth / 2, ( 0.5 * ScTileHeigth - 0.5 * ScTileSlitWidth ) / 2,
                                             ScTileLength / 2 );
    dd4hep::Volume lvHcalInnCellLowScTileBoxVol;
    lvHcalInnCellLowScTileBoxVol =
        dd4hep::Volume( "lvHcalInnCellLowScTileBox", HcalInnCellLowScTileRootBox, description.material( "Air" ) );
    lvHcalInnCellLowScTileBoxVol.setVisAttributes( description, "vHcal:Air" );
    pv = lvHcalInnCellLowScTileBoxVol.placeVolume( lvHcalInnCellLowScTileVol, dd4hep::Position( 0, 0, 0 ) );
    pv->SetName( "pvInnCellLowScTile" );
    pv.addPhysVolID( "sctile", 1 );
    registerVolume( lvHcalInnCellLowScTileBoxVol.name(), lvHcalInnCellLowScTileBoxVol );

    // ******************************************************
    // Building the upper forward big inner spacer
    // ******************************************************

    dd4hep::Box  HcalInnCellUpFwBigSpacerBox( SpacerWidth / 2, 0.5 * SpacerHeigth / 2,
                                             ( 0.5 * SpacerLength + 0.5 * MsPlStepLength ) / 2 );
    dd4hep::Tube HcalInnCellUpFwBigSpacerCalHole( 0, MsPlCalHoleR, ( SpacerWidth + Tolerance ) / 2 );

    dd4hep::Position pos_UpFwSpacerHole( 0, -0.25 * SpacerHeigth, -0.25 * SpacerLength + 0.25 * MsPlStepLength );

    dd4hep::SubtractionSolid HcalInnCellUpFwBigSpacer( HcalInnCellUpFwBigSpacerBox, HcalInnCellUpFwBigSpacerCalHole,
                                                       dd4hep::Transform3D( rot_CalHole, pos_UpFwSpacerHole ) );

    dd4hep::Volume lvHcalInnCellUpFwBigSpacerVol;
    lvHcalInnCellUpFwBigSpacerVol =
        dd4hep::Volume( "lvHcalInnCellUpFwBigSpacer", HcalInnCellUpFwBigSpacer, description.material( "CaloSteel" ) );
    lvHcalInnCellUpFwBigSpacerVol.setVisAttributes( description, "vHcal:MsPlSteel" );

    dd4hep::Box    HcalInnCellUpFwBigSpacerRootBox( SpacerWidth / 2, 0.5 * SpacerHeigth / 2,
                                                 ( 0.5 * SpacerLength + 0.5 * MsPlStepLength ) / 2 );
    dd4hep::Volume lvHcalInnCellUpFwBigSpacerBoxVol;
    lvHcalInnCellUpFwBigSpacerBoxVol = dd4hep::Volume( "lvHcalInnCellUpFwBigSpacerBox", HcalInnCellUpFwBigSpacerRootBox,
                                                       description.material( "Air" ) );
    lvHcalInnCellUpFwBigSpacerBoxVol.setVisAttributes( description, "vHcal:Air" );
    lvHcalInnCellUpFwBigSpacerBoxVol.placeVolume( lvHcalInnCellUpFwBigSpacerVol, dd4hep::Position( 0, 0, 0 ) );

    registerVolume( lvHcalInnCellUpFwBigSpacerBoxVol.name(), lvHcalInnCellUpFwBigSpacerBoxVol );

    // ******************************************************
    // Building the lower forward big inner spacer
    // ******************************************************

    dd4hep::Box  HcalInnCellLowFwBigSpacerBox( SpacerWidth / 2, 0.5 * SpacerHeigth / 2,
                                              ( 0.5 * SpacerLength + 0.5 * MsPlStepLength ) / 2 );
    dd4hep::Tube HcalInnCellLowFwBigSpacerCalHole( 0, MsPlCalHoleR, ( SpacerWidth + Tolerance ) / 2 );

    dd4hep::Position pos_LowFwSpacerHole( 0, 0.25 * SpacerHeigth, -0.25 * SpacerLength + 0.25 * MsPlStepLength );

    dd4hep::SubtractionSolid HcalInnCellLowFwBigSpacer( HcalInnCellLowFwBigSpacerBox, HcalInnCellLowFwBigSpacerCalHole,
                                                        dd4hep::Transform3D( rot_CalHole, pos_LowFwSpacerHole ) );

    dd4hep::Volume lvHcalInnCellLowFwBigSpacerVol;
    lvHcalInnCellLowFwBigSpacerVol =
        dd4hep::Volume( "lvHcalInnCellLowFwBigSpacer", HcalInnCellLowFwBigSpacer, description.material( "CaloSteel" ) );
    lvHcalInnCellLowFwBigSpacerVol.setVisAttributes( description, "vHcal:MsPlSteel" );

    dd4hep::Box    HcalInnCellLowFwBigSpacerRootBox( SpacerWidth / 2, 0.5 * SpacerHeigth / 2,
                                                  ( 0.5 * SpacerLength + 0.5 * MsPlStepLength ) / 2 );
    dd4hep::Volume lvHcalInnCellLowFwBigSpacerBoxVol;
    lvHcalInnCellLowFwBigSpacerBoxVol = dd4hep::Volume(
        "lvHcalInnCellLowFwBigSpacerBox", HcalInnCellLowFwBigSpacerRootBox, description.material( "Air" ) );
    lvHcalInnCellLowFwBigSpacerBoxVol.setVisAttributes( description, "vHcal:Air" );
    lvHcalInnCellLowFwBigSpacerBoxVol.placeVolume( lvHcalInnCellLowFwBigSpacerVol, dd4hep::Position( 0, 0, 0 ) );

    registerVolume( lvHcalInnCellLowFwBigSpacerBoxVol.name(), lvHcalInnCellLowFwBigSpacerBoxVol );

    // ******************************************************
    // Building the upper backward big inner spacer
    // ******************************************************

    dd4hep::Box  HcalInnCellUpBkBigSpacerBox( SpacerWidth / 2, 0.5 * SpacerHeigth / 2,
                                             ( 0.5 * SpacerLength + 0.5 * MsPlStepLength ) / 2 );
    dd4hep::Tube HcalInnCellUpBkBigSpacerCalHole( 0, MsPlCalHoleR, ( SpacerWidth + Tolerance ) / 2 );

    dd4hep::Position pos_UpBkSpacerHole( 0, -0.25 * SpacerHeigth, 0.25 * SpacerLength - 0.25 * MsPlStepLength );

    dd4hep::SubtractionSolid HcalInnCellUpBkBigSpacer( HcalInnCellUpBkBigSpacerBox, HcalInnCellUpBkBigSpacerCalHole,
                                                       dd4hep::Transform3D( rot_CalHole, pos_UpBkSpacerHole ) );

    dd4hep::Volume lvHcalInnCellUpBkBigSpacerVol;
    lvHcalInnCellUpBkBigSpacerVol =
        dd4hep::Volume( "lvHcalInnCellUpBkBigSpacer", HcalInnCellUpBkBigSpacer, description.material( "CaloSteel" ) );
    lvHcalInnCellUpBkBigSpacerVol.setVisAttributes( description, "vHcal:SpacerSteel" );

    dd4hep::Box    HcalInnCellUpBkBigSpacerRootBox( SpacerWidth / 2, 0.5 * SpacerHeigth / 2,
                                                 ( 0.5 * SpacerLength + 0.5 * MsPlStepLength ) / 2 );
    dd4hep::Volume lvHcalInnCellUpBkBigSpacerBoxVol;
    lvHcalInnCellUpBkBigSpacerBoxVol = dd4hep::Volume( "lvHcalInnCellUpBkBigSpacerBox", HcalInnCellUpBkBigSpacerRootBox,
                                                       description.material( "Air" ) );
    lvHcalInnCellUpBkBigSpacerBoxVol.setVisAttributes( description, "vHcal:Air" );
    lvHcalInnCellUpBkBigSpacerBoxVol.placeVolume( lvHcalInnCellUpBkBigSpacerVol, dd4hep::Position( 0, 0, 0 ) );

    registerVolume( lvHcalInnCellUpBkBigSpacerBoxVol.name(), lvHcalInnCellUpBkBigSpacerBoxVol );

    // ******************************************************
    // Building the lower backward big inner spacer
    // ******************************************************

    dd4hep::Box  HcalInnCellLowBkBigSpacerBox( SpacerWidth / 2, 0.5 * SpacerHeigth / 2,
                                              ( 0.5 * SpacerLength + 0.5 * MsPlStepLength ) / 2 );
    dd4hep::Tube HcalInnCellLowBkBigSpacerCalHole( 0, MsPlCalHoleR, ( SpacerWidth + Tolerance ) / 2 );

    dd4hep::Position pos_LowBkSpacerHole( 0, 0.25 * SpacerHeigth, 0.25 * SpacerLength - 0.25 * MsPlStepLength );

    dd4hep::SubtractionSolid HcalInnCellLowBkBigSpacer( HcalInnCellLowBkBigSpacerBox, HcalInnCellLowBkBigSpacerCalHole,
                                                        dd4hep::Transform3D( rot_CalHole, pos_LowBkSpacerHole ) );

    dd4hep::Volume lvHcalInnCellLowBkBigSpacerVol;
    lvHcalInnCellLowBkBigSpacerVol =
        dd4hep::Volume( "lvHcalInnCellLowBkBigSpacer", HcalInnCellLowBkBigSpacer, description.material( "CaloSteel" ) );
    lvHcalInnCellLowBkBigSpacerVol.setVisAttributes( description, "vHcal:MsPlSteel" );

    dd4hep::Box    HcalInnCellLowBkBigSpacerRootBox( SpacerWidth / 2, 0.5 * SpacerHeigth / 2,
                                                  ( 0.5 * SpacerLength + 0.5 * MsPlStepLength ) / 2 );
    dd4hep::Volume lvHcalInnCellLowBkBigSpacerBoxVol;
    lvHcalInnCellLowBkBigSpacerBoxVol = dd4hep::Volume(
        "lvHcalInnCellLowBkBigSpacerBox", HcalInnCellLowBkBigSpacerRootBox, description.material( "Air" ) );
    lvHcalInnCellLowBkBigSpacerBoxVol.setVisAttributes( description, "vHcal:Air" );
    lvHcalInnCellLowBkBigSpacerBoxVol.placeVolume( lvHcalInnCellLowBkBigSpacerVol, dd4hep::Position( 0, 0, 0 ) );

    registerVolume( lvHcalInnCellLowBkBigSpacerBoxVol.name(), lvHcalInnCellLowBkBigSpacerBoxVol );

    // ******************************************************
    // Importing the forward and backward, upper and lower small inner spacers
    // ******************************************************

    dd4hep::Box HcalInnCellSmSpacerBox( SpacerWidth / 2, 0.5 * SpacerHeigth / 2,
                                        ( 0.5 * SpacerLength - 0.5 * MsPlStepLength ) / 2 );

    dd4hep::Volume lvHcalInnCellUpFwSmSpacerVol =
        dd4hep::Volume( "lvHcalInnCellUpFwSmSpacer", HcalInnCellSmSpacerBox, description.material( "CaloSteel" ) );
    lvHcalInnCellUpFwSmSpacerVol.setVisAttributes( description, "vHcal:MsPlSteel" );
    dd4hep::Volume lvHcalInnCellLowFwSmSpacerVol =
        dd4hep::Volume( "lvHcalInnCellLowFwSmSpacer", HcalInnCellSmSpacerBox, description.material( "CaloSteel" ) );
    lvHcalInnCellLowFwSmSpacerVol.setVisAttributes( description, "vHcal:MsPlSteel" );
    dd4hep::Volume lvHcalInnCellUpBkSmSpacerVol =
        dd4hep::Volume( "lvHcalInnCellUpBkSmSpacer", HcalInnCellSmSpacerBox, description.material( "CaloSteel" ) );
    lvHcalInnCellUpBkSmSpacerVol.setVisAttributes( description, "vHcal:MsPlSteel" );
    dd4hep::Volume lvHcalInnCellLowBkSmSpacerVol =
        dd4hep::Volume( "lvHcalInnCellLowBkSmSpacer", HcalInnCellSmSpacerBox, description.material( "CaloSteel" ) );
    lvHcalInnCellLowBkSmSpacerVol.setVisAttributes( description, "vHcal:MsPlSteel" );

    // ******************************************************
    // Building the working stuff of the inner cells
    // ******************************************************

    dd4hep::Box    HcalInnCellUpRightComPartWorkStuffBox( 0.5 * CellPeriodWidth / 2, 0.5 * MsPlHeight / 2,
                                                       2.0 * MsPlStepLength / 2 );
    dd4hep::Volume lvHcalInnCellUpRightComPartWorkStuffVol;
    lvHcalInnCellUpRightComPartWorkStuffVol = dd4hep::Volume(
        "lvHcalInnCellUpRightComPartWorkStuff", HcalInnCellUpRightComPartWorkStuffBox, description.material( "Air" ) );
    lvHcalInnCellUpRightComPartWorkStuffVol.setVisAttributes( description, "vHcal:Air" );
    lvHcalInnCellUpRightComPartWorkStuffVol.placeVolume(
        lvHcalInnCellUpMsPlBoxVol, dd4hep::Position( InnCellXPos1stLayer * CellPeriodWidth, 0, 0 ) );
    lvHcalInnCellUpRightComPartWorkStuffVol.placeVolume(
        lvHcalInnCellUpFwSmSpacerVol,
        dd4hep::Position( InnCellXPos2ndLayer * CellPeriodWidth, -0.25 * MsPlHeight + 0.25 * SpacerHeigth,
                          -1.25 * MsPlStepLength + 0.25 * SpacerLength ) );
    lvHcalInnCellUpRightComPartWorkStuffVol.placeVolume(
        lvHcalInnCellUpScTileBoxVol,
        dd4hep::Position( InnCellXPos2ndLayer * CellPeriodWidth,
                          -0.25 * MsPlHeight + 0.5 * ScTileSlitWidth + 0.25 * ScTileHeigth, -0.5 * MsPlStepLength ) );
    lvHcalInnCellUpRightComPartWorkStuffVol.placeVolume(
        lvHcalInnCellUpBkBigSpacerBoxVol,
        dd4hep::Position( InnCellXPos2ndLayer * CellPeriodWidth, -0.25 * MsPlHeight + 0.25 * SpacerHeigth,
                          0.75 * MsPlStepLength - 0.25 * SpacerLength ) );

    registerVolume( lvHcalInnCellUpRightComPartWorkStuffVol.name(), lvHcalInnCellUpRightComPartWorkStuffVol );

    dd4hep::Box    HcalInnCellUpLeftComPartWorkStuffBox( 0.5 * CellPeriodWidth / 2, 0.5 * MsPlHeight / 2,
                                                      2.0 * MsPlStepLength / 2 );
    dd4hep::Volume lvHcalInnCellUpLeftComPartWorkStuffVol;
    lvHcalInnCellUpLeftComPartWorkStuffVol = dd4hep::Volume(
        "lvHcalInnCellUpLeftComPartWorkStuff", HcalInnCellUpLeftComPartWorkStuffBox, description.material( "Air" ) );
    lvHcalInnCellUpLeftComPartWorkStuffVol.setVisAttributes( description, "vHcal:Air" );
    lvHcalInnCellUpLeftComPartWorkStuffVol.placeVolume(
        lvHcalInnCellUpMsPlBoxVol, dd4hep::Position( InnCellXPos1stLayer * CellPeriodWidth, 0, 0 ) );
    lvHcalInnCellUpLeftComPartWorkStuffVol.placeVolume(
        lvHcalInnCellUpBkSmSpacerVol,
        dd4hep::Position( InnCellXPos2ndLayer * CellPeriodWidth, -0.25 * MsPlHeight + 0.25 * SpacerHeigth,
                          1.25 * MsPlStepLength - 0.25 * SpacerLength ) );
    lvHcalInnCellUpLeftComPartWorkStuffVol.placeVolume(
        lvHcalInnCellUpScTileBoxVol,
        dd4hep::Position( InnCellXPos2ndLayer * CellPeriodWidth,
                          -0.25 * MsPlHeight + 0.5 * ScTileSlitWidth + 0.25 * ScTileHeigth, 0.5 * MsPlStepLength ) );
    lvHcalInnCellUpLeftComPartWorkStuffVol.placeVolume(
        lvHcalInnCellUpFwBigSpacerBoxVol,
        dd4hep::Position( InnCellXPos2ndLayer * CellPeriodWidth, -0.25 * MsPlHeight + 0.25 * SpacerHeigth,
                          -0.75 * MsPlStepLength + 0.25 * SpacerLength ) );

    dd4hep::Box    HcalInnCellLowRightComPartWorkStuffBox( 0.5 * CellPeriodWidth / 2, 0.5 * MsPlHeight / 2,
                                                        2.0 * MsPlStepLength / 2 );
    dd4hep::Volume lvHcalInnCellLowRightComPartWorkStuffVol;
    lvHcalInnCellLowRightComPartWorkStuffVol =
        dd4hep::Volume( "lvHcalInnCellLowRightComPartWorkStuff", HcalInnCellLowRightComPartWorkStuffBox,
                        description.material( "Air" ) );
    lvHcalInnCellLowRightComPartWorkStuffVol.setVisAttributes( description, "vHcal:Air" );
    lvHcalInnCellLowRightComPartWorkStuffVol.placeVolume(
        lvHcalInnCellLowMsPlBoxVol, dd4hep::Position( InnCellXPos1stLayer * CellPeriodWidth, 0, 0 ) );
    lvHcalInnCellLowRightComPartWorkStuffVol.placeVolume(
        lvHcalInnCellLowFwSmSpacerVol,
        dd4hep::Position( InnCellXPos2ndLayer * CellPeriodWidth, 0.25 * MsPlHeight - 0.25 * SpacerHeigth,
                          -1.25 * MsPlStepLength + 0.25 * SpacerLength ) );
    lvHcalInnCellLowRightComPartWorkStuffVol.placeVolume(
        lvHcalInnCellLowScTileBoxVol,
        dd4hep::Position( InnCellXPos2ndLayer * CellPeriodWidth,
                          0.25 * MsPlHeight - 0.5 * ScTileSlitWidth - 0.25 * ScTileHeigth, -0.5 * MsPlStepLength ) );
    lvHcalInnCellLowRightComPartWorkStuffVol.placeVolume(
        lvHcalInnCellLowBkBigSpacerBoxVol,
        dd4hep::Position( InnCellXPos2ndLayer * CellPeriodWidth, 0.25 * MsPlHeight - 0.25 * SpacerHeigth,
                          0.75 * MsPlStepLength - 0.25 * SpacerLength ) );

    dd4hep::Box    HcalInnCellLowLeftComPartWorkStuffBox( 0.5 * CellPeriodWidth / 2, 0.5 * MsPlHeight / 2,
                                                       2.0 * MsPlStepLength / 2 );
    dd4hep::Volume lvHcalInnCellLowLeftComPartWorkStuffVol;
    lvHcalInnCellLowLeftComPartWorkStuffVol = dd4hep::Volume(
        "lvHcalInnCellLowLeftComPartWorkStuff", HcalInnCellLowLeftComPartWorkStuffBox, description.material( "Air" ) );
    lvHcalInnCellLowLeftComPartWorkStuffVol.setVisAttributes( description, "vHcal:Air" );
    lvHcalInnCellLowLeftComPartWorkStuffVol.placeVolume(
        lvHcalInnCellLowMsPlBoxVol, dd4hep::Position( InnCellXPos1stLayer * CellPeriodWidth, 0, 0 ) );
    lvHcalInnCellLowLeftComPartWorkStuffVol.placeVolume(
        lvHcalInnCellLowBkSmSpacerVol,
        dd4hep::Position( InnCellXPos2ndLayer * CellPeriodWidth, 0.25 * MsPlHeight - 0.25 * SpacerHeigth,
                          1.25 * MsPlStepLength - 0.25 * SpacerLength ) );
    lvHcalInnCellLowLeftComPartWorkStuffVol.placeVolume(
        lvHcalInnCellLowScTileBoxVol,
        dd4hep::Position( InnCellXPos2ndLayer * CellPeriodWidth,
                          0.25 * MsPlHeight - 0.5 * ScTileSlitWidth - 0.25 * ScTileHeigth, 0.5 * MsPlStepLength ) );
    lvHcalInnCellLowLeftComPartWorkStuffVol.placeVolume(
        lvHcalInnCellLowFwBigSpacerBoxVol,
        dd4hep::Position( InnCellXPos2ndLayer * CellPeriodWidth, 0.25 * MsPlHeight - 0.25 * SpacerHeigth,
                          -0.75 * MsPlStepLength + 0.25 * SpacerLength ) );

    dd4hep::Box    HcalInnCellWorkStuffBox( InnCellXSize / 2, 0.5 * MsPlHeight / 2, 6.0 * MsPlStepLength / 2 );
    dd4hep::Volume lvHcalInnCellUpRightWorkStuffVol;
    lvHcalInnCellUpRightWorkStuffVol =
        dd4hep::Volume( "lvHcalInnCellUpRightWorkStuff", HcalInnCellWorkStuffBox, description.material( "Air" ) );
    lvHcalInnCellUpRightWorkStuffVol.setVisAttributes( description, "vHcal:Air" );
    dd4hep::Volume lvHcalInnCellLowRightWorkStuffVol;
    lvHcalInnCellLowRightWorkStuffVol =
        dd4hep::Volume( "lvHcalInnCellLowRightWorkStuff", HcalInnCellWorkStuffBox, description.material( "Air" ) );
    lvHcalInnCellLowRightWorkStuffVol.setVisAttributes( description, "vHcal:Air" );
    dd4hep::Volume lvHcalInnCellUpLeftWorkStuffVol;
    lvHcalInnCellUpLeftWorkStuffVol =
        dd4hep::Volume( "lvHcalInnCellUpLeftWorkStuff", HcalInnCellWorkStuffBox, description.material( "Air" ) );
    lvHcalInnCellUpLeftWorkStuffVol.setVisAttributes( description, "vHcal:Air" );
    dd4hep::Volume lvHcalInnCellLowLeftWorkStuffVol;
    lvHcalInnCellLowLeftWorkStuffVol =
        dd4hep::Volume( "lvHcalInnCellLowLeftWorkStuff", HcalInnCellWorkStuffBox, description.material( "Air" ) );
    lvHcalInnCellLowLeftWorkStuffVol.setVisAttributes( description, "vHcal:Air" );

    if ( m_build_inner_cell_workparts ) {
      int n_cellworkpart = 0;
      for ( int nx = 0; nx < 7; ++nx ) {
        for ( int nz = 0; nz < 3; ++nz ) {
          auto handle_pv = [&pv, &n_cellworkpart]( std::string _name ) {
            pv->SetName( Form( "pv%s_%02d", _name.c_str(), n_cellworkpart ) );
            pv.addPhysVolID( "cellworkpart", n_cellworkpart );
          };
          double           xpos1 = ( nx - 3.0 ) * CellPeriodWidth;
          double           ypos  = 0;
          double           zpos  = ( 2 * nz - 2.0 ) * MsPlStepLength;
          dd4hep::Position pos1( xpos1, ypos, zpos );
          pv = lvHcalInnCellUpRightWorkStuffVol.placeVolume( lvHcalInnCellUpRightComPartWorkStuffVol, pos1 );
          handle_pv( "HcalInnUpRightCellUpRightComPartWorkStuff" );
          pv = lvHcalInnCellLowRightWorkStuffVol.placeVolume( lvHcalInnCellLowRightComPartWorkStuffVol, pos1 );
          handle_pv( "HcalInnLowRightCellLowRightComPartWorkStuff" );
          pv = lvHcalInnCellUpLeftWorkStuffVol.placeVolume( lvHcalInnCellUpLeftComPartWorkStuffVol, pos1 );
          handle_pv( "HcalInnUpLeftCellUpLeftComPartWorkStuff" );
          pv = lvHcalInnCellLowLeftWorkStuffVol.placeVolume( lvHcalInnCellLowLeftComPartWorkStuffVol, pos1 );
          handle_pv( "HcalInnLowLeftCellLowLeftComPartWorkStuff" );
          ++n_cellworkpart;
          if ( nx < 6 ) {
            double           xpos2 = ( nx - 2.5 ) * CellPeriodWidth;
            dd4hep::Position pos2( xpos2, ypos, zpos );
            pv = lvHcalInnCellUpRightWorkStuffVol.placeVolume( lvHcalInnCellUpLeftComPartWorkStuffVol, pos2 );
            handle_pv( "HcalInnUpRightCellUpLeftComPartWorkStuff" );
            pv = lvHcalInnCellLowRightWorkStuffVol.placeVolume( lvHcalInnCellLowLeftComPartWorkStuffVol, pos2 );
            handle_pv( "HcalInnLowRightCellLowLeftComPartWorkStuff" );
            pv = lvHcalInnCellUpLeftWorkStuffVol.placeVolume( lvHcalInnCellUpRightComPartWorkStuffVol, pos2 );
            handle_pv( "HcalInnUpLeftCellUpRightComPartWorkStuff" );
            pv = lvHcalInnCellLowLeftWorkStuffVol.placeVolume( lvHcalInnCellLowRightComPartWorkStuffVol, pos2 );
            handle_pv( "HcalInnLowLeftCellLowRightComPartWorkStuff" );
            ++n_cellworkpart;
          }
        }
      }
    }

    registerVolume( lvHcalInnCellUpRightWorkStuffVol.name(), lvHcalInnCellUpRightWorkStuffVol );
    registerVolume( lvHcalInnCellLowRightWorkStuffVol.name(), lvHcalInnCellLowRightWorkStuffVol );
    registerVolume( lvHcalInnCellUpLeftWorkStuffVol.name(), lvHcalInnCellUpLeftWorkStuffVol );
    registerVolume( lvHcalInnCellLowLeftWorkStuffVol.name(), lvHcalInnCellLowLeftWorkStuffVol );
  }

  void HcalBuild::build_inner_section() {
    dd4hep::PlacedVolume pv;

    // ******************************************************
    // Loading parameters
    // ******************************************************

    double ModXSize        = dd4hep::_toDouble( "HcalModXSize" );
    double ModYSize        = dd4hep::_toDouble( "HcalModYSize" );
    double YGap            = dd4hep::_toDouble( "HcalYGap" );
    double TotLength       = dd4hep::_toDouble( "HcalTotLength" );
    double SubModXSize     = dd4hep::_toDouble( "HcalSubModXSize" );
    double Tolerance       = dd4hep::_toDouble( "HcalTolerance" );
    double InnCellXSize    = dd4hep::_toDouble( "HcalInnCellXSize" );
    double InnCellYSize    = dd4hep::_toDouble( "HcalInnCellYSize" );
    double FwFaceLength    = dd4hep::_toDouble( "HcalFwFaceLength" );
    double MsPlStepLength  = dd4hep::_toDouble( "HcalMsPlStepLength" );
    double BkFaceTotLength = dd4hep::_toDouble( "HcalBkFaceTotLength" );
    double MsPlHeight      = dd4hep::_toDouble( "HcalMsPlHeight" );
    double Rotation180     = dd4hep::_toDouble( "HcalRotation180" );
    double BeamPlugXSize   = dd4hep::_toDouble( "HcalBeamPlugXSize" );
    double BeamPlugYSize   = dd4hep::_toDouble( "HcalBeamPlugYSize" );
    double BackShieldingXHalfSize        = dd4hep::_toDouble( "HcalBackShieldingXHalfSize" );
    double BackShieldingZSize            = dd4hep::_toDouble( "HcalBackShieldingZSize" );
    double BackShieldingXOffset          = dd4hep::_toDouble( "HcalBackShieldingXOffset" );
    double InnerReinforceSubstituteXSize = dd4hep::_toDouble( "HcalInnerReinforceSubstituteXSize" );
    double InnerReinforceSubstituteYSize = dd4hep::_toDouble( "HcalInnerReinforceSubstituteYSize" );
    double InnerReinforceSubstituteZSize = dd4hep::_toDouble( "HcalInnerReinforceSubstituteZSize" );

    std::vector<double> DxLeft, DxRight;
    for (int i=7; i <= 20; ++i){
      double DxLeft_i  = dd4hep::_toDouble( Form("HcalDxLeft%02d",i) );
      DxLeft.push_back( DxLeft_i );
      double DxRight_i = dd4hep::_toDouble( Form( "HcalDxRight%02d", i ) );
      DxRight.push_back( DxRight_i );
    }

    // ******************************************************
    // Building the work part of the half submodule
    // ******************************************************
    dd4hep::Box    HcalInnHalfSubModuleWorkPartBox( 2.0 * InnCellXSize / 2, 2.0 * InnCellYSize / 2,
                                                 ( FwFaceLength + 6.0 * MsPlStepLength + BkFaceTotLength ) / 2 );
    dd4hep::Volume lvHcalInnHalfSubModuleWorkPartVol( "lvHcalInnHalfSubModuleWorkPart", HcalInnHalfSubModuleWorkPartBox,
                                                      description.material( "Air" ) );
    lvHcalInnHalfSubModuleWorkPartVol.setVisAttributes( description, "vHcal:Air" );
    deInnSubModule = dd4hep::DetElement( "HcalInnHalfSubModuleBase", 0 );
    if ( m_build_inner_cells ) {
      lvHcalInnHalfSubModuleWorkPartVol.placeVolume(
          volume( "lvHcalFwFaceSubModule" ), dd4hep::Position( 0, 0, -3.0 * MsPlStepLength - 0.5 * BkFaceTotLength ) );
      lvHcalInnHalfSubModuleWorkPartVol.placeVolume(
          volume( "lvHcalBkFaceSubModule" ), dd4hep::Position( 0, 0, 0.5 * FwFaceLength + 3.0 * MsPlStepLength ) );
      double abs_xpos             = 0.5 * InnCellXSize;
      double abs_ypos             = 0.25 * MsPlHeight;
      double zpos                 = 0.5 * FwFaceLength - 0.5 * BkFaceTotLength;
      auto   addCellAndDetElement = [&pv, &lvHcalInnHalfSubModuleWorkPartVol, this]( std::string      _name,
                                                                                   dd4hep::Position _pos, int _id ) {
        pv = lvHcalInnHalfSubModuleWorkPartVol.placeVolume(
            volume( dd4hep::_toString( _name.c_str(), "lv%s" ).c_str() ), _pos );
        pv->SetName( dd4hep::_toString( _name.c_str(), "pv%s" ).c_str() );
        pv.addPhysVolID( "cell", _id );
        this->deInnCellTmp = dd4hep::DetElement( dd4hep::_toString( _id, "HcalInnCell_%02d" ), _id );
        this->deInnCellTmp.setPlacement( pv );
        this->deInnSubModule.add( this->deInnCellTmp );
      };
      addCellAndDetElement( "HcalInnCellLowLeftWorkStuff", dd4hep::Position( -abs_xpos, -abs_ypos, zpos ), 0 );
      addCellAndDetElement( "HcalInnCellUpLeftWorkStuff", dd4hep::Position( -abs_xpos, +abs_ypos, zpos ), 1 );
      addCellAndDetElement( "HcalInnCellLowRightWorkStuff", dd4hep::Position( +abs_xpos, -abs_ypos, zpos ), 2 );
      addCellAndDetElement( "HcalInnCellUpRightWorkStuff", dd4hep::Position( +abs_xpos, +abs_ypos, zpos ), 3 );
    }
    registerVolume( lvHcalInnHalfSubModuleWorkPartVol.name(), lvHcalInnHalfSubModuleWorkPartVol );

    double moduleLength = TotLength - BackShieldingZSize;
    // ******************************************************
    // Building the inner non standard half-module
    // ******************************************************
    dd4hep::Box    HcalInnMidlHalfModuleBox( ( 0.5 * ModXSize - 0.5 * SubModXSize ) / 2, ModYSize / 2, (moduleLength) / 2 );
    dd4hep::Volume lvHcalInnMidlHalfModuleLeftVol( "lvHcalInnMidlHalfModuleLeft", HcalInnMidlHalfModuleBox,
                                                   description.material( "Air" ) );
    lvHcalInnMidlHalfModuleLeftVol.setVisAttributes( description, "vHcal:Air" );
    dd4hep::Volume lvHcalInnMidlHalfModuleRightVol( "lvHcalInnMidlHalfModuleRight", HcalInnMidlHalfModuleBox,
                                                    description.material( "Air" ) );
    lvHcalInnMidlHalfModuleRightVol.setVisAttributes( description, "vHcal:Air" );

    // ******************************************************
    // Building the inner standard half-module
    // ******************************************************
    dd4hep::Box    HcalInnHalfModuleBox( 0.5 * ModXSize / 2, ModYSize / 2, moduleLength / 2 );
    dd4hep::Volume lvHcalInnHalfModuleVol( "lvHcalInnHalfModule", HcalInnHalfModuleBox, description.material( "Air" ) );
    lvHcalInnHalfModuleVol.setVisAttributes( description, "vHcal:Air" );

    dd4hep::Volume lvHcalHalfSubModuleReinforceVol = volume( "lvHcalHalfSubModuleReinforce" );
    lvHcalHalfSubModuleReinforceVol.setVisAttributes( description, "vHcal:Air" );

    deInnModule          = dd4hep::DetElement( "HcalInnHalfModuleBase", 0 );
    deInnMidlModuleLeft  = dd4hep::DetElement( "HcalInnMidHalfModuleLeftBase", 0 );
    deInnMidlModuleRight = dd4hep::DetElement( "HcalInnMidHalfModuleRightBase", 0 );
    if ( m_build_inner_submodules ) {
      int n_submodule = 0;
      for ( int nx = 0; nx < 8; ++nx ) {
        double ypos           = 0;
        double zpos_reinforce = 0.5 * FwFaceLength + 3.0 * MsPlStepLength + 0.5 * BkFaceTotLength;
        double zpos_workpart = -0.5 * moduleLength + 0.5 * FwFaceLength + 3.0 * MsPlStepLength + 0.5 * BkFaceTotLength;
        auto   addSubmoduleAndDetElement = [&lvHcalHalfSubModuleReinforceVol, &lvHcalInnHalfSubModuleWorkPartVol, ypos,
                                          zpos_reinforce, zpos_workpart,
                                          this]( dd4hep::Volume& _lv, dd4hep::DetElement& _de, double _xpos, int _id ) {
          _lv.placeVolume( lvHcalHalfSubModuleReinforceVol, dd4hep::Position( _xpos, ypos, zpos_reinforce ) );
          dd4hep::PlacedVolume _pv =
              _lv.placeVolume( lvHcalInnHalfSubModuleWorkPartVol, dd4hep::Position( _xpos, ypos, zpos_workpart ) );
          _pv->SetName( dd4hep::_toString( _id, "pvInnHalfSubModuleWorkPart_%02d" ).c_str() );
          _pv.addPhysVolID( "submodule", _id );
          dd4hep::DetElement _deInnSubModuleTmp =
              this->deInnSubModule.clone( dd4hep::_toString( _id, "HcalInnHalfSubModule_%02d" ), _id );
          _deInnSubModuleTmp.setPlacement( _pv );
          _de.add( _deInnSubModuleTmp );
        };

        addSubmoduleAndDetElement( lvHcalInnHalfModuleVol, deInnModule, ( -7.0 + 2 * nx ) * InnCellXSize, n_submodule );
        if ( nx < 7 ) {
          addSubmoduleAndDetElement( lvHcalInnMidlHalfModuleLeftVol, deInnMidlModuleLeft,
                                     ( -6.0 + 2 * nx ) * InnCellXSize, n_submodule + 1 );
          addSubmoduleAndDetElement( lvHcalInnMidlHalfModuleRightVol, deInnMidlModuleRight,
                                     ( -6.0 + 2 * nx ) * InnCellXSize, n_submodule + 1 );
        }
        ++n_submodule;
      }
    }
    registerVolume( lvHcalInnHalfModuleVol.name(), lvHcalInnHalfModuleVol );
    registerVolume( lvHcalInnMidlHalfModuleLeftVol.name(), lvHcalInnMidlHalfModuleLeftVol );
    registerVolume( lvHcalInnMidlHalfModuleRightVol.name(), lvHcalInnMidlHalfModuleRightVol );

    // ******************************************************
    // Building the HCAL inner section
    // ******************************************************
    dd4hep::Box HcalInnerSectionBox( 0.5 * ModXSize / 2, ( 14.0 * ModYSize + 14 * YGap ) / 2, TotLength / 2 );
    dd4hep::Box HcalInnerSectionBoxSubtract( BeamPlugXSize / 2,
                                             BeamPlugYSize / 2,
                                             ( TotLength + Tolerance ) / 2 );
    dd4hep::Box HcalInnerReinforceSubstituteBox( 0.5*InnerReinforceSubstituteXSize / 2,
                                                     InnerReinforceSubstituteYSize / 2,
                                                     InnerReinforceSubstituteZSize / 2 );
    dd4hep::Box HcalInnerReinforceSubstituteBoxSubtract( BeamPlugXSize / 2,
                                                         BeamPlugYSize / 2,
                                                        ( InnerReinforceSubstituteZSize + Tolerance ) / 2 );

    build_back_shield_bricks();

    for ( std::string nside : {"left", "right"} ) { // 0 ->left side, 1 -> right side
      std::vector<double>* Dx                         = 0;
      dd4hep::DetElement*  deInnRegion                = 0;
      dd4hep::DetElement*  deInnMidlModule            = 0;
      dd4hep::Volume*      lvHcalInnMidlHalfModuleVol = 0;
      std::string          name_HcalInnerSection      = "";
      std::string          name_HcalInnerReinforceSubstitute = "";
      double               iside                      = 0;
      dd4hep::RotationZYX rot;
      if ( nside == "left" ) {
        Dx                         = &DxLeft;
        deInnRegionLeft            = dd4hep::DetElement( "HcalLeftInnerSection", 0 );
        deInnRegion                = &deInnRegionLeft;
        deInnMidlModule            = &deInnMidlModuleLeft;
        lvHcalInnMidlHalfModuleVol = &lvHcalInnMidlHalfModuleLeftVol;
        name_HcalInnerSection      = "lvHcalInnerSectionLeft";
        name_HcalInnerReinforceSubstitute = "lvHcalInnerReinforceSubstituteLeft";
        iside                      = +1;
        rot = dd4hep::RotationZYX( 0, 0, 0 );
      } else if ( nside == "right" ) {
        Dx                         = &DxRight;
        deInnRegionRight           = dd4hep::DetElement( "HcalRightInnerSection", 1 );
        deInnRegion                = &deInnRegionRight;
        deInnMidlModule            = &deInnMidlModuleRight;
        lvHcalInnMidlHalfModuleVol = &lvHcalInnMidlHalfModuleRightVol;
        name_HcalInnerSection      = "lvHcalInnerSectionRight";
        name_HcalInnerReinforceSubstitute = "lvHcalInnerReinforceSubstituteRight";
        iside                      = -1;
        rot = dd4hep::RotationZYX( Rotation180, 0, 0 );
      }
      dd4hep::SubtractionSolid HcalInnerSection( HcalInnerSectionBox, HcalInnerSectionBoxSubtract,
                                                 dd4hep::Position( iside*(-0.25 * ModXSize), 0, 0 ) );

      dd4hep::Volume lvHcalInnerSectionVol( name_HcalInnerSection, HcalInnerSection, description.material( "Air" ) );
      lvHcalInnerSectionVol.setVisAttributes( description, "vHcal:Air" );

      if ( m_build_inner_modules ) {
        int n_module = 0;
        for ( int n = 0; n < 14; ++n ) {
          double ypos = ( n - 6.5 ) * ( ModYSize + YGap );
          double zpos = -0.5*TotLength+0.5*moduleLength;
          if ( ( n == 6 ) || ( n == 7 ) ) {
            double xpos = iside*(Dx->at( n ) + 0.25 * SubModXSize) ;
            pv = lvHcalInnerSectionVol.placeVolume( *lvHcalInnMidlHalfModuleVol,
                                                   dd4hep::Transform3D( rot, dd4hep::Position( xpos, ypos, zpos ) ) );
            deInnModuleTmp =
                deInnMidlModule->clone( dd4hep::_toString( n_module, "HcalInnHalfModule_%02d" ), n_module );
          } else {
            double xpos = iside*Dx->at( n );
            pv = lvHcalInnerSectionVol.placeVolume( lvHcalInnHalfModuleVol,
                                                    dd4hep::Transform3D( rot, dd4hep::Position( xpos, ypos, zpos ) ) );
            deInnModuleTmp = deInnModule.clone( dd4hep::_toString( n_module, "HcalInnHalfModule_%02d" ), n_module );
          }
          pv->SetName( dd4hep::_toString( n_module, "pvInnHalfModule_%02d" ).c_str() );
          pv.addPhysVolID( "module", n_module );
          deInnModuleTmp.setPlacement( pv );
          deInnRegion->add( deInnModuleTmp );
          ++n_module;
        }
      }

      dd4hep::SubtractionSolid HcalInnerReinforceSubstitute(
          HcalInnerReinforceSubstituteBox, HcalInnerReinforceSubstituteBoxSubtract,
          dd4hep::Position( iside * ( -0.5 * InnerReinforceSubstituteXSize / 2 ), 0, 0 ) );
      dd4hep::Volume lvHcalInnerReinforceSubstitute( name_HcalInnerReinforceSubstitute, HcalInnerReinforceSubstitute,
                                                     description.material( "Air" ) );
      lvHcalInnerReinforceSubstitute.setVisAttributes( description, "vHcal:Air" );

      if ( m_build_back_shield){
        std::string lvBackShielding_name = build_back_shield( nside );
        double x_shield = iside*(-0.25*ModXSize+0.5*BackShieldingXHalfSize+BackShieldingXOffset);
        dd4hep::Position pos_shield( x_shield, 0, 0);
        lvHcalInnerReinforceSubstitute.placeVolume( volume( lvBackShielding_name.c_str() ), pos_shield );
      }
      lvHcalInnerSectionVol.placeVolume( lvHcalInnerReinforceSubstitute, dd4hep::Position(0,0, 0.5*TotLength-0.5*InnerReinforceSubstituteZSize) );

      registerVolume( lvHcalInnerSectionVol.name(), lvHcalInnerSectionVol );
    }
  }

  void HcalBuild::build_outer_cell() {
    dd4hep::PlacedVolume pv;

    // ******************************************************
    // Loading parameters
    // ******************************************************

    double MsPlWidth           = dd4hep::_toDouble( "HcalMsPlWidth" );
    double MsPlHeight          = dd4hep::_toDouble( "HcalMsPlHeight" );
    double MsPlStepLength      = dd4hep::_toDouble( "HcalMsPlStepLength" );
    double MsPlCalHoleR        = dd4hep::_toDouble( "HcalMsPlCalHoleR" );
    double Rotation90          = dd4hep::_toDouble( "HcalRotation90" );
    double ScTileWidth         = dd4hep::_toDouble( "HcalScTileWidth" );
    double ScTileHeigth        = dd4hep::_toDouble( "HcalScTileHeigth" );
    double ScTileLength        = dd4hep::_toDouble( "HcalScTileLength" );
    double ScTileCalHoleR      = dd4hep::_toDouble( "HcalScTileCalHoleR" );
    double SpacerWidth         = dd4hep::_toDouble( "HcalSpacerWidth" );
    double SpacerHeigth        = dd4hep::_toDouble( "HcalSpacerHeigth" );
    double SpacerLength        = dd4hep::_toDouble( "HcalSpacerLength" );
    double CellPeriodWidth     = dd4hep::_toDouble( "HcalCellPeriodWidth" );
    double OutCellXPos1stLayer = dd4hep::_toDouble( "HcalOutCellXPos1stLayer" );
    double OutCellXPos2ndLayer = dd4hep::_toDouble( "HcalOutCellXPos2ndLayer" );
    double OutCellXPos3rdLayer = dd4hep::_toDouble( "HcalOutCellXPos3rdLayer" );
    double OutCellXPos4thLayer = dd4hep::_toDouble( "HcalOutCellXPos4thLayer" );
    double OutCellXSize        = dd4hep::_toDouble( "HcalOutCellXSize" );

    // ******************************************************
    // Building the Master Plate (MsPl)
    // ******************************************************

    dd4hep::Tube   HcalOutCellCalHoleMsPl( 0, MsPlCalHoleR, MsPlWidth / 2 );
    dd4hep::Volume lvHcalOutCellCalHoleMsPlVol;
    lvHcalOutCellCalHoleMsPlVol =
        dd4hep::Volume( "lvHcalOutCellCalHoleMsPl", HcalOutCellCalHoleMsPl, description.material( "Air" ) );
    lvHcalOutCellCalHoleMsPlVol.setVisAttributes( description, "vHcal:Holes" );

    dd4hep::Box HcalOutCellMsPlBox( MsPlWidth / 2, MsPlHeight / 2, 2.0 * MsPlStepLength / 2 );

    dd4hep::RotationZYX rot_90( 0, Rotation90, 0 );
    dd4hep::Position    pos_1stCalHole( 0, 0, -0.5 * MsPlStepLength );
    dd4hep::Position    pos_2ndCalHole( 0, 0, 0.5 * MsPlStepLength );

    dd4hep::Volume lvHcalOutCellMsPlVol;
    lvHcalOutCellMsPlVol =
        dd4hep::Volume( "lvHcalOutCellMsPl", HcalOutCellMsPlBox, description.material( "CaloSteel" ) );
    lvHcalOutCellMsPlVol.setVisAttributes( description, "vHcal:MsPlSteel" );
    lvHcalOutCellMsPlVol.placeVolume( lvHcalOutCellCalHoleMsPlVol, dd4hep::Transform3D( rot_90, pos_1stCalHole ) );
    lvHcalOutCellMsPlVol.placeVolume( lvHcalOutCellCalHoleMsPlVol, dd4hep::Transform3D( rot_90, pos_2ndCalHole ) );

    registerVolume( lvHcalOutCellMsPlVol.name(), lvHcalOutCellMsPlVol );

    // ******************************************************
    // Building the scintillating tile
    // ******************************************************

    dd4hep::Tube   HcalOutCellCalHoleScTile( 0, ScTileCalHoleR, ScTileWidth / 2 );
    dd4hep::Volume lvHcalOutCellCalHoleScTileVol;
    lvHcalOutCellCalHoleScTileVol =
        dd4hep::Volume( "lvHcalOutCellCalHoleScTile", HcalOutCellCalHoleScTile, description.material( "Air" ) );
    lvHcalOutCellCalHoleScTileVol.setVisAttributes( description, "vHcal:Holes" );

    dd4hep::Box HcalOutCellScTileBox( ScTileWidth / 2, ScTileHeigth / 2, ScTileLength / 2 );

    dd4hep::Volume lvHcalOutCellScTileVol;
    lvHcalOutCellScTileVol =
        dd4hep::Volume( "lvHcalOutCellScTile", HcalOutCellScTileBox, description.material( "CaloSc" ) );

    dd4hep::Position pos_ScTileCalHole( 0, 0, 0 );

    lvHcalOutCellScTileVol.placeVolume( lvHcalOutCellCalHoleScTileVol,
                                        dd4hep::Transform3D( rot_90, pos_ScTileCalHole ) );

    lvHcalOutCellScTileVol.setVisAttributes( description, "vHcal:Sc" );
    lvHcalOutCellScTileVol.setSensitiveDetector( sensitive );

    registerVolume( lvHcalOutCellScTileVol.name(), lvHcalOutCellScTileVol );

    // ******************************************************
    // Building the forward and backward big inner spacers
    // ******************************************************

    dd4hep::Tube HcalOutCellCalHoleSpacer( 0, MsPlCalHoleR, SpacerWidth / 2 );
    dd4hep::Box  HcalOutCellBigSpacerBox( SpacerWidth / 2, SpacerHeigth / 2,
                                         ( 0.5 * SpacerLength + 0.5 * MsPlStepLength ) / 2 );

    dd4hep::Volume lvHcalOutCellCalHoleSpacerVol;
    lvHcalOutCellCalHoleSpacerVol =
        dd4hep::Volume( "lvHcalOutCellCalHoleSpacer", HcalOutCellCalHoleSpacer, description.material( "Air" ) );
    lvHcalOutCellCalHoleSpacerVol.setVisAttributes( description, "vHcal:Holes" );

    dd4hep::Position pos_FwSpacerCalHole( 0, 0, -0.25 * SpacerLength + 0.25 * MsPlStepLength );
    dd4hep::Position pos_BkSpacerCalHole( 0, 0, 0.25 * SpacerLength - 0.25 * MsPlStepLength );

    dd4hep::Volume lvHcalOutCellFwBigSpacerVol;
    lvHcalOutCellFwBigSpacerVol =
        dd4hep::Volume( "lvHcalOutCellFwBigSpacer", HcalOutCellBigSpacerBox, description.material( "CaloSteel" ) );
    lvHcalOutCellFwBigSpacerVol.setVisAttributes( description, "vHcal:MsPlSteel" );
    lvHcalOutCellFwBigSpacerVol.placeVolume( lvHcalOutCellCalHoleSpacerVol,
                                             dd4hep::Transform3D( rot_90, pos_FwSpacerCalHole ) );

    dd4hep::Volume lvHcalOutCellBkBigSpacerVol;
    lvHcalOutCellBkBigSpacerVol =
        dd4hep::Volume( "lvHcalOutCellBkBigSpacer", HcalOutCellBigSpacerBox, description.material( "CaloSteel" ) );
    lvHcalOutCellBkBigSpacerVol.setVisAttributes( description, "vHcal:MsPlSteel" );
    lvHcalOutCellBkBigSpacerVol.placeVolume( lvHcalOutCellCalHoleSpacerVol,
                                             dd4hep::Transform3D( rot_90, pos_BkSpacerCalHole ) );

    registerVolume( lvHcalOutCellFwBigSpacerVol.name(), lvHcalOutCellFwBigSpacerVol );
    registerVolume( lvHcalOutCellBkBigSpacerVol.name(), lvHcalOutCellBkBigSpacerVol );

    // ******************************************************
    // Building the forward and backward small inner spacers (single volume)
    // ******************************************************

    dd4hep::Box HcalOutCellSmSpacerBox( SpacerWidth / 2, SpacerHeigth / 2,
                                        ( 0.5 * SpacerLength - 0.5 * MsPlStepLength ) / 2 );

    dd4hep::Volume lvHcalOutCellSmSpacerVol;
    lvHcalOutCellSmSpacerVol =
        dd4hep::Volume( "lvHcalOutCellSmSpacer", HcalOutCellSmSpacerBox, description.material( "CaloSteel" ) );
    lvHcalOutCellSmSpacerVol.setVisAttributes( description, "vHcal:MsPlSteel" );

    // ******************************************************
    // Building the working stuff (common part)
    // ******************************************************

    dd4hep::Box    HcalOutCellComPartWorkStuff( CellPeriodWidth / 2, MsPlHeight / 2, 2.0 * MsPlStepLength / 2 );
    dd4hep::Volume lvHcalOutCellComPartWorkStuffVol;
    lvHcalOutCellComPartWorkStuffVol =
        dd4hep::Volume( "lvHcalOutCellComPartWorkStuff", HcalOutCellComPartWorkStuff, description.material( "Air" ) );
    lvHcalOutCellComPartWorkStuffVol.setVisAttributes( description, "vHcal:Air" );

    lvHcalOutCellComPartWorkStuffVol.placeVolume( lvHcalOutCellMsPlVol,
                                                  dd4hep::Position( OutCellXPos1stLayer * CellPeriodWidth, 0, 0 ) );
    lvHcalOutCellComPartWorkStuffVol.placeVolume(
        lvHcalOutCellSmSpacerVol,
        dd4hep::Position( OutCellXPos2ndLayer * CellPeriodWidth, 0, -1.25 * MsPlStepLength + 0.25 * SpacerLength ) );
    pv = lvHcalOutCellComPartWorkStuffVol.placeVolume(
        lvHcalOutCellScTileVol, dd4hep::Position( OutCellXPos2ndLayer * CellPeriodWidth, 0, -0.5 * MsPlStepLength ) );
    pv->SetName( "OutCellScTile_0" );
    pv.addPhysVolID( "sctile", 0 );
    lvHcalOutCellComPartWorkStuffVol.placeVolume(
        lvHcalOutCellBkBigSpacerVol,
        dd4hep::Position( OutCellXPos2ndLayer * CellPeriodWidth, 0, 0.75 * MsPlStepLength - 0.25 * SpacerLength ) );
    lvHcalOutCellComPartWorkStuffVol.placeVolume( lvHcalOutCellMsPlVol,
                                                  dd4hep::Position( OutCellXPos3rdLayer * CellPeriodWidth, 0, 0 ) );
    lvHcalOutCellComPartWorkStuffVol.placeVolume(
        lvHcalOutCellSmSpacerVol,
        dd4hep::Position( OutCellXPos4thLayer * CellPeriodWidth, 0, 1.25 * MsPlStepLength - 0.25 * SpacerLength ) );
    pv = lvHcalOutCellComPartWorkStuffVol.placeVolume(
        lvHcalOutCellScTileVol, dd4hep::Position( OutCellXPos4thLayer * CellPeriodWidth, 0, 0.5 * MsPlStepLength ) );
    pv->SetName( "OutCellScTile_1" );
    pv.addPhysVolID( "sctile", 1 );
    lvHcalOutCellComPartWorkStuffVol.placeVolume(
        lvHcalOutCellFwBigSpacerVol,
        dd4hep::Position( OutCellXPos4thLayer * CellPeriodWidth, 0, -0.75 * MsPlStepLength + 0.25 * SpacerLength ) );

    registerVolume( lvHcalOutCellComPartWorkStuffVol.name(), lvHcalOutCellComPartWorkStuffVol );

    // ******************************************************
    // Assembling the working stuff
    // ******************************************************

    dd4hep::Box    HcalOutCellWorkStuffBox( OutCellXSize / 2, MsPlHeight / 2, 6.0 * MsPlStepLength / 2 );
    dd4hep::Volume lvHcalOutCellWorkStuffVol;
    lvHcalOutCellWorkStuffVol =
        dd4hep::Volume( "lvHcalOutCellWorkStuff", HcalOutCellWorkStuffBox, description.material( "Air" ) );
    lvHcalOutCellWorkStuffVol.setVisAttributes( description, "vHcal:Air" );

    if ( m_build_outer_cell_workparts ) {
      int n_cellworkpart = 0;
      for ( int nx = 0; nx < 13; ++nx ) {
        for ( int nz = 0; nz < 3; ++nz ) {
          pv = lvHcalOutCellWorkStuffVol.placeVolume(
              lvHcalOutCellComPartWorkStuffVol,
              dd4hep::Position( ( nx - 6.0 ) * CellPeriodWidth, 0, ( 2 * nz - 2.0 ) * MsPlStepLength ) );
          pv->SetName( dd4hep::_toString( n_cellworkpart, "pvOutCellComPartWorkStuff_%02d" ).c_str() );
          pv.addPhysVolID( "cellworkpart", n_cellworkpart );
          ++n_cellworkpart;
        }
      }
    }

    registerVolume( lvHcalOutCellWorkStuffVol.name(), lvHcalOutCellWorkStuffVol );
  }

  void HcalBuild::build_outer_section() {
    dd4hep::PlacedVolume pv;

    // ******************************************************
    // Loading parameters
    // ******************************************************

    double ModXSize        = dd4hep::_toDouble( "HcalModXSize" );
    double ModYSize        = dd4hep::_toDouble( "HcalModYSize" );
    double YGap            = dd4hep::_toDouble( "HcalYGap" );
    double TotLength       = dd4hep::_toDouble( "HcalTotLength" );
    double Tolerance       = dd4hep::_toDouble( "HcalTolerance" );
    double OutCellXSize    = dd4hep::_toDouble( "HcalOutCellXSize" );
    double OutCellYSize    = dd4hep::_toDouble( "HcalOutCellYSize" );
    double FwFaceLength    = dd4hep::_toDouble( "HcalFwFaceLength" );
    double MsPlStepLength  = dd4hep::_toDouble( "HcalMsPlStepLength" );
    double BkFaceTotLength = dd4hep::_toDouble( "HcalBkFaceTotLength" );
    double BackShieldingZSize = dd4hep::_toDouble("HcalBackShieldingZSize");
    double InnerReinforceSubstituteXSize = dd4hep::_toDouble( "HcalInnerReinforceSubstituteXSize" );
    double InnerReinforceSubstituteYSize = dd4hep::_toDouble( "HcalInnerReinforceSubstituteYSize" );
    double OuterReinforceSubstituteXSize = dd4hep::_toDouble( "HcalOuterReinforceSubstituteXSize" );
    double OuterReinforceSubstituteYSize = dd4hep::_toDouble( "HcalOuterReinforceSubstituteYSize" );
    double OuterReinforceSubstituteZSize = dd4hep::_toDouble( "HcalOuterReinforceSubstituteZSize" );
    std::vector<double> DxLeft, DxRight;
    for (int i=1; i<= 26; ++i){
      double DxLeft_i = dd4hep::_toDouble( Form("HcalDxLeft%02d",i) );
      DxLeft.push_back( DxLeft_i );
      double DxRight_i = dd4hep::_toDouble( Form( "HcalDxRight%02d", i ) );
      DxRight.push_back( DxRight_i );
    }

    // ******************************************************
    // Building the work part of the half submodule
    // ******************************************************

    dd4hep::Box    HcalOutHalfSubModuleWorkPartBox( OutCellXSize / 2, OutCellYSize / 2,
                                                 ( FwFaceLength + 6.0 * MsPlStepLength + BkFaceTotLength ) / 2 );
    dd4hep::Volume lvHcalOutHalfSubModuleWorkPartVol;
    lvHcalOutHalfSubModuleWorkPartVol = dd4hep::Volume(
        "lvHcalOutHalfSubModuleWorkPart", HcalOutHalfSubModuleWorkPartBox, description.material( "Air" ) );
    lvHcalOutHalfSubModuleWorkPartVol.setVisAttributes( description, "vHcal:Air" );
    deOutSubModule = dd4hep::DetElement( "HcalOutHalfSubModuleBase", 0 );
    if ( m_build_outer_cells ) {
      lvHcalOutHalfSubModuleWorkPartVol.placeVolume(
          volume( "lvHcalFwFaceSubModule" ), dd4hep::Position( 0, 0, -3.0 * MsPlStepLength - 0.5 * BkFaceTotLength ) );
      pv = lvHcalOutHalfSubModuleWorkPartVol.placeVolume(
          volume( "lvHcalOutCellWorkStuff" ), dd4hep::Position( 0, 0, 0.5 * FwFaceLength - 0.5 * BkFaceTotLength ) );
      pv->SetName( "pvOutCellWorkStuff" );
      pv.addPhysVolID( "cell", 0 );
      lvHcalOutHalfSubModuleWorkPartVol.placeVolume(
          volume( "lvHcalBkFaceSubModule" ), dd4hep::Position( 0, 0, 0.5 * FwFaceLength + 3.0 * MsPlStepLength ) );
      deOutCellTmp = dd4hep::DetElement( "HcalOutCell", 0 );
      deOutCellTmp.setPlacement( pv );
      deOutSubModule.add( deOutCellTmp );
    }
    registerVolume( lvHcalOutHalfSubModuleWorkPartVol.name(), lvHcalOutHalfSubModuleWorkPartVol );

    // ******************************************************
    // Building the outer half module
    // ******************************************************
    double moduleLength = TotLength - BackShieldingZSize;
    dd4hep::Box    HcalOutHalfModuleBox( 0.5 * ModXSize / 2, ModYSize / 2, moduleLength / 2 );
    dd4hep::Volume lvHcalOutHalfModuleVol;
    lvHcalOutHalfModuleVol =
        dd4hep::Volume( "lvHcalOutHalfModule", HcalOutHalfModuleBox, description.material( "Air" ) );
    lvHcalOutHalfModuleVol.setVisAttributes( description, "vHcal:Air" );
    deOutModule = dd4hep::DetElement( "HcalOutHalfModuleBase", 0 );
    if ( m_build_outer_submodules ) {
      for ( int nx = 0; nx < 8; ++nx ) {
        pv = lvHcalOutHalfModuleVol.placeVolume(
            lvHcalOutHalfSubModuleWorkPartVol,
            dd4hep::Position( ( nx - 3.5 ) * OutCellXSize, 0,
                              -0.5 * moduleLength + 0.5 * FwFaceLength + 3.0 * MsPlStepLength + 0.5 * BkFaceTotLength ) );
        pv->SetName( dd4hep::_toString( nx, "pvOutHalfSubModuleWorkPart_%02d" ).c_str() );
        pv.addPhysVolID( "submodule", nx );
        deOutSubModuleTmp = deOutSubModule.clone( dd4hep::_toString( nx, "HcalOutHalfModule_%02d" ), nx );
        deOutSubModuleTmp.setPlacement( pv );
        deOutModule.add( deOutSubModuleTmp );
        lvHcalOutHalfModuleVol.placeVolume(
            volume("lvHcalHalfSubModuleReinforce"),
            dd4hep::Position((nx - 3.5) * OutCellXSize, 0,
                             0.5 * FwFaceLength + 3.0 * MsPlStepLength + 0.5 * BkFaceTotLength));
      }
    }
    registerVolume( lvHcalOutHalfModuleVol.name(), lvHcalOutHalfModuleVol );

    // ******************************************************
    // Building the left outer section
    // ******************************************************

    dd4hep::Box HcalOuterSectionBoxLeft( ModXSize / 2, 26 * ( ModYSize + YGap ) / 2, TotLength / 2 );
    dd4hep::Box HcalOuterSectionBoxLeftSubtract( 0.5 * ModXSize / 2, 14 * ( ModYSize + YGap ) / 2,
                                                 ( TotLength + Tolerance ) / 2 );
    dd4hep::SubtractionSolid HcalOuterSectionLeft( HcalOuterSectionBoxLeft, HcalOuterSectionBoxLeftSubtract,
                                                   dd4hep::Position( -0.25 * ModXSize, 0, 0 ) );
    dd4hep::Volume lvHcalOuterSectionLeftVol;
    lvHcalOuterSectionLeftVol =
        dd4hep::Volume( "lvHcalOuterSectionLeft", HcalOuterSectionLeft, description.material( "Air" ) );
    lvHcalOuterSectionLeftVol.setVisAttributes( description, "vHcal:Air" );

    dd4hep::Box HcalOuterReinforceSubstitute(0.5*OuterReinforceSubstituteXSize / 2,
                                                 OuterReinforceSubstituteYSize / 2,
                                                 OuterReinforceSubstituteZSize / 2);
    dd4hep::Box HcalOuterReinforceSubstituteSubtract(InnerReinforceSubstituteXSize / 2,
                                                     InnerReinforceSubstituteYSize / 2,
                                                     (OuterReinforceSubstituteZSize + Tolerance) / 2);
    dd4hep::SubtractionSolid HcalOuterReinforceSubstituteLeft(
        HcalOuterReinforceSubstitute, HcalOuterReinforceSubstituteSubtract,
        dd4hep::Position( -0.25 * OuterReinforceSubstituteXSize, 0, 0 ) );
    dd4hep::Volume lvHcalOuterReinforceSubstituteLeft( "lvHcalOuterReinforceSubstituteLeft", HcalOuterReinforceSubstituteLeft, description.material( "Air" ) );
    lvHcalOuterReinforceSubstituteLeft.setVisAttributes( description, "vHcal:Air" );
    lvHcalOuterSectionLeftVol.placeVolume(lvHcalOuterReinforceSubstituteLeft , dd4hep::Position(0,0,0.5*TotLength-0.5*OuterReinforceSubstituteZSize) );

    // ******************************************************
    // Building the right outer section
    // ******************************************************
    dd4hep::Box HcalOuterSectionBoxRight( ModXSize / 2, 26 * ( ModYSize + YGap ) / 2, TotLength / 2 );
    dd4hep::Box HcalOuterSectionBoxRightSubtract( 0.5 * ModXSize / 2, 14 * ( ModYSize + YGap ) / 2,
                                                  ( TotLength + Tolerance ) / 2 );
    dd4hep::SubtractionSolid HcalOuterSectionRight( HcalOuterSectionBoxRight, HcalOuterSectionBoxRightSubtract,
                                                    dd4hep::Position( 0.25 * ModXSize, 0, 0 ) );
    dd4hep::Volume lvHcalOuterSectionRightVol;
    lvHcalOuterSectionRightVol =
        dd4hep::Volume( "lvHcalOuterSectionRight", HcalOuterSectionRight, description.material( "Air" ) );
    lvHcalOuterSectionRightVol.setVisAttributes( description, "vHcal:Air" );

    dd4hep::SubtractionSolid HcalOuterReinforceSubstituteRight(
        HcalOuterReinforceSubstitute, HcalOuterReinforceSubstituteSubtract,
        dd4hep::Position( +0.25 * OuterReinforceSubstituteXSize, 0, 0 ) );
    dd4hep::Volume lvHcalOuterReinforceSubstituteRight( "lvHcalOuterReinforceSubstituteRight", HcalOuterReinforceSubstituteRight, description.material( "Air" ) );
    lvHcalOuterReinforceSubstituteRight.setVisAttributes( description, "vHcal:Air" );
    lvHcalOuterSectionRightVol.placeVolume(lvHcalOuterReinforceSubstituteRight , dd4hep::Position(0,0,0.5*TotLength-0.5*OuterReinforceSubstituteZSize) );


    if ( m_build_outer_modules ) {
      for ( int nside : {0, 1} ) { // nside=0 -> left side, nside=1 -> right side
        dd4hep::Volume*      lvSection   = 0;
        std::vector<double>* Dx          = 0;
        dd4hep::DetElement*  deOutRegion = 0;
        int                  n_module    = 0;
        if ( nside == 0 ) {
          lvSection       = &lvHcalOuterSectionLeftVol;
          Dx              = &DxLeft;
          deOutRegionLeft = dd4hep::DetElement( "HcalLeftOutRegion", 0 );
          deOutRegion     = &deOutRegionLeft;
        } else if ( nside == 1 ) {
          lvSection        = &lvHcalOuterSectionRightVol;
          Dx               = &DxRight;
          deOutRegionRight = dd4hep::DetElement( "HcalRightOutRegion", 0 );
          deOutRegion      = &deOutRegionRight;
        }
        for ( int nx = 0; nx < 2; ++nx ) {
          for ( int ny = 0; ny < 26; ++ny ) {
            double xpos = 0;
            if ( nside == 0 && nx == 0 ) xpos = -0.25 * ModXSize + Dx->at( ny ); // left, interrupted col
            if ( nside == 0 && nx == 1 ) xpos = +0.25 * ModXSize + Dx->at( ny ); // left, full col
            if ( nside == 1 && nx == 0 ) xpos = -0.25 * ModXSize - Dx->at( ny ); // right, full col
            if ( nside == 1 && nx == 1 ) xpos = +0.25 * ModXSize - Dx->at( ny ); // right, interrupted col
            double ypos = ( ny - 12.5 ) * ( ModYSize + YGap );
            double zpos = -0.5*TotLength+0.5*moduleLength;
            if ( ( nside != nx ) || ( ny < 5.5 ) || ( ny > 19.5 ) ) {
              pv = lvSection->placeVolume( lvHcalOutHalfModuleVol, dd4hep::Position( xpos, ypos, zpos ) );
              pv->SetName( dd4hep::_toString( n_module, "pvOutHalfModule_%02d" ).c_str() );
              pv.addPhysVolID( "module", n_module );
              deOutModuleTmp = deOutModule.clone( dd4hep::_toString( n_module, "HcalOutHalfModule_%02d" ), n_module );
              deOutModuleTmp.setPlacement( pv );
              deOutRegion->add( deOutModuleTmp );
            }
            ++n_module;
          }
        }
      }
    }
    registerVolume( lvHcalOuterSectionLeftVol.name(), lvHcalOuterSectionLeftVol );
    registerVolume( lvHcalOuterSectionRightVol.name(), lvHcalOuterSectionRightVol );
  }

  void HcalBuild::build_back_shield_bricks(){

    double BackShieldingXColumnSize   = dd4hep::_toDouble( "HcalBackShieldingXColumnSize" );
    double BackShieldingXHalfSize     = dd4hep::_toDouble( "HcalBackShieldingXHalfSize" );
    double BackShieldingZSize         = dd4hep::_toDouble( "HcalBackShieldingZSize" );
    double BackShieldingZReducedSize  = dd4hep::_toDouble( "HcalBackShieldingZReducedSize" );
    double BackShieldingBrick1YSize   = dd4hep::_toDouble( "HcalBackShieldingBrick1YSize" );
    double BackShieldingBrick2YSize   = dd4hep::_toDouble( "HcalBackShieldingBrick2YSize" );
    double BackShieldingBrick4YSize   = dd4hep::_toDouble( "HcalBackShieldingBrick4YSize" );
    double BackShieldingBrick3YSize   = dd4hep::_toDouble( "HcalBackShieldingBrick3YSize" );


    dd4hep::Box HcalBackShieldingBrick1Box( BackShieldingXHalfSize/2.0,
                                            BackShieldingBrick1YSize/2.0,
                                            BackShieldingZReducedSize/2.0 );
    dd4hep::Box HcalBackShieldingBrick2Box( BackShieldingXColumnSize/2.0,
                                            BackShieldingBrick2YSize/2.0,
                                            BackShieldingZReducedSize/2.0 );
    dd4hep::Box HcalBackShieldingBrick3Box( BackShieldingXColumnSize/2.0,
                                            BackShieldingBrick3YSize/2.0,
                                            BackShieldingZSize/2.0 );
    dd4hep::Box HcalBackShieldingBrick4Box( BackShieldingXColumnSize/2.0,
                                            BackShieldingBrick4YSize/2.0,
                                            BackShieldingZReducedSize/2.0 );

    dd4hep::Volume lvHcalBackShieldingBrick1( "lvHcalBackShieldingBrick1", HcalBackShieldingBrick1Box, description.material( "Tungsten" ) );
    dd4hep::Volume lvHcalBackShieldingBrick2( "lvHcalBackShieldingBrick2", HcalBackShieldingBrick2Box, description.material( "Tungsten" ) );
    dd4hep::Volume lvHcalBackShieldingBrick3( "lvHcalBackShieldingBrick3", HcalBackShieldingBrick3Box, description.material( "Tungsten" ) );
    dd4hep::Volume lvHcalBackShieldingBrick4( "lvHcalBackShieldingBrick4", HcalBackShieldingBrick4Box, description.material( "Tungsten" ) );

    lvHcalBackShieldingBrick1.setVisAttributes( description, "vHcal:Tungsten" );
    lvHcalBackShieldingBrick2.setVisAttributes( description, "vHcal:Tungsten" );
    lvHcalBackShieldingBrick3.setVisAttributes( description, "vHcal:Tungsten" );
    lvHcalBackShieldingBrick4.setVisAttributes( description, "vHcal:Tungsten" );

    registerVolume( lvHcalBackShieldingBrick1.name(), lvHcalBackShieldingBrick1 );
    registerVolume( lvHcalBackShieldingBrick2.name(), lvHcalBackShieldingBrick2 );
    registerVolume( lvHcalBackShieldingBrick3.name(), lvHcalBackShieldingBrick3 );
    registerVolume( lvHcalBackShieldingBrick4.name(), lvHcalBackShieldingBrick4 );
  }

  std::string HcalBuild::build_back_shield(std::string side_name) {

    // ******************************************************
    // Loading parameters
    // ******************************************************
    double BackShieldingMainHoleXSize = dd4hep::_toDouble( "HcalBackShieldingMainHoleXSize" );
    double BackShieldingXColumnSize   = dd4hep::_toDouble( "HcalBackShieldingXColumnSize" );
    double BackShieldingXHalfSize     = dd4hep::_toDouble( "HcalBackShieldingXHalfSize" );
    double BackShieldingYSize         = dd4hep::_toDouble( "HcalBackShieldingYSize" );
    double BackShieldingZSize         = dd4hep::_toDouble( "HcalBackShieldingZSize" );
    double BackShieldingZReducedSize  = dd4hep::_toDouble( "HcalBackShieldingZReducedSize" );
    double BackShieldingMainHoleYSize = dd4hep::_toDouble( "HcalBackShieldingMainHoleYSize" );
    double BackShieldingBrick1YSize   = dd4hep::_toDouble( "HcalBackShieldingBrick1YSize" );
    double BackShieldingBrick2YSize   = dd4hep::_toDouble( "HcalBackShieldingBrick2YSize" );
    double BackShieldingBrick4YSize   = dd4hep::_toDouble( "HcalBackShieldingBrick4YSize" );
    double BackShieldingBrick3YSize   = dd4hep::_toDouble( "HcalBackShieldingBrick3YSize" );
    double Delta                      = dd4hep::_toDouble( "HcalDelta" );

    double iside = 0;
    std::string lv_name;
    if ( side_name == "left" ) {
      iside   = 1;
      lv_name = "lvHcalBackShieldingLeft";
    } else if ( side_name == "right" ) {
      iside   = -1;
      lv_name = "lvHcalBackShieldingRight";
    } else {
      // TODO: throw error from here
    }

    dd4hep::Box HcalBackShieldingBox( BackShieldingXHalfSize/2.0,
                                      BackShieldingYSize/2.0,
                                      BackShieldingZSize/2.0 );
    dd4hep::Box HcalBackShieldingMainHole( BackShieldingMainHoleXSize,
                                           BackShieldingMainHoleYSize/2.0,
                                           BackShieldingZSize/2.0+Delta );
    dd4hep::Position pos_BackShieldingMainHole( iside*(-0.5*BackShieldingXHalfSize), 0.0, 0.0);
    dd4hep::SubtractionSolid HcalBackShielding( HcalBackShieldingBox,
                                                HcalBackShieldingMainHole,
                                                pos_BackShieldingMainHole );
    dd4hep::Volume lvHcalBackShielding( lv_name, HcalBackShielding, description.material( "Air" ) );
    lvHcalBackShielding.setVisAttributes( description, "vHcal:Air" );

    double x_column = iside*(0.5*BackShieldingXHalfSize-0.5*BackShieldingXColumnSize);
    double y_brick1 = 0.5*BackShieldingYSize-0.5*BackShieldingBrick1YSize;
    double y_brick2 = y_brick1-0.5*BackShieldingBrick1YSize-0.5*BackShieldingBrick2YSize;
    double y_brick3 = y_brick2-0.5*BackShieldingBrick2YSize-0.5*BackShieldingBrick3YSize;
    double y_brick4 = y_brick3-0.5*BackShieldingBrick3YSize-0.5*BackShieldingBrick4YSize;
    double z_brick_reducedZ = 0.5*BackShieldingZSize-0.5*BackShieldingZReducedSize;

    lvHcalBackShielding.placeVolume( volume("lvHcalBackShieldingBrick1"), dd4hep::Position(0, y_brick1, z_brick_reducedZ)  );
    lvHcalBackShielding.placeVolume( volume("lvHcalBackShieldingBrick2"), dd4hep::Position(x_column, y_brick2, z_brick_reducedZ)  );
    lvHcalBackShielding.placeVolume( volume("lvHcalBackShieldingBrick3"), dd4hep::Position(x_column, y_brick3, 0)  );
    lvHcalBackShielding.placeVolume( volume("lvHcalBackShieldingBrick4"), dd4hep::Position(x_column, y_brick4, z_brick_reducedZ)  );
    lvHcalBackShielding.placeVolume( volume("lvHcalBackShieldingBrick3"), dd4hep::Position(x_column, -y_brick3, 0)  );
    lvHcalBackShielding.placeVolume( volume("lvHcalBackShieldingBrick2"), dd4hep::Position(x_column, -y_brick2, z_brick_reducedZ)  );
    lvHcalBackShielding.placeVolume( volume("lvHcalBackShieldingBrick1"), dd4hep::Position(0, -y_brick1, z_brick_reducedZ)  );

    registerVolume( lv_name.c_str(), lvHcalBackShielding );
    return lv_name;
  }


  dd4hep::Volume HcalBuild::build_hcal() {

    dd4hep::PlacedVolume pv;

    // ******************************************************
    // Loading parameters
    // ******************************************************
    double TotXSize          = dd4hep::_toDouble( "HcalTotXSize" );
    double TotYSize          = dd4hep::_toDouble( "HcalTotYSize" );
    double TotSpace          = dd4hep::_toDouble( "HcalTotSpace" );
    double BeamPlugXSize     = dd4hep::_toDouble( "HcalBeamPlugXSize" );
    double YShift            = dd4hep::_toDouble( "HcalYShift" );
    double TiltAngle         = dd4hep::_toDouble( "HcalTiltAngle" );
    double ModXSize          = dd4hep::_toDouble( "HcalModXSize" );
    double BeamPlugOffset    = dd4hep::_toDouble( "HcalBeamPlugOffset" );
    dd4hep::Polycone HcalPipeHole = get_PipeHole();

    // ******************************************************
    // Build HCAL: left side
    // ******************************************************
    dd4hep::Box      HcalBoxLeft( 0.5 * TotXSize / 2, TotYSize / 2, TotSpace / 2 );
    dd4hep::RotationZYX rot_HcalLeft( 0., 0., -TiltAngle );
    dd4hep::Position    pos_HcalLeft( -TotXSize * 0.25, -YShift, 0. );
    dd4hep::SubtractionSolid HcalTotLeft( HcalBoxLeft, HcalPipeHole,
                                          dd4hep::Transform3D( rot_HcalLeft, pos_HcalLeft ) );
    dd4hep::Volume lvHcalLeftVol( "lvHcalLeft", HcalTotLeft, description.material( "Air" ) );
    lvHcalLeftVol.setVisAttributes( description, "vHcal:Air" );
    if (m_build_BeamPlug){
      lvHcalLeftVol.placeVolume( volume( "lvPlugSupportLeft" ),
                                 dd4hep::Position( -0.25 * TotXSize + 0.25 * BeamPlugXSize, 0, BeamPlugOffset ) );
    }
    pv = lvHcalLeftVol.placeVolume( volume( "lvHcalInnerSectionLeft" ), dd4hep::Position( -0.25 * ModXSize, 0, 0 ) );
    pv->SetName( "pvHcalLeftInnRegion" );
    pv.addPhysVolID( "region", 1 );
    deInnRegionLeft.setPlacement( pv );
    pv = lvHcalLeftVol.placeVolume( volume( "lvHcalOuterSectionLeft" ), dd4hep::Position( 0, 0, 0 ) );
    pv->SetName( "pvHcalLeftOutRegion" );
    pv.addPhysVolID( "region", 0 );
    deOutRegionLeft.setPlacement( pv );
    registerVolume( lvHcalLeftVol.name(), lvHcalLeftVol );

    // ******************************************************
    // Build HCAL: right side
    // ******************************************************
    dd4hep::Box      HcalBoxRight( 0.5 * TotXSize / 2, TotYSize / 2, TotSpace / 2 );
    dd4hep::RotationZYX rot_HcalRight( 0., 0., -TiltAngle );
    dd4hep::Position    pos_HcalRight( TotXSize * 0.25, -YShift, 0. );
    dd4hep::SubtractionSolid HcalTotRight( HcalBoxRight, HcalPipeHole,
                                           dd4hep::Transform3D( rot_HcalRight, pos_HcalRight ) );
    dd4hep::Volume lvHcalRightVol( "lvHcalRight", HcalTotRight, description.material( "Air" ) );
    lvHcalRightVol.setVisAttributes( description, "vHcal:Air" );
    if (m_build_BeamPlug) {
      lvHcalRightVol.placeVolume( volume("lvPlugSupportRight"),
                                  dd4hep::Position( 0.25 * TotXSize - 0.25 * BeamPlugXSize, 0, BeamPlugOffset ) );
    }
    pv = lvHcalRightVol.placeVolume( volume( "lvHcalInnerSectionRight" ), dd4hep::Position( 0.25 * ModXSize, 0, 0 ) );
    pv->SetName( "pvHcalRightInnRegion" );
    pv.addPhysVolID( "region", 1 );
    deInnRegionRight.setPlacement( pv );
    pv = lvHcalRightVol.placeVolume( volume( "lvHcalOuterSectionRight" ), dd4hep::Position( 0, 0, 0 ) );
    pv->SetName( "pvHcalRightOutRegion" );
    pv.addPhysVolID( "region", 0 );
    deOutRegionRight.setPlacement( pv );

    // ******************************************************
    // Build whole HCAL
    // ******************************************************
    dd4hep::Box      HcalBox( ( TotXSize ) / 2, ( TotYSize ) / 2, ( TotSpace ) / 2 );
    dd4hep::RotationZYX rot_HcalTot( 0., 0., -TiltAngle );
    dd4hep::Position    pos_HcalTot( 0, -YShift, 0. );
    dd4hep::SubtractionSolid HcalTot( HcalBox, HcalPipeHole, dd4hep::Transform3D( rot_HcalTot, pos_HcalTot ) );
    dd4hep::Volume lvHcalVol( "lvHcal", HcalTot, description.material( "Air" ) );
    lvHcalVol.setVisAttributes( description, "vHcal:Air" );


    pv = lvHcalVol.placeVolume( lvHcalLeftVol, dd4hep::Position( 0.25 * TotXSize, 0, 0 ) );
    pv->SetName( "pvHcalLeft" );
    pv.addPhysVolID( "side", 0 );
    deLeft  = dd4hep::DetElement( detector, "HcalLeft", 0 );
    deLeft.setPlacement( pv );
    deLeft.add( deOutRegionLeft );
    deLeft.add( deInnRegionLeft );

    pv = lvHcalVol.placeVolume( lvHcalRightVol, dd4hep::Position( -0.25 * TotXSize, 0, 0 ) );
    pv->SetName( "pvHcalRight" );
    pv.addPhysVolID( "side", 1 );
    deRight = dd4hep::DetElement( detector, "HcalRight", 1 );
    deRight.setPlacement( pv );
    deRight.add( deOutRegionRight );
    deRight.add( deInnRegionRight );

    registerVolume( lvHcalVol.name(), lvHcalVol );

    return lvHcalVol;
  }

} // namespace

static dd4hep::Ref_t create_element( dd4hep::Detector& description, xml_h e, dd4hep::SensitiveDetector sens_det ) {

  xml_det_t x_det = e;
  HcalBuild builder( description, x_det, sens_det );
  string    det_name = x_det.nameStr();

  dd4hep::printout( dd4hep::DEBUG, "CreateHcal", "Detector name: %s with ID: %d", det_name.c_str(), x_det.id() );

  PlacedVolume pv;
  sens_det.setType( "calorimeter" );
  builder.buildVolumes( e );

  // ******************************************************
  // Build HCAL beam plug
  // ******************************************************

  builder.build_beam_plug( "left" );
  builder.build_beam_plug( "right" );

  // ******************************************************
  // Build HCAL inner section
  // ******************************************************

  builder.build_inner_cell();
  builder.build_inner_section();

  // ******************************************************
  // Build HCAL outer section
  // ******************************************************

  builder.build_outer_cell();
  builder.build_outer_section();

  // ******************************************************
  // Assemble HCAL left and right regions
  // ******************************************************

  dd4hep::Volume lvHcalVol = builder.build_hcal();

  // ******************************************************
  // Place the HCAL in the world
  // ******************************************************

  pv = builder.placeDetector( lvHcalVol );
  pv.addPhysVolID( "system", x_det.id() );

  return builder.detector;
}

DECLARE_DETELEMENT( LHCb_Hcal_v1_0, create_element )
