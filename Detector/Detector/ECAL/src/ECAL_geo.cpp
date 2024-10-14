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
#include <Core/DeIOV.h>
#include <DD4hep/DetFactoryHelper.h>
#include <XML/Layering.h>
#include <iostream>
using namespace std;
using namespace dd4hep;

namespace {

  struct EcalBuild : public dd4hep::xml::tools::VolumeBuilder {
    bool m_build_Fibers    = true;
    bool m_build_Cells     = true;
    bool m_build_Modules   = true;
    bool m_build_EcalLeft  = true;
    bool m_build_EcalRight = true;
    bool m_build_Ecal      = true;

    dd4hep::DetElement              deCell, deInnModuleTmp, deMidModuleTmp, deOutModuleTmp, deRegionTmp;
    std::vector<dd4hep::DetElement> deInnModule, deMidModule, deOutModule;
    std::vector<dd4hep::DetElement> deInnBlock, deMidBlock, deOutBlock;
    std::vector<dd4hep::DetElement> deRegion, deSide;

    EcalBuild( dd4hep::Detector& description, xml_elt_t e, dd4hep::SensitiveDetector sens );
    void           build_inner_cell();
    void           build_middle_cell();
    void           build_outer_cell();
    void           build_inner_module();
    void           build_middle_module();
    void           build_outer_module();
    void           build_inner_sections();
    void           build_middle_sections();
    void           build_outer_sections();
    dd4hep::Volume build_ecal();
  };
  // ******************************************************
  // Initializing constructor
  // ******************************************************

  EcalBuild::EcalBuild( dd4hep::Detector& dsc, xml_elt_t e, dd4hep::SensitiveDetector sens )
      : dd4hep::xml::tools::VolumeBuilder( dsc, e, sens ) {
    // Process debug flags
    xml_comp_t x_dbg = x_det.child( _U( debug ), false );
    if ( x_dbg ) {
      for ( xml_coll_t i( x_dbg, _U( item ) ); i; ++i ) {
        xml_comp_t  c( i );
        std::string n = c.nameStr();
        if ( n == "debug" )
          debug = c.attr<int>( _U( value ) ) != 0;
        else if ( n == "EcalBuildFibers" )
          m_build_Fibers = c.attr<int>( _U( value ) ) != 0;
        else if ( n == "EcalBuildCells" )
          m_build_Cells = c.attr<int>( _U( value ) ) != 0;
        else if ( n == "EcalBuildModules" )
          m_build_Modules = c.attr<int>( _U( value ) ) != 0;
      }
    }
    if ( !m_build_Modules ) { m_build_Cells = false; };
    if ( !m_build_Cells ) { m_build_Fibers = false; };
  }

  void EcalBuild::build_inner_cell() {

    // ******************************************************
    // Loading parameters
    // ******************************************************

    double SteelOffset      = dd4hep::_toDouble( "EcalSteelOffset" );
    double PlasticOffset    = dd4hep::_toDouble( "EcalPlasticOffset" );
    double InnCellXYSize    = dd4hep::_toDouble( "EcalInnCellXYSize" );
    double StackLength      = dd4hep::_toDouble( "EcalStackLength" );
    double PbThick          = dd4hep::_toDouble( "EcalPbThick" );
    double InnSubCellXYSize = dd4hep::_toDouble( "EcalInnSubCellXYSize" );
    double ScThick          = dd4hep::_toDouble( "EcalScThick" );
    double PaperThick       = dd4hep::_toDouble( "EcalPaperThick" );
    double SteelThick       = dd4hep::_toDouble( "EcalSteelThick" );
    double PlasticThick     = dd4hep::_toDouble( "EcalPlasticThick" );
    double PbOffset         = dd4hep::_toDouble( "EcalPbOffset" );
    double FiberRadius      = dd4hep::_toDouble( "EcalFiberRadius" );

    // ******************************************************
    // Building the fibers tube
    // Same for all three ECAL sections
    // ******************************************************

    dd4hep::Tube   Fiber_Tube_Pb( 0, FiberRadius, PbThick / 2 );
    dd4hep::Volume lvFiberPbVol( "lvFiberPb", Fiber_Tube_Pb, description.material( "EcalFiber" ) );
    lvFiberPbVol.setVisAttributes( description, "vEcal:Fiber" );
    registerVolume( lvFiberPbVol.name(), lvFiberPbVol );

    // ******************************************************
    // Building the Pb cell box and install fibers
    // ******************************************************

    dd4hep::Box Pb_Cell_Box( InnCellXYSize / 2, InnCellXYSize / 2, PbThick / 2 );

    dd4hep::Volume lvInnCellPbVol( "lvInnCellPb", Pb_Cell_Box, description.material( "CaloPb" ) );
    lvInnCellPbVol.setVisAttributes( description, "vEcal:PbTiles" );

    double fiber_posx0 = -1.5 * InnSubCellXYSize; // -1.5*(121.2/3/4) = -15.15
    double fiber_posy0 = +1.5 * InnSubCellXYSize;

    double fiber_posx, fiber_posy;

    if ( m_build_Fibers ) {
      for ( Int_t nx = 0; nx < 4; ++nx ) {
        fiber_posx = fiber_posx0 + nx * InnSubCellXYSize;
        for ( Int_t ny = 0; ny < 4; ++ny ) {
          fiber_posy = fiber_posy0 - ny * InnSubCellXYSize;
          lvInnCellPbVol.placeVolume( lvFiberPbVol, dd4hep::Position( fiber_posx, fiber_posy, 0 ) );
        }
      }
    }

    // ******************************************************
    // Building the inner cell: EcalSc box, 66 (Pb+fibers) boxes
    // and steel and plastic plates (back and front)
    // ******************************************************

    dd4hep::Box    Inner_Cell_Main_Box( InnCellXYSize / 2, InnCellXYSize / 2, StackLength / 2 );
    dd4hep::Volume lvInnCellVol;
    lvInnCellVol = dd4hep::Volume( "lvInnCell", Inner_Cell_Main_Box, description.material( "CaloSc" ) );
    lvInnCellVol.setVisAttributes( description, "vEcal:Sc" );
    lvInnCellVol.setSensitiveDetector( sensitive );

    dd4hep::Box    Inner_Cell_Plastic_Box( InnCellXYSize / 2, InnCellXYSize / 2, PlasticThick / 2 );
    dd4hep::Volume lvInnCellPlasticVol;
    lvInnCellPlasticVol =
        dd4hep::Volume( "lvInnCellPlastic", Inner_Cell_Plastic_Box, description.material( "EcalPlastic" ) );
    lvInnCellPlasticVol.setVisAttributes( description, "vEcal:PlasticPlate" );

    dd4hep::Box    Inner_Cell_Steel_Box( InnCellXYSize / 2, InnCellXYSize / 2, SteelThick / 2 );
    dd4hep::Volume lvInnCellSteelVol;
    lvInnCellSteelVol = dd4hep::Volume( "lvInnCellSteel", Inner_Cell_Steel_Box, description.material( "CaloSteel" ) );
    lvInnCellSteelVol.setVisAttributes( description, "vEcal:StPlate" );

    lvInnCellVol.placeVolume( lvInnCellSteelVol, dd4hep::Position( 0, 0, SteelOffset ) );
    lvInnCellVol.placeVolume( lvInnCellSteelVol, dd4hep::Position( 0, 0, -1. * SteelOffset ) );
    lvInnCellVol.placeVolume( lvInnCellPlasticVol, dd4hep::Position( 0, 0, PlasticOffset ) );
    lvInnCellVol.placeVolume( lvInnCellPlasticVol, dd4hep::Position( 0, 0, -1. * PlasticOffset ) );

    double ScPbPaper_thickness = ScThick;
    ScPbPaper_thickness += PbThick;
    ScPbPaper_thickness += 2. * PaperThick;

    int nlayers = 66;
    for ( Int_t n = 0; n < nlayers; ++n ) {
      lvInnCellVol.placeVolume( lvInnCellPbVol, dd4hep::Position( 0, 0, PbOffset + n * ( ScPbPaper_thickness ) ) );
    }

    registerVolume( lvInnCellVol.name(), lvInnCellVol );
  }

  void EcalBuild::build_middle_cell() {

    // ******************************************************
    // Loading parameters
    // ******************************************************

    double SteelOffset      = dd4hep::_toDouble( "EcalSteelOffset" );
    double PlasticOffset    = dd4hep::_toDouble( "EcalPlasticOffset" );
    double MidCellXYSize    = dd4hep::_toDouble( "EcalMidCellXYSize" );
    double StackLength      = dd4hep::_toDouble( "EcalStackLength" );
    double PbThick          = dd4hep::_toDouble( "EcalPbThick" );
    double MidSubCellXYSize = dd4hep::_toDouble( "EcalMidSubCellXYSize" );
    double ScThick          = dd4hep::_toDouble( "EcalScThick" );
    double PaperThick       = dd4hep::_toDouble( "EcalPaperThick" );
    double SteelThick       = dd4hep::_toDouble( "EcalSteelThick" );
    double PlasticThick     = dd4hep::_toDouble( "EcalPlasticThick" );
    double PbOffset         = dd4hep::_toDouble( "EcalPbOffset" );

    // ******************************************************
    // Building the Pb cell box and install fibers
    // Fiber volume is defined in EcalBuild::build_inner_cell()
    // ******************************************************

    dd4hep::Box Pb_Mid_Cell_Box( MidCellXYSize / 2, MidCellXYSize / 2, PbThick / 2 );

    dd4hep::Volume lvMidCellPbVol( "lvMidCellPb", Pb_Mid_Cell_Box, description.material( "CaloPb" ) );
    lvMidCellPbVol.setVisAttributes( description, "vEcal:PbTiles" );

    double fiber_posx0 = -2.5 * MidSubCellXYSize; // -1.5*(121.2/3/4) = -15.15
    double fiber_posy0 = 2.5 * MidSubCellXYSize;

    double fiber_posx, fiber_posy;

    if ( m_build_Fibers ) {
      for ( Int_t nx = 0; nx < 6; ++nx ) {
        fiber_posx = fiber_posx0 + nx * MidSubCellXYSize;
        for ( Int_t ny = 0; ny < 6; ++ny ) {
          fiber_posy = fiber_posy0 - ny * MidSubCellXYSize;
          lvMidCellPbVol.placeVolume( volume( "lvFiberPb" ), dd4hep::Position( fiber_posx, fiber_posy, 0 ) );
        }
      }
    }

    // ******************************************************
    // Building the middle cell: EcalSc box, 66 (Pb+fibers) boxes
    // and steel and plastic plates (back and front)
    // ******************************************************

    dd4hep::Box    Mid_Cell_Main_Box( MidCellXYSize / 2, MidCellXYSize / 2, StackLength / 2 );
    dd4hep::Volume lvMidCellVol;
    lvMidCellVol = dd4hep::Volume( "lvMidCell", Mid_Cell_Main_Box, description.material( "CaloSc" ) );
    lvMidCellVol.setVisAttributes( description, "vEcal:Sc" );
    lvMidCellVol.setSensitiveDetector( sensitive );

    dd4hep::Box    Mid_Cell_Plastic_Box( MidCellXYSize / 2, MidCellXYSize / 2, PlasticThick / 2 );
    dd4hep::Volume lvMidCellPlasticVol;
    lvMidCellPlasticVol =
        dd4hep::Volume( "lvMidCellPlastic", Mid_Cell_Plastic_Box, description.material( "EcalPlastic" ) );
    lvMidCellPlasticVol.setVisAttributes( description, "vEcal:PlasticPlate" );

    dd4hep::Box    Mid_Cell_Steel_Box( MidCellXYSize / 2, MidCellXYSize / 2, SteelThick / 2 );
    dd4hep::Volume lvMidCellSteelVol;
    lvMidCellSteelVol = dd4hep::Volume( "lvMidCellSteel", Mid_Cell_Steel_Box, description.material( "CaloSteel" ) );
    lvMidCellSteelVol.setVisAttributes( description, "vEcal:StPlate" );

    lvMidCellVol.placeVolume( lvMidCellSteelVol, dd4hep::Position( 0, 0, SteelOffset ) );
    lvMidCellVol.placeVolume( lvMidCellSteelVol, dd4hep::Position( 0, 0, -1. * SteelOffset ) );
    lvMidCellVol.placeVolume( lvMidCellPlasticVol, dd4hep::Position( 0, 0, PlasticOffset ) );
    lvMidCellVol.placeVolume( lvMidCellPlasticVol, dd4hep::Position( 0, 0, -1. * PlasticOffset ) );

    double ScPbPaper_thickness = ScThick;
    ScPbPaper_thickness += PbThick;
    ScPbPaper_thickness += 2. * PaperThick;

    int nlayers = 66;
    for ( Int_t n = 0; n < nlayers; ++n ) {
      lvMidCellVol.placeVolume( lvMidCellPbVol, dd4hep::Position( 0, 0, PbOffset + n * ( ScPbPaper_thickness ) ) );
    }

    registerVolume( lvMidCellVol.name(), lvMidCellVol );
  }

