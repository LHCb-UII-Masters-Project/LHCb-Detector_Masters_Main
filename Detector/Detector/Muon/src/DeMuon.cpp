/***************************************************************************** \
 * (c) Copyright 2000-2019 CERN for the benefit of the LHCb Collaboration      *
 *                                                                             *
 * This software is distributed under the terms of the GNU General Public      *
 * Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
 *                                                                             *
 * In applying this licence, CERN does not waive the privileges and immunities *
 * granted to it by virtue of its status as an Intergovernmental Organization  *
 * or submit itself to any jurisdiction.                                       *
\*****************************************************************************/

#include <Core/DeIOV.h>
#include <Core/DetectorDataService.h>
#include <Core/Keys.h>
#include <Core/MagneticFieldExtension.h>
#include <Core/MagneticFieldGridReader.h>
#include <Core/Utils.h>
#include <DD4hep/Detector.h>
#include <DD4hep/Factories.h>
#include <DD4hep/Printout.h>
#include <Detector/Muon/DeMuon.h>
#include <Detector/Muon/DeMuonChamber.h>
#include <Detector/Muon/DeMuonGasGap.h>
#include <Detector/Muon/DeMuonRegion.h>
#include <Detector/Muon/DeMuonSide.h>
#include <Detector/Muon/DeMuonStation.h>
#include <Detector/Muon/MuonChamberGrid.h>
#include <Detector/Muon/MuonConstants.h>
#include <Detector/Muon/PackMCHit.h>
#include <Detector/Muon/ReadoutCond.h>
#include <Detector/Muon/TileID.h>
#include <Detector/Muon/Types.h>
#include <array>
#include <deque>
#include <exception>
#include <iostream>
#include <nlohmann/json.hpp>
#include <numeric>
#include <range/v3/view/enumerate.hpp>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

using LHCb::Detector::Muon::ReadoutCond;

using DeMuon = LHCb::Detector::detail::DeMuonObject; // non e' bello fa casino con  " using DeMuon =
                                                     // DeMuonElement<detail::DeMuonObject>; "
//
using FrontEndID      = LHCb::Detector::Muon::FrontEndID;
using MuonChamberGrid = LHCb::Detector::Muon::detail::MuonChamberGrid;
using TileID          = LHCb::Detector::Muon::TileID;
using Layout          = LHCb::Detector::Muon::Layout;
using Chamber         = LHCb::Detector::DeMuonChamber;
//
//
// stationNumber -> station number from 2 to 5,
// regionNumber  -> region  number from 1 to 4,
// chamberNumber -> chamber number from 1 to 192 (max number of chambers in R4)
// ==>> these are needed to read the element detector tags in dd4hep.
//      to be consistent with the Boole and Rec code we will avoid to use this
//      numbering as much as possible in favour of the indexes:
// iSta -> station index from 0 to 3,
// iReg -> region  index from 0 to 3,
// iCha -> chamber index from 0 to 191 <==> be careful !! don't confuse with the index icha of the vector of chamber
// detector elements, given by getChamberVectorIndex
//
//
DeMuon::DeMuonObject( dd4hep::DetElement const& de, dd4hep::cond::ConditionUpdateContext& ctxt )
    // 11009 == CLID_DEMuonDetector
    : DeIOVObject( de, ctxt, 11009 )
    , m_stations{{{de.child( "M2Station" ), ctxt, 2},
                  {de.child( "M3Station" ), ctxt, 3},
                  {de.child( "M4Station" ), ctxt, 4},
                  {de.child( "M5Station" ), ctxt, 5}}}
    , m_daq( de, ctxt ) {

  using KeyMaker = dd4hep::ConditionKey::KeyMaker;

  for ( const auto& item : ctxt.get<nlohmann::json>( KeyMaker{de, "ChambersGrids"}.hash ).items() ) {
    m_chambersGrids.emplace( item.key(), item.value() );
  }

  for ( auto& chGrid : m_chambersGrids ) {
    dd4hep::printout( dd4hep::DEBUG, "DeMuonObject", "%s", chGrid.first.c_str() );
  }

  // Fill arrays of maps to help in chamber location
  FillChamberIndexMap();
  //
  // this part below is essentially FillGeoInfo
  FillGeoInfo();
  //
  m_readoutModules = ctxt.condition( hash_key( de, "ReadoutModules" ), true )
                         .get<nlohmann::json>()
                         .get<std::map<std::string, ReadoutCond>>();
}

LHCb::Detector::detail::DeMuonStationObject::DeMuonStationObject( const dd4hep::DetElement&             de,
                                                                  dd4hep::cond::ConditionUpdateContext& ctxt,
                                                                  unsigned int                          iStation )
    : DeIOVObject( de, ctxt, 11009 )
    , m_stationID{iStation}
    , m_sides{{{de.child( "M" + std::to_string( m_stationID ) + "ASide" ), ctxt, 0, m_stationID},
               {de.child( "M" + std::to_string( m_stationID ) + "CSide" ), ctxt, 1, m_stationID}}} {
  // Get the global z position of the station

  dd4hep::printout( dd4hep::DEBUG, "DeMuonStationObject", "DetElement Path %s", de.path().c_str() );

  ROOT::Math::XYZPoint globalPoint = this->toGlobal( ROOT::Math::XYZPoint( 0., 0., 0. ) );
  m_Xstation                       = globalPoint.x();
  m_Ystation                       = globalPoint.y();
  m_Zstation                       = globalPoint.z();

  dd4hep::printout( dd4hep::DEBUG, "DeMuonStationObject", "Station %i %i %f", iStation, m_stationID, m_Zstation );
}

LHCb::Detector::detail::DeMuonSideObject::DeMuonSideObject( const dd4hep::DetElement&             de,
                                                            dd4hep::cond::ConditionUpdateContext& ctxt,
                                                            unsigned int iSide, unsigned int stationID )
    : DeIOVObject( de, ctxt, 11009 )
    , m_sideID{iSide}
    , m_regions{{{de.child( "M" + std::to_string( stationID ) + "R1" + m_Side[iSide] + "Side" ), ctxt, 1, m_sideID,
                  stationID},
                 {de.child( "M" + std::to_string( stationID ) + "R2" + m_Side[iSide] + "Side" ), ctxt, 2, m_sideID,
                  stationID},
                 {de.child( "M" + std::to_string( stationID ) + "R3" + m_Side[iSide] + "Side" ), ctxt, 3, m_sideID,
                  stationID},
                 {de.child( "M" + std::to_string( stationID ) + "R4" + m_Side[iSide] + "Side" ), ctxt, 4, m_sideID,
                  stationID}}} {

  dd4hep::printout( dd4hep::DEBUG, "DeMuonSideObject", "DetElement Path %s", de.path().c_str() );
  dd4hep::printout( dd4hep::DEBUG, "DeMuonSideObject", "Station %i side %s %i", stationID, m_Side[iSide].c_str(),
                    iSide );
  //

  ROOT::Math::XYZPoint globalPoint = this->toGlobal( ROOT::Math::XYZPoint( 0., 0., 0. ) );
  m_Xside                          = globalPoint.x();
  m_Yside                          = globalPoint.y();
  m_Zside                          = globalPoint.z();
}

LHCb::Detector::detail::DeMuonRegionObject::DeMuonRegionObject( const dd4hep::DetElement&             de,
                                                                dd4hep::cond::ConditionUpdateContext& ctxt,
                                                                unsigned int iRegion, unsigned int sideID,
                                                                unsigned int stationID )
    : DeIOVObject( de, ctxt, 11009 ), m_regionID{iRegion}, m_sideID{sideID}, m_stationID{stationID} {

  dd4hep::printout( dd4hep::DEBUG, "DeMuonRegionObject", "Station %i, side %i, region %i", stationID, sideID, iRegion );
  dd4hep::printout( dd4hep::DEBUG, "DeMuonRegionObject", "DetElement Path %s", de.path().c_str() );

  ROOT::Math::XYZPoint globalPoint = this->toGlobal( ROOT::Math::XYZPoint( 0., 0., 0. ) );
  m_Xreg                           = globalPoint.x();
  m_Yreg                           = globalPoint.y();
  m_Zreg                           = globalPoint.z();

  auto chambersList = de.children();

  for ( const auto& [chamber_key, chamber_value] : chambersList ) {

    dd4hep::printout( dd4hep::DEBUG, "DeMuonRegionObject", "%s %s", chamber_value.placementPath().c_str(),
                      chamber_key.c_str() );
    //
    dd4hep::DetElement const chamber_det = de.child( chamber_key );
    m_chambers.emplace_back( chamber_det, ctxt );
    //
  }
}
//
// **************************************************************************************
//
// methods to get the DeMuonChamber Detector Element and/or the related chamber infos
//
// **************************************************************************************
//
std::optional<DeMuon::ChamberInfo> DeMuon::GetChamberInfoFromNum( int ista, int ireg,
                                                                  unsigned int chamberNumber ) const {

  const auto detCh = getChamber( ista, ireg, chamberNumber );
  //  unsigned int ChNum       = detCh.chamberNumber();
  double xCh         = detCh.XPos();
  double yCh         = detCh.YPos();
  double zCh         = detCh.ZPos();
  auto   detFirstgap = detCh.getGap( 0 );
  double Dx          = ( detFirstgap.DX() );
  double Dy          = ( detFirstgap.DY() );
  double Dz          = ( detFirstgap.DZ() );
  //
  return ChamberInfo{detCh, detFirstgap, chamberNumber, xCh, yCh, zCh, Dx, Dy, Dz};
}
//
Chamber DeMuon::getChamber( unsigned int ista, unsigned int ireg, unsigned int chamberNumber ) const {

  const auto& map = m_chamberNumber_Index[ista][ireg];
  auto        it  = map.find( chamberNumber );
  if ( it == map.end() ) {
    dd4hep::printout( dd4hep::DEBUG, "DeMuon",
                      "GetChamber ERROR: the combination station/region/chamber does not exists" );
    return nullptr;
  }
  return &m_stations[ista].m_sides[it->second.side].m_regions[ireg].m_chambers[it->second.index];
}

