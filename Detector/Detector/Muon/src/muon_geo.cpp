/*****************************************************************************\
* (c) Copyright 2023 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/Printout.h"
#include "DD4hep/PropertyTable.h"
#include "DDCond/ConditionsTags.h"
#include "XML/Utilities.h"
#include "XML/XMLElements.h"
#include "XML/config.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

namespace {

  /// Helper class to build the VP detector of LHCb
  struct MuonBuild : public dd4hep::xml::tools::VolumeBuilder {

    /// Chamber element to be cloned
    // dd4hep::DetElement de_chamber;
    dd4hep::Material copper;
    dd4hep::Material polyurethane;
    dd4hep::Material g10;
    dd4hep::Material air;

    /// Flag to disable building supports
    bool b_build_support = true;
    /// Flag to disable building the Muon chambers
    bool b_build_stations = true;
    /// Flag to disable building the Muon filters
    bool b_build_filters = true;
    /// Flag to disable building the Muon plugs
    bool b_build_plugs = true;

    // common information
    int                 nStations;
    std::array<char, 2> nameSide;
    std::string         select_volume{"lvMuon"};
    double              muon_eps;

    struct chamberPos {
      double x;
      double y;
      double z;
      int    side;

      dd4hep::Position pos() { return dd4hep::Position( x, y, z ); }
    };
    std::map<std::string, chamberPos> chambers;

    /// Initializing constructor
    MuonBuild( dd4hep::Detector& description, xml_elt_t e, dd4hep::SensitiveDetector sens );
    //
    dd4hep::DetElement build_chamber_prototype( int stationNumber, int regionNumber );
    void               build_regions( int stationNumber, dd4hep::Volume& lvStation );
    void               build_support();
    void               build_stations();
    void               build_filters();
    void               build_plugs();
    void               build_detector();
    void               build_muon();
  };

  // Initialising constructor
  MuonBuild::MuonBuild( dd4hep::Detector& dsc, xml_elt_t e, dd4hep::SensitiveDetector sens )
      : dd4hep::xml::tools::VolumeBuilder( dsc, e, sens ) {

    xml_comp_t x_dbg = x_det.child( _U( debug ), false );
    if ( x_dbg ) {
      for ( xml_coll_t i( x_dbg, _U( item ) ); i; ++i ) {
        xml_comp_t  c( i );
        std::string n = c.nameStr();
        if ( n == "debug" )
          debug = c.attr<int>( _U( value ) ) != 0;
        else if ( n == "b_build_support" )
          b_build_support = c.attr<int>( _U( value ) ) != 0;
        else if ( n == "b_build_stations" )
          b_build_stations = c.attr<int>( _U( value ) ) != 0;
        else if ( n == "b_build_filters" )
          b_build_filters = c.attr<int>( _U( value ) ) != 0;
        else if ( n == "b_build_plugs" )
          b_build_plugs = c.attr<int>( _U( value ) ) != 0;
        else if ( n == "select_volume" )
          select_volume = c.attr<std::string>( _U( value ) );
      }
    }
    //    std::cout << "Defining materials used" << std::endl;
    copper       = description.material( "Copper" );
    polyurethane = description.material( "MUON:Polyurethane" );
    g10          = description.material( "MUON:G10" );
    air          = description.material( "Air" );

    //    std::cout << "Defining common parameters" << std::endl;
    nStations = dd4hep::_toInt( "MUON:NumberOfStations" );
    nameSide  = {'A', 'C'};
    muon_eps  = dd4hep::_toDouble( "MUON:Epsilon" );

    xml_comp_t x_chm = x_det.child( _U( chambers ), false );
    if ( x_chm ) {
      for ( xml_coll_t i( x_chm, _U( chamber ) ); i; ++i ) {
        xml_comp_t  c( i );
        std::string n    = c.nameStr();
        double      x    = dd4hep::_toDouble( c.x() );
        double      y    = dd4hep::_toDouble( c.y() );
        double      z    = dd4hep::_toDouble( c.z() );
        int         side = i.attr<int>( _U( side ) );

        chambers[n] = {x, y, z, side};
      }
    }
  }

  // Supports
  void MuonBuild::build_support() {
    dd4hep::PlacedVolume pv;

    double shift_Z = dd4hep::_toDouble( "MUON:PanelZShift" );
    for ( int stationNumber = 2; stationNumber < nStations + 2; stationNumber++ ) {
      dd4hep::Volume lvStation = volume( Form( "M%01d", stationNumber ) );

      double size_outer_X = dd4hep::_toDouble( Form( "MUON:M%01dXOuterSize", stationNumber ) );
      double size_outer_Y = dd4hep::_toDouble( Form( "MUON:M%01dYOuterSize", stationNumber ) );
      double size_outer_Z = 1.5 * dd4hep::mm;

      dd4hep::Box supportOuterBox( size_outer_X / 2.0, size_outer_Y / 2.0, size_outer_Z / 2.0 );

      double size_inner_X = dd4hep::_toDouble( Form( "MUON:M%01dXInnerSize", stationNumber ) ) + 2 * dd4hep::mm;
      double size_inner_Y = dd4hep::_toDouble( Form( "MUON:M%01dYInnerSize", stationNumber ) );
      double size_inner_Z = 2 * dd4hep::mm;

      dd4hep::Box              supportInnerBox( size_inner_X / 2.0, size_inner_Y / 2.0, size_inner_Z / 2.0 );
      dd4hep::SubtractionSolid support(
          supportOuterBox, supportInnerBox,
          dd4hep::Position( -1 * size_outer_X / 2 + size_inner_X / 2 - 1 * dd4hep::mm, 0, 0 ) );

      dd4hep::Volume lvSupportFrontPanel( Form( "M%01dFrontPanelSupport", stationNumber ), support,
                                          description.material( "Aluminum" ) );

      pv = lvStation.placeVolume( lvSupportFrontPanel, dd4hep::Position( 0.0, 0.0, -shift_Z ) );
      pv->SetName( Form( "M%01dFrontPanel", stationNumber ) );
      lvSupportFrontPanel.setVisAttributes( description, "MUON:SupportVis" );

      dd4hep::Volume lvSupportRearPanel( Form( "M%01dRearPanelSupport", stationNumber ), support,
                                         description.material( "Aluminum" ) );

      pv = lvStation.placeVolume( lvSupportRearPanel, dd4hep::Position( 0.0, 0.0, shift_Z ) );
      pv->SetName( Form( "M%01dRearPanel", stationNumber ) );
      lvSupportRearPanel.setVisAttributes( description, "MUON:SupportVis" );
    }
  }

  // Filters (bricks and tricks)
  void MuonBuild::build_filters() {

    dd4hep::PlacedVolume pv;
    double               pos_x, pos_y, pos_z;
    dd4hep::Position     pos;
    dd4hep::Position     shift1, shift2;

    dd4hep::Volume lvMuon  = volume( select_volume.c_str() );
    double         rot_ang = dd4hep::_toDouble( "MUON:beamAngle" );

    // basic elements
    int nBlocks = dd4hep::_toInt( "MUON:nBlocks" );
    for ( int blockNumber = 1; blockNumber < nBlocks + 1; blockNumber++ ) {

      double brick_size_x = dd4hep::_toDouble( dd4hep::_toString( blockNumber, "MUON:Block%01dSizeX" ) );
      double brick_size_y = dd4hep::_toDouble( dd4hep::_toString( blockNumber, "MUON:Block%01dSizeY" ) );
      double brick_size_z = dd4hep::_toDouble( dd4hep::_toString( blockNumber, "MUON:Block%01dSizeZ" ) );

      dd4hep::Box    boxBlock( ( brick_size_x - muon_eps ) / 2.0, ( brick_size_y - muon_eps ) / 2.0,
                            ( brick_size_z - muon_eps ) / 2.0 );
      dd4hep::Volume lvBlock( Form( "lvBlock%01d", blockNumber ), boxBlock, description.material( "MUON:FilterIron" ) );
      registerVolume( lvBlock.name(), lvBlock );
    }

    int nPlates = dd4hep::_toInt( "MUON:nPlates" );
    for ( int plateNumber = 1; plateNumber < nPlates + 1; plateNumber++ ) {

      double plate_size_x = dd4hep::_toDouble( dd4hep::_toString( plateNumber, "MUON:Plate%01dSizeX" ) );
      double plate_size_y = dd4hep::_toDouble( dd4hep::_toString( plateNumber, "MUON:Plate%01dSizeY" ) );
      double plate_size_z = dd4hep::_toDouble( dd4hep::_toString( plateNumber, "MUON:Plate%01dSizeZ" ) );

      dd4hep::Box    boxPlate( ( plate_size_x - muon_eps ) / 2.0, ( plate_size_y - muon_eps ) / 2.0,
                            ( plate_size_z - muon_eps ) / 2.0 );
      dd4hep::Volume lvPlate( Form( "lvSteelPlate%01d", plateNumber ), boxPlate, description.material( "MUON:Steel" ) );
      registerVolume( lvPlate.name(), lvPlate );
    }

    int nConcretes = dd4hep::_toInt( "MUON:nConcretes" );
    for ( int concreteNumber = 1; concreteNumber < nConcretes + 1; concreteNumber++ ) {
      double concrete_size_x = dd4hep::_toDouble( dd4hep::_toString( concreteNumber, "MUON:Concrete%01dSizeX" ) );
      double concrete_size_y = dd4hep::_toDouble( dd4hep::_toString( concreteNumber, "MUON:Concrete%01dSizeY" ) );
      double concrete_size_z = dd4hep::_toDouble( dd4hep::_toString( concreteNumber, "MUON:Concrete%01dSizeZ" ) );

      dd4hep::Box    boxConcrete( ( concrete_size_x - muon_eps ) / 2.0, ( concrete_size_y - muon_eps ) / 2.0,
                               ( concrete_size_z - muon_eps ) / 2.0 );
      dd4hep::Volume lvConcrete( Form( "lvConcrete%01d", concreteNumber ), boxConcrete,
                                 description.material( "MUON:Concrete" ) );
      registerVolume( lvConcrete.name(), lvConcrete );
    }

    int nAirGaps = dd4hep::_toInt( "MUON:nFilterAirGaps" );
    for ( int airGapNumber = 1; airGapNumber < nAirGaps + 1; airGapNumber++ ) {
      double airGap_size_x = dd4hep::_toDouble( dd4hep::_toString( airGapNumber, "MUON:FilterAirGap%01dSizeX" ) );
      double airGap_size_y = dd4hep::_toDouble( dd4hep::_toString( airGapNumber, "MUON:FilterAirGap%01dSizeY" ) );
      double airGap_size_z = dd4hep::_toDouble( dd4hep::_toString( airGapNumber, "MUON:FilterAirGap%01dSizeZ" ) );

      dd4hep::Box    boxAirGap( ( airGap_size_x - muon_eps ) / 2.0, ( airGap_size_y - muon_eps ) / 2.0,
                             ( airGap_size_z - muon_eps ) / 2.0 );
      dd4hep::Volume lvAirGap( Form( "lvAirGap%01d", airGapNumber ), boxAirGap, description.material( "Air" ) );
      registerVolume( lvAirGap.name(), lvAirGap );
    }

    // helper function, to mimick the paramphysvol in detdesc.
    auto parametrical_pv = []( dd4hep::Assembly mother, dd4hep::Volume vol, dd4hep::Position pos,
                               std::vector<dd4hep::Position> shifts, std::vector<int> steps, std::string name,
                               std::array<double, 3> additional_rotation = {0, 0, 0} ) {
      unsigned int ndim    = steps.size();
      unsigned int nclones = std::accumulate( steps.begin(), steps.end(), 1u, std::multiplies<unsigned int>() );

      for ( unsigned int clone = 0; clone < nclones; clone++ ) {
        dd4hep::PlacedVolume pv     = nullptr;
        dd4hep::Position     newPos = pos;

        for ( unsigned int dim = 0; dim < ndim; dim++ ) {
          int div = std::accumulate( steps.begin(), steps.begin() + dim, 1u, std::multiplies<int>() );
          newPos += ( clone / div ) % steps[dim] * shifts[dim];
        }
        // check if an additional rotation has to be added to the volume
        if ( ( additional_rotation[0] == 0 ) && ( additional_rotation[1] == 0 ) && ( additional_rotation[2] == 0 ) ) {
          pv = mother.placeVolume( vol, newPos );
        } else {
          if ( additional_rotation[0] != 0 ) {
            pv = mother.placeVolume( vol, dd4hep::Transform3D( dd4hep::RotationX( additional_rotation[0] ), newPos ) );
          } else if ( additional_rotation[1] != 0 ) {
            pv = mother.placeVolume( vol, dd4hep::Transform3D( dd4hep::RotationY( additional_rotation[1] ), newPos ) );
          } else if ( additional_rotation[2] != 0 ) {
            pv = mother.placeVolume( vol, dd4hep::Transform3D( dd4hep::RotationZ( additional_rotation[2] ), newPos ) );
          }
        }
        pv->SetName( Form( "%s_%d", name.c_str(), clone ) );
      }
      return;
    };

    // helper function, to retrieve positions from paramters.xml
    auto get_pos = [&]( std::string spos_x, std::string spos_y, std::string spos_z ) {
      pos_x = dd4hep::_toDouble( spos_x );
      pos_y = dd4hep::_toDouble( spos_y );
      pos_z = dd4hep::_toDouble( spos_z );
      return dd4hep::Position( pos_x, pos_y, pos_z );
    };

    /*    auto add_pv = [&](dd4hep::Assembly mother, dd4hep::Volume vol, dd4hep::Position pos, std::string name){
              dd4hep::PlacedVolume pv;
              pv = mother.placeVolume(vol,pos);
              pv->SetName(name.c_str());
        };
    */
    // assembly volume lvRow1
    dd4hep::Assembly lvRow1( "lvRow1" );

    pos = get_pos( "MUON:Row1SteelPlate6PosX", "MUON:Row1SteelPlate6PosY", "0.0" );
    pv  = lvRow1.placeVolume( volume( "lvSteelPlate6" ), pos );
    pv->SetName( "pvSteelP6Row1" );

    pos          = get_pos( "MUON:Row1Block2PosX", "MUON:Row1Block2PosY", "0.0" );
    double rot_z = dd4hep::_toDouble( "MUON:Row1Block2RotZ" );
    pv           = lvRow1.placeVolume( volume( "lvBlock2" ), dd4hep::Transform3D( dd4hep::RotationZ( rot_z ), pos ) );
    pv->SetName( "pvBlock2Row1" );

    pos         = get_pos( "MUON:Row1AirGapPosX", "MUON:Row1AirGapPosY", "0.0" );
    shift1      = get_pos( "MUON:Row1AirGapShiftX", "0.0", "0.0" );
    shift2      = get_pos( "0.0", "MUON:Row1AirGapShiftY", "0.0" );
    int number1 = dd4hep::_toInt( "MUON:Row1AirGapNumber1" );
    int number2 = dd4hep::_toInt( "MUON:Row1AirGapNumber2" );

    parametrical_pv( lvRow1, volume( "lvAirGap1" ), pos, {shift1, shift2}, {number1, number2}, "pvAirGap1a" );

    pos = get_pos( "MUON:Row1Concrete2PosX", "MUON:Row1Concrete2PosY", "0.0" );
    pv  = lvRow1.placeVolume( volume( "lvConcrete2" ), pos );
    pv->SetName( "pvConcrete2Row1" );

    pos = get_pos( "MUON:Row1Plate1PosX", "MUON:Row1Plate1PosY", "0.0" );
    pv  = lvRow1.placeVolume( volume( "lvSteelPlate1" ), pos );
    pv->SetName( "pvSteelP1Row1" );

    pos = get_pos( "MUON:Row1Concrete1PosX", "MUON:Row1Concrete1PosY", "0.0" );
    pv  = lvRow1.placeVolume( volume( "lvConcrete1" ), pos );
    pv->SetName( "pvConcrete1Row1" );

    pos = get_pos( "0.0", "MUON:Row1Concrete5PosY", "0.0" );
    pv  = lvRow1.placeVolume( volume( "lvConcrete5" ), pos );
    pv->SetName( "pvConcrete5Row1" );

    pos = get_pos( "0.0", "MUON:Row1Plate5PosY", "0.0" );
    pv  = lvRow1.placeVolume( volume( "lvSteelPlate3" ), pos );
    pv->SetName( "pvSteelP3Row1" );

    pos     = get_pos( "MUON:Row1Block1PosX", "MUON:Row1Block1PosY", "0.0" );
    shift1  = get_pos( "MUON:Row1Block1ShiftX", "0.0", "0.0" );
    shift2  = get_pos( "0.0", "MUON:Row1Block1ShiftY", "0.0" );
    number1 = dd4hep::_toInt( "MUON:Row1Block1Number1" );
    number2 = dd4hep::_toInt( "MUON:Row1Block1Number2" );
    parametrical_pv( lvRow1, volume( "lvBlock1" ), pos, {shift1, shift2}, {number1, number2}, "pvBlock1Row1" );

    // assembly volume lvRow2
    dd4hep::Assembly lvRow2( "lvRow2" );

    pos = get_pos( "MUON:Row2Row1PosX", "0.0", "0.0" );
    pv  = lvRow2.placeVolume( lvRow1, pos );
    pv->SetName( "pvRow1Row2" );

    pos = get_pos( "MUON:Row2SteelPlate6PosX", "MUON:Row2SteelPlate6PosY", "0.0" );
    pv  = lvRow2.placeVolume( volume( "lvSteelPlate6" ), pos );
    pv->SetName( "pvSteelP6Row2" );

    pos   = get_pos( "MUON:Row2Block2PosX", "MUON:Row2Block2PosY", "0.0" );
    rot_z = dd4hep::_toDouble( "MUON:Row2Block2RotZ" );
    pv    = lvRow2.placeVolume( volume( "lvBlock2" ), dd4hep::Transform3D( dd4hep::RotationZ( rot_z ), pos ) );
    pv->SetName( "pvBlock2Row2" );

    pos     = get_pos( "MUON:Row2Concrete1PosX", "MUON:Row2Concrete1PosY", "0.0" );
    shift1  = get_pos( "0.0", "MUON:Row2Concrete1ShiftY", "0.0" );
    number1 = dd4hep::_toInt( "MUON:Row2Concrete1Number1" );
    parametrical_pv( lvRow2, volume( "lvConcrete1" ), pos, {shift1}, {number1}, "pvConcrete1Row2" );

    pos = get_pos( "MUON:Row2SteelPlate2PosX", "MUON:Row2SteelPlate2PosY", "0.0" );
    pv  = lvRow2.placeVolume( volume( "lvSteelPlate2" ), pos );
    pv->SetName( "pvSteelP2Row2" );

    // Assembly volume lvCenter1
    dd4hep::Assembly lvCenter1( "lvCenter1" );

    pos     = get_pos( "MUON:Center1SteelPlate6PosX", "MUON:Center1SteelPlate6PosY", "0.0" );
    shift1  = get_pos( "MUON:Center1SteelPlate6Shift1X", "0.0", "0.0" );
    number1 = dd4hep::_toInt( "MUON:Center1SteelPlate6Number1" );
    parametrical_pv( lvCenter1, volume( "lvSteelPlate6" ), pos, {shift1}, {number1}, "pvSteelP6Center1" );

    pos     = get_pos( "MUON:Center1Block2PosX", "MUON:Center1Block2PosY", "0.0" );
    rot_z   = dd4hep::_toDouble( "MUON:Center1Block2RotZ" );
    shift1  = get_pos( "MUON:Center1Block2Shift1X", "0.0", "0.0" );
    number1 = dd4hep::_toInt( "MUON:Center1Block2Number1" );
    parametrical_pv( lvCenter1, volume( "lvBlock2" ), pos, {shift1}, {number1}, "pvBlock2Center1", {0, 0, rot_z} );

    pos = get_pos( "MUON:Center1AirGap3PosX", "MUON:Center1AirGap3PosY", "0.0" );
    pv  = lvCenter1.placeVolume( volume( "lvAirGap3" ), pos );
    pv->SetName( "pvGap3Center1" );

    number1 = dd4hep::_toInt( "MUON:Center1Block1Number1" );
    number2 = dd4hep::_toInt( "MUON:Center1Block1Number2" );
    pos     = get_pos( "MUON:Center1Block1PosX", "MUON:Center1Block1PosY", "0.0" );
    shift1  = get_pos( "MUON:Center1Block1Shift1X", "0.0", "0.0" );
    shift2  = get_pos( "MUON:Center1Block1Shift2X", "MUON:Center1Block1Shift2Y", "0.0" );
    parametrical_pv( lvCenter1, volume( "lvBlock1" ), pos, {shift1, shift2}, {number1, number2}, "pvBlock1Center1" );

    number1 = dd4hep::_toInt( "MUON:Center1AirGap1Number1" );
    pos     = get_pos( "MUON:Center1AirGap1PosX", "MUON:Center1AirGap1PosY", "0.0" );
    shift1  = get_pos( "MUON:Center1AirGap1Shift1X", "MUON:Center1AirGap1Shift1Y", "0.0" );
    parametrical_pv( lvCenter1, volume( "lvAirGap1" ), pos, {shift1}, {number1}, "pvGap6Center1" );

    number1 = dd4hep::_toInt( "MUON:Center1AirGap2Number1" );
    number2 = dd4hep::_toInt( "MUON:Center1AirGap2Number2" );
    pos     = get_pos( "MUON:Center1AirGap2PosX", "MUON:Center1AirGap2PosY", "0.0" );
    shift1  = get_pos( "MUON:Center1AirGap2Shift1X", "0.0", "0.0" );
    shift2  = get_pos( "MUON:Center1AirGap2Shift2X", "0.0", "0.0" );
    parametrical_pv( lvCenter1, volume( "lvAirGap2" ), pos, {shift1, shift2}, {number1, number2}, "pvGap2Center1" );

    number1 = dd4hep::_toInt( "MUON:Center1AirGap2bNumber1" );
    pos     = get_pos( "MUON:Center1AirGap2bPosX", "MUON:Center1AirGap2bPosY", "0.0" );
    shift1  = get_pos( "MUON:Center1AirGap2Shift1X", "0.0", "0.0" );
    parametrical_pv( lvCenter1, volume( "lvAirGap2" ), pos, {shift1}, {number1}, "pvGap2bCenter1" );

    pos = get_pos( "MUON:Center1SteelPlate7PosX", "0.0", "0.0" );
    pv  = lvCenter1.placeVolume( volume( "lvSteelPlate7" ), pos );
    pv->SetName( "pvSteelP7Center1" );

    pos = get_pos( "MUON:Center1Concrete6PosX", "MUON:Center1Concrete6PosY", "0.0" );
    pv  = lvCenter1.placeVolume( volume( "lvConcrete6" ), pos );
    pv->SetName( "pvConcrete6Center1" );

    pos = get_pos( "MUON:Center1AirGap4PosX", "MUON:Center1AirGap4PosY", "0.0" );
    pv  = lvCenter1.placeVolume( volume( "lvAirGap4" ), pos );
    pv->SetName( "pvGap4Center1" );

    pos     = get_pos( "MUON:Center1Block3PosX", "MUON:Center1Block3PosY", "0.0" );
    rot_z   = dd4hep::_toDouble( "MUON:Center1Block3RotZ" );
    shift1  = get_pos( "MUON:Center1Block3Shift1X", "MUON:Center1Block3Shift1Y", "0.0" );
    number1 = dd4hep::_toInt( "MUON:Center1Block3Number1" );
    parametrical_pv( lvCenter1, volume( "lvBlock3" ), pos, {shift1}, {number1}, "pvBlock3Center1", {0, 0, rot_z} );

    pos = get_pos( "MUON:Center1Block4PosX", "MUON:Center1Block4PosY", "0.0" );
    pv  = lvCenter1.placeVolume( volume( "lvBlock4" ), pos );
    pv->SetName( "pvBlock4Center1" );

    pos = get_pos( "MUON:Center1SteelPlate8PosX", "MUON:Center1SteelPlate8PosY", "0.0" );
    pv  = lvCenter1.placeVolume( volume( "lvSteelPlate8" ), pos );
    pv->SetName( "pvSteelP8Center1" );

    pos = get_pos( "MUON:Center1Concrete7PosX", "MUON:Center1Concrete7PosY", "0.0" );
    pv  = lvCenter1.placeVolume( volume( "lvConcrete7" ), pos );
    pv->SetName( "pvConcrete7Center1" );

    pos     = get_pos( "MUON:Center1Block3bPosX", "MUON:Center1Block3bPosY", "0.0" );
    shift1  = get_pos( "MUON:Center1Block3bShift1X", "MUON:Center1Block3bShift1Y", "0.0" );
    number1 = dd4hep::_toInt( "MUON:Center1Block3bNumber1" );
    parametrical_pv( lvCenter1, volume( "lvBlock3" ), pos, {shift1}, {number1}, "pvBlock3bCenter1" );

    pos = get_pos( "MUON:Center1AirGap5PosX", "MUON:Center1AirGap5PosY", "0.0" );
    pv  = lvCenter1.placeVolume( volume( "lvAirGap5" ), pos );
    pv->SetName( "pvGap5Center1" );

    pos = get_pos( "0.0", "MUON:Center1SteelPlate4PosY", "0.0" );
    pv  = lvCenter1.placeVolume( volume( "lvSteelPlate4" ), pos );
    pv->SetName( "pvSteelP4Center1" );

    pos     = get_pos( "MUON:Center1Concrete1PosX", "MUON:Center1Concrete1PosY", "0.0" );
    shift1  = get_pos( "MUON:Center1Concrete1Shift1X", "0.0", "0.0" );
    number1 = dd4hep::_toInt( "MUON:Center1Concrete1Number1" );
    parametrical_pv( lvCenter1, volume( "lvConcrete1" ), pos, {shift1}, {number1}, "pvConcrete1Center1" );

    // Assembly volume lvCenter2
    dd4hep::Assembly lvCenter2( "lvCenter2" );

    number1 = dd4hep::_toInt( "MUON:Center2Block1Number1" );
    number2 = dd4hep::_toInt( "MUON:Center2Block1Number2" );
    pos     = get_pos( "MUON:Center2Block1PosX", "MUON:Center2Block1PosY", "0.0" );
    shift1  = get_pos( "MUON:Center2Block1Shift1X", "0.0", "0.0" );
    shift2  = get_pos( "MUON:Center2Block1Shift2X", "MUON:Center2Block1Shift2Y", "0.0" );
    parametrical_pv( lvCenter2, volume( "lvBlock1" ), pos, {shift1, shift2}, {number1, number2}, "pvBlock1Center2" );

    // this part has been replicated without using the helping function...
    pos    = get_pos( "MUON:Center2Block3PosX", "MUON:Center2Block3PosY", "0.0" );
    shift1 = get_pos( "MUON:Center2Block3Shift1X", "MUON:Center2Block3Shift1Y", "0.0" );
    // number1 = dd4hep::_toInt("MUON:Center2Block3Number1");
    rot_z = dd4hep::_toDouble( "MUON:Center2Block3RotZ" );
    pv    = lvCenter2.placeVolume( volume( "lvBlock3" ), pos );
    pv->SetName( "pvBlock3Center2_0" );
    pv = lvCenter2.placeVolume( volume( "lvBlock3" ), dd4hep::Transform3D( dd4hep::RotationZ( rot_z ), pos + shift1 ) );
    pv->SetName( "pvBlock3Center2_1" );

    pos = get_pos( "MUON:Center2Block4PosX", "MUON:Center2Block4PosY", "0.0" );
    pv  = lvCenter2.placeVolume( volume( "lvBlock4" ), pos );
    pv->SetName( "pvBlock4Center2" );

    number1 = dd4hep::_toInt( "MUON:Center2AirGap1Number1" );
    number2 = dd4hep::_toInt( "MUON:Center2AirGap1Number2" );
    pos     = get_pos( "MUON:Center2AirGap1PosX", "MUON:Center2AirGap1PosY", "0.0" );
    shift1  = get_pos( "MUON:Center2AirGap1Shift1X", "0.0", "0.0" );
    shift2  = get_pos( "MUON:Center2AirGap1Shift2X", "MUON:Center2AirGap1Shift2Y", "0.0" );
    parametrical_pv( lvCenter2, volume( "lvAirGap1" ), pos, {shift1, shift2}, {number1, number2}, "pvGap1Center2" );

    pos = get_pos( "MUON:Center2Concrete2PosX", "MUON:Center2Concrete2PosY", "0.0" );
    pv  = lvCenter2.placeVolume( volume( "lvConcrete2" ), pos );
    pv->SetName( "pvConcrete2Center2" );

    pos = get_pos( "MUON:Center2Concrete5PosX", "MUON:Center2Concrete5PosY", "0.0" );
    pv  = lvCenter2.placeVolume( volume( "lvConcrete5" ), pos );
    pv->SetName( "pvConcrete5Center2" );

    pos = get_pos( "MUON:Center2SteelPlate1PosX", "MUON:Center2SteelPlate1PosY", "0.0" );
    pv  = lvCenter2.placeVolume( volume( "lvSteelPlate1" ), pos );
    pv->SetName( "pvSteelP1Center2" );

    pos = get_pos( "0.0", "MUON:Center2SteelPlate9PosY", "0.0" );
    pv  = lvCenter2.placeVolume( volume( "lvSteelPlate9" ), pos );
    pv->SetName( "pvSteelP9Center2" );

    pos = get_pos( "MUON:Center2Concrete7PosX", "MUON:Center2Concrete7PosY", "0.0" );
    pv  = lvCenter2.placeVolume( volume( "lvConcrete7" ), pos );
    pv->SetName( "pvConcrete7Center2" );

    pos = get_pos( "MUON:Center2SteelPlate8PosX", "MUON:Center2SteelPlate8PosY", "0.0" );
    pv  = lvCenter2.placeVolume( volume( "lvSteelPlate8" ), pos );
    pv->SetName( "pvSteelP8Center2" );

    pos = get_pos( "MUON:Center2AirGap5PosX", "MUON:Center2AirGap5PosY", "0.0" );
    pv  = lvCenter2.placeVolume( volume( "lvAirGap5" ), pos );
    pv->SetName( "pvAirGap5Center2" );

    // prepare the half filter ..
    // HalfFilter1
    dd4hep::Assembly lvHalfFilter1( "HalfFilter1" );

    number1      = dd4hep::_toInt( "MUON:HalfFilter1Row1Number1" );
    pos          = get_pos( "MUON:HalfFilter1Row1PosX", "MUON:HalfFilter1Row1PosY", "0.0" );
    double rot_y = dd4hep::_toDouble( "MUON::HalfFilter1Row1RotY" );
    shift1       = get_pos( "0.0", "MUON:HalfFilter1Row1Shift1Y", "0.0" );
    parametrical_pv( lvHalfFilter1, lvRow1, pos, {shift1}, {number1}, "pvRow1HF1", {0.0, rot_y, 0.0} );

    //  Done manually
    //    number1 =  dd4hep::_toInt("MUON:HalfFilter1Row1bNumber1");
    pos    = get_pos( "MUON:HalfFilter1Row1bPosX", "MUON:HalfFilter1Row1bPosY", "0.0" );
    rot_y  = dd4hep::_toDouble( "MUON::HalfFilter1Row1bRotY" );
    shift1 = get_pos( "0.0", "MUON:HalfFilter1Row1bShift1Y", "0.0" );
    pv     = lvHalfFilter1.placeVolume( lvRow1, pos );
    pv->SetName( "pvRow1bHF1_0" );
    pv = lvHalfFilter1.placeVolume( lvRow1, dd4hep::Transform3D( dd4hep::RotationY( rot_y ), pos + shift1 ) );
    pv->SetName( "pvRow1bHF1_1" );
    //

    number1 = dd4hep::_toInt( "MUON:HalfFilter1Block1Number1" );
    number2 = dd4hep::_toInt( "MUON:HalfFilter1Block1Number2" );
    pos     = get_pos( "MUON:HalfFilter1Block1PosX", "MUON:HalfFilter1Block1PosY", "0.0" );
    shift1  = get_pos( "MUON:HalfFilter1Block1Shift1X", "0.0", "0.0" );
    shift2  = get_pos( "0.0", "MUON:HalfFilter1Block1Shift2Y", "0.0" );
    parametrical_pv( lvHalfFilter1, volume( "lvBlock1" ), pos, {shift1, shift2}, {number1, number2}, "pvBlock1HF1" );

    number1 = dd4hep::_toInt( "MUON:HalfFilter1AirGap1Number1" );
    number2 = dd4hep::_toInt( "MUON:HalfFilter1AirGap1Number2" );
    pos     = get_pos( "MUON:HalfFilter1AirGap1PosX", "MUON:HalfFilter1AirGap1PosY", "0.0" );
    shift1  = get_pos( "MUON:HalfFilter1AirGap1Shift1X", "0.0", "0.0" );
    shift2  = get_pos( "0.0", "MUON:HalfFilter1AirGap1Shift2Y", "0.0" );
    parametrical_pv( lvHalfFilter1, volume( "lvAirGap1" ), pos, {shift1, shift2}, {number1, number2}, "pvGap1HF1" );

    number1 = dd4hep::_toInt( "MUON:HalfFilter1AirGap1bNumber1" );
    pos     = get_pos( "MUON:HalfFilter1AirGap1bPosX", "MUON:HalfFilter1AirGap1bPosY", "0.0" );
    shift1  = get_pos( "MUON:HalfFilter1AirGap1bShift1X", "MUON:HalfFilter1AirGap1bShift1Y", "0.0" );
    parametrical_pv( lvHalfFilter1, volume( "lvAirGap1" ), pos, {shift1}, {number1}, "pvGap1bHF1" );

    number1 = dd4hep::_toInt( "MUON:HalfFilter1Block3Number1" );
    pos     = get_pos( "MUON:HalfFilter1Block3PosX", "MUON:HalfFilter1Block3PosY", "0.0" );
    shift1  = get_pos( "MUON:HalfFilter1Block3Shift1X", "MUON:HalfFilter1Block3Shift1Y", "0.0" );
    rot_z   = dd4hep::_toDouble( "MUON:HalfFilter1Block3RotZ" );
    parametrical_pv( lvHalfFilter1, volume( "lvBlock3" ), pos, {shift1}, {number1}, "pvBlock3HF1", {0.0, 0.0, rot_z} );

    number1 = dd4hep::_toInt( "MUON:HalfFilter1Block1bNumber1" );
    pos     = get_pos( "MUON:HalfFilter1Block1bPosX", "MUON:HalfFilter1Block1bPosY", "0.0" );
    shift1  = get_pos( "MUON:HalfFilter1Block1bShift1X", "MUON:HalfFilter1Block1bShift1Y", "0.0" );
    parametrical_pv( lvHalfFilter1, volume( "lvBlock1" ), pos, {shift1}, {number1}, "pvBlock1bHF1" );

    number1 = dd4hep::_toInt( "MUON:HalfFilter1Concrete3Number1" );
    pos     = get_pos( "MUON:HalfFilter1Concrete3PosX", "MUON:HalfFilter1Concrete3PosY", "0.0" );
    shift1  = get_pos( "0.0", "MUON:HalfFilter1Concrete3Shift1Y", "0.0" );
    parametrical_pv( lvHalfFilter1, volume( "lvConcrete3" ), pos, {shift1}, {number1}, "pvConcrete3HF1" );

    number1 = dd4hep::_toInt( "MUON:HalfFilter1SteelPlate4Number1" );
    pos     = get_pos( "MUON:HalfFilter1SteelPlate4PosX", "MUON:HalfFilter1SteelPlate4PosY", "0.0" );
    shift1  = get_pos( "0.0", "MUON:HalfFilter1SteelPlate4Shift1Y", "0.0" );
    parametrical_pv( lvHalfFilter1, volume( "lvSteelPlate4" ), pos, {shift1}, {number1}, "pvSteelP4HF1" );

    pos = get_pos( "MUON:HalfFilter1Center1PosX", "0.0", "0.0" );
    pv  = lvHalfFilter1.placeVolume( lvCenter1, pos );
    pv->SetName( "pvCenter1HF1" );

    number1 = dd4hep::_toInt( "MUON:HalfFilter1AirGap6Number1" );
    number2 = dd4hep::_toInt( "MUON:HalfFilter1AirGap6Number2" );
    pos     = get_pos( "MUON:HalfFilter1AirGap6PosX", "MUON:HalfFilter1AirGap6PosY", "0.0" );
    shift1  = get_pos( "0.0", "MUON:HalfFilter1AirGap6Shift1Y", "0.0" );
    shift2  = get_pos( "0.0", "MUON:HalfFilter1AirGap6Shift2Y", "0.0" );
    parametrical_pv( lvHalfFilter1, volume( "lvAirGap6" ), pos, {shift1, shift2}, {number1, number2}, "pvGap6HF1" );

    number1 = dd4hep::_toInt( "MUON:HalfFilter1SteelPlate10Number1" );
    number2 = dd4hep::_toInt( "MUON:HalfFilter1SteelPlate10Number2" );
    pos     = get_pos( "MUON:HalfFilter1SteelPlate10PosX", "MUON:HalfFilter1SteelPlate10PosY", "0.0" );
    shift1  = get_pos( "0.0", "MUON:HalfFilter1SteelPlate10Shift1Y", "0.0" );
    shift2  = get_pos( "0.0", "MUON:HalfFilter1SteelPlate10Shift2Y", "0.0" );
    parametrical_pv( lvHalfFilter1, volume( "lvSteelPlate10" ), pos, {shift1, shift2}, {number1, number2},
                     "pvSteelP10HF1" );

    pos = get_pos( "MUON:HalfFilter1SteelPlate12PosX", "MUON:HalfFilter1SteelPlate12PosY", "0.0" );
    pv  = lvHalfFilter1.placeVolume( volume( "lvSteelPlate12" ), pos );
    pv->SetName( "pvSteelP12HF1" );

    pos = get_pos( "MUON:HalfFilter1SteelPlate11PosX", "MUON:HalfFilter1SteelPlate11PosY", "0.0" );
    pv  = lvHalfFilter1.placeVolume( volume( "lvSteelPlate11" ), pos );
    pv->SetName( "pvSteelP11HF1" );

    registerVolume( lvHalfFilter1.name(), lvHalfFilter1 );

    // HalfFilter2
    dd4hep::Assembly lvHalfFilter2( "HalfFilter2" );

    //    number1 =  dd4hep::_toInt("MUON:HalfFilter2Row2Number1");
    pos    = get_pos( "MUON:HalfFilter2Row2PosX", "MUON:HalfFilter2Row2PosY", "0.0" );
    rot_y  = dd4hep::_toDouble( "MUON::HalfFilter2Row2RotY" );
    shift1 = get_pos( "0.0", "MUON:HalfFilter2Row2Shift1Y", "0.0" );

    pv = lvHalfFilter2.placeVolume( lvRow2, pos );
    pv->SetName( "pvRow2HF2_0" );

    pv = lvHalfFilter2.placeVolume( lvRow2, dd4hep::Transform3D( dd4hep::RotationY( rot_y ), pos + shift1 ) );
    pv->SetName( "pvRow2HF2_1" );
    //
    number1 = dd4hep::_toInt( "MUON:HalfFilter2Row2bNumber1" );
    pos     = get_pos( "MUON:HalfFilter2Row2bPosX", "MUON:HalfFilter2Row2bPosY", "0.0" );
    shift1  = get_pos( "0.0", "MUON:HalfFilter2Row2bShift1Y", "0.0" );
    parametrical_pv( lvHalfFilter2, lvRow2, pos, {shift1}, {number1}, "pvRow2bHF2" );
    //
    number1 = dd4hep::_toInt( "MUON:HalfFilter2Block1Number1" );
    pos     = get_pos( "MUON:HalfFilter2Block1PosX", "MUON:HalfFilter2Block1PosY", "0.0" );
    shift1  = get_pos( "MUON:HalfFilter2Block1Shift1X", "0.0", "0.0" );
    parametrical_pv( lvHalfFilter2, volume( "lvBlock1" ), pos, {shift1}, {number1}, "pvBlock1HF2" );
    //
    number1 = dd4hep::_toInt( "MUON:HalfFilter2Block3Number1" );
    pos     = get_pos( "MUON:HalfFilter2Block3PosX", "MUON:HalfFilter2Block3PosY", "0.0" );
    shift1  = get_pos( "0.0", "MUON:HalfFilter2Block3Shift1Y", "0.0" );
    parametrical_pv( lvHalfFilter2, volume( "lvBlock3" ), pos, {shift1}, {number1}, "pvBlock3HF2" );
    //
    number1 = dd4hep::_toInt( "MUON:HalfFilter2AirGap1Number1" );
    pos     = get_pos( "MUON:HalfFilter2AirGap1PosX", "MUON:HalfFilter2AirGap1PosY", "0.0" );
    shift1  = get_pos( "MUON:HalfFilter2AirGap1Shift1X", "0.0", "0.0" );
    parametrical_pv( lvHalfFilter2, volume( "lvAirGap1" ), pos, {shift1}, {number1}, "pvGap1HF2" );
    //
    number1 = dd4hep::_toInt( "MUON:HalfFilter2Block1bNumber1" );
    pos     = get_pos( "MUON:HalfFilter2Block1bPosX", "MUON:HalfFilter2Block1bPosY", "0.0" );
    shift1  = get_pos( "MUON:HalfFilter2Block1bShift1X", "0.0", "0.0" );
    parametrical_pv( lvHalfFilter2, volume( "lvBlock1" ), pos, {shift1}, {number1}, "pvBlock1bHF2" );
    //
    number1 = dd4hep::_toInt( "MUON:HalfFilter2Block3bNumber1" );
    pos     = get_pos( "MUON:HalfFilter2Block3bPosX", "MUON:HalfFilter2Block3bPosY", "0.0" );
    shift1  = get_pos( "MUON:HalfFilter2Block3bShift1X", "0.0", "0.0" );
    rot_z   = dd4hep::_toDouble( "MUON:HalfFilter2Block3bRotZ" );
    parametrical_pv( lvHalfFilter2, volume( "lvBlock3" ), pos, {shift1}, {number1}, "pvBlock3bHF2", {0.0, 0.0, rot_z} );
    //
    number1                 = dd4hep::_toInt( "MUON:HalfFilter2SteelPlate9Number1" );
    number1                 = dd4hep::_toInt( "MUON:HalfFilter2AirGap2Number1" );
    number2                 = dd4hep::_toInt( "MUON:HalfFilter2AirGap2Number2" );
    int number3             = dd4hep::_toInt( "MUON:HalfFilter2AirGap2Number3" );
    pos                     = get_pos( "MUON:HalfFilter2AirGap2PosX", "MUON:HalfFilter2AirGap2PosY", "0.0" );
    shift1                  = get_pos( "MUON:HalfFilter2AirGap2Shift1X", "0.0", "0.0" );
    shift2                  = get_pos( "MUON:HalfFilter2AirGap2Shift2X", "0.0", "0.0" );
    dd4hep::Position shift3 = get_pos( "MUON:HalfFilter2AirGap2Shift3X", "0.0", "0.0" );
    parametrical_pv( lvHalfFilter2, volume( "lvAirGap2" ), pos, {shift1, shift2, shift3}, {number1, number2, number3},
                     "pvGap2HF2" );
    //
    number1 = dd4hep::_toInt( "MUON:HalfFilter2Concrete4Number1" );
    pos     = get_pos( "MUON:HalfFilter2Concrete4PosX", "MUON:HalfFilter2Concrete4PosY", "0.0" );
    shift1  = get_pos( "0.0", "MUON:HalfFilter2Concrete4Shift1Y", "0.0" );
    parametrical_pv( lvHalfFilter2, volume( "lvConcrete4" ), pos, {shift1}, {number1}, "pvConcrete4HF2" );
    //
    pos    = get_pos( "MUON:HalfFilter2SteelPlate9PosX", "MUON:HalfFilter2SteelPlate9PosY", "0.0" );
    shift1 = get_pos( "0.0", "MUON:HalfFilter2SteelPlate9Shift1Y", "0.0" );
    parametrical_pv( lvHalfFilter2, volume( "lvSteelPlate9" ), pos, {shift1}, {number1}, "pvSteelP9HF2" );
    //
    number1 = dd4hep::_toInt( "MUON:HalfFilter2AirGap2bNumber1" );
    pos     = get_pos( "MUON:HalfFilter2AirGap2bPosX", "MUON:HalfFilter2AirGap2bPosY", "0.0" );
    shift1  = get_pos( "MUON:HalfFilter2AirGap2bShift1X", "0.0", "0.0" );
    parametrical_pv( lvHalfFilter2, volume( "lvAirGap2" ), pos, {shift1}, {number1}, "pvGap2bHF2" );
    //
    pos = get_pos( "MUON:HalfFilter2Center2PosX", "0.0", "0.0" );
    pv  = lvHalfFilter2.placeVolume( lvCenter2, pos );
    pv->SetName( "pvCenter2HF2" );
    //
    number1 = dd4hep::_toInt( "MUON:HalfFilter2AirGap7Number1" );
    number2 = dd4hep::_toInt( "MUON:HalfFilter2AirGap7Number2" );
    pos     = get_pos( "MUON:HalfFilter2AirGap7PosX", "MUON:HalfFilter2AirGap7PosY", "0.0" );
    shift1  = get_pos( "0.0", "MUON:HalfFilter2AirGap7Shift1Y", "0.0" );
    shift2  = get_pos( "0.0", "MUON:HalfFilter2AirGap7Shift2Y", "0.0" );
    parametrical_pv( lvHalfFilter2, volume( "lvAirGap7" ), pos, {shift1, shift2}, {number1, number2}, "pvGap7HF2" );
    //
    number1 = dd4hep::_toInt( "MUON:HalfFilter2SteelPlate5Number1" );
    number2 = dd4hep::_toInt( "MUON:HalfFilter2SteelPlate5Number2" );
    pos     = get_pos( "MUON:HalfFilter2SteelPlate5PosX", "MUON:HalfFilter2SteelPlate5PosY", "0.0" );
    shift1  = get_pos( "0.0", "MUON:HalfFilter2SteelPlate5Shift1Y", "0.0" );
    shift2  = get_pos( "0.0", "MUON:HalfFilter2SteelPlate5Shift2Y", "0.0" );
    parametrical_pv( lvHalfFilter2, volume( "lvSteelPlate5" ), pos, {shift1, shift2}, {number1, number2},
                     "pvSteelP5HF2" );
    //
    pos = get_pos( "MUON:HalfFilter2SteelPlate11PosX", "MUON:HalfFilter2SteelPlate11PosY", "0.0" );
    pv  = lvHalfFilter2.placeVolume( volume( "lvSteelPlate11" ), pos );
    pv->SetName( "pvSteelP11HF2" );
    //
    pos = get_pos( "MUON:HalfFilter2SteelPlate12PosX", "MUON:HalfFilter2SteelPlate12PosY", "0.0" );
    pv  = lvHalfFilter2.placeVolume( volume( "lvSteelPlate12" ), pos );
    pv->SetName( "pvSteelP12HF2" );

    registerVolume( lvHalfFilter2.name(), lvHalfFilter2 );

    // First 3 walls are similar, last one has to be treated separately
    std::array<std::string, 3> buildingBlocks = {"HalfFilter1", "HalfFilter2", "HalfFilter2"};

    int nFilters = dd4hep::_toInt( "MUON:NumberOfFilters" );

    for ( int filterNumber = 0; filterNumber < nFilters - 1; filterNumber++ ) {
      // define the Station Volume

      double filter_size_x = dd4hep::_toDouble( Form( "MUON:Filter%01dXOuterSize", filterNumber + 1 ) );
      double filter_size_y = dd4hep::_toDouble( Form( "MUON:Filter%01dYOuterSize", filterNumber + 1 ) );
      double filter_size_z = dd4hep::_toDouble( Form( "MUON:Filter%01dZOuterSize", filterNumber + 1 ) );

      dd4hep::Box filterBox( filter_size_x / 2.0, filter_size_y / 2.0, filter_size_z / 2.0 );

      // plug box dimensions
      double plug_size_X = dd4hep::_toDouble( Form( "MUON:Filter%01dPlugBoxSizeX", filterNumber + 1 ) );
      double plug_size_Y = dd4hep::_toDouble( Form( "MUON:Filter%01dPlugBoxSizeY", filterNumber + 1 ) );
      double plug_size_Z = dd4hep::_toDouble( Form( "MUON:Filter%01dPlugBoxSizeZ", filterNumber + 1 ) );

      dd4hep::Box PlugBox( ( plug_size_X - muon_eps ) / 2.0, ( plug_size_Y - muon_eps ) / 2.0,
                           ( plug_size_Z - muon_eps ) / 2.0 );

      // plug hole dimensions
      double plug_cone_length    = dd4hep::_toDouble( Form( "MUON:Filter%01dPlugHoleSizeZ", filterNumber + 1 ) );
      double plug_cone_outer_rad = dd4hep::_toDouble( Form( "MUON:Filter%01dPlugHoleOuterRad", filterNumber + 1 ) );
      double plug_cone_inner_rad = dd4hep::_toDouble( Form( "MUON:Filter%01dPlugHoleInnerRad", filterNumber + 1 ) );

      dd4hep::Cone SubtractionCone( "PlugHole", plug_cone_length + muon_eps, 0.0, plug_cone_inner_rad, 0.0,
                                    plug_cone_outer_rad );

      // subtract the hole from the filter volume
      dd4hep::SubtractionSolid MuFilterVol(
          filterBox, SubtractionCone,
          dd4hep::Transform3D( dd4hep::RotationX( -1 * rot_ang ), dd4hep::Position( 0.0, 200.0 * dd4hep::mm, 0.0 ) ) );
      dd4hep::Volume lvFilter( Form( "lvMuFilter%01d", filterNumber + 1 ), MuFilterVol, description.air() );
      //      dd4hep::Assembly lvFilter( Form( "lvMuFilter%01d", filterNumber + 1 ) );
      lvFilter.setVisAttributes( description, "Muon:StationVis" );

      dd4hep::SubtractionSolid PlugA( PlugBox, SubtractionCone,
                                      dd4hep::Position( -plug_size_X / 2.0, -7.5 * dd4hep::mm, 0.0 ) );
      dd4hep::SubtractionSolid PlugC( PlugBox, SubtractionCone,
                                      dd4hep::Position( plug_size_X / 2.0, -7.5 * dd4hep::mm, 0.0 ) );

      double filter_side_offset = dd4hep::_toDouble( "MUON:FilterSideOffset" );

      dd4hep::Volume lvPlugA( Form( "lvFilterBeam%01dA", filterNumber + 1 ), PlugA,
                              description.material( "MUON:FilterIron" ) );
      dd4hep::Volume lvPlugC( Form( "lvFilterBeam%01dC", filterNumber + 1 ), PlugC,
                              description.material( "MUON:FilterIron" ) );

      // It would be better if we place inside the sides. //Rotation now?
      pv = lvFilter.placeVolume( lvPlugA,
                                 dd4hep::Position( plug_size_X / 2.0 + filter_side_offset, 0.2075 * dd4hep::m, 0.0 ) );
      pv->SetName( Form( "FilterBeam%01dA", filterNumber + 1 ) );
      lvPlugA.setVisAttributes( description, "MUON:BeamPlugVis" );

      pv = lvFilter.placeVolume( lvPlugC,
                                 dd4hep::Position( -plug_size_X / 2.0 - filter_side_offset, 0.2075 * dd4hep::m, 0.0 ) );
      pv->SetName( Form( "FilterBeam%01dC", filterNumber + 1 ) );
      lvPlugC.setVisAttributes( description, "MUON:BeamPlugVis" );

      pv = lvFilter.placeVolume(
          volume( buildingBlocks[filterNumber] ),
          dd4hep::Position( -dd4hep::_toDouble( Form( "MUON:HalfSideF%01dPosX", filterNumber + 1 ) ) -
                                filter_side_offset,
                            0.0, 0.0 ) );
      pv->SetName( Form( "pvHalfFilter%dCSide", filterNumber + 1 ) );

      pv = lvFilter.placeVolume(
          volume( buildingBlocks[filterNumber] ),
          dd4hep::Transform3D(
              dd4hep::RotationZYX( 0, 180 * dd4hep::degree, 0 ),
              dd4hep::Position( dd4hep::_toDouble( Form( "MUON:HalfSideF%01dPosX", filterNumber + 1 ) ) +
                                    filter_side_offset,
                                0.0, 0.0 ) ) );
      pv->SetName( Form( "pvHalfFilter%dASide", filterNumber + 1 ) );

      lvFilter.setVisAttributes( description, "MUON:FilterVis" );
      pv = lvMuon.placeVolume( lvFilter,
                               dd4hep::Transform3D( dd4hep::RotationX( rot_ang ),
                                                    get_pos( Form( "MUON:Filter%01dPositionX", filterNumber + 1 ),
                                                             Form( "MUON:Filter%01dPositionY", filterNumber + 1 ),
                                                             Form( "MUON:Filter%01dPositionZ", filterNumber + 1 ) ) ) );
      pv->SetName( dd4hep::_toString( filterNumber + 1, "pvMuFilter%01d" ).c_str() );
      ////pv.addPhysVolID( "filter", filterNumber );
      ////registerVolume( lvFilter.name(), lvFilter );
      dd4hep::DetElement deFilter = DetElement( detector, Form( "deMuFilter%d", filterNumber + 1 ), filterNumber );
      deFilter.setPlacement( pv );
    }

    if ( nFilters == 4 ) {
      // Extra wall at the end

      // similar to the old implementation with an union, the last filter (MuFilter4) is an assembly
      dd4hep::Assembly lvFilter( "lvMuFilter4" );

      // external box
      dd4hep::Box ext_filterBox( dd4hep::_toDouble( "MUON:Filter4ExtBoxSizeX" ) / 2,
                                 dd4hep::_toDouble( "MUON:Filter4ExtBoxSizeY" ) / 2,
                                 dd4hep::_toDouble( "MUON:Filter4ExtBoxSizeZ" ) / 2 );
      // internal box
      dd4hep::Box int_filterBox( dd4hep::_toDouble( "MUON:Filter4IntBoxSizeX" ) / 2,
                                 dd4hep::_toDouble( "MUON:Filter4IntBoxSizeY" ) / 2,
                                 dd4hep::_toDouble( "MUON:Filter4IntBoxSizeZ" ) / 2 );

      dd4hep::SubtractionSolid MuFilterVol(
          ext_filterBox, int_filterBox,
          dd4hep::Transform3D( dd4hep::RotationX( rot_ang ), dd4hep::Position( 0.0, 200.0 * dd4hep::mm, 0.0 ) ) );

      dd4hep::Volume lvFilterLower( "lvMuFilter4Low", MuFilterVol, description.material( "MUON:FilterIron" ) );

      dd4hep::Box extra_filterBox( dd4hep::_toDouble( "MUON:Filter4ExtraBoxSizeX" ) / 2,
                                   dd4hep::_toDouble( "MUON:Filter4ExtraBoxSizeY" ) / 2,
                                   dd4hep::_toDouble( "MUON:Filter4ExtraBoxSizeZ" ) / 2 );

      dd4hep::Volume lvFilterExtraIron( "lvMuFilter4ExtraIron", extra_filterBox,
                                        description.material( "MUON:FilterIron" ) );

      pv = lvFilter.placeVolume( lvFilterLower, dd4hep::Position( 0.0, 0.0, 0.0 ) );
      pv->SetName( "pvMuFilter4Low" );
      pv = lvFilter.placeVolume( lvFilterExtraIron,
                                 dd4hep::Position( 0.0,
                                                   dd4hep::_toDouble( "MUON:Filter4ExtBoxSizeY" ) / 2 +
                                                       dd4hep::_toDouble( "MUON:Filter4ExtraBoxSizeY" ) / 2,
                                                   dd4hep::_toDouble( "MUON:Filter4ExtBoxSizeZ" ) / 2 -
                                                       dd4hep::_toDouble( "MUON:Filter4ExtraBoxSizeZ" ) / 2 ) );

      pv = lvMuon.placeVolume( lvFilter, dd4hep::Transform3D( dd4hep::RotationX( rot_ang ),
                                                              get_pos( "MUON:Filter4PositionX", "MUON:Filter4PositionY",
                                                                       "MUON:Filter4PositionZ" ) ) );
      pv->SetName( "pvMuFilter4" );
      dd4hep::DetElement deFilter = DetElement( detector, "deMuFilter4", 3 );
      deFilter.setPlacement( pv );
      ////pv.addPhysVolID( "filter", 3 );
      ////registerVolume( lvFilter.name(), lvFilter );
    }
  }

  void MuonBuild::build_plugs() {
    dd4hep::PlacedVolume pv;

    for ( int stationNumber = 2; stationNumber < nStations + 2; stationNumber++ ) {
      dd4hep::Volume lvStation = volume( Form( "M%01d", stationNumber ) );

      double size_X = dd4hep::_toDouble( Form( "MUON:M%01dPlugBoxSizeX", stationNumber ) );
      double size_Y = dd4hep::_toDouble( Form( "MUON:M%01dPlugBoxSizeY", stationNumber ) );
      double size_Z = dd4hep::_toDouble( Form( "MUON:M%01dPlugBoxSizeZ", stationNumber ) );

      dd4hep::Box PlugBox( size_X / 2.0, size_Y / 2.0, size_Z / 2.0 );

      double cone_length    = dd4hep::_toDouble( Form( "MUON:M%01dPlugHoleSizeZ", stationNumber ) );
      double cone_outer_rad = dd4hep::_toDouble( Form( "MUON:M%01dPlugHoleOuterRad", stationNumber ) );
      double cone_inner_rad = dd4hep::_toDouble( Form( "MUON:M%01dPlugHoleInnerRad", stationNumber ) );

      dd4hep::Cone SubtractionCone( "PlugHole", cone_length + muon_eps, 0.0, cone_inner_rad, 0.0, cone_outer_rad );

      dd4hep::SubtractionSolid PlugA( PlugBox, SubtractionCone, dd4hep::Position( -size_X / 2.0, 0.0, 0.0 ) );
      dd4hep::SubtractionSolid PlugC( PlugBox, SubtractionCone, dd4hep::Position( size_X / 2.0, 0.0, 0.0 ) );

      dd4hep::Volume lvPlugA( Form( "lvM%01dPlugA", stationNumber ), PlugA, description.material( "MUON:FilterIron" ) );
      dd4hep::Volume lvPlugC( Form( "lvM%01dPlugC", stationNumber ), PlugC, description.material( "MUON:FilterIron" ) );

      // It would be better if we place inside the sides.
      pv = lvStation.placeVolume( lvPlugA, dd4hep::Position( size_X / 2.0, 0.0, 0.0 ) );
      pv->SetName( Form( "M%01dPlugA", stationNumber ) );
      lvPlugA.setVisAttributes( description, "MUON:BeamPlugVis" );

      pv = lvStation.placeVolume( lvPlugC, dd4hep::Position( -size_X / 2.0, 0.0, 0.0 ) );
      pv->SetName( Form( "M%01dPlugC", stationNumber ) );
      lvPlugC.setVisAttributes( description, "MUON:BeamPlugVis" );
    }
  }

  dd4hep::DetElement MuonBuild::build_chamber_prototype( int stationNumber, int regionNumber ) {

    dd4hep::PlacedVolume pv;
    //
    //    std::cout << "Building prototype chamber" << std::endl;
    // chamber dimensions depending on station and regio
    double chamberSizeX = dd4hep::_toDouble( Form( "MUON:M%01dR%01dChamberBoxSizeX", stationNumber, regionNumber ) );
    double chamberSizeY = dd4hep::_toDouble( Form( "MUON:M%01dChamberBoxSizeY", stationNumber ) );
    double chamberSizeZ = dd4hep::_toDouble( Form( "MUON:M%01dChamberBoxSizeZ", stationNumber ) );

    // gap dimensions depending on station and region
    double gasgapBoxSizeX = dd4hep::_toDouble( Form( "MUON:M%01dR%01dGasGapBoxSizeX", stationNumber, regionNumber ) );
    double gasgapBoxSizeY = dd4hep::_toDouble( Form( "MUON:M%01dGasGapBoxSizeY", stationNumber ) );
    //    double gasgapboxSizeZ = dd4hep::_toDouble("MUON:M2R1GasGapBoxSizeZ"); <- this can be guessed

    // Keep the same logic used for the previous geometry description
    // the chamber is divid into 4 parts that are repeated made of a
    // building block composed by (pulyurethane, G10, copper, gas, copper,
    // G10, pulyurethane) and two ending parts

    /*
    This is the old representation
    /--EndCap --/---------------Gap---------------/...*3.../--EndCap --/
    |C| G |  P  |  P  | G |C|         |C| G |  P  |        |  P  | G |C|
    |o| 1 |  o  |  o  | 1 |o|    G    |o| 1 |  o  |        |  o  | 1 |o|
    |p| 0 |  l  |  l  | 0 |p|    a    |p| 0 |  l  |        |  l  | 0 |p|
    |p|   |  y  |  y  |   |p|    s    |p|   |  y  |........|  y  |   |p|
    |e|   |  u  |  u  |   |e|    G    |e|   |  u  |        |  u  |   |e|
    |r|   |  r  |  r  |   |r|    a    |r|   |  r  |        |  r  |   |r|
    | |   |  e  |  e  |   | |    p    | |   |  e  |        |  e  |   | |
    | |   |  t  |  t  |   | |         | |   |  t  |        |  t  |   | |
                / L9  / L7  /   L5    / L3  / L1  /
    */

    double copperLayerThick       = dd4hep::_toDouble( "MUON:CopperLayerThickness" );  // .03 mm
    double g10LayerThick          = dd4hep::_toDouble( "MUON:G10LayerThickness" );     // 1.6 mm
    double polyurethaneLayerThick = dd4hep::_toDouble( "MUON:PolyurethaneThickness" ); // 2.9 mm
    double gasgapThick            = dd4hep::_toDouble( "MUON:GasGapThickness" );       // 5.  mm

    // First plane also called EndCap
    dd4hep::Box    boxCopperLayer( gasgapBoxSizeX / 2.0, gasgapBoxSizeY / 2.0, ( copperLayerThick - muon_eps ) / 2.0 );
    dd4hep::Volume lvCopperLayer( "lvCopperLayer", boxCopperLayer, copper );
    lvCopperLayer.setVisAttributes( description, "MUON:CopperLayerVis" );

    dd4hep::Box    boxG10Layer( gasgapBoxSizeX / 2.0, gasgapBoxSizeY / 2.0, ( g10LayerThick - muon_eps ) / 2.0 );
    dd4hep::Volume lvG10Layer( "lvG10Layer", boxG10Layer, g10 );
    lvG10Layer.setVisAttributes( description, "MUON:G10LayerVis" );

    dd4hep::Box    boxPolyurethaneLayer( gasgapBoxSizeX / 2.0, gasgapBoxSizeY / 2.0,
                                      ( polyurethaneLayerThick - muon_eps ) / 2.0 );
    dd4hep::Volume lvPolyurethaneLayer( "lvPolyurethane", boxPolyurethaneLayer, polyurethane );
    lvPolyurethaneLayer.setVisAttributes( description, "MUON:PolyurethaneLayerVis" );

    dd4hep::Box    boxGasLayer( gasgapBoxSizeX / 2.0, gasgapBoxSizeY / 2.0, ( gasgapThick - muon_eps ) / 2.0 );
    dd4hep::Volume lvGasLayer( "lvGasLayer", boxGasLayer, description.material( "MUON:S_CPC_gas" ) );
    lvGasLayer.setVisAttributes( description, "MUON:GasGapVis" );
    lvGasLayer.setSensitiveDetector( sensitive );

    dd4hep::Box              boxOuterFrame( chamberSizeX / 2.0, chamberSizeY / 2.0, chamberSizeZ / 2.0 );
    dd4hep::Box              boxInnerFrame( gasgapBoxSizeX / 2.0 + muon_eps, gasgapBoxSizeY / 2.0 + muon_eps,
                               chamberSizeZ / 2.0 + muon_eps ); //<- no error here
    dd4hep::SubtractionSolid frame( boxOuterFrame, boxInnerFrame );
    dd4hep::Volume           lvFrame( "lvFrame", frame, description.material( "MUON:G10Air" ) );
    lvFrame.setVisAttributes( description, "MUON:FrameVis" );

    dd4hep::Assembly lvChamber( Form( "lvChamberM%dR%d", stationNumber, regionNumber ) );
    double           pos_X = 0.0;
    double           pos_Y = 0.0;
    double           pos_Z = 0.0;

    // Build the two endcaps (a layer of copper/g10/polyurethane that close the interanl gasgap structure)
    double         size_endcap_Z = copperLayerThick + g10LayerThick + polyurethaneLayerThick;
    dd4hep::Box    boxEndCap0( gasgapBoxSizeX / 2.0, gasgapBoxSizeY / 2.0, ( size_endcap_Z - muon_eps ) / 2.0 );
    dd4hep::Volume lvEndCap0( "lvEndCap0", boxEndCap0, air );
    // dd4hep::Assembly lvEndCap0("lvEndCap0")
    pos_Z = ( g10LayerThick + polyurethaneLayerThick ) / 2.0;
    pv    = lvEndCap0.placeVolume( lvCopperLayer, dd4hep::Position( pos_X, pos_Y, pos_Z ) );
    pv->SetName( "pvCopperEndCap0" );

    pos_Z = polyurethaneLayerThick / 2.0 - copperLayerThick / 2.0;
    pv    = lvEndCap0.placeVolume( lvG10Layer, dd4hep::Position( pos_X, pos_Y, pos_Z ) );
    pv->SetName( "pvG10EndCap0" );

    pos_Z = -( copperLayerThick + g10LayerThick ) / 2;
    pv    = lvEndCap0.placeVolume( lvPolyurethaneLayer, dd4hep::Position( pos_X, pos_Y, pos_Z ) );
    pv->SetName( "pvPolyEndCap0" );

    dd4hep::Box    boxEndCap1( gasgapBoxSizeX / 2.0, gasgapBoxSizeY / 2.0, ( size_endcap_Z - muon_eps ) / 2.0 );
    dd4hep::Volume lvEndCap1( "lvEndCap1", boxEndCap1, air );
    // dd4hep::Assembly lvEndCap1("lvEndCap1");
    pos_Z = -( g10LayerThick + polyurethaneLayerThick ) / 2.0;
    pv    = lvEndCap1.placeVolume( lvCopperLayer, dd4hep::Position( pos_X, pos_Y, pos_Z ) );
    pv->SetName( "pvCopperEndCap1" );

    pos_Z = -polyurethaneLayerThick / 2.0 + copperLayerThick / 2.0;
    pv    = lvEndCap1.placeVolume( lvG10Layer, dd4hep::Position( pos_X, pos_Y, pos_Z ) );
    pv->SetName( "pvG10EndCap1" );

    pos_Z = ( copperLayerThick + g10LayerThick ) / 2;
    pv    = lvEndCap1.placeVolume( lvPolyurethaneLayer, dd4hep::Position( pos_X, pos_Y, pos_Z ) );
    pv->SetName( "pvPolyEndCap1" );

    // Build the repeating structure of the gasgap made of polyurethane g10 copper gasmixture copper g10 polyurethane
    // this structure is divided into 5 parts:
    // layer 1: polyurethane -> gasgaplayer1
    // layer 2: g10, copper  -> gasgaplayer3
    // layer 3: gas mixture  -> gasgaplayer5
    // layer 4: g10, copper  -> gasgaplayer7
    // layer 5: polyurethane -> gasgaplayer9
    // This part is repeated 4 times in the detector

    // gasgaplayer 1
    double         size_Z = polyurethaneLayerThick;
    dd4hep::Box    boxGasGapLayer1( gasgapBoxSizeX / 2.0, gasgapBoxSizeY / 2.0, ( size_Z - muon_eps ) / 2.0 );
    dd4hep::Volume lvGasGapLayer1( "lvGasGapLayer1", boxGasGapLayer1, air );

    pos_Z = 0.0;
    pv    = lvGasGapLayer1.placeVolume( lvPolyurethaneLayer, dd4hep::Position( pos_X, pos_Y, pos_Z ) );
    pv->SetName( "pvGasGapLayer1" );

    // gasgaplayer3
    size_Z = g10LayerThick + copperLayerThick;
    dd4hep::Box    boxGasGapLayer3( gasgapBoxSizeX / 2.0, gasgapBoxSizeY / 2.0, ( size_Z - muon_eps ) / 2.0 );
    dd4hep::Volume lvGasGapLayer3( "lvGasGapLayer3", boxGasGapLayer3, air );

    pos_Z = copperLayerThick / 2.0;
    pv    = lvGasGapLayer3.placeVolume( lvG10Layer, dd4hep::Position( pos_X, pos_Y, pos_Z ) );
    pv->SetName( "pvG10GapLayer3" );

    pos_Z = -g10LayerThick / 2.0;
    pv    = lvGasGapLayer3.placeVolume( lvCopperLayer, dd4hep::Position( pos_X, pos_Y, pos_Z ) );
    pv->SetName( "pvCopperGapLayer3" );

    // gasgaplayer5
    size_Z = gasgapThick;
    pos_Z  = 0.0;
    dd4hep::Box    boxGasGapLayer5( gasgapBoxSizeX / 2.0, gasgapBoxSizeY / 2.0, ( size_Z - muon_eps ) / 2.0 );
    dd4hep::Volume lvGasGapLayer5( "lvGasGapLayer5", boxGasGapLayer5, air );
    // pv = lvGasGapLayer5.placeVolume( lvGasLayer, dd4hep::Position( pos_X, pos_Y, pos_Z ) );
    dd4hep::PlacedVolume pvGasGap = lvGasGapLayer5.placeVolume( lvGasLayer, dd4hep::Position( pos_X, pos_Y, pos_Z ) );
    pvGasGap->SetName( "pvGasGapLayer5" );

    // gasgaplayer7
    size_Z = g10LayerThick + copperLayerThick;
    dd4hep::Box    boxGasGapLayer7( gasgapBoxSizeX / 2.0, gasgapBoxSizeY / 2.0, ( size_Z - muon_eps ) / 2.0 );
    dd4hep::Volume lvGasGapLayer7( "lvGasGapLayer7", boxGasGapLayer7, air );

    pos_Z = -copperLayerThick / 2.0;
    pv    = lvGasGapLayer7.placeVolume( lvG10Layer, dd4hep::Position( pos_X, pos_Y, pos_Z ) );
    pv->SetName( "pvG10GapLayer7" );

    pos_Z = g10LayerThick / 2.0;
    pv    = lvGasGapLayer7.placeVolume( lvCopperLayer, dd4hep::Position( pos_X, pos_Y, pos_Z ) );
    pv->SetName( "pvCopperGapLayer7" );

    // gasgaplayer9
    size_Z = polyurethaneLayerThick;
    dd4hep::Box    boxGasGapLayer9( gasgapBoxSizeX / 2.0, gasgapBoxSizeY / 2.0, ( size_Z - muon_eps ) / 2.0 );
    dd4hep::Volume lvGasGapLayer9( "lvGasGapLayer9", boxGasGapLayer9, air );

    pv = lvGasGapLayer9.placeVolume( lvPolyurethaneLayer, dd4hep::Position( pos_X, pos_Y, pos_Z ) );
    pv->SetName( "pvGasGapLayer9" );

    // Put the 5 layers together
    double      size_gap_Z = polyurethaneLayerThick * 2.0 + g10LayerThick * 2.0 + copperLayerThick * 2.0 + gasgapThick;
    dd4hep::Box boxGasGap( gasgapBoxSizeX / 2.0, gasgapBoxSizeY / 2.0, ( size_gap_Z - muon_eps ) / 2.0 );
    dd4hep::Volume lvGasGap( "lvGasGap", boxGasGap, air );

    pos_Z = gasgapThick / 2.0 + copperLayerThick + g10LayerThick + polyurethaneLayerThick / 2.0;
    lvGasGap.placeVolume( lvGasGapLayer1, dd4hep::Position( pos_X, pos_Y, pos_Z ) );
    pos_Z = gasgapThick / 2.0 + copperLayerThick / 2.0 + g10LayerThick / 2.0;
    lvGasGap.placeVolume( lvGasGapLayer3, dd4hep::Position( pos_X, pos_Y, pos_Z ) );
    pos_Z = 0.0;
    // dd4hep::PlacedVolume pvGasGap = lvGasGap.placeVolume( lvGasGapLayer5, dd4hep::Position( pos_X, pos_Y, pos_Z ) );
    pv = lvGasGap.placeVolume( lvGasGapLayer5, dd4hep::Position( pos_X, pos_Y, pos_Z ) );
    // pvGasGap->SetName( "pvGasGap" );
    // pvGasGap.addPhysVolID( "gap", id );
    pos_Z = -( gasgapThick / 2.0 + copperLayerThick / 2.0 + g10LayerThick / 2.0 );
    lvGasGap.placeVolume( lvGasGapLayer7, dd4hep::Position( pos_X, pos_Y, pos_Z ) );
    pos_Z = -( gasgapThick / 2.0 + copperLayerThick + g10LayerThick + polyurethaneLayerThick / 2.0 );
    lvGasGap.placeVolume( lvGasGapLayer9, dd4hep::Position( pos_X, pos_Y, pos_Z ) );

    // Detector elements
    dd4hep::DetElement deChamber( "chamber", id );

    // Place frame around
    pos_X                = 0.0;
    pos_Y                = 0.0;
    pos_Z                = 0.0;
    dd4hep::Position pos = dd4hep::Position( pos_X, pos_Y, pos_Z );
    pv                   = lvChamber.placeVolume( lvFrame, pos );

    // Place the two endcaps
    int tot_num_gaps = dd4hep::_toInt( "MUON:NGaps" );

    pos = dd4hep::Position( pos_X, pos_Y, size_gap_Z * tot_num_gaps / 2.0 + size_endcap_Z / 2.0 );
    lvChamber.placeVolume( lvEndCap0, pos );
    pos = dd4hep::Position( pos_X, pos_Y, -size_gap_Z * tot_num_gaps / 2.0 - size_endcap_Z / 2.0 );
    lvChamber.placeVolume( lvEndCap1, pos );

    // replicate the 4 gaps
    for ( int num_gap = 0; num_gap < tot_num_gaps; num_gap++ ) {
      //
      pos = dd4hep::Position( pos_X, pos_Y, -size_gap_Z * ( num_gap - ( tot_num_gaps - 1.0 ) / 2.0 ) );
      //
      // reference is from the middle
      pv                      = lvChamber.placeVolume( lvGasGap, pos );
      std::string gasgap_name = Form( "GasGap%d", num_gap + 1 );
      pv->SetName( gasgap_name.c_str() );
      pv.addPhysVolID( "GasGap", num_gap );
      dd4hep::DetElement deGasGap = dd4hep::DetElement( deChamber, Form( "GasGap%d", num_gap + 1 ), num_gap + 1 );
      deGasGap.setPlacement( pv );
      //
      dd4hep::DetElement deSensitiveGasGap = dd4hep::DetElement( deGasGap, "GasGapLayer", 1 );
      deSensitiveGasGap.setPlacement( pvGasGap );
      pvGasGap.addPhysVolID( "GasGapLayer", 1 );
    }

    lvChamber.setVisAttributes( description, "MUON:ChamberVis" );
    registerVolume( lvChamber.name(), lvChamber );
    return deChamber;
  }

  void MuonBuild::build_detector() {

    /*

      Regions are divided into the two sides. This is made in order to simplify the alignment

      _______________________      ________________________
      |                     |      |                      |
      |          Reg2 SideA |      | Reg2 SideC           |
      |      _______________|      |_______________       |
      |      |              |      |              |       |
      |      |   Reg1 SideA |      | Reg1 SideC   |       |
      |      |     _________|      |_________     |       |
      |      |     |                        |     |       |
      |------|-----|                        |-----|-------|
      |      |     |_________      _________|     |       |
      |      |              |      |              |       |
      |      |              |      |              |       |
      |      |______________|      |______________|       |
      |                     |      |                      |
      |                     |      |                      |
      |_____________________|      |______________________|

    */

    dd4hep::PlacedVolume pv;

    dd4hep::Box    muon_box( "MUON:BoxX/2", "MUON:BoxY/2", "MUON:BoxZ/2" );
    dd4hep::Volume lvMuon( select_volume.c_str(), muon_box, description.air() );
    lvMuon.setVisAttributes( description, "MUON:MuonVis" );

    if ( b_build_stations ) {

      dd4hep::DetElement deStation;

      // loop over the stations
      for ( int stationNumber = 2; stationNumber < nStations + 2; stationNumber++ ) {

        // define the Station Volume
        //        std::cout << "Station M" << stationNumber << std::endl;
        dd4hep::Box stationBox( Form( "MUON:M%01dXOuterSize", stationNumber ),
                                Form( "MUON:M%01dYOuterSize", stationNumber ),
                                Form( "MUON:M%01dZOuterSize", stationNumber ) );
        //        dd4hep::Volume lvStation( Form( "M%01d", stationNumber ), stationBox, description.air() );
        dd4hep::Assembly lvStation( Form( "M%01d", stationNumber ) );
        lvStation.setVisAttributes( description, "Muon:StationVis" );
        deStation = dd4hep::DetElement( detector, Form( "M%01dStation", stationNumber ), stationNumber - 2 );

        // get the station Z position
        double station_z = dd4hep::_toDouble( dd4hep::_toString( stationNumber, "MUON:M%01dPositionZ" ) );

        //        std::cout << "lvSide " << std::endl;
        std::array<dd4hep::Assembly, 2>   lvSide = {dd4hep::Assembly( "lvSideA" ), dd4hep::Assembly( "lvSideC" )};
        std::array<dd4hep::DetElement, 2> deSide = {
            dd4hep::DetElement( deStation, Form( "M%01dASide", stationNumber ), 0 ),
            dd4hep::DetElement( deStation, Form( "M%01dCSide", stationNumber ), 1 )};

        const int nRegions = dd4hep::_toDouble( "MUON:NumberOfRegions" );
        for ( int regionNumber = 1; regionNumber <= nRegions; regionNumber++ ) {
          // build Chamber prototype and clone
          dd4hep::DetElement deChamberProto = build_chamber_prototype( stationNumber, regionNumber );

          // build Region

          // instead of an assembly we could make a subtraction volume... is it a good idea?
          //          std::cout << "lvRegion " << std::endl;
          std::array<dd4hep::Assembly, 2> lvRegion = {
              dd4hep::Assembly( Form( "lvRegionM%01dR%01dSideA", stationNumber, regionNumber ) ),
              dd4hep::Assembly( Form( "lvRegionM%01dR%01dSideC", stationNumber, regionNumber ) )};

          std::array<dd4hep::DetElement, 2> deRegion = {
              dd4hep::DetElement( deSide[0], Form( "M%01dR%01dASide", stationNumber, regionNumber ), regionNumber - 1 ),
              dd4hep::DetElement( deSide[1], Form( "M%01dR%01dCSide", stationNumber, regionNumber ),
                                  regionNumber - 1 )};

          //          std::cout << "deChamber " << std::endl;
          // put chambers in region side A(side=0) and in region side C(side=1)
          //
          dd4hep::DetElement deChamber;

          const int nChambersInRegion = dd4hep::_toInt( Form( "MUON:NChambersR%d", regionNumber ) );
          //
          for ( int chamberNumber = 1; chamberNumber <= nChambersInRegion; chamberNumber++ ) {
            // Clone prototype
            //            std::cout << "clone chamber" << std::endl;

            deChamber =
                deChamberProto.clone( Form( "M%01dR%01dCham%03d", stationNumber, regionNumber, chamberNumber ) );

            // recover from registered volumes
            dd4hep::Assembly lvChamber = volume( Form( "lvChamberM%dR%d", stationNumber, regionNumber ) );
            //( Form( "lvChamberM%dR%d", stationNumber, regionNumber ) );

            std::string name = Form( "M%dR%dCham%03d", stationNumber, regionNumber, chamberNumber );
            int         side = chambers[name].side;
            //            std::cout << name << " :: " << side << " - " << chambers[name].pos() << std::endl;
            pv = lvRegion[side].placeVolume( lvChamber, chambers[name].pos() );

            pv->SetName( Form( "pvM%dR%dCham%02d", stationNumber, regionNumber, chamberNumber ) );

            //
            pv.addPhysVolID( "Chamber", chamberNumber - 1 );
            deChamber.setPlacement( pv );
            deRegion[side].add( deChamber );
            //
            //	    std::cout << "muon_geo: put chamber " << name << " into Station M" << stationNumber << " Region R"<<
            // regionNumber << " side "<< side << " cha ID "<< chaId<<std::endl;
            //
          }
          //
          // Place region into side
          for ( unsigned int iside = 0; iside < 2; iside++ ) {
            // std::cout << "muon_geo: put region R" << regionNumber << " into Station M" << stationNumber << std::endl;
            pv = lvSide[iside].placeVolume( lvRegion[iside], dd4hep::Position( 0.0, 0.0, 0.0 ) );
            pv->SetName( Form( "pvM%01dR%01d%cSide", stationNumber, regionNumber, nameSide[iside] ) );
            pv.addPhysVolID( "Region", regionNumber - 1 );
            // pv.addPhysVolID( "region", regionNumber);
            deRegion[iside].setPlacement( pv );
          }
        }
        // Put Sides into Station
        for ( unsigned int iside = 0; iside < 2; iside++ ) {
          // std::cout << "muon_geo: put side " << nameSide[iside] << " into Station M" << stationNumber << std::endl;
          pv = lvStation.placeVolume(
              lvSide[iside],
              dd4hep::Position(
                  dd4hep::_toDouble( Form( "MUON:M%01d%cSideXOffset", stationNumber, nameSide[iside] ) ),
                  dd4hep::_toDouble( Form( "MUON:M%01d%cSideYOffset", stationNumber, nameSide[iside] ) ),
                  dd4hep::_toDouble( Form( "MUON:M%01d%cSideZOffset", stationNumber, nameSide[iside] ) ) ) );
          pv->SetName( Form( "pvM%01d%cSide", stationNumber, nameSide[iside] ) );
          pv.addPhysVolID( "Side", iside );
          deSide[iside].setPlacement( pv );
        }

        // place Station Volume inside the Muon Detector Volume
        // std::cout << "muon_geo: put Station M" << stationNumber << " into lvMuon " << lvMuon.name() << std::endl;
        double rot_ang = dd4hep::_toDouble( "MUON:beamAngle" );
        pv             = lvMuon.placeVolume(
            lvStation, dd4hep::Transform3D( dd4hep::RotationX( rot_ang ), dd4hep::Position( 0.0, 0.0, station_z ) ) );
        pv->SetName( dd4hep::_toString( stationNumber, "M%01d" ).c_str() );
        pv.addPhysVolID( "Station", stationNumber - 2 );
        // pv.addPhysVolID( "station", stationNumber);
        deStation.setPlacement( pv );
        registerVolume( lvStation.name(), lvStation );
      }

      // std::cout << "registerVolume!" << lvMuon.name() << std::endl;
    }
    registerVolume( lvMuon.name(), lvMuon );
    if ( b_build_support ) { build_support(); }
    if ( b_build_plugs ) { build_plugs(); }
    if ( b_build_filters ) { build_filters(); }
  }

  // Initiate Muon Detector
  void MuonBuild::build_muon() {

    sensitive.setType( "tracker" );

    load( x_det, "include" );
    buildVolumes( x_det );
    placeDaughters( detector, dd4hep::Volume(), x_det );
    build_detector();
    dd4hep::Volume       vol = volume( select_volume );
    dd4hep::PlacedVolume pv  = placeDetector( vol );
    pv.addPhysVolID( "system", id );
    //    dd4hep::detail::destroyHandle( de_muon_with_support );

    return;
  }

} // namespace

static dd4hep::Ref_t create_element( dd4hep::Detector& description, xml_h e, dd4hep::SensitiveDetector sens_det ) {
  MuonBuild builder( description, e, sens_det );
  builder.build_muon();

  return builder.detector;
}
DECLARE_DETELEMENT( LHCb_Muon_v1_0, create_element )