  void EcalBuild::build_outer_cell() {

    // ******************************************************
    // Loading parameters
    // ******************************************************

    double SteelOffset      = dd4hep::_toDouble( "EcalSteelOffset" );
    double PlasticOffset    = dd4hep::_toDouble( "EcalPlasticOffset" );
    double OutCellXYSize    = dd4hep::_toDouble( "EcalOutCellXYSize" );
    double StackLength      = dd4hep::_toDouble( "EcalStackLength" );
    double PbThick          = dd4hep::_toDouble( "EcalPbThick" );
    double OutSubCellXYSize = dd4hep::_toDouble( "EcalOutSubCellXYSize" );
    double ScThick          = dd4hep::_toDouble( "EcalScThick" );
    double PaperThick       = dd4hep::_toDouble( "EcalPaperThick" );
    double SteelThick       = dd4hep::_toDouble( "EcalSteelThick" );
    double PlasticThick     = dd4hep::_toDouble( "EcalPlasticThick" );
    double PbOffset         = dd4hep::_toDouble( "EcalPbOffset" );

    // ******************************************************
    // Building the Pb cell box and install fibers
    // Fiber volume is defined in EcalBuild::build_inner_cell()
    // ******************************************************

    dd4hep::Box Pb_Out_Cell_Box( OutCellXYSize / 2, OutCellXYSize / 2, PbThick / 2 );

    dd4hep::Volume lvOutCellPbVol( "lvOutCellPb", Pb_Out_Cell_Box, description.material( "CaloPb" ) );
    lvOutCellPbVol.setVisAttributes( description, "vEcal:PbTiles" );

    double fiber_posx0 = -3.5 * OutSubCellXYSize;
    double fiber_posy0 = +3.5 * OutSubCellXYSize;

    double fiber_posx, fiber_posy;

    if ( m_build_Fibers ) {
      for ( Int_t nx = 0; nx < 8; ++nx ) {
        fiber_posx = fiber_posx0 + nx * OutSubCellXYSize;
        for ( Int_t ny = 0; ny < 8; ++ny ) {
          fiber_posy = fiber_posy0 - ny * OutSubCellXYSize;
          lvOutCellPbVol.placeVolume( volume( "lvFiberPb" ), dd4hep::Position( fiber_posx, fiber_posy, 0 ) );
        }
      }
    }

    // ******************************************************
    // Building the outer cell: EcalSc box, 66 (Pb+fibers) boxes
    // and steel and plastic plates (back and front)
    // ******************************************************

    dd4hep::Box    Out_Cell_Main_Box( OutCellXYSize / 2, OutCellXYSize / 2, StackLength / 2 );
    dd4hep::Volume lvOutCellVol;
    lvOutCellVol = dd4hep::Volume( "lvOutCell", Out_Cell_Main_Box, description.material( "CaloSc" ) );
    lvOutCellVol.setVisAttributes( description, "vEcal:Sc" );
    lvOutCellVol.setSensitiveDetector( sensitive );

    dd4hep::Box    Out_Cell_Plastic_Box( OutCellXYSize / 2, OutCellXYSize / 2, PlasticThick / 2 );
    dd4hep::Volume lvOutCellPlasticVol;
    lvOutCellPlasticVol =
        dd4hep::Volume( "lvOutCellPlastic", Out_Cell_Plastic_Box, description.material( "EcalPlastic" ) );
    lvOutCellPlasticVol.setVisAttributes( description, "vEcal:PlasticPlate" );

    dd4hep::Box    Out_Cell_Steel_Box( OutCellXYSize / 2, OutCellXYSize / 2, SteelThick / 2 );
    dd4hep::Volume lvOutCellSteelVol;
    lvOutCellSteelVol = dd4hep::Volume( "lvOutCellSteel", Out_Cell_Steel_Box, description.material( "CaloSteel" ) );
    lvOutCellSteelVol.setVisAttributes( description, "vEcal:StPlate" );

    lvOutCellVol.placeVolume( lvOutCellSteelVol, dd4hep::Position( 0, 0, SteelOffset ) );
    lvOutCellVol.placeVolume( lvOutCellSteelVol, dd4hep::Position( 0, 0, -1. * SteelOffset ) );
    lvOutCellVol.placeVolume( lvOutCellPlasticVol, dd4hep::Position( 0, 0, PlasticOffset ) );
    lvOutCellVol.placeVolume( lvOutCellPlasticVol, dd4hep::Position( 0, 0, -1. * PlasticOffset ) );

    double ScPbPaper_thickness = ScThick;
    ScPbPaper_thickness += PbThick;
    ScPbPaper_thickness += 2. * PaperThick;

    int nlayers = 66;
    for ( Int_t n = 0; n < nlayers; ++n ) {
      lvOutCellVol.placeVolume( lvOutCellPbVol, dd4hep::Position( 0, 0, PbOffset + n * ( ScPbPaper_thickness ) ) );
    }

    registerVolume( lvOutCellVol.name(), lvOutCellVol );
  }

  void EcalBuild::build_inner_module() {

    // ******************************************************
    // Loading parameters
    // ******************************************************

    double ModXSize            = dd4hep::_toDouble( "EcalModXSize" );
    double ModYSize            = dd4hep::_toDouble( "EcalModYSize" );
    double InnModLength        = dd4hep::_toDouble( "EcalInnModLength" );
    double InnFrontCoverOffset = dd4hep::_toDouble( "EcalInnFrontCoverOffset" );
    double InnBackCoverOffset  = dd4hep::_toDouble( "EcalInnBackCoverOffset" );
    double InnReadOutOffset    = dd4hep::_toDouble( "EcalInnReadOutOffset" );
    double StackLength         = dd4hep::_toDouble( "EcalStackLength" );
    double InnStackOffset      = dd4hep::_toDouble( "EcalInnStackOffset" );
    double InnFrontCoverLength = dd4hep::_toDouble( "EcalInnFrontCoverLength" );
    double InnBackCoverLength  = dd4hep::_toDouble( "EcalInnBackCoverLength" );
    double InnReadOutX         = dd4hep::_toDouble( "EcalInnReadOutX" );
    double InnReadOutY         = dd4hep::_toDouble( "EcalInnReadOutY" );
    double InnReadOutLength    = dd4hep::_toDouble( "EcalInnReadOutLength" );
    double TapeThick           = dd4hep::_toDouble( "EcalTapeThick" );
    double InnCellXYSize       = dd4hep::_toDouble( "EcalInnCellXYSize" );

    // ******************************************************
    // Define the temporary module detector element
    // ******************************************************

    deInnModuleTmp = DetElement( "InnModuleTmp", 999 );
    dd4hep::PlacedVolume pv;

    // ******************************************************
    // Building the front and back cover volumes
    // ******************************************************

    dd4hep::Box    Inner_Module_Front_Cover_Box( ModXSize / 2, ModYSize / 2, InnFrontCoverLength / 2 );
    dd4hep::Volume lvInnFrontCoverVol( "lvInnFrontCover", Inner_Module_Front_Cover_Box,
                                       description.material( "EcalPlasticInnFC" ) );
    lvInnFrontCoverVol.setVisAttributes( description, "vEcal:Plastic" );
    registerVolume( lvInnFrontCoverVol.name(), lvInnFrontCoverVol );

    dd4hep::Box    Inner_Module_Back_Cover_Box( ModXSize / 2, ModYSize / 2, InnBackCoverLength / 2 );
    dd4hep::Volume lvInnBackCoverVol( "lvInnBackCover", Inner_Module_Back_Cover_Box,
                                      description.material( "EcalPlasticBC" ) );
    lvInnBackCoverVol.setVisAttributes( description, "vEcal:Plastic" );
    registerVolume( lvInnBackCoverVol.name(), lvInnBackCoverVol );

    // ******************************************************
    // Building the Read Out Geometry (single plastic box)
    // ******************************************************

    dd4hep::Box    Inner_Read_Out_Box( InnReadOutX / 2, InnReadOutY / 2, InnReadOutLength / 2 );
    dd4hep::Volume lvInnReadOutBoxVol( "lvInnReadOutBox", Inner_Read_Out_Box, description.material( "EcalPlasticBC" ) );
    lvInnReadOutBoxVol.setVisAttributes( description, "vEcal:ReadOutBox" );

    dd4hep::Box    Inner_Module_Read_Out( ModXSize / 2, ModYSize / 2, InnReadOutLength / 2 );
    dd4hep::Volume lvInnReadOutVol( "lvInnReadOut", Inner_Module_Read_Out, description.material( "Air" ) );
    lvInnReadOutVol.placeVolume( lvInnReadOutBoxVol, dd4hep::Position( 0, 0, 0 ) );
    lvInnReadOutVol.setVisAttributes( description, "vEcal:Air" );
    registerVolume( lvInnReadOutVol.name(), lvInnReadOutVol );

    // ******************************************************
    // Building the Inner Module stack geometry
    // ******************************************************

    dd4hep::Box    Inner_Stack_Main_Box( ( ModXSize - 2 * TapeThick ) / 2, ( ModYSize - 2 * TapeThick ) / 2,
                                      StackLength / 2 );
    dd4hep::Volume lvInnStackVol( "lvInnStack", Inner_Stack_Main_Box, description.material( "Air" ) );

    if ( m_build_Cells ) {
      for ( Int_t nx = 0; nx < 3; ++nx ) {
        for ( Int_t ny = 0; ny < 3; ++ny ) {
          pv = lvInnStackVol.placeVolume(
              volume( "lvInnCell" ), dd4hep::Position( ( -1 + nx ) * InnCellXYSize, ( -1 + ny ) * InnCellXYSize, 0 ) );
          pv->SetName( dd4hep::_toString( ny + 3 * nx, "pvInnCell_%02d" ).c_str() );
          pv.addPhysVolID( "cell", ny + 3 * nx );
          deCell = DetElement( deInnModuleTmp, dd4hep::_toString( ny + 3 * nx, "InnCell_%02d" ).c_str(), ny + 3 * nx );
          deCell.setPlacement( pv );
        }
      }
    }

    lvInnStackVol.setVisAttributes( description, "vEcal:Stack" );
    registerVolume( lvInnStackVol.name(), lvInnStackVol );

    // ******************************************************
    // Assemble all parts of the Inner Module:
    // - Front cover
    // - Back cover
    // - Read out
    // - 3x3 inner cell matrix (InnStack)
    // ******************************************************

    dd4hep::Box    Inner_Module_Box( ModXSize / 2, ModYSize / 2, InnModLength / 2 );
    dd4hep::Volume lvInnModVol( "lvInnMod", Inner_Module_Box, description.material( "CaloSteel" ) );
    lvInnModVol.setVisAttributes( description, "vEcal:Steel" );
    lvInnModVol.placeVolume( volume( "lvInnFrontCover" ), dd4hep::Position( 0, 0, InnFrontCoverOffset ) );
    lvInnModVol.placeVolume( volume( "lvInnBackCover" ), dd4hep::Position( 0, 0, InnBackCoverOffset ) );
    lvInnModVol.placeVolume( volume( "lvInnReadOut" ), dd4hep::Position( 0, 0, InnReadOutOffset ) );
    lvInnModVol.placeVolume( lvInnStackVol, dd4hep::Position( 0, 0, InnStackOffset ) );

    registerVolume( lvInnModVol.name(), lvInnModVol );
  }