//
// Get index of the chamber in the m_chambers vector given a chamber tile
int DeMuon::getChamberIdxFromTile( const TileID& tile ) const {

  auto ireg = tile.region();
  auto iqua = tile.quarter();
  auto nx   = tile.nX();
  auto ny   = tile.nY();

  int index = getChamberVectorIndex( nx, ny, ireg, iqua );
  return index;
}
//
int DeMuon::getChamberVectorIndex( int nx, int ny, int ireg, int iqua ) const {

  unsigned int gx = LHCb::Detector::Muon::grid[ireg].first;
  unsigned int gy = LHCb::Detector::Muon::grid[ireg].second;

  int fx = 0;
  int fy = 0;

  if ( iqua == 0 ) {
    fx = gx * 2 - 1 - nx;
    fy = gy * 2 - 1 - ny;
  } else if ( iqua == 1 ) {
    fx = gx * 2 - 1 - nx;
    fy = ny;
  } else if ( iqua == 2 ) {
    fx = nx;
    fy = ny;
  } else if ( iqua == 3 ) {
    fx = nx;
    fy = gy * 2 - 1 - ny;
  }
  //
  dd4hep::printout( dd4hep::DEBUG, "DeMuon",
                    "getChamberVectorIndex DEBUG: nx: %i, ny: %i, gx: %i, gy: %i, fx: %i, fy: %i ", nx, ny, gx, gy, fx,
                    fy );
  //
  // decide which side of the muon station we are
  int side         = iqua / 2;
  int lower_side   = 3 * side + pow( -1, side ) * iqua;
  int sub_quadrant = ( nx / gx ) + ( ny / gy ) * 2;

  dd4hep::printout( dd4hep::DEBUG, "DeMuon",
                    "getChamberVectorIndex DEBUG: iqua: %i, side: %i, lower_side: %i, sub_quadrant: %i", iqua, side,
                    lower_side, sub_quadrant );

  if ( ( sub_quadrant == 0 ) || ( sub_quadrant > 3 ) ) return -1;

  int icha = fx + fy * 2 * gx;

  int sq_factor = 0;
  // if subquadrant is aside the hole iCha has to be raised without taking into account the hole
  if ( sub_quadrant == 1 ) {
    sq_factor = ( fy % gy ) * gx + gx * side;
  } else if ( lower_side == 1 ) {
    sq_factor = ( ( fy / gy ) + ( gy - 1 ) ) * gx;
  }
  // if lower side the upper side chambers have to be taken into account
  int offset = lower_side * LHCb::Detector::Muon::nChambers[ireg] / 4;
  //
  icha = icha - sq_factor + offset;
  dd4hep::printout( dd4hep::DEBUG, "DeMuon",
                    "getChamberVectorIndex DEBUG: ireg: %i, iqua: %i, side: %i, sub_quad: %i, lside: %i, sq_f: %i, "
                    "offs: %i, icha: %i",
                    ireg, iqua, side, sub_quadrant, lower_side, sq_factor, offset, icha );
  return icha;
}
//
int DeMuon::getChamberNumber( const TileID& tile ) const {

  int index = getChamberIdxFromTile( tile );
  if ( index < 0 ) return -1;
  auto         ista          = tile.station();
  auto         ireg          = tile.region();
  auto         iqua          = tile.quarter();
  unsigned int isid          = iqua / 2;
  auto         chamberNumber = m_stations[ista].m_sides[isid].m_regions[ireg].m_chambers[index].m_ChamberNumber;
  return chamberNumber;
}
//
Chamber DeMuon::getChamberFromTile( const TileID& tile ) const {

  int index = getChamberIdxFromTile( tile );
  if ( index < 0 ) return nullptr;
  auto         ista = tile.station();
  auto         ireg = tile.region();
  auto         iqua = tile.quarter();
  unsigned int isid = iqua / 2;

  return Chamber( &m_stations[ista].m_sides[isid].m_regions[ireg].m_chambers[index] );
}
//
// **************************************************************************************
//
//
std::vector<std::pair<FrontEndID, std::array<float, 4>>>
DeMuon::listOfPhysChannels( ROOT::Math::XYZPoint my_entry, ROOT::Math::XYZPoint my_exit, int ireg, int icha ) const {
  //
  // this icha is not the index in the chamber vector but chamberNumber - Muon::firstChamber !
  //
  unsigned int chamberNumberIn( 0 );
  int          iregIn( -1 );
  int          ichaIn( -1 );
  int          istaIn = getStationFromZ( my_entry.z() ) - Muon::firstStation;
  //
  unsigned int chamberNumberOut( 0 );
  int          iregOut( -1 );
  int          istaOut = getStationFromZ( my_exit.z() ) - Muon::firstStation;

  if ( ( ireg == -1 ) && ( icha == -1 ) ) {
    // Hit entry
    Hit2ChamberNumber( my_entry, istaIn, chamberNumberIn, iregIn );
    dd4hep::printout( dd4hep::DEBUG, "DeMuon",
                      "In Station N :: %i, Region N :: %i, Chamber N :: %i, Entry pos :: (%f, %f, %f)", istaIn, iregIn,
                      chamberNumberIn, my_entry.x(), my_entry.y(), my_entry.z() );
    ichaIn = chamberNumberIn - Muon::firstChamber;
    if ( chamberNumberIn == 0 || iregIn == -1 ) {
      dd4hep::printout( dd4hep::INFO, "DeMuon", "listOfPhysChannels INFO: no chamber found for entry point" );
      return {};
    }

    // Hit exit
    Hit2ChamberNumber( my_exit, istaOut, chamberNumberOut, iregOut );
    dd4hep::printout( dd4hep::DEBUG, "DeMuon",
                      "Out Station N :: %i, Region N :: %i, Chamber N :: %i, Entry pos :: (%f, %f, %f)", istaOut,
                      iregOut, chamberNumberOut, my_exit.x(), my_exit.y(), my_exit.z() );

    if ( chamberNumberOut == 0 || iregOut == -1 ) {
      dd4hep::printout( dd4hep::INFO, "DeMuon", "listOfPhysChannels INFO: no chamber found for exit point" );
      return {};
    }

    if ( chamberNumberIn != chamberNumberOut || iregIn != iregOut ) {
      dd4hep::printout(
          dd4hep::ERROR, "DeMuon",
          "listOfPhysChannels ERROR: Hit entry and exit are in different chambers! Returning an empty list" );
      return {};
    }

  } else {
    // If given.. assign region and chamber number
    iregIn = ireg;
    ichaIn = icha;
  }
  //
  //
  // Getting the chamber infos: chamber and first gap detector elements and box half-dimensions (dx, dy)
  auto ChamberInfo            = GetChamberInfoFromNum( istaIn, iregIn, ichaIn + Muon::firstChamber );
  using DeMuonChamber         = LHCb::Detector::detail::DeMuonChamberObject;
  const DeMuonChamber& det_ch = ChamberInfo->detCh;
  double               dx     = ChamberInfo->Dx;
  double               dy     = ChamberInfo->Dy;
  //
  ROOT::Math::XYZPoint GlobalPoint =
      ROOT::Math::XYZPoint( ( ( my_entry.x() + my_exit.x() ) / 2 ), ( ( my_entry.y() + my_exit.y() ) / 2 ),
                            ( ( my_entry.z() + my_exit.z() ) / 2 ) );
  ROOT::Math::XYZPoint entryInCh = ROOT::Math::XYZPoint( my_entry.x(), my_entry.y(), my_entry.z() );
  ROOT::Math::XYZPoint exitInCh  = ROOT::Math::XYZPoint( my_exit.x(), my_exit.y(), my_exit.z() );
  //
  // pointInGasGap calls LHCb::Detector::detail::DeIOVObject::isInside( const ROOT::Math::XYZPoint& globalPoint )
  // isInside transforms GlobalPoint from Global to the Local GasGap element
  auto pointInGasGap = det_ch.checkPointInGasGap( GlobalPoint );

  if ( !pointInGasGap ) {
    dd4hep::printout( dd4hep::DEBUG, "DeMuon",
                      "listOfPhysChannels DEBUG: Could not find the gap. Returning an empty list." );
    return {};
  }
  // gap detector element from pointInGasGap struct
  const auto& gapLayer = pointInGasGap->gasGap;
  //
  // calculate the edges of the gap detector element respect to Global system
  ROOT::Math::XYZPoint lowerleft  = gapLayer.toGlobal( ROOT::Math::XYZPoint{-dx, -dy, 0} );
  ROOT::Math::XYZPoint lowerright = gapLayer.toGlobal( ROOT::Math::XYZPoint{dx, -dy, 0} );
  ROOT::Math::XYZPoint upperright = gapLayer.toGlobal( ROOT::Math::XYZPoint{dx, dy, 0} );
  //
  // Refer the distances to Local system [ the gap ]
  ROOT::Math::XYZPoint new_entry = gapLayer.toLocal( entryInCh );
  ROOT::Math::XYZPoint new_exit  = gapLayer.toLocal( exitInCh );
  //
  // Define relative dimensions
  double mod_xen( 0 ), mod_yen( 0 ), mod_xex( 0 ), mod_yex( 0 );
  //
  // edge values are needed to determine the orientation of the chamber,
  // then the the relative positions of entry and exit points can be evaluated:
  if ( abs( dx ) > std::numeric_limits<double>::epsilon() && abs( dy ) > std::numeric_limits<double>::epsilon() ) {
    if ( ( lowerleft.x() < lowerright.x() ) && ( lowerleft.y() < upperright.y() ) ) {

      mod_xen = ( new_entry.x() + dx ) / ( 2 * dx );
      mod_yen = ( new_entry.y() + dy ) / ( 2 * dy );
      mod_xex = ( new_exit.x() + dx ) / ( 2 * dx );
      mod_yex = ( new_exit.y() + dy ) / ( 2 * dy );
    } else if ( lowerleft.x() < lowerright.x() && ( lowerleft.y() > upperright.y() ) ) {

      mod_xen = ( new_entry.x() + dx ) / ( 2 * dx );
      mod_yen = ( -new_entry.y() + dy ) / ( 2 * dy );
      mod_xex = ( new_exit.x() + dx ) / ( 2 * dx );
      mod_yex = ( -new_exit.y() + dy ) / ( 2 * dy );
    } else if ( lowerleft.x() > lowerright.x() && ( lowerleft.y() > upperright.y() ) ) {

      mod_xen = ( -new_entry.x() + dx ) / ( 2 * dx );
      mod_yen = ( -new_entry.y() + dy ) / ( 2 * dy );
      mod_xex = ( -new_exit.x() + dx ) / ( 2 * dx );
      mod_yex = ( -new_exit.y() + dy ) / ( 2 * dy );
      dd4hep::printout( dd4hep::INFO, "DeMuon", "listOfPhysChannels INFO: Should never enter here " );
    } else if ( lowerleft.x() > lowerright.x() && ( lowerleft.y() < upperright.y() ) ) {

      mod_xen = ( -new_entry.x() + dx ) / ( 2 * dx );
      mod_yen = ( new_entry.y() + dy ) / ( 2 * dy );
      mod_xex = ( -new_exit.x() + dx ) / ( 2 * dx );
      mod_yex = ( new_exit.y() + dy ) / ( 2 * dy );
      dd4hep::printout( dd4hep::INFO, "DeMuon", "listOfPhysChannels INFO: Should never enter here " );
    }
  } else {
    dd4hep::printout( dd4hep::ERROR, "DeMuon",
                      "listOfPhysChannels ERROR: Null chamber dimensions. Returning an empty list." );
    return {};
  }
  //
  // Gets list of channels and convert relative distances into absolute ones
  std::string tag  = gridTag( istaIn + Muon::firstStation, iregIn + Muon::firstRegion );
  auto        grid = m_chambersGrids.find( tag );
  if ( grid == m_chambersGrids.end() ) {
    dd4hep::printout( dd4hep::ERROR, "DeMuon", "listOfPhysChannels ERROR: chamber grid %s not found ", tag.c_str() );
    return {};
  }
  auto myPair = grid->second.listOfPhysChannels( mod_xen, mod_yen, mod_xex, mod_yex );
  //
  int gapIndex = ( pointInGasGap->number - 1 );
  for ( auto& [myFE, myVec] : myPair ) {
    assert( myVec.size() == 4 );
    myFE.setLayer( gapIndex / 2 );
    for ( auto&& [i, my] : ranges::views::enumerate( myVec ) ) {
      my *= 2 * static_cast<float>( i % 2 ? dy : dx );
      // Added resolution effect
      if ( std::abs( my ) < 0.0001 ) my = 0;
    }
  };
  //
  return myPair;
}
//
//
void DeMuon::Hit2ChamberNumber( ROOT::Math::XYZPoint myPoint, int ista, unsigned int& chamberNumber, int& ireg ) const {
  //
  float x = myPoint.x();
  float y = myPoint.y();
  float z = myPoint.z();
  //
  int  icha         = -1;
  int  isid         = 0;
  bool chamberFound = false;
  //
  // given the hit x,y point returns the index of the m_chambers vectors filled in DeMuonRegionObject:
  // chamberMostLikely ==> looks at the x,y space of the station# as a perfect grid, where each cell has the
  // dimensions of the chambers in M#R1
  chamberMostLikely( x, y, ista, isid, ireg, icha );
  if ( icha == -1 ) {

    // if icha == -1 ==> (x, y) is outside of the "perfect grid", so it looks to be outside of the station# area
    dd4hep::printout( dd4hep::DEBUG, "DeMuon", "Hit2ChamberNumber DEBUG: x,y point returns icha = -1 " );
    int jreg = ireg;
    int jsid = -1;
    //
    // chambersOut ==> looks for the hitted chamber close to the inner or outer edges of the station# area and
    // fill a vector, chaInfos, with the selected chambers
    std::vector<unsigned int> chaInfos = chambersOut( ista, jsid, jreg, x, y );
    // loopOnChambers ==> loops on the chaInfos vector to select the chamber that contains the hit (x, y)
    chamberFound                       = loopOnChambers( myPoint, chaInfos, ista, chamberNumber, jreg );
    if ( chamberFound ) {
      ireg = jreg;
      return;
    } else {
      chamberNumber = 0;
      ireg          = -1;
      return;
    }
  }
  //
  unsigned int regionNumber  = ireg + Muon::firstRegion;
  unsigned int stationNumber = ista + Muon::firstStation;
  chamberNumber              = m_stations[ista].m_sides[isid].m_regions[ireg].m_chambers[icha].m_ChamberNumber;

  dd4hep::printout( dd4hep::DEBUG, "DeMuon", "Hit2ChamberNumber DEBUG: Chamber Number %i, region %i, station %i",
                    chamberNumber, regionNumber, stationNumber );

  if ( regionNumber != 0 && chamberNumber != 0 ) {

    const auto& det_ch = m_stations[ista].m_sides[isid].m_regions[ireg].m_chambers[icha];
    dd4hep::printout( dd4hep::DEBUG, "DeMuon",
                      "Hit2ChamberNumber DEBUG: from det_ch Chamber Number %i, region %i, station %i",
                      det_ch.m_ChamberNumber, det_ch.m_RegionNumber, det_ch.m_StationNumber );
    //
    unsigned int         FirstGap = 0;
    auto&                det_gap  = det_ch.m_gasGaps[FirstGap];
    ROOT::Math::XYZPoint GapPos   = det_gap.getPosition();
    float                DxGap    = m_sensitiveAreaX[ista * 4 + ireg] / 2.;
    float                DyGap    = m_sensitiveAreaY[ista * 4 + ireg] / 2.;
    float                DzGap    = m_sensitiveAreaZ[ista * 4 + ireg] / 2.;

    dd4hep::printout( dd4hep::DEBUG, "DeMuon",
                      "Hit2ChamberNumber DEBUG: from det_gap gap refx %f, refy %f, refz0 %f, Dx %f, Dy %f, Dz %f",
                      GapPos.x(), GapPos.y(), GapPos.z(), DxGap, DyGap, DzGap );
    //
    // For calls not providing z hit take the z of the first gap layer
    if ( !z ) {
      double zGap = GapPos.z();
      myPoint.SetZ( zGap );
    }
    //
    const auto& InGasGap = det_ch.checkPointInGasGap( myPoint );

    if ( !InGasGap ) {

      // if the chamber selected by chamberMostLikely "likely chamber" does not contain the hit (x, y) ==>

      float xref = GapPos.x();
      float yref = GapPos.y();
      //
      int jreg = ireg;
      int jsid = isid;
      //
      // chambersAround ==> looks around the found "likely chamber" and fill a vector, chaInfos, with the
      // new selected chambers
      std::vector<unsigned int> chaInfos = chambersAround( ista, jsid, jreg, xref, yref, x, y );
      //
      // loopOnChambers ==> loops on the chaInfos vector to select the chamber that contains the hit (x, y)
      chamberFound = loopOnChambers( myPoint, chaInfos, ista, chamberNumber, ireg );
    } // if ( !InGasGap )

    else {
      chamberFound = true;
    }
  }
  //
  if ( !chamberFound ) {
    dd4hep::printout( dd4hep::INFO, "DeMuon", "Hit2ChamberNumber INFO: no chambers found" );
    chamberNumber = 0;
    ireg          = -1;
  }
  return;
}
//
//
bool DeMuon::loopOnChambers( ROOT::Math::XYZPoint myPoint, std::vector<unsigned int> chaInfos, int ista,
                             unsigned int& chamberNumber, int& jreg ) const {

  float z = myPoint.z();

  int jcha = -1;
  int jsid = -1;

  bool chamberFound = false;

  int dim = chaInfos.size();
  dd4hep::printout( dd4hep::DEBUG, "DeMuon", "loopOnChambers DEBUG: dimension chamber vector, dim %i", dim );

  if ( dim % 3 == 0 ) {

    int nCha = dim / 3;
    int jj   = 0;

    for ( int ii = 0; ii < nCha; ii++ ) {

      jsid                       = chaInfos.at( jj );
      jreg                       = chaInfos.at( jj + 1 );
      jcha                       = chaInfos.at( jj + 2 );
      unsigned int regionNumber  = jreg + Muon::firstRegion;
      unsigned int stationNumber = ista + Muon::firstStation;
      chamberNumber              = m_stations[ista].m_sides[jsid].m_regions[jreg].m_chambers[jcha].m_ChamberNumber;

      dd4hep::printout( dd4hep::DEBUG, "DeMuon", "loopOnChambers DEBUG: NEW Chamber Number %i, region %i, station %i",
                        chamberNumber, regionNumber, stationNumber );

      if ( regionNumber != 0 && chamberNumber != 0 ) {

        const auto& new_det_ch = m_stations[ista].m_sides[jsid].m_regions[jreg].m_chambers[jcha];
        dd4hep::printout( dd4hep::DEBUG, "DeMuon",
                          "loopOnChambers DEBUG: from NEW det_ch Chamber Number %i, region %i, station %i",
                          new_det_ch.m_ChamberNumber, new_det_ch.m_RegionNumber, new_det_ch.m_StationNumber );
        //
        auto&                new_det_gap = new_det_ch.m_gasGaps[0];
        ROOT::Math::XYZPoint new_GapPos  = new_det_gap.getPosition();
        // For calls not providing z hit take the z of the first gap layer
        if ( !z ) {
          double new_zGap = new_GapPos.z();
          myPoint.SetZ( new_zGap );
        }
        //
        const auto& InNewGasGap = new_det_ch.checkPointInGasGap( myPoint );
        if ( InNewGasGap ) {
          chamberFound = true;
          dd4hep::printout( dd4hep::DEBUG, "DeMuon",
                            "loopOnChambers DEBUG: New selected chamber contains the hit, ii %i, jj %i", ii, jj );
          break;
        } else if ( !InNewGasGap ) {
          dd4hep::printout( dd4hep::DEBUG, "DeMuon",
                            "loopOnChambers DEBUG: New selected chamber does not contain the hit, ii %i, jj %i", ii,
                            jj );
        }
      }
      jj = jj + 3;
    } // loop on found chambers

  } else {
    dd4hep::printout( dd4hep::DEBUG, "DeMuon", "loopOnChambers DEBUG: new search around x,y point fails, dim %i", dim );
  } // check dim
  //
  if ( !chamberFound ) {
    dd4hep::printout( dd4hep::DEBUG, "DeMuon", "loopOnChambers DEBUG: no chambers found" );
    chamberNumber = 0;
    jreg          = -1;
  }

  return chamberFound;
}
//
//
std::vector<unsigned int> DeMuon::chambersAround( int ista, int jsid, int jreg, float xref, float yref, float x,
                                                  float y ) const {

  std::vector<unsigned int> chambersInfo;

  int   jcha( -1 );
  int   x_sgn( 0 ), y_sgn( 0 );
  int   xref_sgn( 0 ), yref_sgn( 0 );
  float x_foxHit( 0.0 ), y_foxHit( 0.0 );
  float x_foxHit2( 0.0 ), x_foxB( 0.0 );
  //
  int in_sid = jsid;
  int in_reg = jreg;
  //
  float xstep = m_sensitiveAreaX[ista * 4] / 2.; // chamber sensitive area in R1
  float ystep = m_sensitiveAreaY[ista * 4] / 2.;
  //
  x_sgn    = (int)( ( abs( x ) - abs( xref ) ) / abs( abs( x ) - abs( xref ) ) );
  y_sgn    = (int)( ( abs( y ) - abs( yref ) ) / abs( abs( y ) - abs( yref ) ) );
  xref_sgn = (int)( xref / abs( xref ) );
  yref_sgn = (int)( yref / abs( yref ) );
  //
  dd4hep::printout( dd4hep::DEBUG, "DeMuon",
                    "chambersAround DEBUG: hit not found, try to look around x_ref %f, x_hit %f, xref_sgn %i, x_sgn "
                    "%i, y_ref %f, y_hit %f, yref_sgn %i, y_sgn %i",
                    xref, x, xref_sgn, x_sgn, yref, y, yref_sgn, y_sgn );
  //
  float tolerance = 20.; // 2 cm
  // look if the hit_x is close to the center of the selected chamber
  bool close_CHcc_X = ( abs( abs( x ) - abs( xref ) ) < tolerance );
  // look if the hit_x is very close to one of the borders of the selected chamber
  bool close_CHb_X = false;
  if ( x_sgn > 0 ) {
    close_CHb_X = ( abs( abs( x ) - abs( xref + xstep * xref_sgn ) ) < tolerance / 2. );
    x_foxB      = x - ( xstep * 0.5 ) * xref_sgn;
  } else if ( x_sgn < 0 ) {
    close_CHb_X = ( abs( abs( x ) - abs( xref - xstep * xref_sgn ) ) < tolerance / 2. );
    x_foxB      = x + ( xstep * 0.5 ) * xref_sgn;
  }
  //
  if ( x_sgn > 0 ) {
    x_foxHit  = x + xstep * xref_sgn;
    x_foxHit2 = x - xstep * xref_sgn;
  } else if ( x_sgn < 0 ) {
    x_foxHit  = x - xstep * xref_sgn;
    x_foxHit2 = x + xstep * xref_sgn;
  }
  //
  // given the the new hit x,y point returns the index of the m_chambers vectors filled in DeMuonRegionObject
  dd4hep::printout( dd4hep::DEBUG, "DeMuon", "chambersAround DEBUG: hit not found, test x_foxHit %f, y %f ", x_foxHit,
                    y );
  chamberMostLikely( x_foxHit, y, ista, jsid, jreg, jcha );
  if ( jcha != -1 ) {
    chambersInfo.push_back( jsid );
    chambersInfo.push_back( jreg );
    chambersInfo.push_back( jcha );
  }
  //
  jcha = -1;
  jsid = in_sid;
  jreg = in_reg;
  //
  if ( y_sgn > 0 ) {
    y_foxHit = y + ystep * yref_sgn;
  } else if ( y_sgn < 0 ) {
    y_foxHit = y - ystep * yref_sgn;
  }
  //
  // given the the new hit x,y point returns the index of the m_chambers vectors filled in DeMuonRegionObject
  dd4hep::printout( dd4hep::DEBUG, "DeMuon", "chambersAround DEBUG: hit not found, test x %f, y_foxHit %f ", x,
                    y_foxHit );
  chamberMostLikely( x, y_foxHit, ista, jsid, jreg, jcha );
  if ( jcha != -1 ) {
    chambersInfo.push_back( jsid );
    chambersInfo.push_back( jreg );
    chambersInfo.push_back( jcha );
  }
  //
  jcha = -1;
  jsid = in_sid;
  jreg = in_reg;
  //
  // given the the new hit x,y point returns the index of the m_chambers vectors filled in DeMuonRegionObject
  dd4hep::printout( dd4hep::DEBUG, "DeMuon", "chambersAround DEBUG: hit not found, test x_foxHit %f, y_foxHit %f ",
                    x_foxHit, y_foxHit );
  chamberMostLikely( x_foxHit, y_foxHit, ista, jsid, jreg, jcha );
  if ( jcha != -1 ) {
    chambersInfo.push_back( jsid );
    chambersInfo.push_back( jreg );
    chambersInfo.push_back( jcha );
  }
  //
  if ( close_CHb_X ) {
    jcha = -1;
    jsid = in_sid;
    jreg = in_reg;
    //
    // given the the new hit x,y point returns the index of the m_chambers vectors filled in DeMuonRegionObject
    dd4hep::printout( dd4hep::DEBUG, "DeMuon", "chambersAround DEBUG: hit not found, test x_foxB %f, y_foxHit %f ",
                      x_foxB, y_foxHit );
    chamberMostLikely( x_foxB, y_foxHit, ista, jsid, jreg, jcha );
    if ( jcha != -1 ) {
      chambersInfo.push_back( jsid );
      chambersInfo.push_back( jreg );
      chambersInfo.push_back( jcha );
    }
  }
  //
  if ( close_CHcc_X ) {
    jcha = -1;
    jsid = in_sid;
    jreg = in_reg;
    //
    // given the the new hit x,y point returns the index of the m_chambers vectors filled in DeMuonRegionObject
    dd4hep::printout( dd4hep::DEBUG, "DeMuon", "chambersAround DEBUG: hit not found, test x_foxHit2 %f, y_foxHit %f ",
                      x_foxHit2, y_foxHit );
    chamberMostLikely( x_foxHit2, y_foxHit, ista, jsid, jreg, jcha );
    if ( jcha != -1 ) {
      chambersInfo.push_back( jsid );
      chambersInfo.push_back( jreg );
      chambersInfo.push_back( jcha );
    }
  }
  //
  return chambersInfo;
}
//
//
std::vector<unsigned int> DeMuon::chambersOut( int ista, int jsid, int jreg, float x, float y ) const {

  std::vector<unsigned int> chambersInfo;
  //
  int jcha( -1 );
  int x_sgn = (int)( x / abs( x ) );
  int y_sgn = (int)( y / abs( y ) );
  //
  int in_sid = jsid;
  int in_reg = jreg;
  //
  float x_foxHit  = x;
  float y_foxHit  = y;
  float y_foxHit2 = y;
  //
  float minY = m_stationBox[ista].innerY;
  //
  float xstep = m_sensitiveAreaX[ista * 4] / 2.; // chamber sensitive half area in R1
  float ystep = m_sensitiveAreaY[ista * 4] / 2.;
  //
  auto nx = static_cast<unsigned>( std::abs( x / ( xstep * 2. ) ) );
  auto ny = static_cast<unsigned>( std::abs( y / ( ystep * 2. ) ) );
  //
  float deltaInnerY = abs( y ) - minY;
  //
  bool InnerRegion = ( ( nx == 0 ) && ( ny == 0 ) );
  bool OuterRegion = ( ( deltaInnerY > ystep * 2 ) || ( nx > 12 ) );
  //
  dd4hep::printout( dd4hep::DEBUG, "DeMuon",
                    "chambersOut DEBUG: hit not found, try to look around, x_hit %f, x_sgn %i, y_hit %f, y_sgn %i", x,
                    x_sgn, y, y_sgn );
  dd4hep::printout(
      dd4hep::DEBUG, "DeMuon",
      "chambersOut DEBUG: hit not found, try to look around, minY %f, deltaInnerY %f, xstep %f, ystep %f, nx %i, ny %i",
      minY, deltaInnerY, xstep, ystep, nx, ny );

  if ( InnerRegion ) {
    x_foxHit  = x + xstep * x_sgn;
    y_foxHit  = y + ystep * y_sgn;
    y_foxHit2 = y - ystep * y_sgn;
  } else if ( OuterRegion ) {
    x_foxHit = x - xstep * x_sgn;
    y_foxHit = y - ystep * y_sgn;
  }
  //
  // given the the new hit x,y point returns the index of the m_chambers vectors filled in DeMuonRegionObject
  dd4hep::printout( dd4hep::DEBUG, "DeMuon", "chambersOut DEBUG: hit not found, test x_foxHit %f, y %f", x_foxHit, y );
  chamberMostLikely( x_foxHit, y, ista, jsid, jreg, jcha );
  if ( jcha != -1 ) {
    chambersInfo.push_back( jsid );
    chambersInfo.push_back( jreg );
    chambersInfo.push_back( jcha );
  }
  //
  jcha = -1;
  jsid = in_sid;
  jreg = in_reg;
  //
  // given the the new hit x,y point returns the index of the m_chambers vectors filled in DeMuonRegionObject
  dd4hep::printout( dd4hep::DEBUG, "DeMuon", "chambersOut DEBUG: hit not found, test x %f, y_foxHit %f", x, y_foxHit );
  chamberMostLikely( x, y_foxHit, ista, jsid, jreg, jcha );
  if ( jcha != -1 ) {
    chambersInfo.push_back( jsid );
    chambersInfo.push_back( jreg );
    chambersInfo.push_back( jcha );
  }
  //
  jcha = -1;
  jsid = in_sid;
  jreg = in_reg;
  //
  // given the the new hit x,y point returns the index of the m_chambers vectors filled in DeMuonRegionObject
  dd4hep::printout( dd4hep::DEBUG, "DeMuon", "chambersOut DEBUG: hit not found, test x_foxHit %f, y_foxHit %f",
                    x_foxHit, y_foxHit );
  chamberMostLikely( x_foxHit, y_foxHit, ista, jsid, jreg, jcha );
  if ( jcha != -1 ) {
    chambersInfo.push_back( jsid );
    chambersInfo.push_back( jreg );
    chambersInfo.push_back( jcha );
  }
  //
  if ( InnerRegion ) {
    jcha = -1;
    jsid = in_sid;
    jreg = in_reg;
    //
    // given the the new hit x,y point returns the index of the m_chambers vectors filled in DeMuonRegionObject
    dd4hep::printout( dd4hep::DEBUG, "DeMuon", "chambersOut DEBUG: hit not found, test x %f, y_foxHit2 %f", x,
                      y_foxHit2 );
    chamberMostLikely( x, y_foxHit2, ista, jsid, jreg, jcha );
    if ( jcha != -1 ) {
      chambersInfo.push_back( jsid );
      chambersInfo.push_back( jreg );
      chambersInfo.push_back( jcha );
    }
    //
    jcha = -1;
    jsid = in_sid;
    jreg = in_reg;
    //
    // given the the new hit x,y point returns the index of the m_chambers vectors filled in DeMuonRegionObject
    dd4hep::printout( dd4hep::DEBUG, "DeMuon", "chambersOut DEBUG: hit not found, test x_foxHit %f, y_foxHit2 %f",
                      x_foxHit, y_foxHit2 );
    chamberMostLikely( x_foxHit, y_foxHit2, ista, jsid, jreg, jcha );
    if ( jcha != -1 ) {
      chambersInfo.push_back( jsid );
      chambersInfo.push_back( jreg );
      chambersInfo.push_back( jcha );
    }
  }
  return chambersInfo;
}
//
//
void DeMuon::FillChamberIndexMap() {

  for ( unsigned int ista = 0; ista < Muon::nStations; ista++ ) {
    for ( unsigned int ireg = 0; ireg < Muon::nRegions; ireg++ ) {
      for ( unsigned int iqua = 0; iqua < Muon::nQuadrants; iqua++ ) {
        for ( unsigned int nx = 0; nx < Muon::grid[ireg].first * 2; nx++ ) {
          for ( unsigned int ny = 0; ny < Muon::grid[ireg].second * 2; ny++ ) {
            unsigned int isid  = iqua / 2;
            int          index = getChamberVectorIndex( nx, ny, ireg, iqua );
            if ( index > -1 ) {
              auto chamberNumber = static_cast<unsigned int>(
                  m_stations[ista].m_sides[isid].m_regions[ireg].m_chambers[index].m_ChamberNumber );
              dd4hep::printout( dd4hep::DEBUG, "DeMuon",
                                "Stat :: %i // Reg :: %i // isid :: %i // chamberNumber :: %i // index :: %i", ista,
                                ireg, isid, chamberNumber, index );
              m_chamberNumber_Index[ista][ireg].insert( {chamberNumber, {index, isid, iqua, nx, ny}} );
            }
          }
        }
      }
    }
  }
}
//
void DeMuon::FillGeoInfo() {

  auto get_x = []( const auto& point ) { return point.x(); };
  auto get_y = []( const auto& point ) { return point.y(); };

  auto verticesValues = []( const auto& gasgap, auto getter ) {
    const auto dx = gasgap.m_DXgap;
    const auto dy = gasgap.m_DYgap;

    return std::array{std::abs( getter( gasgap.toGlobal( ROOT::Math::XYZPoint{-dx, -dy, 0} ) ) ),
                      std::abs( getter( gasgap.toGlobal( ROOT::Math::XYZPoint{-dx, dy, 0} ) ) ),
                      std::abs( getter( gasgap.toGlobal( ROOT::Math::XYZPoint{dx, -dy, 0} ) ) ),
                      std::abs( getter( gasgap.toGlobal( ROOT::Math::XYZPoint{dx, dy, 0} ) ) )};
  };

  auto get_gasgap = [this]( int nx, int ny, unsigned int ista, unsigned int ireg,
                            unsigned int iqua ) -> const LHCb::Detector::detail::DeMuonGasGapObject& {
    int          index   = getChamberVectorIndex( nx, ny, ireg, iqua );
    unsigned int isid    = iqua / 2;
    const auto&  chamber = m_stations[ista].m_sides[isid].m_regions[ireg].m_chambers[index];
    return chamber.m_gasGaps[0];
  };

  auto min_of = []( const auto minVal, const auto& values ) {
    return std::accumulate( begin( values ), end( values ), minVal,
                            []( double a, double b ) { return std::min( a, b ); } );
  };
  auto max_of = []( const auto maxVal, const auto& values ) {
    return std::accumulate( begin( values ), end( values ), maxVal,
                            []( double a, double b ) { return std::max( a, b ); } );
  };

  // this method should mimic what was done previously in DetDesc
  for ( auto& istation : m_stations ) {
    //
    unsigned int onlyOneSide = 0;
    DeMuonSide   det_side( &( istation.m_sides[onlyOneSide] ) );
    unsigned int ista     = istation.m_stationID - Muon::firstStation;
    unsigned int nRegions = 0;
    m_posZstation[ista]   = istation.m_Zstation;

    /// Evaluate chamberVertex for the Inner and Outer chambers
    double minX = 10000.0, minY = 10000.0, maxX = 0.0, maxY = 0.0;

    std::array<ROOT::Math::XYZPoint, 4> chamberVertex;
    for ( unsigned int iqua = 0; iqua < Muon::nQuadrants; iqua++ ) {

      // Internal region
      unsigned int ireg = 0;
      minY = min_of( minY, verticesValues( get_gasgap( 0, Muon::grid[ireg].second, ista, ireg, iqua ), get_y ) );
      minX = min_of( minX, verticesValues( get_gasgap( Muon::grid[ireg].first, 0, ista, ireg, iqua ), get_x ) );

      // External region
      ireg = 3;
      for ( unsigned int nx = 0; nx < 2 * Muon::grid[3].first - 1; nx++ ) {
        maxY = max_of( maxY,
                       verticesValues( get_gasgap( nx, 2 * Muon::grid[ireg].second - 1, ista, ireg, iqua ), get_y ) );
      }
      for ( unsigned int ny = 0; ny < 2 * Muon::grid[3].second - 1; ny++ ) {
        maxX =
            max_of( maxX, verticesValues( get_gasgap( 2 * Muon::grid[ireg].first - 1, ny, ista, ireg, iqua ), get_x ) );
      }
    }

    m_stationBox[ista] = {minX, minY, maxX, maxY};

    const std::array<LHCb::Detector::DeMuonRegion, 4> all_regions = det_side.regions();
    for ( const auto& iregion : all_regions ) {

      unsigned int onlyFirstChamber = 0;
      auto         det_ch           = iregion.getChamber( onlyFirstChamber );

      int ireg = det_ch.regionNumber() - Muon::firstRegion;
      //
      if ( ista == 0 ) { // chamber-grid is the same in all stations
        m_cgX[ireg] = LHCb::Detector::Muon::grid[ireg].first;
        m_cgY[ireg] = LHCb::Detector::Muon::grid[ireg].second;

        m_layout[ireg] = Layout{Muon::grid[ireg].first, Muon::grid[ireg].second};
      }
      //
      // Fill logical channel grids
      FillSystemGrids( ista + Muon::firstStation, ireg + Muon::firstRegion );
      //
      m_chamberPerRegion[ista * 4 + ireg] = 2 * iregion.chambersNumber();

      unsigned int onlyFirstGap = 0;
      auto         det_gap      = det_ch.getGap( onlyFirstGap );
      //
      // Retrieve the chamber sensitive area
      m_chamberAreaX[ista * 4 + ireg] = det_ch.DX() * 2.;
      m_chamberAreaY[ista * 4 + ireg] = det_ch.DY() * 2.;
      m_chamberAreaZ[ista * 4 + ireg] = det_ch.DZ() * 2.;
      double area                     = det_ch.DX() * det_ch.DY() * 4.;
      m_areaChamber[ista * 4 + ireg]  = area;
      dd4hep::printout( dd4hep::DEBUG, "DeMuonObject", "area %d", area );
      //
      m_sensitiveAreaX[ista * 4 + ireg] = det_gap.DX() * 2.;
      m_sensitiveAreaY[ista * 4 + ireg] = det_gap.DY() * 2.;
      m_sensitiveAreaZ[ista * 4 + ireg] = det_gap.DZ() * 2.;
      //
      // This part can be simplified returning just the constant
      m_gapPerRegion[ista * 4 + ireg] = Muon::nGaps;
      m_gapPerFE[ista * 4 + ireg]     = m_gapPerRegion[ista * 4 + ireg] / 2;
      //
      // The grid condition is the size of physical readout channels for a certain
      // readout type (i.e. wires or cathode pads) normalized to the
      // number of physical channels of the same type in the chamber
      // By convention 0=wires, 1=cathode pads
      std::string tag     = gridTag( ista + Muon::firstStation, ireg + Muon::firstRegion );
      auto        theGrid = m_chambersGrids.find( tag );
      if ( theGrid == m_chambersGrids.end() ) {
        dd4hep::printout( dd4hep::ERROR, "DeMuonObject", "FillSystemGrids ERROR: chamber grid %s not found ",
                          tag.c_str() );
        return;
      }
      // Readout number
      unsigned int nreadout = ( theGrid->second.getGrid2SizeY() > 1 ) ? 2 : 1;

      m_readoutNumber[ista * 4 + ireg] = nreadout;
      for ( unsigned int i = 0; i < nreadout; i++ ) {
        if ( i == 0 ) {
          m_phChannelNX[i][ista * 4 + ireg] = theGrid->second.getGrid1SizeX();
          m_phChannelNY[i][ista * 4 + ireg] = theGrid->second.getGrid1SizeY();
          m_readoutType[i][ista * 4 + ireg] = ( theGrid->second.getReadoutGrid() )[i];
        }
        if ( i == 1 ) {
          m_phChannelNX[i][ista * 4 + ireg] = theGrid->second.getGrid2SizeX();
          m_phChannelNY[i][ista * 4 + ireg] = theGrid->second.getGrid2SizeY();
          m_readoutType[i][ista * 4 + ireg] = ( theGrid->second.getReadoutGrid() )[i];
        }
      }
      int maps                           = ( theGrid->second.getMapGrid() ).size() / 2;
      m_LogMapPerRegion[ista * 4 + ireg] = maps;

      if ( nreadout == 1 ) {
        for ( int i = 0; i < maps; i++ ) {
          m_LogMapRType[i][ista * 4 + ireg] = m_readoutType[0][ista * 4 + ireg];
          m_LogMapMergex[i][ista * 4 + ireg] =
              ( theGrid->second.getMapGrid() )[i * 2]; // these are the log OR of the physical channels
          m_LogMapMergey[i][ista * 4 + ireg] =
              ( theGrid->second.getMapGrid() )[i * 2 + 1]; // these are the log OR of the physical channels
        }
      } else if ( nreadout == 2 ) {
        for ( int i = 0; i < maps; i++ ) {
          m_LogMapRType[i][ista * 4 + ireg]  = m_readoutType[i][ista * 4 + ireg];
          m_LogMapMergex[i][ista * 4 + ireg] = ( theGrid->second.getMapGrid() )[i * 2];
          m_LogMapMergey[i][ista * 4 + ireg] = ( theGrid->second.getMapGrid() )[i * 2 + 1];
        }
      }
      //
      // ==>> fill m_padSizeX and m_padSizeY
      unsigned int part = ista * 4 + ireg;
      //
      // one readout
      if ( m_readoutNumber[part] == 1 ) {
        // 1 map
        if ( m_LogMapPerRegion[part] == 1 ) {
          // already pads...
          m_padSizeX[part] = ( m_sensitiveAreaX[part] / m_phChannelNX[0][part] ) * m_LogMapMergex[0][part];
          m_padSizeY[part] = ( m_sensitiveAreaY[part] / m_phChannelNY[0][part] ) * m_LogMapMergey[0][part];

        } else if ( m_LogMapPerRegion[part] == 2 ) {
          int mgx = 0;
          int mgy = 0;
          mgx =
              ( m_LogMapMergex[0][part] > m_LogMapMergex[1][part] ) ? m_LogMapMergex[1][part] : m_LogMapMergex[0][part];
          mgy =
              ( m_LogMapMergey[0][part] > m_LogMapMergey[1][part] ) ? m_LogMapMergey[1][part] : m_LogMapMergey[0][part];
          m_padSizeX[part] = ( m_sensitiveAreaX[part] / m_phChannelNX[0][part] ) * mgx;
          m_padSizeY[part] = ( m_sensitiveAreaY[part] / m_phChannelNY[0][part] ) * mgy;
        }
      } else if ( m_readoutNumber[part] == 2 ) {
        // the convention is always anode first...
        int mgx          = 0;
        int mgy          = 0;
        mgx              = m_LogMapMergex[0][part];
        mgy              = m_LogMapMergey[1][part];
        m_padSizeX[part] = ( m_sensitiveAreaX[part] / m_phChannelNX[0][part] ) * mgx;
        m_padSizeY[part] = ( m_sensitiveAreaY[part] / m_phChannelNY[1][part] ) * mgy;
      }
      //
      nRegions++;
      //
      //
    }
    m_regsperSta[ista] = nRegions;
  }
}
//
std::optional<ROOT::Math::XYZPoint> DeMuon::getPCCenter( FrontEndID fe, int icha, int ista, int ireg ) const {

  // Getting the chamber infos: chamber and first gap detector elements,
  // box half-dimensions (dx, dy) and chamber position

  if ( ista < 0 || ireg < 0 || icha < 0 ) {
    dd4hep::printout( dd4hep::ERROR, "DeMuon", "getPCCenter ERROR: istation :: %d, iregion :: %d, ichamber :: %d ",
                      ista, ireg, icha );
    return {};
  }

  auto        ChamberInfo  = GetChamberInfoFromNum( ista, ireg, icha + Muon::firstChamber );
  const auto& firstGap_det = ChamberInfo->detFirstgap;
  double      dx           = ChamberInfo->Dx;
  double      dy           = ChamberInfo->Dy;

  std::string tag     = gridTag( ista + Muon::firstStation, ireg + Muon::firstRegion );
  auto        theGrid = m_chambersGrids.find( tag );
  if ( theGrid == m_chambersGrids.end() ) {
    dd4hep::printout( dd4hep::ERROR, "DeMuon", "getPCCenter ERROR: chamber grid %s not found ", tag.c_str() );
    return {};
  }
  //
  double xcenter_norma = -1.;
  double ycenter_norma = -1.;
  bool   sc            = theGrid->second.getPCCenter( fe, xcenter_norma, ycenter_norma );
  //
  if ( sc == false ) return {};
  double xcenter_gap = xcenter_norma * 2 * dx - dx;
  double ycenter_gap = ycenter_norma * 2 * dy - dy;

  ROOT::Math::XYZPoint loc( xcenter_gap, ycenter_gap, 0 );
  ROOT::Math::XYZPoint glob = firstGap_det.toGlobal( loc );
  return glob;
}
//
void DeMuon::chamberMostLikely( float x, float y, int ista, int& isid, int& ireg, int& icha ) const {

  int jreg = -1; // region index from 0 to 3
  if ( ireg != -1 ) jreg = ireg;

  float xs = m_sensitiveAreaX[ista * 4]; // chamber sensitive area in R1
  float ys = m_sensitiveAreaY[ista * 4];

  auto nx = static_cast<unsigned>( fabsf( x / xs ) );
  auto ny = static_cast<unsigned>( fabsf( y / ys ) );
  //
  float rx = x / xs;
  float ry = y / ys;
  //
  dd4hep::printout(
      dd4hep::DEBUG, "DeMuon",
      "chamberMostLikely DEBUG 1: x :: %f, y :: %f, xs :: %f, ys :: %f, nx :: %i, ny :: %i, rx :: %f, ry :: %f", x, y,
      xs, ys, nx, ny, rx, ry );

  // check the region iReg ==>
  for ( unsigned int iRx = 0; iRx < 4; iRx++ ) {

    if ( nx < 2 * m_cgX.at( iRx ) && ny < 2 * m_cgY.at( iRx ) ) {
      dd4hep::printout( dd4hep::DEBUG, "DeMuon", "ChamberMostLikely DEBUG: ireg %i, iRx %i", ireg, iRx );
      ireg = iRx;
      break;
    } else {
      if ( iRx < 2 ) nx /= 2;
    }
  }
  //
  if ( ireg == -1 ) {
    dd4hep::printout( dd4hep::DEBUG, "DeMuon", "ChamberMostLikely DEBUG: region not found " );
    return;
  }

  // iqua(quadrant) definition
  //      +y
  //     0 | 3    lower_side = 0
  //+x <---+---
  //     1 | 2    lower_side = 1
  //
  int iqua = 0;
  if ( ( y > 0. ) && ( x < 0. ) ) {
    iqua = 3;
  } else if ( ( y < 0. ) && ( x > 0. ) ) {
    iqua = 1;
  } else if ( ( y < 0. ) && ( x < 0. ) ) {
    iqua = 2;
  }
  //
  isid = iqua / 2;
  //
  xs = m_sensitiveAreaX[ista * 4 + ireg]; // chamber sensitive area in iReg
  ys = m_sensitiveAreaY[ista * 4 + ireg];
  //
  auto nnx = static_cast<unsigned>( fabsf( x / xs ) );
  auto nny = static_cast<unsigned>( fabsf( y / ys ) );
  //
  float rrx = x / xs;
  float rry = y / ys;
  //
  dd4hep::printout(
      dd4hep::DEBUG, "DeMuon",
      "chamberMostLikely DEBUG 2: x :: %f, y :: %f, xs :: %f, ys :: %f, nx :: %i, ny :: %i, rx :: %f, ry :: %f", x, y,
      xs, ys, nnx, nny, rrx, rry );
  //
  // check nnx ( no problems with nny ==> Dy_chamber does not change in all regions )
  //
  dd4hep::printout( dd4hep::DEBUG, "DeMuon", "ChamberMostLikely DEBUG: ireg %i, nnx %i", ireg, nnx );
  if ( ( ireg == 3 ) && ( nnx > 3 ) ) return; // out of the station .. !!
  //
  if ( ( ireg < 3 ) && ( nnx > 1 ) ) { // it could be a wrong region

    int newreg = -1;
    for ( unsigned int iRx = ireg; iRx < 4; iRx++ ) {

      if ( nnx < 2 * m_cgX.at( iRx ) && nny < 2 * m_cgY.at( iRx ) ) {
        dd4hep::printout( dd4hep::DEBUG, "DeMuon", "ChamberMostLikely DEBUG: newreg %i, iRx %i", newreg, iRx );
        newreg = iRx;
        break;
      } else {
        if ( iRx < 2 ) nnx /= 2;
      }
    }
    if ( newreg == -1 ) {
      dd4hep::printout( dd4hep::DEBUG, "DeMuon", "ChamberMostLikely DEBUG: newreg not found " );
      return;
    }
    ireg = newreg;
    xs   = m_sensitiveAreaX[ista * 4 + ireg]; // chamber sensitive area in new iReg
    ys   = m_sensitiveAreaY[ista * 4 + ireg];
    //
    nnx = static_cast<unsigned>( fabsf( x / xs ) );
    nny = static_cast<unsigned>( fabsf( y / ys ) );
    //
    rrx = x / xs;
    rry = y / ys;
    //
    dd4hep::printout( dd4hep::DEBUG, "DeMuon",
                      "chamberMostLikely DEBUG 3: x :: %f, y :: %f, xs :: %f, ys :: %f, nx :: %i, ny :: %i, rx :: %f, "
                      "ry :: %f, reg:: %i",
                      x, y, xs, ys, nnx, nny, rrx, rry, ireg );
    //
  }
  //
  if ( ( jreg != -1 ) && ( ireg != jreg ) ) {
    dd4hep::printout( dd4hep::DEBUG, "DeMuon", "ChamberMostLikely DEBUG : ireg %i, jreg %i, nnx %i, nny %i", ireg, jreg,
                      nnx, nny );
    // return;
  }
  //
  icha = getChamberVectorIndex( nnx, nny, ireg, iqua );
  return;
}
//
//
std::string DeMuon::gridTag( unsigned int stationNumber, unsigned int regionNumber ) const {
  std::string gridTag = "M" + std::to_string( stationNumber ) + "R" + std::to_string( regionNumber );
  return gridTag;
}
//
//
void DeMuon::FillSystemGrids( unsigned int stationNumber, unsigned int regionNumber ) {

  unsigned int ista = stationNumber - Muon::firstStation;
  unsigned int ireg = regionNumber - Muon::firstRegion;

  unsigned int index   = ireg + ista * LHCb::Detector::Muon::nRegions;
  std::string  tag     = gridTag( stationNumber, regionNumber );
  auto         theGrid = m_chambersGrids.find( tag );
  if ( theGrid == m_chambersGrids.end() ) {
    dd4hep::printout( dd4hep::ERROR, "DeMuon", "FillSystemGrids ERROR: chamber grid %s not found ", tag.c_str() );
    return;
  }

  std::vector<unsigned int> readoutType = theGrid->second.getReadoutGrid();
  std::vector<unsigned int> mapType     = theGrid->second.getMapGrid();

  // Physical channels
  int gr1X = LHCb::Detector::Muon::grid[ireg].first * theGrid->second.getGrid1SizeX();
  int gr1Y = LHCb::Detector::Muon::grid[ireg].second * theGrid->second.getGrid1SizeY();
  int gr2X = LHCb::Detector::Muon::grid[ireg].first * theGrid->second.getGrid2SizeX();
  int gr2Y = LHCb::Detector::Muon::grid[ireg].second * theGrid->second.getGrid2SizeY();

  // Logical channels multiplicity
  int xm0( 0 );
  int xm1( 0 );
  int ym0( 0 );
  int ym1( 0 );
  xm0 = mapType.at( 0 );
  ym0 = mapType.at( 1 );
  if ( mapType.size() == 4 ) {
    xm1 = mapType.at( 2 );
    ym1 = mapType.at( 3 );
  }

  // Service variables
  int axm0( 0 );
  int axm1( 0 );
  int aym0( 0 );
  int aym1( 0 );
  int agr1X( 0 );
  int agr1Y( 0 );
  int agr2X( 0 );
  int agr2Y( 0 );

  // Only one readout
  if ( readoutType.size() == 1 ) {
    // and only one map
    if ( mapType.size() == 2 ) {
      // so horiz, vert and pad all same
      m_logHorizGridX.at( index ) = gr1X / xm0;
      m_logHorizGridY.at( index ) = gr1Y / ym0;
      m_logVertGridX.at( index )  = gr1X / xm0;
      m_logVertGridY.at( index )  = gr1Y / ym0;
      m_padGridX.at( index )      = gr1X / xm0;
      m_padGridY.at( index )      = gr1Y / ym0;
    } else {
      // If two maps to be crossed
      if ( xm0 < xm1 ) {
        // xm0 is the vertical map, xm1 is the horizontal map
        m_logVertGridX.at( index ) = gr1X / xm0;
        m_logVertGridY.at( index ) = gr1Y / ym0;

        m_logHorizGridX.at( index ) = gr1X / xm1;
        m_logHorizGridY.at( index ) = gr1Y / ym1;

        m_padGridX.at( index ) = gr1X / xm0;
        m_padGridY.at( index ) = gr1Y / ym1;
      } else {
        // xm1 is the vertical map, xm0 is the horizontal map
        m_logVertGridX.at( index ) = gr1X / xm1;
        m_logVertGridY.at( index ) = gr1Y / ym1;

        m_logHorizGridX.at( index ) = gr1X / xm0;
        m_logHorizGridY.at( index ) = gr1Y / ym0;

        m_padGridX.at( index ) = gr1X / xm1;
        m_padGridY.at( index ) = gr1Y / ym0;
      }
    }
  } else if ( readoutType.size() == 2 ) {
    // There are 2 different readouts
    if ( !readoutType.at( 0 ) ) {
      // First readout is anode readout xm0
      // must be two maps for anode and cathode seperately
      if ( gr1X / xm0 > gr2X / xm1 ) {
        axm0  = xm0;
        axm1  = xm1;
        aym0  = ym0;
        aym1  = ym1;
        agr1X = gr1X;
        agr1Y = gr1Y;
        agr2X = gr2X;
        agr2Y = gr2Y;
      } else {
        axm0  = xm1;
        axm1  = xm0;
        aym0  = ym1;
        aym1  = ym0;
        agr1X = gr2X;
        agr1Y = gr2Y;
        agr2X = gr1X;
        agr2Y = gr1Y;
      }
    } else {
      // map over was reversed, otherwise the same
      if ( gr1X / xm1 > xm1 / xm0 ) {
        axm0  = xm1;
        axm1  = xm0;
        aym0  = ym1;
        aym1  = ym0;
        agr1X = gr1X;
        agr1Y = gr1Y;
        agr2X = gr2X;
        agr2Y = gr2Y;
      } else {
        axm0  = xm0;
        axm1  = xm1;
        aym0  = ym0;
        aym1  = ym1;
        agr1X = gr2X;
        agr1Y = gr2Y;
        agr2X = gr1X;
        agr2Y = gr1Y;
      }
    }
    m_logVertGridX.at( index ) = agr1X / axm0;
    m_logVertGridY.at( index ) = agr1Y / aym0;

    m_logHorizGridX.at( index ) = agr2X / axm1;
    m_logHorizGridY.at( index ) = agr2Y / aym1;

    m_padGridX.at( index ) = agr1X / axm0;
    m_padGridY.at( index ) = agr2Y / aym1;

  } else {
    dd4hep::printout( dd4hep::ERROR, "DeMuon", "FillSystemGrids ERROR: Wrong size readout vector %i != from 1 or 2!!!!",
                      readoutType.size() );
  }
  // End of FE work
  return;
}