  void EcalBuild::build_middle_module() {

    // ******************************************************
    // Loading parameters
    // ******************************************************

    double ModXSize            = dd4hep::_toDouble( "EcalModXSize" );
    double ModYSize            = dd4hep::_toDouble( "EcalModYSize" );
    double MidModLength        = dd4hep::_toDouble( "EcalMidModLength" );
    double MidFrontCoverOffset = dd4hep::_toDouble( "EcalMidFrontCoverOffset" );
    double MidBackCoverOffset  = dd4hep::_toDouble( "EcalMidBackCoverOffset" );
    double MidReadOutOffset    = dd4hep::_toDouble( "EcalMidReadOutOffset" );
    double StackLength         = dd4hep::_toDouble( "EcalStackLength" );
    double MidStackOffset      = dd4hep::_toDouble( "EcalMidStackOffset" );
    double MidFrontCoverLength = dd4hep::_toDouble( "EcalMidFrontCoverLength" );
    double MidBackCoverLength  = dd4hep::_toDouble( "EcalMidBackCoverLength" );
    double TubeInnRadius       = dd4hep::_toDouble( "EcalTubeInnRadius" );
    double TubeOutRadius       = dd4hep::_toDouble( "EcalTubeOutRadius" );
    double MidReadOutLength    = dd4hep::_toDouble( "EcalMidReadOutLength" );
    double TapeThick           = dd4hep::_toDouble( "EcalTapeThick" );
    double MidCellXYSize       = dd4hep::_toDouble( "EcalMidCellXYSize" );

    // ******************************************************
    // Define the temporary module detector element
    // ******************************************************

    deMidModuleTmp = DetElement( "MidModuleTmp", 998 );
    dd4hep::PlacedVolume pv;

    // ******************************************************
    // Building the front and back cover volumes
    // ******************************************************

    dd4hep::Box    Middle_Module_Front_Cover_Box( ModXSize / 2, ModYSize / 2, MidFrontCoverLength / 2 );
    dd4hep::Volume lvMidFrontCoverVol( "lvMidFrontCover", Middle_Module_Front_Cover_Box,
                                       description.material( "EcalPlasticMidFC" ) );
    lvMidFrontCoverVol.setVisAttributes( description, "vEcal:Plastic" );
    registerVolume( lvMidFrontCoverVol.name(), lvMidFrontCoverVol );

    dd4hep::Box    Middle_Module_Back_Cover_Box( ModXSize / 2, ModYSize / 2, MidBackCoverLength / 2 );
    dd4hep::Volume lvMidBackCoverVol( "lvMidBackCover", Middle_Module_Back_Cover_Box,
                                      description.material( "EcalPlasticBC" ) );
    lvMidBackCoverVol.setVisAttributes( description, "vEcal:Plastic" );
    registerVolume( lvMidBackCoverVol.name(), lvMidBackCoverVol );

    // ******************************************************
    // Building the Read Out Geometry (2x2 steel tubes)
    // ******************************************************

    dd4hep::Tube   Middle_Read_Out_Tube( TubeInnRadius, TubeOutRadius, MidReadOutLength / 2 );
    dd4hep::Volume lvMidReadOutTubeVol( "lvMidReadOutTube", Middle_Read_Out_Tube, description.material( "CaloSteel" ) );
    lvMidReadOutTubeVol.setVisAttributes( description, "vEcal:ROSteel" );

    dd4hep::Box    Middle_Module_Read_Out( ModXSize / 2, ModYSize / 2, MidReadOutLength / 2 );
    dd4hep::Volume lvMidReadOutVol( "lvMidReadOut", Middle_Module_Read_Out, description.material( "Air" ) );

    for ( Int_t nx = 0; nx < 2; ++nx ) {
      for ( Int_t ny = 0; ny < 2; ++ny ) {
        lvMidReadOutVol.placeVolume(
            lvMidReadOutTubeVol, dd4hep::Position( ( nx - 0.5 ) * MidCellXYSize, ( 0.5 - ny ) * MidCellXYSize, 0 ) );
      }
    }

    lvMidReadOutVol.setVisAttributes( description, "vEcal:Air" );
    registerVolume( lvMidReadOutVol.name(), lvMidReadOutVol );

    // ******************************************************
    // Building the Middle Module stack geometry
    // ******************************************************

    dd4hep::Box    Middle_Stack_Main_Box( ( ModXSize - 2 * TapeThick ) / 2, ( ModYSize - 2 * TapeThick ) / 2,
                                       StackLength / 2 );
    dd4hep::Volume lvMidStackVol( "lvMidStack", Middle_Stack_Main_Box, description.material( "Air" ) );

    if ( m_build_Cells ) {
      for ( Int_t nx = 0; nx < 2; ++nx ) {
        for ( Int_t ny = 0; ny < 2; ++ny ) {
          pv = lvMidStackVol.placeVolume( volume( "lvMidCell" ), dd4hep::Position( ( -0.5 + nx ) * MidCellXYSize,
                                                                                   ( -0.5 + ny ) * MidCellXYSize, 0 ) );
          pv->SetName( dd4hep::_toString( ny + 2 * nx, "pvMidCell_%02d" ).c_str() );
          pv.addPhysVolID( "cell", ny + 2 * nx );
          deCell = DetElement( deMidModuleTmp, dd4hep::_toString( ny + 2 * nx, "MidCell_%02d" ).c_str(), ny + 2 * nx );
          deCell.setPlacement( pv );
        }
      }
    }

    lvMidStackVol.setVisAttributes( description, "vEcal:Stack" );
    registerVolume( lvMidStackVol.name(), lvMidStackVol );

    // ******************************************************
    // Assemble all parts of the Middle Module:
    // - Front cover
    // - Back cover
    // - Read out
    // - 2x2 Middle cell matrix (MidStack)
    // ******************************************************

    dd4hep::Box    Middle_Module_Box( ModXSize / 2, ModXSize / 2, MidModLength / 2 );
    dd4hep::Volume lvMidModVol( "lvMidMod", Middle_Module_Box, description.material( "CaloSteel" ) );
    lvMidModVol.setVisAttributes( description, "vEcal:Steel" );
    lvMidModVol.placeVolume( volume( "lvMidFrontCover" ), dd4hep::Position( 0, 0, MidFrontCoverOffset ) );
    lvMidModVol.placeVolume( volume( "lvMidBackCover" ), dd4hep::Position( 0, 0, MidBackCoverOffset ) );
    lvMidModVol.placeVolume( volume( "lvMidReadOut" ), dd4hep::Position( 0, 0, MidReadOutOffset ) );
    lvMidModVol.placeVolume( lvMidStackVol, dd4hep::Position( 0, 0, MidStackOffset ) );

    registerVolume( lvMidModVol.name(), lvMidModVol );
  }

  void EcalBuild::build_outer_module() {

    // ******************************************************
    // Loading parameters
    // ******************************************************

    double ModXSize            = dd4hep::_toDouble( "EcalModXSize" );
    double ModYSize            = dd4hep::_toDouble( "EcalModYSize" );
    double OutModLength        = dd4hep::_toDouble( "EcalOutModLength" );
    double OutFrontCoverOffset = dd4hep::_toDouble( "EcalOutFrontCoverOffset" );
    double OutBackCoverOffset  = dd4hep::_toDouble( "EcalOutBackCoverOffset" );
    double OutReadOutOffset    = dd4hep::_toDouble( "EcalOutReadOutOffset" );
    double StackLength         = dd4hep::_toDouble( "EcalStackLength" );
    double OutStackOffset      = dd4hep::_toDouble( "EcalOutStackOffset" );
    double OutFrontCoverLength = dd4hep::_toDouble( "EcalOutFrontCoverLength" );
    double OutBackCoverLength  = dd4hep::_toDouble( "EcalOutBackCoverLength" );
    double TubeInnRadius       = dd4hep::_toDouble( "EcalTubeInnRadius" );
    double TubeOutRadius       = dd4hep::_toDouble( "EcalTubeOutRadius" );
    double OutReadOutLength    = dd4hep::_toDouble( "EcalOutReadOutLength" );
    double TapeThick           = dd4hep::_toDouble( "EcalTapeThick" );

    // ******************************************************
    // Define the temporary module detector element
    // ******************************************************

    deOutModuleTmp = DetElement( "OutModuleTmp", 997 );
    dd4hep::PlacedVolume pv;

    // ******************************************************
    // Building the front and back cover volumes
    // ******************************************************

    dd4hep::Box    Outer_Module_Front_Cover_Box( ModXSize / 2, ModYSize / 2, OutFrontCoverLength / 2 );
    dd4hep::Volume lvOutFrontCoverVol( "lvOutFrontCover", Outer_Module_Front_Cover_Box,
                                       description.material( "EcalPlasticOutFC" ) );
    lvOutFrontCoverVol.setVisAttributes( description, "vEcal:Plastic" );
    registerVolume( lvOutFrontCoverVol.name(), lvOutFrontCoverVol );

    dd4hep::Box    Outer_Module_Back_Cover_Box( ModXSize / 2, ModYSize / 2, OutBackCoverLength / 2 );
    dd4hep::Volume lvOutBackCoverVol( "lvOutBackCover", Outer_Module_Back_Cover_Box,
                                      description.material( "EcalPlasticBC" ) );
    lvOutBackCoverVol.setVisAttributes( description, "vEcal:Plastic" );
    registerVolume( lvOutBackCoverVol.name(), lvOutBackCoverVol );

    // ******************************************************
    // Building the Read Out Geometry
    // ******************************************************

    dd4hep::Tube   Outer_Read_Out_Tube( TubeInnRadius, TubeOutRadius, OutReadOutLength / 2 );
    dd4hep::Volume lvOutReadOutTubeVol( "lvOutReadOutTube", Outer_Read_Out_Tube, description.material( "CaloSteel" ) );
    lvOutReadOutTubeVol.setVisAttributes( description, "vEcal:ROSteel" );

    dd4hep::Box    Outer_Module_Read_Out( ModXSize / 2, ModYSize / 2, OutReadOutLength / 2 );
    dd4hep::Volume lvOutReadOutVol( "lvOutReadOut", Outer_Module_Read_Out, description.material( "Air" ) );
    lvOutReadOutVol.placeVolume( lvOutReadOutTubeVol, dd4hep::Position( 0, 0, 0 ) );
    lvOutReadOutVol.setVisAttributes( description, "vEcal:Air" );
    registerVolume( lvOutReadOutVol.name(), lvOutReadOutVol );

    // ******************************************************
    // Building the Outer Module stack geometry
    // ******************************************************

    dd4hep::Box    Outer_Stack_Main_Box( ( ModXSize - 2 * TapeThick ) / 2, ( ModYSize - 2 * TapeThick ) / 2,
                                      StackLength / 2 );
    dd4hep::Volume lvOutStackVol( "lvOutStack", Outer_Stack_Main_Box, description.material( "Air" ) );

    if ( m_build_Cells ) {
      pv = lvOutStackVol.placeVolume( volume( "lvOutCell" ), dd4hep::Position( 0, 0, 0 ) );
      pv->SetName( "pvOutCell" );
      pv.addPhysVolID( "cell", 0 );
      deCell = DetElement( deOutModuleTmp, "OutCell", 0 );
      deCell.setPlacement( pv );
    }
    lvOutStackVol.setVisAttributes( description, "vEcal:Stack" );
    registerVolume( lvOutStackVol.name(), lvOutStackVol );

    // ******************************************************
    // Assemble all parts of the Outer Module:
    // - Front cover
    // - Back cover
    // - Read out
    // - OutStack
    // ******************************************************

    dd4hep::Box    Outer_Module_Box( ModXSize / 2, ModXSize / 2, OutModLength / 2 );
    dd4hep::Volume lvOutModVol( "lvOutMod", Outer_Module_Box, description.material( "CaloSteel" ) );
    lvOutModVol.setVisAttributes( description, "vEcal:Steel" );
    lvOutModVol.placeVolume( volume( "lvOutFrontCover" ), dd4hep::Position( 0, 0, OutFrontCoverOffset ) );
    lvOutModVol.placeVolume( volume( "lvOutBackCover" ), dd4hep::Position( 0, 0, OutBackCoverOffset ) );
    lvOutModVol.placeVolume( volume( "lvOutReadOut" ), dd4hep::Position( 0, 0, OutReadOutOffset ) );
    lvOutModVol.placeVolume( lvOutStackVol, dd4hep::Position( 0, 0, OutStackOffset ) );

    registerVolume( lvOutModVol.name(), lvOutModVol );
  }