std::optional<DeMuon::TilePosition> DeMuon::position( const TileID& tile ) {

  // Check of channel/pad grids initialisation? (to be implemented)

  unsigned int ista = tile.station(); // - Muon::firstStation; is Station starting from 0 or 2?
  unsigned int ireg = tile.region();  // - Muon::firstRegion; is Region starting form 0 or 1?

  unsigned int index = ista * Muon::nRegions + ireg;

  const auto tileGridX = tile.layout().xGrid();
  const auto tileGridY = tile.layout().yGrid();
  dd4hep::printout( dd4hep::DEBUG, "DeMuon", " position :: grid (%i,%i)?", tile.layout().xGrid(),
                    tile.layout().yGrid() );

  if ( tileGridX == Muon::grid[ireg].first && tileGridY == Muon::grid[ireg].second ) {
    // chamber
    dd4hep::printout( dd4hep::DEBUG, "DeMuon", " position :: CHAMBER!" );

    return getXYZChamber( tile, true );

  } else if ( tileGridX == m_padGridX[index] && tileGridY == m_padGridY[index] ) {
    // pad
    dd4hep::printout( dd4hep::DEBUG, "DeMuon", " position :: PAD!" );
    return getXYZPad( tile );

  } else if ( tileGridX == m_logHorizGridX[index] && tileGridY == m_logHorizGridY[index] ) {
    // horizontal logical channel
    dd4hep::printout( dd4hep::DEBUG, "DeMuon", " position :: HORIZONTAL LOGICAL!" );

    return getXYZLogical( tile );
  } else if ( tileGridX == m_logVertGridX[index] && tileGridY == m_logVertGridY[index] ) {
    // vertical logical channel
    dd4hep::printout( dd4hep::DEBUG, "DeMuon", " position :: VERTICAL LOGICAL!" );
    return getXYZLogical( tile );
  } else if ( tileGridX == 1 && tileGridY == 1 ) {
    // twelfth (to be understood)
    dd4hep::printout( dd4hep::DEBUG, "DeMuon", " position :: TWELFTH!" );
    return {}; // getXYZTwelfth( tile );
  }

  return {};
}
//
std::optional<DeMuon::TilePosition> DeMuon::getXYZChamber( const TileID& tile, bool toGlob = true ) {

  auto chamber = getChamberFromTile( tile );
  if ( chamber == nullptr ) return DeMuon::TilePosition( 0., 0., 0., 0., 0., 0. );

  return getXYZ( chamber, -1, toGlob );
}
//
std::optional<DeMuon::TilePosition> DeMuon::getXYZ( const Chamber& chamber, const int& gapNumber, bool toGlob ) {

  if ( gapNumber == -1 ) {

    const auto& detFirstGap = chamber.getGap( 0 );

    double Dx = detFirstGap.DX();
    double Dy = detFirstGap.DY();
    double Dz = chamber.DZ();
    if ( toGlob ) {
      return DeMuon::TilePosition( chamber.XPos(), chamber.YPos(), chamber.ZPos(), Dx, Dy, Dz );
    } else {
      return DeMuon::TilePosition( 0., 0., 0., Dx, Dy, Dz );
    }

  } else {
    // (Flavio) check this!
    // in the old one the layer was also taken for position trasformation
    // from local to global [GOSH!]
    const auto& gap = chamber.getGap( gapNumber );

    double Dx = gap.DX();
    double Dy = gap.DY();
    double Dz = gap.DZ();

    if ( toGlob ) {
      return DeMuon::TilePosition( gap.XPos(), gap.YPos(), gap.ZPos(), Dx, Dy, Dz );
    } else {
      return DeMuon::TilePosition( 0., 0., 0., Dx, Dy, Dz );
    }
  }

  return {};
}