  void EcalBuild::build_inner_sections() {

    // ******************************************************
    // Loading parameters
    // ******************************************************

    double ModXSize     = dd4hep::_toDouble( "EcalModXSize" );
    double ModYSize     = dd4hep::_toDouble( "EcalModYSize" );
    double InnModLength = dd4hep::_toDouble( "EcalInnModLength" );
    double InnXSize     = dd4hep::_toDouble( "EcalInnXSize" );
    double InnYSize     = dd4hep::_toDouble( "EcalInnYSize" );
    double SupportXSize = dd4hep::_toDouble( "EcalSupportXSize" );
    double SupportYSize = dd4hep::_toDouble( "EcalSupportYSize" );
    double Tolerance    = dd4hep::_toDouble( "EcalTolerance" );

    dd4hep::PlacedVolume pv;
    std::vector<int>     id_block1, id_block2, id_block3;

    // ******************************************************
    // Build Ecal Inner Block 1
    // ******************************************************

    dd4hep::Box    Inner_Block_1( 8 * ModXSize / 2, 2 * ModYSize / 2, InnModLength / 2 );
    dd4hep::Volume lvInnBlock1Vol( "lvInnBlock1", Inner_Block_1, description.material( "Air" ) );

    int mod_id = 0;

    if ( m_build_Modules ) {
      for ( Int_t nx = 0; nx < 8; ++nx ) {
        for ( Int_t ny = 0; ny < 2; ++ny ) {
          pv = lvInnBlock1Vol.placeVolume( volume( "lvInnMod" ),
                                           dd4hep::Position( ( -3.5 + nx ) * ModXSize, ( -0.5 + ny ) * ModYSize, 0 ) );
          pv->SetName( dd4hep::_toString( ny + 2 * nx, "pvInnModule_%02d" ).c_str() );
          pv.addPhysVolID( "module", ny + 2 * nx );
          deInnModule.push_back(
              deInnModuleTmp.clone( dd4hep::_toString( mod_id, "InnModuleBase_%d" ).c_str(), ny + 2 * nx ) );
          deInnModule.at( mod_id ).setPlacement( pv );
          id_block1.push_back( mod_id );
          mod_id++;
        }
      }
    }
    lvInnBlock1Vol.setVisAttributes( description, "vEcal:Air" );
    registerVolume( lvInnBlock1Vol.name(), lvInnBlock1Vol );

    // ******************************************************
    // Build Ecal Inner Block 2
    // ******************************************************

    dd4hep::Box    Inner_Block_2( 6 * ModXSize / 2, 2 * ModYSize / 2, InnModLength / 2 );
    dd4hep::Volume lvInnBlock2Vol( "lvInnBlock2", Inner_Block_2, description.material( "Air" ) );
    if ( m_build_Modules ) {
      for ( Int_t nx = 0; nx < 6; ++nx ) {
        for ( Int_t ny = 0; ny < 2; ++ny ) {
          pv = lvInnBlock2Vol.placeVolume( volume( "lvInnMod" ),
                                           dd4hep::Position( ( -2.5 + nx ) * ModXSize, ( -0.5 + ny ) * ModYSize, 0 ) );
          pv->SetName( dd4hep::_toString( ny + 2 * ( nx + 2 ), "pvInnModule_%02d" ).c_str() );
          pv.addPhysVolID( "module", ny + 2 * ( nx + 2 ) );
          deInnModule.push_back(
              deInnModuleTmp.clone( dd4hep::_toString( mod_id, "InnModuleBase_%d" ).c_str(), ny + 2 * ( nx + 2 ) ) );
          deInnModule.at( mod_id ).setPlacement( pv );
          id_block2.push_back( mod_id );
          mod_id++;
        }
      }
    }
    lvInnBlock2Vol.setVisAttributes( description, "vEcal:Air" );
    registerVolume( lvInnBlock2Vol.name(), lvInnBlock2Vol );

    // ******************************************************
    // Build Ecal Inner Block 3
    // ******************************************************

    dd4hep::Box    Inner_Block_3( 6 * ModXSize / 2, 2 * ModYSize / 2, InnModLength / 2 );
    dd4hep::Volume lvInnBlock3Vol( "lvInnBlock3", Inner_Block_3, description.material( "Air" ) );
    if ( m_build_Modules ) {
      for ( Int_t nx = 0; nx < 6; ++nx ) {
        for ( Int_t ny = 0; ny < 2; ++ny ) {
          pv = lvInnBlock3Vol.placeVolume( volume( "lvInnMod" ),
                                           dd4hep::Position( ( -2.5 + nx ) * ModXSize, ( -0.5 + ny ) * ModYSize, 0 ) );
          pv->SetName( dd4hep::_toString( ny + 2 * nx, "pvInnModule_%02d" ).c_str() );
          pv.addPhysVolID( "module", ny + 2 * nx );
          deInnModule.push_back(
              deInnModuleTmp.clone( dd4hep::_toString( mod_id, "InnModuleBase_%d" ).c_str(), ny + 2 * nx ) );
          deInnModule.at( mod_id ).setPlacement( pv );
          id_block3.push_back( mod_id );
          mod_id++;
        }
      }
    }
    lvInnBlock3Vol.setVisAttributes( description, "vEcal:Air" );
    registerVolume( lvInnBlock3Vol.name(), lvInnBlock3Vol );

    // ******************************************************
    // Build Ecal Inner Section Left
    // ******************************************************

    dd4hep::Box              Inner_Box_Left( 0.5 * InnXSize / 2, InnYSize / 2, InnModLength / 2 );
    dd4hep::Box              Support_Box_Left( SupportXSize / 2, SupportYSize / 2, ( InnModLength + Tolerance ) / 2 );
    dd4hep::SubtractionSolid Ecal_Inner_Section_Left( Inner_Box_Left, Support_Box_Left,
                                                      dd4hep::Position( -InnXSize * 0.25, 0, 0 ) );

    dd4hep::Volume lvInnSectionLeftVol( "lvInnSectionLeft", Ecal_Inner_Section_Left, description.material( "Air" ) );
    lvInnSectionLeftVol.setVisAttributes( description, "vEcal:Air" );

    Int_t b1size = id_block1.size();
    Int_t b2size = id_block2.size();
    Int_t b3size = id_block3.size();

    int innmax = b1size + b2size + b3size;

    pv = lvInnSectionLeftVol.placeVolume( lvInnBlock1Vol, dd4hep::Position( 0, ( -5 ) * ModYSize, 0 ) );
    pv->SetName( "pvLeftInnBlock_0" );
    pv.addPhysVolID( "block", 0 );
    deInnBlock.push_back( DetElement( "InnBlock0", 0 ) );
    deInnBlock.at( 0 ).setPlacement( pv );
    for ( int i = 0; i < b1size; ++i ) {
      auto j = id_block1.at( i );
      deInnBlock.at( 0 ).add( deInnModule.at( j ).clone( dd4hep::_toString( j, "InnModule_%d" ).c_str(), j ) );
    }

    pv = lvInnSectionLeftVol.placeVolume( lvInnBlock1Vol, dd4hep::Position( 0, ( -3 ) * ModYSize, 0 ) );
    pv->SetName( "pvLeftInnBlock_1" );
    pv.addPhysVolID( "block", 1 );
    deInnBlock.push_back( DetElement( "InnBlock1", 1 ) );
    deInnBlock.at( 1 ).setPlacement( pv );
    for ( int i = 0; i < b1size; ++i ) {
      auto j = id_block1.at( i );
      deInnBlock.at( 1 ).add( deInnModule.at( j ).clone( dd4hep::_toString( j, "InnModule_%d" ).c_str(), j ) );
    }

    pv = lvInnSectionLeftVol.placeVolume( lvInnBlock2Vol, dd4hep::Position( ModXSize, ( -1 ) * ModYSize, 0 ) );
    pv->SetName( "pvLeftInnBlock_2" );
    pv.addPhysVolID( "block", 2 );
    deInnBlock.push_back( DetElement( "InnBlock2", 2 ) );
    deInnBlock.at( 2 ).setPlacement( pv );
    for ( int i = 0; i < b2size; ++i ) {
      auto j = id_block2.at( i );
      deInnBlock.at( 2 ).add( deInnModule.at( j ).clone( dd4hep::_toString( j, "InnModule_%d" ).c_str(), j ) );
    }

    pv = lvInnSectionLeftVol.placeVolume( lvInnBlock2Vol, dd4hep::Position( ModXSize, ( +1 ) * ModYSize, 0 ) );
    pv->SetName( "pvLeftInnBlock_3" );
    pv.addPhysVolID( "block", 3 );
    deInnBlock.push_back( DetElement( "InnBlock3", 3 ) );
    deInnBlock.at( 3 ).setPlacement( pv );
    for ( int i = 0; i < b2size; ++i ) {
      auto j = id_block2.at( i );
      deInnBlock.at( 3 ).add( deInnModule.at( j ).clone( dd4hep::_toString( j, "InnModule_%d" ).c_str(), j ) );
    }

    pv = lvInnSectionLeftVol.placeVolume( lvInnBlock1Vol, dd4hep::Position( 0., ( +3 ) * ModYSize, 0 ) );
    pv->SetName( "pvLeftInnBlock_4" );
    pv.addPhysVolID( "block", 4 );
    deInnBlock.push_back( DetElement( "InnBlock4", 4 ) );
    deInnBlock.at( 4 ).setPlacement( pv );
    for ( int i = 0; i < b1size; ++i ) {
      auto j = id_block1.at( i );
      deInnBlock.at( 4 ).add( deInnModule.at( j ).clone( dd4hep::_toString( j, "InnModule_%d" ).c_str(), j ) );
    }

    pv = lvInnSectionLeftVol.placeVolume( lvInnBlock1Vol, dd4hep::Position( 0., ( +5 ) * ModYSize, 0 ) );
    pv->SetName( "pvLeftInnBlock_5" );
    pv.addPhysVolID( "block", 5 );
    deInnBlock.push_back( DetElement( "InnBlock5", 5 ) );
    deInnBlock.at( 5 ).setPlacement( pv );
    for ( int i = 0; i < b1size; ++i ) {
      auto j = id_block1.at( i );
      deInnBlock.at( 5 ).add( deInnModule.at( j ).clone( dd4hep::_toString( j, "InnModule_%d" ).c_str(), j ) );
    }

    lvInnSectionLeftVol.setVisAttributes( description, "vEcal:Air" );
    registerVolume( lvInnSectionLeftVol.name(), lvInnSectionLeftVol );

    // ******************************************************
    // Build Ecal Inner Section Right
    // ******************************************************

    dd4hep::Box              Inner_Box_Right( 0.5 * InnXSize / 2, InnYSize / 2, InnModLength / 2 );
    dd4hep::Box              Support_Box_Right( SupportXSize / 2, SupportYSize / 2, ( InnModLength + Tolerance ) / 2 );
    dd4hep::SubtractionSolid Ecal_Inner_Section_Right( Inner_Box_Right, Support_Box_Right,
                                                       dd4hep::Position( InnXSize * 0.25, 0, 0 ) );

    dd4hep::Volume lvInnSectionRightVol( "lvInnSectionRight", Ecal_Inner_Section_Right, description.material( "Air" ) );
    lvInnSectionRightVol.setVisAttributes( description, "vEcal:Air" );

    pv = lvInnSectionRightVol.placeVolume( lvInnBlock1Vol, dd4hep::Position( 0, ( -5 ) * ModYSize, 0 ) );
    pv->SetName( "pvRightInnBlock_0" );
    pv.addPhysVolID( "block", 0 );
    deInnBlock.push_back( DetElement( "InnBlock6", 6 ) );
    deInnBlock.at( 6 ).setPlacement( pv );
    for ( int i = 0; i < b1size; ++i ) {
      auto j = id_block1.at( i );
      deInnBlock.at( 6 ).add(
          deInnModule.at( j ).clone( dd4hep::_toString( innmax + j, "InnModule_%d" ).c_str(), innmax + j ) );
    }

    pv = lvInnSectionRightVol.placeVolume( lvInnBlock1Vol, dd4hep::Position( 0, ( -3 ) * ModYSize, 0 ) );
    pv->SetName( "pvRightInnBlock_1" );
    pv.addPhysVolID( "block", 1 );
    deInnBlock.push_back( DetElement( "InnBlock7", 7 ) );
    deInnBlock.at( 7 ).setPlacement( pv );
    for ( int i = 0; i < b1size; ++i ) {
      auto j = id_block1.at( i );
      deInnBlock.at( 7 ).add(
          deInnModule.at( j ).clone( dd4hep::_toString( innmax + j, "InnModule_%d" ).c_str(), innmax + j ) );
    }

    pv = lvInnSectionRightVol.placeVolume( lvInnBlock3Vol, dd4hep::Position( -ModXSize, ( -1 ) * ModYSize, 0 ) );
    pv->SetName( "pvRightInnBlock_2" );
    pv.addPhysVolID( "block", 2 );
    deInnBlock.push_back( DetElement( "InnBlock8", 8 ) );
    deInnBlock.at( 8 ).setPlacement( pv );
    for ( int i = 0; i < b3size; ++i ) {
      auto j = id_block3.at( i );
      deInnBlock.at( 8 ).add(
          deInnModule.at( j ).clone( dd4hep::_toString( innmax + j, "InnModule_%d" ).c_str(), innmax + j ) );
    }

    pv = lvInnSectionRightVol.placeVolume( lvInnBlock3Vol, dd4hep::Position( -ModXSize, ( +1 ) * ModYSize, 0 ) );
    pv->SetName( "pvRightInnBlock_3" );
    pv.addPhysVolID( "block", 3 );
    deInnBlock.push_back( DetElement( "InnBlock9", 9 ) );
    deInnBlock.at( 9 ).setPlacement( pv );
    for ( int i = 0; i < b3size; ++i ) {
      auto j = id_block3.at( i );
      deInnBlock.at( 9 ).add(
          deInnModule.at( j ).clone( dd4hep::_toString( innmax + j, "InnModule_%d" ).c_str(), innmax + j ) );
    }

    pv = lvInnSectionRightVol.placeVolume( lvInnBlock1Vol, dd4hep::Position( 0., ( +3 ) * ModYSize, 0 ) );
    pv->SetName( "pvRightInnBlock_4" );
    pv.addPhysVolID( "block", 4 );
    deInnBlock.push_back( DetElement( "InnBlock10", 10 ) );
    deInnBlock.at( 10 ).setPlacement( pv );
    for ( int i = 0; i < b1size; ++i ) {
      auto j = id_block1.at( i );
      deInnBlock.at( 10 ).add(
          deInnModule.at( j ).clone( dd4hep::_toString( innmax + j, "InnModule_%d" ).c_str(), innmax + j ) );
    }

    pv = lvInnSectionRightVol.placeVolume( lvInnBlock1Vol, dd4hep::Position( 0., ( +5 ) * ModYSize, 0 ) );
    pv->SetName( "pvRightInnBlock_5" );
    pv.addPhysVolID( "block", 5 );
    deInnBlock.push_back( DetElement( "InnBlock11", 11 ) );
    deInnBlock.at( 11 ).setPlacement( pv );
    for ( int i = 0; i < b1size; ++i ) {
      auto j = id_block1.at( i );
      deInnBlock.at( 11 ).add(
          deInnModule.at( j ).clone( dd4hep::_toString( innmax + j, "InnModule_%d" ).c_str(), innmax + j ) );
    }

    registerVolume( lvInnSectionRightVol.name(), lvInnSectionRightVol );
  }