std::optional<DeMuon::TilePosition> DeMuon::getXYZPad( const TileID& tile ) {

  unsigned int ireg = tile.region();

  auto chamberTile = m_layout[ireg].contains( tile );
  auto chamber     = getChamberFromTile( chamberTile );
  if ( chamber == nullptr ) return DeMuon::TilePosition( 0., 0., 0., 0., 0., 0. );

  // number of pads or logical channels in a chamber is :
  // layout.xGrid() / chamberLayoutX , padLayoutY / chamberLayoutY
  int xRatio = tile.layout().xGrid() / Muon::grid[ireg].first;
  int yRatio = tile.layout().yGrid() / Muon::grid[ireg].second;

  // Need to work out offset within chamber
  // initally ignore the "reflectional" nature of the quarters
  int xOffset = tile.nX() - ( xRatio * chamberTile.nX() );
  int yOffset = tile.nY() - ( yRatio * chamberTile.nY() );

  // Do the reflection of the tileID structure into Cartesian coordinates
  if ( tile.quarter() == 1 ) {
    yOffset = yRatio - ( 1 + yOffset );
  } else if ( tile.quarter() == 2 ) {
    xOffset = xRatio - ( 1 + xOffset );
    yOffset = yRatio - ( 1 + yOffset );
  } else if ( tile.quarter() == 3 ) {
    xOffset = xRatio - ( 1 + xOffset );
  }

  int        gapNumber = -1;
  auto const pos       = getXYZ( chamber, gapNumber, false );

  if ( !pos ) {
    dd4hep::printout( dd4hep::ERROR, "DeMuon", "Failed to get chamber for pad tile: %s", tile );
    return pos;
  }

  // now make the calculations of x,y and z (and dx, dy and dz)
  // in chamber reference frame.
  double x = ( pos->x() - pos->dX() ) +
             ( ( static_cast<double>( xOffset ) / static_cast<double>( xRatio ) ) * ( 2.0 * pos->dX() ) );
  double y = ( pos->y() - pos->dY() ) +
             ( ( static_cast<double>( yOffset ) / static_cast<double>( yRatio ) ) * ( 2.0 * pos->dY() ) );
  double z = pos->z();

  // this is the half width of the Pad
  double Dx = pos->dX() / ( static_cast<double>( xRatio ) );
  double Dy = pos->dY() / ( static_cast<double>( yRatio ) );

  // correct to the center of the pad
  x = x + Dx;
  y = y + Dy;

  ROOT::Math::XYZPoint center( x, y, z );
  ROOT::Math::XYZPoint corner( x + Dx, y + Dy, z + pos->dZ() );

  // Correct XYZ and Deltas for local to Global transformation
  ROOT::Math::XYZPoint delta( Dx, Dy, pos->dZ() );

  if ( !( chamber == nullptr ) ) {
    auto center_global = chamber->toGlobal( center );
    x                  = center_global.x();
    y                  = center_global.y();
    z                  = center_global.z();
    localToGlobal( &chamber, center, corner, delta );
  }

  dd4hep::printout( dd4hep::DEBUG, "DeMuon", " getXYZPad:: loc to glob %f %f %f %i %f %f %f", Dx, Dy, pos->dZ(),
                    chamber.chamberNumber(), delta.x(), delta.y(), delta.z() );

  return DeMuon::TilePosition{x, y, z, delta.x(), delta.y(), delta.z()};
}