  void EcalBuild::build_middle_sections() {

    // ******************************************************
    // Loading parameters
    // ******************************************************

    double ModXSize     = dd4hep::_toDouble( "EcalModXSize" );
    double ModYSize     = dd4hep::_toDouble( "EcalModYSize" );
    double MidModLength = dd4hep::_toDouble( "EcalMidModLength" );
    double MidXSize     = dd4hep::_toDouble( "EcalMidXSize" );
    double MidYSize     = dd4hep::_toDouble( "EcalMidYSize" );
    double InnXSize     = dd4hep::_toDouble( "EcalInnXSize" );
    double InnYSize     = dd4hep::_toDouble( "EcalInnYSize" );
    double Tolerance    = dd4hep::_toDouble( "EcalTolerance" );

    dd4hep::PlacedVolume pv;

    // ******************************************************
    // Build Ecal Middle Block
    // ******************************************************

    dd4hep::Box    Middle_Block( 8 * ModXSize / 2, 2 * ModYSize / 2, MidModLength / 2 );
    dd4hep::Volume lvMidBlockVol( "lvMidBlock", Middle_Block, description.material( "Air" ) );

    int mod_id = 0;
    if ( m_build_Modules ) {
      for ( Int_t nx = 0; nx < 8; ++nx ) {
        for ( Int_t ny = 0; ny < 2; ++ny ) {
          pv = lvMidBlockVol.placeVolume( volume( "lvMidMod" ),
                                          dd4hep::Position( ( -3.5 + nx ) * ModXSize, ( -0.5 + ny ) * ModYSize, 0 ) );
          pv->SetName( dd4hep::_toString( mod_id, "pvMidModule_%02d" ).c_str() );
          pv.addPhysVolID( "module", ny + 2 * nx );
          deMidModule.push_back(
              deMidModuleTmp.clone( dd4hep::_toString( mod_id, "MidModuleBase_%d" ).c_str(), mod_id ) );
          deMidModule.at( mod_id ).setPlacement( pv );
          mod_id++;
        }
      }
    }
    lvMidBlockVol.setVisAttributes( description, "vEcal:Air" );
    registerVolume( lvMidBlockVol.name(), lvMidBlockVol );

    // ******************************************************
    // Build Ecal Middle Section Left
    // ******************************************************

    dd4hep::Box              Middle_Box_Left( 0.5 * MidXSize / 2, MidYSize / 2, MidModLength / 2 );
    dd4hep::Box              Middle_Sub_Box_Left( InnXSize / 2, InnYSize / 2, ( MidModLength + Tolerance ) / 2 );
    dd4hep::SubtractionSolid Ecal_Middle_Section_Left( Middle_Box_Left, Middle_Sub_Box_Left,
                                                       dd4hep::Position( -MidXSize * 0.25, 0, 0 ) );

    dd4hep::Volume lvMidSectionLeftVol( "lvMidSectionLeft", Ecal_Middle_Section_Left, description.material( "Air" ) );
    int            midmax = deMidModule.size();
    for ( Int_t nx = 0; nx < 2; ++nx ) {
      for ( Int_t ny = 0; ny < 10; ++ny ) {
        if ( nx == 0 && ny > 1 && ny < 8 ) continue;
        Double_t pos_x = ModXSize * ( -4 + 8 * nx );
        Double_t pos_y = ModYSize * ( -9 + 2 * ny );
        Int_t    n     = ny + 10 * nx;
        pv             = lvMidSectionLeftVol.placeVolume( lvMidBlockVol, dd4hep::Position( pos_x, pos_y, 0 ) );
        pv->SetName( dd4hep::_toString( n, "pvRightMidBlock_%02d" ).c_str() );
        pv.addPhysVolID( "block", n );
        deMidBlock.push_back( DetElement( dd4hep::_toString( n, "MidBlock%02d" ), n ) );
        deMidBlock.back().setPlacement( pv );
        for ( int i = 0; i < midmax; ++i ) {
          deMidBlock.back().add( deMidModule.at( i ).clone( dd4hep::_toString( i, "MidModule_%d" ).c_str(), i ) );
        }
      }
    }
    lvMidSectionLeftVol.setVisAttributes( description, "vEcal:Air" );
    registerVolume( lvMidSectionLeftVol.name(), lvMidSectionLeftVol );

    // ******************************************************
    // Build Ecal Middle Section Right
    // ******************************************************

    dd4hep::Box              Middle_Box_Right( 0.5 * MidXSize / 2, MidYSize / 2, MidModLength / 2 );
    dd4hep::Box              Middle_Sub_Box_Right( InnXSize / 2, InnYSize / 2, ( MidModLength + Tolerance ) / 2 );
    dd4hep::SubtractionSolid Ecal_Middle_Section_Right( Middle_Box_Right, Middle_Sub_Box_Right,
                                                        dd4hep::Position( MidXSize * 0.25, 0, 0 ) );

    dd4hep::Volume lvMidSectionRightVol( "lvMidSectionRight", Ecal_Middle_Section_Right,
                                         description.material( "Air" ) );
    for ( Int_t nx = 0; nx < 2; ++nx ) {
      for ( Int_t ny = 0; ny < 10; ++ny ) {
        if ( nx == 1 && ny > 1 && ny < 8 ) continue;
        Double_t pos_x = ModXSize * ( -4 + 8 * nx );
        Double_t pos_y = ModYSize * ( -9 + 2 * ny );
        Int_t    n     = ny + 10 * nx;
        pv             = lvMidSectionRightVol.placeVolume( lvMidBlockVol, dd4hep::Position( pos_x, pos_y, 0 ) );
        pv->SetName( dd4hep::_toString( n, "pvRightMidBlock_%02d" ).c_str() );
        pv.addPhysVolID( "block", n );
        deMidBlock.push_back( DetElement( dd4hep::_toString( n, "MidBlock%02d" ), n ) );
        deMidBlock.back().setPlacement( pv );
        for ( int i = 0; i < midmax; ++i ) {
          deMidBlock.back().add( deMidModule.at( i ).clone( dd4hep::_toString( i, "MidModule_%d" ).c_str(), i ) );
        }
      }
    }
    lvMidSectionRightVol.setVisAttributes( description, "vEcal:Air" );
    registerVolume( lvMidSectionRightVol.name(), lvMidSectionRightVol );
  }

  void EcalBuild::build_outer_sections() {

    // ******************************************************
    // Loading parameters
    // ******************************************************

    double ModXSize     = dd4hep::_toDouble( "EcalModXSize" );
    double ModYSize     = dd4hep::_toDouble( "EcalModYSize" );
    double OutModLength = dd4hep::_toDouble( "EcalOutModLength" );
    double OutXSize     = dd4hep::_toDouble( "EcalOutXSize" );
    double OutYSize     = dd4hep::_toDouble( "EcalOutYSize" );
    double MidXSize     = dd4hep::_toDouble( "EcalMidXSize" );
    double MidYSize     = dd4hep::_toDouble( "EcalMidYSize" );
    double Tolerance    = dd4hep::_toDouble( "EcalTolerance" );

    dd4hep::PlacedVolume pv;

    // ******************************************************
    // Build Ecal Outer Block
    // ******************************************************

    dd4hep::Box    Outer_Block( 16 * ModXSize / 2, 2 * ModYSize / 2, OutModLength / 2 );
    dd4hep::Volume lvOutBlockVol( "lvOutBlock", Outer_Block, description.material( "Air" ) );

    int mod_id = 0;
    if ( m_build_Modules ) {
      for ( Int_t nx = 0; nx < 16; ++nx ) {
        for ( Int_t ny = 0; ny < 2; ++ny ) {
          pv = lvOutBlockVol.placeVolume( volume( "lvOutMod" ),
                                          dd4hep::Position( ( -7.5 + nx ) * ModXSize, ( -0.5 + ny ) * ModYSize, 0 ) );
          pv->SetName( dd4hep::_toString( mod_id, "pvOutModule_%02d" ).c_str() );
          pv.addPhysVolID( "module", ny + 2 * nx );
          deOutModule.push_back(
              deOutModuleTmp.clone( dd4hep::_toString( mod_id, "OutModuleBase_%d" ).c_str(), mod_id ) );
          deOutModule.at( mod_id ).setPlacement( pv );
          mod_id++;
        }
      }
    }
    lvOutBlockVol.setVisAttributes( description, "vEcal:Air" );
    registerVolume( lvOutBlockVol.name(), lvOutBlockVol );

    // ******************************************************
    // Build Ecal Outer Section Left
    // ******************************************************

    dd4hep::Box              Outer_Box_Left( 0.5 * OutXSize / 2, OutYSize / 2, OutModLength / 2 );
    dd4hep::Box              Outer_Sub_Box_Left( MidXSize / 2, MidYSize / 2, ( OutModLength + Tolerance ) / 2 );
    dd4hep::SubtractionSolid Ecal_Outer_Section_Left( Outer_Box_Left, Outer_Sub_Box_Left,
                                                      dd4hep::Position( -OutXSize * 0.25, 0, 0 ) );

    dd4hep::Volume lvOutSectionLeftVol( "lvOutSectionLeft", Ecal_Outer_Section_Left, description.material( "Air" ) );
    lvOutSectionLeftVol.setVisAttributes( description, "vEcal:Air" );

    int outmax = deOutModule.size();
    for ( Int_t nx = 0; nx < 2; ++nx ) {
      for ( Int_t ny = 0; ny < 26; ++ny ) {
        if ( nx == 0 && ny > 7 && ny < 18 ) continue;
        Double_t pos_x = ModXSize * ( -8 + 16 * nx );
        Double_t pos_y = ModYSize * ( -25 + 2 * ny );
        Int_t    n     = ny + 26 * nx;
        pv             = lvOutSectionLeftVol.placeVolume( lvOutBlockVol, dd4hep::Position( pos_x, pos_y, 0 ) );
        pv->SetName( dd4hep::_toString( n, "pvLeftOutBlock_%02d" ).c_str() );
        pv.addPhysVolID( "block", n );
        deOutBlock.push_back( DetElement( dd4hep::_toString( n, "OutBlock%02d" ), n ) );
        deOutBlock.back().setPlacement( pv );
        for ( int i = 0; i < outmax; ++i ) {
          deOutBlock.back().add( deOutModule.at( i ).clone( dd4hep::_toString( i, "OutModule_%d" ).c_str(), i ) );
        }
      }
    }
    lvOutSectionLeftVol.setVisAttributes( description, "vEcal:Air" );
    registerVolume( lvOutSectionLeftVol.name(), lvOutSectionLeftVol );

    // ******************************************************
    // Build Ecal Outer Section Right
    // ******************************************************

    dd4hep::Box              Outer_Box_Right( 0.5 * OutXSize / 2, OutYSize / 2, OutModLength / 2 );
    dd4hep::Box              Outer_Sub_Box_Right( MidXSize / 2, MidYSize / 2, ( OutModLength + Tolerance ) / 2 );
    dd4hep::SubtractionSolid Ecal_Outer_Section_Right( Outer_Box_Right, Outer_Sub_Box_Right,
                                                       dd4hep::Position( OutXSize * 0.25, 0, 0 ) );

    dd4hep::Volume lvOutSectionRightVol( "lvOutSectionRight", Ecal_Outer_Section_Right, description.material( "Air" ) );
    lvOutSectionRightVol.setVisAttributes( description, "vEcal:Air" );
    for ( Int_t nx = 0; nx < 2; ++nx ) {
      for ( Int_t ny = 0; ny < 26; ++ny ) {
        if ( nx == 1 && ny > 7 && ny < 18 ) continue;
        Double_t pos_x = ModXSize * ( -8 + 16 * nx );
        Double_t pos_y = ModYSize * ( -25 + 2 * ny );
        Int_t    n     = ny + 26 * nx;
        pv             = lvOutSectionRightVol.placeVolume( lvOutBlockVol, dd4hep::Position( pos_x, pos_y, 0 ) );
        pv->SetName( dd4hep::_toString( n, "pvRightOutBlock_%02d" ).c_str() );
        pv.addPhysVolID( "block", n );
        deOutBlock.push_back( DetElement( dd4hep::_toString( n, "OutBlock%02d" ), n ) );
        deOutBlock.back().setPlacement( pv );
        for ( int i = 0; i < outmax; ++i ) {
          deOutBlock.back().add( deOutModule.at( i ).clone( dd4hep::_toString( i, "OutModule_%d" ).c_str(), i ) );
        }
      }
    }
    registerVolume( lvOutSectionRightVol.name(), lvOutSectionRightVol );
  }