std::optional<DeMuon::TilePosition> DeMuon::getXYZLogical( const TileID& tile ) {
  // If we get here then the logical strip is potenitally bigger than a chamber
  // if not then we can subcontract to getXYZPad directly

  // to find the x,y,z of the pad one must first find the chamber
  // to find the chamber one must know the tile of the chamber

  unsigned int ista = tile.station();
  unsigned int ireg = tile.region();

  if ( tile.layout().xGrid() >= m_layout[ireg].xGrid() && tile.layout().yGrid() >= m_layout[ireg].yGrid() ) {
    // ok logical channels are within a single chamber, delegate to getXYZpad
    dd4hep::printout( dd4hep::DEBUG, "DeMuon", " getXYZLogical :: is a PAD grid (%i,%i)?", tile.layout().xGrid(),
                      tile.layout().yGrid() );

    return getXYZPad( tile );
  }
  if ( tile.layout().xGrid() >= m_layout[ireg].xGrid() && tile.layout().yGrid() < m_layout[ireg].yGrid() ) {
    // This logical channel crosses chambers in y, break it down into
    // chamber hight pads then combine the chamber extents later
    // number of tiles to make:
    int nTile = m_layout[ireg].yGrid() / tile.layout().yGrid();
    dd4hep::printout( dd4hep::DEBUG, "DeMuon",
                      "getXYZLogical :: Making %i temporary LHCb::Detector::Muon::TileIDs to get all chamber locations",
                      nTile );

    // width of pad (in x) is the same, hight (yGrid) that of the chamber
    Layout tempPadLayout( tile.layout().xGrid(), m_layout[ireg].yGrid() );

    std::vector<LHCb::Detector::Muon::TileID> tempTiles;

    for ( int i = 0; i < nTile; i++ ) {
      int                          yTile = ( tile.nY() * nTile ) + i;
      LHCb::Detector::Muon::TileID tTile( ista, tempPadLayout, ireg, tile.quarter(), tile.nX(), yTile );
      tempTiles.push_back( tTile );
    }

    // loop over the tiles and get max/min x,y,z of the combinations
    double xmin = 0.0;
    double xmax = 0.0;
    double ymin = 0.0;
    double ymax = 0.0;
    double zmin = 0.0;
    double zmax = 0.0;
    //
    std::optional<DeMuon::TilePosition> pos{};

    bool isFirst = true;
    for ( auto& iTile : tempTiles ) {
      if ( !iTile.isValid() ) {
        dd4hep::printout( dd4hep::ERROR, "DeMuon", "getXYZLogical :: iTile not valid" );
        return {};
      }
      pos = getXYZPad( iTile );
      if ( !pos ) {
        dd4hep::printout( dd4hep::ERROR, "DeMuon", "Passed a temporary channel to getXYZPad: failed | tile ID= %s",
                          iTile );
        return pos;
      }
      if ( isFirst ) {
        xmin    = pos->x();
        xmax    = pos->x();
        ymin    = pos->y();
        ymax    = pos->y();
        zmin    = pos->z();
        zmax    = pos->z();
        isFirst = false;
      } else {
        if ( xmax < pos->x() ) { xmax = pos->x(); }
        if ( xmin > pos->x() ) { xmin = pos->x(); }
        if ( ymax < pos->y() ) { ymax = pos->y(); }
        if ( ymin > pos->y() ) { ymin = pos->y(); }
        if ( zmax < pos->z() ) { zmax = pos->z(); }
        if ( zmin > pos->z() ) { zmin = pos->z(); }
      }
    }

    return DeMuon::TilePosition{( xmax + xmin ) / 2.0,
                                ( ymax + ymin ) / 2.0,
                                ( zmax + zmin ) / 2.0,
                                ( ( xmax - xmin ) / 2.0 ) + pos->dX(),
                                ( ( ymax - ymin ) / 2.0 ) + pos->dY(),
                                ( ( zmax - zmin ) / 2.0 ) + pos->dZ()};
  }
  dd4hep::printout( dd4hep::ERROR, "DeMuon", "You requested a logical channel wider than a chamber, failing" );
  throw std::out_of_range( "You requested a logical channel wider than a chamber, failing" );
}
//
void DeMuon::localToGlobal( const Chamber* chamber, const ROOT::Math::XYZPoint& center,
                            const ROOT::Math::XYZPoint& corner, ROOT::Math::XYZPoint& delta ) {

  // this function makes no sense at all but left since it was present
  // also in the previous code

  auto center_global = chamber->toGlobal( center );
  auto corner_global = chamber->toGlobal( corner );
  dd4hep::printout( dd4hep::DEBUG, "DeMuon",
                    "Local to global center :: ( %f , %f , %f )  Local to global corner :: ( %f , %f , %f )  ",
                    center_global.x(), center_global.y(), center_global.z(), corner_global.x(), corner_global.y(),
                    corner_global.z() );

  delta.SetCoordinates( fabs( center_global.x() - corner_global.x() ), fabs( center_global.y() - corner_global.y() ),
                        fabs( center_global.z() - corner_global.z() ) );
}