  dd4hep::Volume EcalBuild::build_ecal() {

    dd4hep::PlacedVolume pv;

    // ******************************************************
    // Loading parameters
    // ******************************************************

    double XSize                     = dd4hep::_toDouble( "EcalXSize" );
    double YSize                     = dd4hep::_toDouble( "EcalYSize" );
    double ZSize                     = dd4hep::_toDouble( "EcalZSize" );
    double YShift                    = dd4hep::_toDouble( "EcalYShift" );
    double TiltAngle                 = dd4hep::_toDouble( "EcalTiltAngle" );
    double ShieldOffset              = dd4hep::_toDouble( "EcalShieldOffset" );
    double SupportOffset             = dd4hep::_toDouble( "EcalSupportOffset" );
    double InnOffset                 = dd4hep::_toDouble( "EcalInnOffset" );
    double MidOffset                 = dd4hep::_toDouble( "EcalMidOffset" );
    double OutOffset                 = dd4hep::_toDouble( "EcalOutOffset" );
    double ModXSize                  = dd4hep::_toDouble( "EcalModXSize" );
    double OutXSize                  = dd4hep::_toDouble( "EcalOutXSize" );
    double OutYSize                  = dd4hep::_toDouble( "EcalOutYSize" );
    double ShieldZSize               = dd4hep::_toDouble( "EcalShieldZSize" );
    double Tolerance                 = dd4hep::_toDouble( "EcalTolerance" );
    double SupportXSize              = dd4hep::_toDouble( "EcalSupportXSize" );
    double SupportYSize              = dd4hep::_toDouble( "EcalSupportYSize" );
    double BeamPlugXSize             = dd4hep::_toDouble( "EcalBeamPlugXSize" );
    double BeamPlugYSize             = dd4hep::_toDouble( "EcalBeamPlugYSize" );
    double BeamPlugZSize             = dd4hep::_toDouble( "EcalBeamPlugZSize" );
    double SteelSheetThick1          = dd4hep::_toDouble( "EcalSteelSheetThick1" );
    double SteelSheetLength1         = dd4hep::_toDouble( "EcalSteelSheetLength1" );
    double SteelSheetThick2          = dd4hep::_toDouble( "EcalSteelSheetThick2" );
    double SteelSheetLength2         = dd4hep::_toDouble( "EcalSteelSheetLength2" );
    double SteelSheetWithHoleThick1  = dd4hep::_toDouble( "EcalSteelSheetWithHoleThick1" );
    double SteelSheetWithHoleStep1   = dd4hep::_toDouble( "EcalSteelSheetWithHoleStep1" );
    double SteelSheetWithHoleThick2  = dd4hep::_toDouble( "EcalSteelSheetWithHoleThick2" );
    double SteelSheetWithHoleStep2   = dd4hep::_toDouble( "EcalSteelSheetWithHoleStep2" );
    double Delta                     = dd4hep::_toDouble( "EcalDelta" );
    double Correction                = dd4hep::_toDouble( "EcalCorrection" );
    double SteelSheetWithHoleRadius1 = dd4hep::_toDouble( "EcalSteelSheetWithHoleRadius1" );
    double SteelSheetWithHoleRadius2 = dd4hep::_toDouble( "EcalSteelSheetWithHoleRadius2" );

    double BeamPlugPipeHoleR1 = dd4hep::_toDouble( "EcalBeamPlugPipeHoleR1" );
    double BeamPlugPipeHoleR2 = dd4hep::_toDouble( "EcalBeamPlugPipeHoleR2" );
    double BeamPlugPipeHoleR3 = dd4hep::_toDouble( "EcalBeamPlugPipeHoleR3" );
    double BeamPlugPipeHoleR4 = dd4hep::_toDouble( "EcalBeamPlugPipeHoleR4" );
    double BeamPlugPipeHoleR5 = dd4hep::_toDouble( "EcalBeamPlugPipeHoleR5" );
    double BeamPlugPipeHoleR6 = dd4hep::_toDouble( "EcalBeamPlugPipeHoleR6" );

    double BeamPlugPipeHoleZ1 = dd4hep::_toDouble( "EcalBeamPlugPipeHoleZ1" );
    double BeamPlugPipeHoleZ2 = dd4hep::_toDouble( "EcalBeamPlugPipeHoleZ2" );
    double BeamPlugPipeHoleZ3 = dd4hep::_toDouble( "EcalBeamPlugPipeHoleZ3" );
    double BeamPlugPipeHoleZ4 = dd4hep::_toDouble( "EcalBeamPlugPipeHoleZ4" );
    double BeamPlugPipeHoleZ5 = dd4hep::_toDouble( "EcalBeamPlugPipeHoleZ5" );
    double BeamPlugPipeHoleZ6 = dd4hep::_toDouble( "EcalBeamPlugPipeHoleZ6" );

    std::vector<double> rmin, rmax, z;

    rmin.push_back( 0.0 );
    rmin.push_back( 0.0 );
    rmin.push_back( 0.0 );
    rmin.push_back( 0.0 );
    rmin.push_back( 0.0 );
    rmin.push_back( 0.0 );

    rmax.push_back( BeamPlugPipeHoleR1 );
    rmax.push_back( BeamPlugPipeHoleR2 );
    rmax.push_back( BeamPlugPipeHoleR3 );
    rmax.push_back( BeamPlugPipeHoleR4 );
    rmax.push_back( BeamPlugPipeHoleR5 );
    rmax.push_back( BeamPlugPipeHoleR6 );

    z.push_back( BeamPlugPipeHoleZ1 );
    z.push_back( BeamPlugPipeHoleZ2 );
    z.push_back( BeamPlugPipeHoleZ3 );
    z.push_back( BeamPlugPipeHoleZ4 );
    z.push_back( BeamPlugPipeHoleZ5 );
    z.push_back( BeamPlugPipeHoleZ6 );

    // ******************************************************
    // Define the Detector Elements
    // ******************************************************

    dd4hep::DetElement deLeft, deRight;
    deLeft  = dd4hep::DetElement( detector, "EcalLeft", 0 );
    deRight = dd4hep::DetElement( detector, "EcalRight", 1 );

    // ******************************************************
    // Build ECAL left and right shields
    // ******************************************************

    dd4hep::Box ShieldBoxLeft( ( OutXSize * 0.5 - Tolerance ) / 2, ( OutYSize - Tolerance ) / 2, ShieldZSize / 2 );
    dd4hep::Box SubShieldBoxLeft( ( SupportXSize + Tolerance ) / 2, ( SupportYSize + Tolerance ) / 2,
                                  ( ShieldZSize + Tolerance / 2 ) );
    dd4hep::SubtractionSolid ShieldSubLeft( ShieldBoxLeft, SubShieldBoxLeft,
                                            dd4hep::Position( -OutXSize * 0.25 + Tolerance * 0.5, 0, 0 ) );

    dd4hep::Volume lvShieldLeftVol( "lvShieldLeft", ShieldSubLeft, description.material( "EcalDrilledAluminium" ) );
    lvShieldLeftVol.setVisAttributes( description, "vEcal:Shield" );

    dd4hep::SubtractionSolid ShieldSubRight( ShieldBoxLeft, SubShieldBoxLeft,
                                             dd4hep::Position( OutXSize * 0.25 - Tolerance * 0.5, 0, 0 ) );

    dd4hep::Volume lvShieldRightVol( "lvShieldRight", ShieldSubRight, description.material( "EcalDrilledAluminium" ) );
    lvShieldRightVol.setVisAttributes( description, "vEcal:Shield" );

    // ******************************************************
    // Build plug support left and right
    // ******************************************************

    dd4hep::Box      PlugboxL( 0.5 * BeamPlugXSize / 2, BeamPlugYSize / 2, BeamPlugZSize / 2 );
    dd4hep::Polycone PipeHoleL( 0, 2 * M_PI, rmin, rmax, z );

    dd4hep::RotationZYX rot_PlugLeft( 0., 0., -TiltAngle );
    dd4hep::Position    pos_PlugLeft( -BeamPlugXSize * 0.25, -YShift, 0. );

    dd4hep::SubtractionSolid PlugWithHoleL( PlugboxL, PipeHoleL, dd4hep::Transform3D( rot_PlugLeft, pos_PlugLeft ) );

    dd4hep::Volume lvPlugSupportLeftVol( "lvPlugSupportLeft", PlugWithHoleL, description.material( "CaloPb" ) );
    lvPlugSupportLeftVol.setVisAttributes( description, "vEcal:PbTiles" );

    dd4hep::RotationZYX rot_PlugRight( 0., 0., -TiltAngle );
    dd4hep::Position    pos_PlugRight( BeamPlugXSize * 0.25, -YShift, 0. );

    dd4hep::SubtractionSolid PlugWithHoleR( PlugboxL, PipeHoleL, dd4hep::Transform3D( rot_PlugRight, pos_PlugRight ) );

    dd4hep::Volume lvPlugSupportRightVol( "lvPlugSupportRight", PlugWithHoleR, description.material( "CaloPb" ) );
    lvPlugSupportRightVol.setVisAttributes( description, "vEcal:PbTiles" );

    // ******************************************************
    // Build steel sheets with hole 1, 2 left and right
    // ******************************************************

    dd4hep::Box  StSheet1L( 0.5 * ( BeamPlugXSize - 2 * SteelSheetThick1 - 2 * SteelSheetThick2 - Delta ) / 2,
                           ( BeamPlugYSize - 2 * SteelSheetThick1 - 2 * SteelSheetThick2 - Delta ) / 2,
                           ( SteelSheetWithHoleThick1 - Delta ) / 2 );
    dd4hep::Tube CylinderHole1L( 0, SteelSheetWithHoleRadius1, ( SteelSheetWithHoleThick1 + Correction ) / 2 );
    dd4hep::SubtractionSolid StSheetWithHole1L(
        StSheet1L, CylinderHole1L,
        dd4hep::Position( -0.25 * ( BeamPlugXSize - 2 * SteelSheetThick1 - 2 * SteelSheetThick2 - Delta ), 0, 0 ) );
    dd4hep::Volume lvSteelSheetWithHole1LeftVol( "lvSteelSheetWithHole1Left", StSheetWithHole1L,
                                                 description.material( "CaloSteel" ) );
    lvSteelSheetWithHole1LeftVol.setVisAttributes( description, "vEcal:Shield" );

    dd4hep::Box  StSheet2L( 0.5 * ( BeamPlugXSize - 2 * SteelSheetThick1 - Delta ) / 2,
                           ( BeamPlugYSize - 2 * SteelSheetThick1 - Delta ) / 2,
                           ( SteelSheetWithHoleThick1 - Delta ) / 2 );
    dd4hep::Tube CylinderHole2L( 0, SteelSheetWithHoleRadius2, ( SteelSheetWithHoleThick2 + Correction ) / 2 );
    dd4hep::SubtractionSolid StSheetWithHole2L(
        StSheet2L, CylinderHole2L, dd4hep::Position( -0.25 * ( BeamPlugXSize - 2 * SteelSheetThick1 - Delta ), 0, 0 ) );
    dd4hep::Volume lvSteelSheetWithHole2LeftVol( "lvSteelSheetWithHole2Left", StSheetWithHole2L,
                                                 description.material( "CaloSteel" ) );
    lvSteelSheetWithHole2LeftVol.setVisAttributes( description, "vEcal:Shield" );

    dd4hep::SubtractionSolid StSheetWithHole1R(
        StSheet1L, CylinderHole1L,
        dd4hep::Position( 0.25 * ( BeamPlugXSize - 2 * SteelSheetThick1 - 2 * SteelSheetThick2 - Delta ), 0, 0 ) );
    dd4hep::Volume lvSteelSheetWithHole1RightVol( "lvSteelSheetWithHole1Right", StSheetWithHole1R,
                                                  description.material( "CaloSteel" ) );
    lvSteelSheetWithHole1RightVol.setVisAttributes( description, "vEcal:Shield" );

    dd4hep::SubtractionSolid StSheetWithHole2R(
        StSheet2L, CylinderHole2L, dd4hep::Position( 0.25 * ( BeamPlugXSize - 2 * SteelSheetThick1 - Delta ), 0, 0 ) );
    dd4hep::Volume lvSteelSheetWithHole2RightVol( "lvSteelSheetWithHole2Right", StSheetWithHole2R,
                                                  description.material( "CaloSteel" ) );
    lvSteelSheetWithHole2RightVol.setVisAttributes( description, "vEcal:Shield" );

    // ******************************************************
    // Build air slots
    // ******************************************************

    dd4hep::Box    HorizontalAirSlot_Box( ( BeamPlugXSize ) / 4, ( SteelSheetThick1 ) / 2,
                                       ( BeamPlugZSize - SteelSheetLength1 ) / 2 );
    dd4hep::Volume lvHorizontalAirSlotVol( "lvHorizontalAirSlot", HorizontalAirSlot_Box,
                                           description.material( "Air" ) );
    lvHorizontalAirSlotVol.setVisAttributes( description, "vEcal:Air" );

    dd4hep::Box    VerticalAirSlot_Box( ( SteelSheetThick1 ) / 2, ( BeamPlugYSize - 2 * SteelSheetThick1 ) / 2,
                                     ( BeamPlugZSize - SteelSheetLength1 ) / 2 );
    dd4hep::Volume lvVerticalAirSlotVol( "lvVerticalAirSlot", VerticalAirSlot_Box, description.material( "Air" ) );
    lvVerticalAirSlotVol.setVisAttributes( description, "vEcal:Air" );

    // ******************************************************
    // Build other steel sheets
    // ******************************************************

    dd4hep::Box    HorizontalSteelSheet1Vol_Box( ( BeamPlugXSize - Delta ) / 4, ( SteelSheetThick1 - Delta ) / 2,
                                              ( SteelSheetLength1 - Delta ) / 2 );
    dd4hep::Volume lvHorizontalSteelSheet1Vol( "lvHorizontalSteelSheet1", HorizontalSteelSheet1Vol_Box,
                                               description.material( "CaloSteel" ) );
    lvHorizontalSteelSheet1Vol.setVisAttributes( description, "vEcal:Shield" );

    dd4hep::Box    VerticalSteelSheet1Vol_Box( ( SteelSheetThick1 - Delta ) / 2,
                                            ( BeamPlugYSize - 2 * SteelSheetThick1 - Delta ) / 2,
                                            ( SteelSheetLength1 - Delta ) / 2 );
    dd4hep::Volume lvVerticalSteelSheet1Vol( "lvVerticalSteelSheet1", VerticalSteelSheet1Vol_Box,
                                             description.material( "CaloSteel" ) );
    lvVerticalSteelSheet1Vol.setVisAttributes( description, "vEcal:Shield" );

    dd4hep::Box    HorizontalSteelSheet2_Box( ( BeamPlugXSize - 2 * SteelSheetThick1 - Delta ) / 4,
                                           ( SteelSheetThick2 - Delta ) / 2, ( SteelSheetLength2 - Delta ) / 2 );
    dd4hep::Volume lvHorizontalSteelSheet2Vol( "lvHorizontalSteelSheet2", HorizontalSteelSheet2_Box,
                                               description.material( "CaloSteel" ) );
    lvHorizontalSteelSheet2Vol.setVisAttributes( description, "vEcal:Shield" );

    dd4hep::Box    VerticalSteelSheet2_Box( ( SteelSheetThick2 - Delta ) / 2,
                                         ( BeamPlugYSize - 2 * SteelSheetThick1 - 2 * SteelSheetThick2 - Delta ) / 2,
                                         ( SteelSheetLength2 - Delta ) / 2 );
    dd4hep::Volume lvVerticalSteelSheet2Vol( "lvVerticalSteelSheet2", VerticalSteelSheet2_Box,
                                             description.material( "CaloSteel" ) );
    lvVerticalSteelSheet2Vol.setVisAttributes( description, "vEcal:Shield" );

    // ******************************************************
    // Insert volumes into plug support left
    // ******************************************************

    lvPlugSupportLeftVol.placeVolume(
        lvHorizontalAirSlotVol,
        dd4hep::Position( 0, -0.5 * ( BeamPlugYSize - SteelSheetThick1 ), 0.5 * SteelSheetLength1 ) );
    lvPlugSupportLeftVol.placeVolume(
        lvHorizontalAirSlotVol,
        dd4hep::Position( 0, 0.5 * ( BeamPlugYSize - SteelSheetThick1 ), 0.5 * SteelSheetLength1 ) );
    lvPlugSupportLeftVol.placeVolume(
        lvVerticalAirSlotVol,
        dd4hep::Position( 0.25 * ( BeamPlugXSize - 2 * SteelSheetThick1 ), 0, 0.5 * SteelSheetLength1 ) );
    lvPlugSupportLeftVol.placeVolume( lvHorizontalSteelSheet1Vol,
                                      dd4hep::Position( 0, -0.5 * ( BeamPlugYSize - SteelSheetThick1 ),
                                                        -0.5 * ( BeamPlugZSize - SteelSheetLength1 ) ) );
    lvPlugSupportLeftVol.placeVolume( lvHorizontalSteelSheet1Vol,
                                      dd4hep::Position( 0, 0.5 * ( BeamPlugYSize - SteelSheetThick1 ),
                                                        -0.5 * ( BeamPlugZSize - SteelSheetLength1 ) ) );
    lvPlugSupportLeftVol.placeVolume( lvVerticalSteelSheet1Vol,
                                      dd4hep::Position( 0.25 * ( BeamPlugXSize - 2 * SteelSheetThick1 ), 0,
                                                        -0.5 * ( BeamPlugZSize - SteelSheetLength1 ) ) );
    lvPlugSupportLeftVol.placeVolume( lvHorizontalSteelSheet2Vol,
                                      dd4hep::Position( -0.5 * SteelSheetThick1,
                                                        -0.5 * ( BeamPlugYSize - SteelSheetThick2 ) + SteelSheetThick1,
                                                        0.5 * ( BeamPlugZSize - SteelSheetLength2 ) ) );
    lvPlugSupportLeftVol.placeVolume( lvHorizontalSteelSheet2Vol,
                                      dd4hep::Position( -0.5 * SteelSheetThick1,
                                                        0.5 * ( BeamPlugYSize - SteelSheetThick2 ) - SteelSheetThick1,
                                                        0.5 * ( BeamPlugZSize - SteelSheetLength2 ) ) );
    lvPlugSupportLeftVol.placeVolume(
        lvVerticalSteelSheet2Vol,
        dd4hep::Position( 0.25 * ( BeamPlugXSize - 2 * SteelSheetThick2 - 4 * SteelSheetThick1 ), 0,
                          0.5 * ( BeamPlugZSize - SteelSheetLength2 ) ) );
    lvPlugSupportLeftVol.placeVolume( lvSteelSheetWithHole1LeftVol,
                                      dd4hep::Position( -0.5 * ( SteelSheetThick1 + SteelSheetThick2 ), 0,
                                                        0.5 * ( BeamPlugZSize - SteelSheetWithHoleThick1 ) ) );
    lvPlugSupportLeftVol.placeVolume(
        lvSteelSheetWithHole1LeftVol,
        dd4hep::Position( -0.5 * ( SteelSheetThick1 + SteelSheetThick2 ), 0,
                          0.5 * ( BeamPlugZSize - SteelSheetWithHoleThick1 ) - SteelSheetWithHoleStep1  ) );
    lvPlugSupportLeftVol.placeVolume(
        lvSteelSheetWithHole2LeftVol,
        dd4hep::Position( -0.5 * SteelSheetThick1, 0,
                          0.5 * BeamPlugZSize - SteelSheetLength2 - 0.5 * SteelSheetWithHoleThick2 ) );
    lvPlugSupportLeftVol.placeVolume( lvSteelSheetWithHole2LeftVol,
                                      dd4hep::Position( -0.5 * SteelSheetThick1, 0,
                                                        0.5 * BeamPlugZSize - SteelSheetLength2 -
                                                            0.5 * SteelSheetWithHoleThick2 -
                                                            SteelSheetWithHoleStep2 ) );
    lvPlugSupportLeftVol.placeVolume( lvSteelSheetWithHole2LeftVol,
                                      dd4hep::Position( -0.5 * SteelSheetThick1, 0,
                                                        0.5 * BeamPlugZSize - SteelSheetLength2 -
                                                            0.5 * SteelSheetWithHoleThick2 -
                                                            2 * SteelSheetWithHoleStep2 ) );

    registerVolume( lvPlugSupportLeftVol.name(), lvPlugSupportLeftVol );

    // ******************************************************
    // Insert volumes into plug support right
    // ******************************************************

    lvPlugSupportRightVol.placeVolume(
        lvHorizontalAirSlotVol,
        dd4hep::Position( 0, -0.5 * ( BeamPlugYSize - SteelSheetThick1 ), 0.5 * SteelSheetLength1 ) );
    lvPlugSupportRightVol.placeVolume(
        lvHorizontalAirSlotVol,
        dd4hep::Position( 0, 0.5 * ( BeamPlugYSize - SteelSheetThick1 ), 0.5 * SteelSheetLength1 ) );
    lvPlugSupportRightVol.placeVolume(
        lvVerticalAirSlotVol,
        dd4hep::Position( -0.25 * ( BeamPlugXSize - 2 * SteelSheetThick1 ), 0, 0.5 * SteelSheetLength1 ) );
    lvPlugSupportRightVol.placeVolume( lvHorizontalSteelSheet1Vol,
                                       dd4hep::Position( 0, -0.5 * ( BeamPlugYSize - SteelSheetThick1 ),
                                                         -0.5 * ( BeamPlugZSize - SteelSheetLength1 ) ) );
    lvPlugSupportRightVol.placeVolume( lvHorizontalSteelSheet1Vol,
                                       dd4hep::Position( 0, 0.5 * ( BeamPlugYSize - SteelSheetThick1 ),
                                                         -0.5 * ( BeamPlugZSize - SteelSheetLength1 ) ) );
    lvPlugSupportRightVol.placeVolume( lvVerticalSteelSheet1Vol,
                                       dd4hep::Position( -0.25 * ( BeamPlugXSize - 2 * SteelSheetThick1 ), 0,
                                                         -0.5 * ( BeamPlugZSize - SteelSheetLength1 ) ) );
    lvPlugSupportRightVol.placeVolume( lvHorizontalSteelSheet2Vol,
                                       dd4hep::Position( 0.5 * SteelSheetThick1,
                                                         -0.5 * ( BeamPlugYSize - SteelSheetThick2 ) + SteelSheetThick1,
                                                         0.5 * ( BeamPlugZSize - SteelSheetLength2 ) ) );
    lvPlugSupportRightVol.placeVolume( lvHorizontalSteelSheet2Vol,
                                       dd4hep::Position( 0.5 * SteelSheetThick1,
                                                         0.5 * ( BeamPlugYSize - SteelSheetThick2 ) - SteelSheetThick1,
                                                         0.5 * ( BeamPlugZSize - SteelSheetLength2 ) ) );
    lvPlugSupportRightVol.placeVolume(
        lvVerticalSteelSheet2Vol,
        dd4hep::Position( -0.25 * ( BeamPlugXSize - 2 * SteelSheetThick2 - 4 * SteelSheetThick1 ), 0,
                          0.5 * ( BeamPlugZSize - SteelSheetLength2 ) ) );
    lvPlugSupportRightVol.placeVolume( lvSteelSheetWithHole1RightVol,
                                       dd4hep::Position( 0.5 * ( SteelSheetThick1 + SteelSheetThick2 ), 0,
                                                         0.5 * ( BeamPlugZSize - SteelSheetWithHoleThick1 ) ) );
    lvPlugSupportRightVol.placeVolume(
        lvSteelSheetWithHole1RightVol,
        dd4hep::Position( 0.5 * ( SteelSheetThick1 + SteelSheetThick2 ), 0,
                          0.5 * ( BeamPlugZSize - SteelSheetWithHoleThick1 ) - SteelSheetWithHoleStep1 ) );
    lvPlugSupportRightVol.placeVolume(
        lvSteelSheetWithHole2RightVol,
        dd4hep::Position( 0.5 * SteelSheetThick1, 0,
                          0.5 * BeamPlugZSize - SteelSheetLength2 - 0.5 * SteelSheetWithHoleThick2 ) );
    lvPlugSupportRightVol.placeVolume( lvSteelSheetWithHole2RightVol,
                                       dd4hep::Position( 0.5 * SteelSheetThick1, 0,
                                                         0.5 * BeamPlugZSize - SteelSheetLength2 -
                                                             0.5 * SteelSheetWithHoleThick2 -
                                                             SteelSheetWithHoleStep2 ) );
    lvPlugSupportRightVol.placeVolume( lvSteelSheetWithHole2RightVol,
                                       dd4hep::Position( 0.5 * SteelSheetThick1, 0,
                                                         0.5 * BeamPlugZSize - SteelSheetLength2 -
                                                             0.5 * SteelSheetWithHoleThick2 -
                                                             2 * SteelSheetWithHoleStep2 ) );

    // ******************************************************
    // Assembling ECAL left and ECAL right
    // ******************************************************

    dd4hep::Box              Ecal_Box_Left( 0.5 * XSize / 2, YSize / 2, ZSize / 2 );
    dd4hep::Polycone         EcalPipeHoleLeft( 0, 2 * M_PI, rmin, rmax, z );
    dd4hep::RotationZYX      rot_ecal_left( 0., 0., -TiltAngle );
    dd4hep::Position         pos_ecal_left( -XSize * 0.25, -YShift, 0. );
    dd4hep::SubtractionSolid Ecal_Total_Left( Ecal_Box_Left, EcalPipeHoleLeft,
                                              dd4hep::Transform3D( rot_ecal_left, pos_ecal_left ) );

    dd4hep::Volume lvEcalLeftVol( "lvEcalLeft", Ecal_Total_Left, description.material( "Air" ) );
    lvEcalLeftVol.setVisAttributes( description, "vEcal:Air" );

    lvEcalLeftVol.placeVolume( lvShieldLeftVol, dd4hep::Position( 0, 0, ShieldOffset ) );
    lvEcalLeftVol.placeVolume( lvPlugSupportLeftVol,
                               dd4hep::Position( -XSize * 0.25 + BeamPlugXSize * 0.25, 0, SupportOffset ) );
    pv = lvEcalLeftVol.placeVolume( volume( "lvInnSectionLeft" ), dd4hep::Position( -12. * ModXSize, 0, InnOffset ) );
    pv->SetName( "pvEcalLeftInnRegion" );
    pv.addPhysVolID( "region", 2 );
    deRegionTmp = dd4hep::DetElement( deLeft, "EcalLeftInnRegion", 2 );
    deRegionTmp.setPlacement( pv );
    for ( int i = 0; i < 6; ++i ) { deRegionTmp.add( deInnBlock.at( i ) ); }
    deRegion.push_back( deRegionTmp );

    pv = lvEcalLeftVol.placeVolume( volume( "lvMidSectionLeft" ), dd4hep::Position( -8. * ModXSize, 0, MidOffset ) );
    pv->SetName( "pvEcalLeftMidRegion" );
    pv.addPhysVolID( "region", 1 );
    deRegionTmp = dd4hep::DetElement( deLeft, "EcalLeftMidRegion", 1 );
    deRegionTmp.setPlacement( pv );
    for ( int i = 0; i < 14; ++i ) { deRegionTmp.add( deMidBlock.at( i ) ); }
    deRegion.push_back( deRegionTmp );

    pv = lvEcalLeftVol.placeVolume( volume( "lvOutSectionLeft" ), dd4hep::Position( 0, 0, OutOffset ) );
    pv->SetName( "pvEcalLeftOutRegion" );
    pv.addPhysVolID( "region", 0 );
    deRegionTmp = dd4hep::DetElement( deLeft, "EcalLeftOutRegion", 0 );
    deRegionTmp.setPlacement( pv );
    for ( int i = 0; i < 42; ++i ) { deRegionTmp.add( deOutBlock.at( i ) ); }
    deRegion.push_back( deRegionTmp );

    registerVolume( lvEcalLeftVol.name(), lvEcalLeftVol );

    dd4hep::RotationZYX      rot_ecal_right( 0., 0., -TiltAngle );
    dd4hep::Position         pos_ecal_right( XSize * 0.25, -YShift, 0. );
    dd4hep::SubtractionSolid Ecal_Total_Right( Ecal_Box_Left, EcalPipeHoleLeft,
                                               dd4hep::Transform3D( rot_ecal_right, pos_ecal_right ) );

    dd4hep::Volume lvEcalRightVol( "lvEcalRight", Ecal_Total_Right, description.material( "Air" ) );
    lvEcalRightVol.setVisAttributes( description, "vEcal:Air" );

    lvEcalRightVol.placeVolume( lvShieldRightVol, dd4hep::Position( 0, 0, ShieldOffset ) );
    lvEcalRightVol.placeVolume( lvPlugSupportRightVol,
                                dd4hep::Position( +XSize * 0.25 - BeamPlugXSize * 0.25, 0, SupportOffset ) );
    pv = lvEcalRightVol.placeVolume( volume( "lvInnSectionRight" ), dd4hep::Position( 12. * ModXSize, 0, InnOffset ) );
    pv->SetName( "pvEcalRightInnRegion" );
    pv.addPhysVolID( "region", 2 );
    deRegionTmp = dd4hep::DetElement( deRight, "EcalRightInnRegion", 5 );
    deRegionTmp.setPlacement( pv );
    for ( int i = 6; i < 12; ++i ) { deRegionTmp.add( deInnBlock.at( i ) ); }
    deRegion.push_back( deRegionTmp );

    pv = lvEcalRightVol.placeVolume( volume( "lvMidSectionRight" ), dd4hep::Position( 8. * ModXSize, 0, MidOffset ) );
    pv->SetName( "pvEcalRightMidRegion" );
    pv.addPhysVolID( "region", 1 );
    deRegionTmp = dd4hep::DetElement( deRight, "EcalRightMidRegion", 4 );
    deRegionTmp.setPlacement( pv );
    for ( int i = 14; i < 28; ++i ) { deRegionTmp.add( deMidBlock.at( i ) ); }
    deRegion.push_back( deRegionTmp );

    pv = lvEcalRightVol.placeVolume( volume( "lvOutSectionRight" ), dd4hep::Position( 0, 0, OutOffset ) );
    pv->SetName( "pvEcalRightOutRegion" );
    pv.addPhysVolID( "region", 0 );
    deRegionTmp = dd4hep::DetElement( deRight, "EcalRightOutRegion", 3 );
    deRegionTmp.setPlacement( pv );
    for ( int i = 42; i < 84; ++i ) { deRegionTmp.add( deOutBlock.at( i ) ); }
    deRegion.push_back( deRegionTmp );

    // ******************************************************
    // Assembling total ECAL
    // ******************************************************

    // dd4hep::Box         Ecal_Box( ( XSize + XMargin ) / 2, ( YSize + YMargin ) / 2, ( ZSize + ZMargin ) / 2 );
    dd4hep::Box              Ecal_Box( ( XSize ) / 2, ( YSize ) / 2, ( ZSize ) / 2 );
    dd4hep::RotationZYX      rot_ecal( 0., 0., -TiltAngle );
    dd4hep::Position         pos_ecal( 0., -YShift, 0. );
    dd4hep::Polycone         EcalPipeHole( 0, 2 * M_PI, rmin, rmax, z );
    dd4hep::SubtractionSolid Ecal_Total( Ecal_Box, EcalPipeHole, dd4hep::Transform3D( rot_ecal, pos_ecal ) );
    dd4hep::Volume           lvEcalVol( "lvEcal", Ecal_Total, description.material( "Air" ) );
    lvEcalVol.setVisAttributes( description, "vEcal:Air" );

    pv = lvEcalVol.placeVolume( lvEcalLeftVol, dd4hep::Position( XSize * 0.25, 0, 0 ) );
    pv->SetName( "pvEcalLeft" );
    pv.addPhysVolID( "side", 0 );
    deLeft.setPlacement( pv );
    deSide.push_back( deLeft );

    pv = lvEcalVol.placeVolume( lvEcalRightVol, dd4hep::Position( -XSize * 0.25, 0, 0 ) );
    pv->SetName( "pvEcalRight" );
    pv.addPhysVolID( "side", 1 );
    deRight.setPlacement( pv );
    deSide.push_back( deRight );
    registerVolume( lvEcalVol.name(), lvEcalVol );

    return lvEcalVol;
  }

} // namespace

static dd4hep::Ref_t create_element( dd4hep::Detector& description, xml_h e, dd4hep::SensitiveDetector sens_det ) {
  xml_det_t x_det = e;
  EcalBuild builder( description, x_det, sens_det );
  string    det_name = x_det.nameStr();
  dd4hep::printout( dd4hep::DEBUG, "CreateEcal", "Detector name: %s with ID: %d", det_name.c_str(), x_det.id() );
  PlacedVolume pv;
  sens_det.setType( "calorimeter" );
  builder.buildVolumes( e );
  // ******************************************************
  // Build ECAL inner section
  // ******************************************************
  builder.build_inner_cell();
  builder.build_inner_module();
  builder.build_inner_sections();

  // ******************************************************
  // Build ECAL middle section
  // ******************************************************
  builder.build_middle_cell();
  builder.build_middle_module();
  builder.build_middle_sections();

  // ******************************************************
  // Build ECAL outer section
  // ******************************************************
  builder.build_outer_cell();
  builder.build_outer_module();
  builder.build_outer_sections();

  // ******************************************************
  // Assemble ECAL left and right regions
  // ******************************************************
  dd4hep::Volume lvEcalVol = builder.build_ecal();

  // ******************************************************
  // Place the ECAL in the world
  // ******************************************************
  pv = builder.placeDetector( lvEcalVol );
  pv.addPhysVolID( "system", x_det.id() );

  return builder.detector;
}

DECLARE_DETELEMENT( LHCb_Ecal_v1_0, create_element )