unsigned int DeMuon::Tile2FirstChamberNumber( const TileID& Tile ) const {
  const auto& chamberNumber = Tile2ChamberNumber( Tile );
  const auto  chamber       = getChamber( Tile.station(), Tile.region(), chamberNumber );
  return chamber.chamberNumber();
}

unsigned int DeMuon::Tile2ChamberNumber( const TileID& tile ) const {
  // Check of channel/pad grids initialisation? (to be implemented)

  unsigned int ista = tile.station(); // - Muon::firstStation; is Station starting from 0 or 2?
  unsigned int ireg = tile.region();  // - Muon::firstRegion; is Region starting form 0 or 1?

  unsigned int index = ista * Muon::nRegions + ireg;

  unsigned int tileGridX = tile.layout().xGrid();
  unsigned int tileGridY = tile.layout().yGrid();

  if ( tileGridX == Muon::grid[ireg].first && tileGridY == Muon::grid[ireg].second ) {
    // chamber
    return getChamberNumber( tile );

  } else if ( tileGridX == m_padGridX[index] && tileGridY == m_padGridY[index] ) {
    // pad
    auto chamberTile = m_layout[ireg].contains( tile );
    return getChamberNumber( chamberTile );

  } else if ( tileGridX == m_logHorizGridX[index] && tileGridY == m_logHorizGridY[index] ) {
    // horizontal logical channel
    auto chamberNumbers = Logical2ChamberNumber( tile );
    return chamberNumbers.at( 0 );
  } else if ( tileGridX == m_logVertGridX[index] && tileGridY == m_logVertGridY[index] ) {
    // vertical logical channel
    auto chamberNumbers = Logical2ChamberNumber( tile );
    return chamberNumbers.at( 0 );
  } else if ( tileGridX == 1 && tileGridY == 1 ) {
    // twelfth (to be understood)
    return {}; // Twelfth2ChamberNumber( tile );
  }

  return {};
}

std::vector<unsigned int> DeMuon::Logical2ChamberNumber( const TileID& tile ) const {

  // If we get here then the logical strip is potenitally bigger than a chamber
  // if not then we can subcontract to getXYZPad directly

  // to find the x,y,z of the pad one must first find the chamber
  // to find the chamber one must know the tile of the chamber

  std::vector<unsigned int> chamberNumber;

  unsigned int ista = tile.station();
  unsigned int ireg = tile.region();

  if ( tile.layout().xGrid() >= m_layout[ireg].xGrid() && tile.layout().yGrid() >= m_layout[ireg].yGrid() ) {

    // ok logical channels are within a single chamber
    TileID chamTile = m_layout[ireg].contains( tile );
    chamberNumber.push_back( getChamberNumber( chamTile ) );

  } else if ( tile.layout().xGrid() >= m_layout[ireg].xGrid() && tile.layout().yGrid() < m_layout[ireg].yGrid() ) {
    // This logical channel crosses chambers in y, break it down into
    // chamber hight pads then combine the chamber extents later
    // number of tiles to make:

    unsigned int nTile = m_layout[ireg].yGrid() / tile.layout().yGrid(); // number of chamber crossed

    // width of pad (in x) is the same, hight (yGrid) that of the chamber
    Layout tempPadLayout( tile.layout().xGrid(), m_layout[ireg].yGrid() );

    std::vector<LHCb::Detector::Muon::TileID> tempTiles;

    for ( unsigned int i = 0; i < nTile; i++ ) {
      int    yTile = ( tile.nY() * nTile ) + i;
      TileID tTile( ista, tempPadLayout, ireg, tile.quarter(), tile.nX(), yTile );
      dd4hep::printout( dd4hep::DEBUG, "DeMuon", "Logical2ChamberNumber i: %i  nX: %i  nY: %i", i, tile.nX(), yTile );

      TileID chamTile = m_layout[ireg].contains( tTile );
      chamberNumber.push_back( getChamberNumber( chamTile ) );
    }

  } else {
    dd4hep::printout( dd4hep::ERROR, "DeMuon",
                      "Logical2ChamberNumber You requested a logical channel wider than a chamber, failing" );
  }
  return chamberNumber;
}

int DeMuon::Hit2ChamberRegionNumber( ROOT::Math::XYZPoint myPoint ) const {

  unsigned int stationNumber = getStationFromZ( myPoint.z() ); // This returns the station number (2,3,4,5)
  Chamber      chamber       = pos2StChamber( myPoint.x(), myPoint.y(), ( stationNumber - Muon::firstStation ) );
  if ( chamber == nullptr ) {
    dd4hep::printout( dd4hep::DEBUG, "DeMuon", "Hit2ChamberRegionNumber >> ERROR chamber not found!" );
    return -1;
  }
  return chamber.regionNumber();
}

unsigned int DeMuon::getStationFromZ( const double z ) const {
  // station number (2,3,4,5) starting from z position in mm
  unsigned int stationNumber = 0;
  double       s_size        = 400 * dd4hep::mm;
  //  double s_off[5] = {12100,15200,16400,17600,18800};
  double offset = 70 * dd4hep::mm;

  for ( auto& station : m_stations ) {
    stationNumber = station.m_stationID;
    if ( std::abs( z - station.m_Zstation + offset ) < s_size ) break;
  }
  return stationNumber;
}

int DeMuon::regNum( const double x, const double y, unsigned int ista ) const {
  ROOT::Math::XYZPoint point( x, y, 0 );
  unsigned int         chamberNumber = 0;
  int                  ireg          = -1;
  Hit2ChamberNumber( point, ista, chamberNumber, ireg );
  if ( ireg == -1 ) {
    dd4hep::printout( dd4hep::DEBUG, "DeMuon", "regNum DEBUG: no chambers found for entry point " );
    return ireg;
  }
  return ireg + Muon::firstRegion;
}

Chamber DeMuon::pos2StChamber( const double x, const double y, unsigned int ista ) const {
  // Hit Z is not know (giving only the station).
  // Take the chamber Z to update the hit later on.
  unsigned int chamberNumber = 0;
  int          ireg          = -1;
  unsigned int StationNumber = ista + Muon::firstStation;
  //
  Hit2ChamberNumber( ROOT::Math::XYZPoint{x, y, 0}, ista, chamberNumber, ireg );
  //
  if ( StationNumber > 1 && StationNumber < ( Muon::firstStation + Muon::nStations ) && ( ireg != -1 ) ) {
    return getChamber( ista, ireg, chamberNumber );

  } else {
    return nullptr;
  }
}

// Returns the Tile for a given chamber
TileID DeMuon::tileChamber( const Chamber chamber ) const {

  // Region and station got from chamber
  unsigned int stationNumber = chamber.stationNumber();
  unsigned int regionNumber  = chamber.regionNumber();
  // Chamber number is needed to find x,y
  unsigned int chamberNumber = chamber.chamberNumber();

  return Chamber2Tile( ( chamberNumber - 1 ), stationNumber - Muon::firstStation, regionNumber - Muon::firstRegion );
}

TileID DeMuon::Chamber2Tile( unsigned int icha, unsigned int ista, unsigned int ireg ) const {

  TileID myTile;

  myTile.setLayout( Layout( m_cgX.at( ireg ), m_cgY.at( ireg ) ) );
  dd4hep::printout( dd4hep::DEBUG, "DeMuon", "Chamber2Tile >> layout :: (%i,%i)", m_cgX.at( ireg ), m_cgY.at( ireg ) );

  unsigned int chamberNumber = icha + Muon::firstChamber;
  const auto&  map           = m_chamberNumber_Index[ista][ireg];
  auto         it            = map.find( chamberNumber );
  if ( it == map.end() ) {
    dd4hep::printout( dd4hep::DEBUG, "DeMuon",
                      "Chamber2Tile ERROR: the combination station/region/chamber does not exists" );
    return {};
  }

  myTile.setX( it->second.nx );
  myTile.setY( it->second.ny );
  myTile.setStation( ista );
  myTile.setRegion( ireg );
  myTile.setQuarter( it->second.quarter );

  dd4hep::printout( dd4hep::DEBUG, "DeMuon", "Chamber2Tile >> X, Y and Q :: %i %i %i for chamber %i, %i", myTile.nX(),
                    myTile.nY(), it->second.quarter, chamberNumber, ireg + Muon::firstRegion );

  return myTile;
}

int DeMuon::sensitiveVolumeID( const ROOT::Math::XYZPoint& myPoint ) const {

  // retrieve station,region,chamber,gap:
  unsigned int stationNumber = getStationFromZ( myPoint.z() );
  //
  unsigned int regionNumber  = 0;
  unsigned int chamberNumber = 0;
  unsigned int gapNumber     = 0;
  Hit2GapNumber( myPoint, stationNumber, gapNumber, chamberNumber, regionNumber );

  if ( regionNumber == 0 ) { return MuonVolumeID::error().volumeID(); }

  // retrieve the quadrant:
  //
  // iqua(quadrant) definition
  //      +y
  //     0 | 3    lower_side = 0
  //+x <---+---
  //     1 | 2    lower_side = 1
  //
  TileID tile;
  Pos2ChamberTile( myPoint.x(), myPoint.y(), myPoint.z(), tile );
  unsigned int quarterNumber = tile.quarter();
  //
  unsigned int sideNumber = quarterNumber / 2; // Quarter 0, 1 --> side 0; Quarter 2, 3 --> side 1

  LHCb::Detector::MuonVolumeID mvid{
      LHCb::Detector::Muon::StationN( stationNumber ), LHCb::Detector::Muon::SideN( sideNumber ),
      LHCb::Detector::Muon::RegionN( regionNumber ), LHCb::Detector::Muon::ChamberN( chamberNumber ),
      LHCb::Detector::Muon::GapN( gapNumber )};
  return mvid.volumeID();
}

void DeMuon::Pos2ChamberTile( const double x, const double y, const double z, TileID& tile ) const {
  unsigned int chamberNumber = 0;
  int          ireg          = -1;

  // Return the chamber number
  Hit2ChamberNumber( ROOT::Math::XYZPoint{x, y, z}, getStationFromZ( z ) - Muon::firstStation, chamberNumber, ireg );
  if ( ireg == -1 ) {
    dd4hep::printout( dd4hep::DEBUG, "DeMuon", "Pos2ChamberTile DEBUG: no chambers found for entry point" );
    // tile = ?
    return;
  }
  // Convert chamber number into a tile
  tile = Chamber2Tile( ( chamberNumber - 1 ), getStationFromZ( z ) - Muon::firstStation, ireg );
  return;
}

void DeMuon::Hit2GapNumber( ROOT::Math::XYZPoint myPoint, unsigned int stationNumber, unsigned int& gapNumber,
                            unsigned int& chamberNumber, unsigned int& regionNumber ) const {

  // This methods sets the gap = 0 if only the
  // station is provided [HitPoint.z() = 0]
  int ireg( -1 );
  Hit2ChamberNumber( myPoint, stationNumber - Muon::firstStation, chamberNumber, ireg );
  if ( ireg == -1 ) {
    dd4hep::printout( dd4hep::DEBUG, "DeMuon", "Hit2GapNumber DEBUG: no chambers found for entry point, ireg %i",
                      ireg );
    chamberNumber = 0;
    regionNumber  = 0;
    gapNumber     = 0;
    return;
  }
  regionNumber = ireg + Muon::firstRegion;

  Chamber chamber = getChamber( stationNumber - Muon::firstStation, regionNumber - Muon::firstRegion, chamberNumber );
  const auto pointInGasGap = chamber.checkPointInGasGap( myPoint );
  if ( !pointInGasGap ) {
    dd4hep::printout( dd4hep::DEBUG, "DeMuon", "Hit2GapNumber DEBUG: selected chamber does not contain the hit" );
    chamberNumber = 0;
    regionNumber  = 0;
    gapNumber     = 0;
    return;
  }
  gapNumber = pointInGasGap->number;
  //
  return;
}
