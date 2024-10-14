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

#include "Detector/Muon/DeMuonChamber.h"
#include "Core/DeIOV.h"
#include "Core/Units.h"
#include "Core/yaml_converters.h"
#include "DD4hep/DetElement.h"
#include "DD4hep/Detector.h"
#include "DD4hep/Printout.h"

#include <array>
#include <iostream>
#include <regex>
#include <string>

using DeMuonChamber = LHCb::Detector::detail::DeMuonChamberObject;

LHCb::Detector::detail::DeMuonChamberObject::DeMuonChamberObject( const dd4hep::DetElement&             de,
                                                                  dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeIOVObject( de, ctxt, 11009 )
    , m_gasGaps{{{de.child( "GasGap1" ).child( "GasGapLayer" ), ctxt, 1},
                 {de.child( "GasGap2" ).child( "GasGapLayer" ), ctxt, 2},
                 {de.child( "GasGap3" ).child( "GasGapLayer" ), ctxt, 3},
                 {de.child( "GasGap4" ).child( "GasGapLayer" ), ctxt, 4}}} {

  ROOT::Math::XYZPoint globalPoint = toGlobal( ( ROOT::Math::XYZPoint( 0., 0., 0. ) ) );

  m_Xcham = globalPoint.x();
  m_Ycham = globalPoint.y();
  m_Zcham = globalPoint.z();
  //
  const std::string& name = de.name(); // M*R*Cham00*
  //
  std::string stanum;
  stanum.assign( name, 0, 2 );
  sscanf( stanum.c_str(), "M%d", &m_StationNumber );
  std::string regnum;
  regnum.assign( name, 2, 2 );
  sscanf( regnum.c_str(), "R%d", &m_RegionNumber );
  std::string chnum;
  chnum.assign( name, 4, 7 );
  sscanf( chnum.c_str(), "Cham%d", &m_ChamberNumber );
  //
  TGeoVolume* vol   = de.volume();
  TGeoShape*  shape = vol->GetShape();
  // Because we know it's a box....
  TGeoBBox* box = (TGeoBBox*)shape;
  m_DXcham      = toLHCbLengthUnits( ( box->GetDX() ) );
  m_DYcham      = toLHCbLengthUnits( ( box->GetDY() ) );
  m_DZcham      = toLHCbLengthUnits( ( box->GetDZ() ) );
  //
}
//
std::optional<DeMuonChamber::PointInGasGap>
DeMuonChamber::checkPointInGasGap( ROOT::Math::XYZPoint GlobalPoint ) const {

  int number = 1;
  //
  for ( const auto& igap : m_gasGaps ) {
    if ( igap.isInside( GlobalPoint ) ) { return PointInGasGap{igap.toLocal( GlobalPoint ), number, &igap}; }
    ++number;
  }
  return {};
}

ROOT::Math::XYZPoint DeMuonChamber::getPosition() const { return ROOT::Math::XYZPoint( m_Xcham, m_Ycham, m_Zcham ); }

std::tuple<ROOT::Math::XYZPoint, ROOT::Math::XYZPoint> DeMuonChamber::getGapPoints( int gap, double LocalX,
                                                                                    double LocalY ) const {
  const auto&  det_gap  = m_gasGaps[gap];
  const double zhalfgap = det_gap.m_DZgap;
  // traslate the xyz local coordinates to global
  //
  const ROOT::Math::XYZPoint pointInLayer1 = ROOT::Math::XYZPoint( LocalX, LocalY, -1 * zhalfgap + 0.1 );
  const ROOT::Math::XYZPoint pointInLayer2 = ROOT::Math::XYZPoint( LocalX, LocalY, zhalfgap - 0.1 );
  const ROOT::Math::XYZPoint point1        = det_gap.toGlobal( pointInLayer1 );
  const ROOT::Math::XYZPoint point2        = det_gap.toGlobal( pointInLayer2 );
  return {point1, point2};
}
//
bool DeMuonChamber::checkHitAndGapInChamber( double GlobalX, double GlobalY ) const {

  // check Hit position in 2D ..
  const auto& det_gap = m_gasGaps[0];
  //  const ROOT::Math::XYZPoint GlobalGapPos =
  //      toGlobal( ROOT::Math::XYZPoint( det_gap.m_Xgap, det_gap.m_Ygap, det_gap.m_Zgap ) );
  //  const double GlobalZ = GlobalGapPos.z();
  const double GlobalZ = det_gap.m_Zgap;
  //
  const ROOT::Math::XYZPoint GlobalPoint = ROOT::Math::XYZPoint( GlobalX, GlobalY, GlobalZ );
  return det_gap.isInside( GlobalPoint );
}
//
bool DeMuonChamber::calculateHitPosInGap( int igap, double xpos, double ypos, double xSlope, double ySlope,
                                          double zavegaps, ROOT::Math::XYZPoint& entryGlobal,
                                          ROOT::Math::XYZPoint& exitGlobal ) const {

  // this gapNumber starts from 0
  dd4hep::printout(
      dd4hep::DEBUG, "DeMuonChamber",
      "calculateHitPosInGap DEBUG: gapNumber :: %i, xpos :: %f, ypos :: %f, xSlope :: %f, ySlope :: %f, zavegaps :: %f",
      igap, xpos, ypos, xSlope, ySlope, zavegaps );
  const auto& det_gap = m_gasGaps[igap];
  //
  const double xhalfgap = det_gap.m_DXgap;
  const double yhalfgap = det_gap.m_DYgap;
  const double zhalfgap = det_gap.m_DZgap;

  ROOT::Math::XYZPoint poslocal = det_gap.toLocal(
      ROOT::Math::XYZPoint( xpos, ypos, zavegaps ) ); // toGlobal( ROOT::Math::XYZPoint( xpos, ypos, zavegaps ) );
  const double         zcenter    = poslocal.z();
  ROOT::Math::XYZPoint slopelocal = ROOT::Math::XYZPoint( xSlope, ySlope, 1.0F );

  const double zinf   = -zhalfgap;
  const double zsup   = +zhalfgap;
  double       xentry = poslocal.x() + ( zinf - zcenter ) * ( slopelocal.x() / slopelocal.z() );
  double       xexit  = poslocal.x() + ( zsup - zcenter ) * ( slopelocal.x() / slopelocal.z() );
  double       yentry = poslocal.y() + ( zinf - zcenter ) * ( slopelocal.y() / slopelocal.z() );
  double       yexit  = poslocal.y() + ( zsup - zcenter ) * ( slopelocal.y() / slopelocal.z() );

  dd4hep::printout(
      dd4hep::DEBUG, "DeMuonChamber",
      "calculateHitPosInGap DEBUG: (xentry,yentry) = (%f,%f) ,(xexit,yexit) = (%f,%f), zinf = %f, zsup = %f", xentry,
      yentry, xexit, yexit, zinf, zsup );

  // check that gap boundaries have not been crossed
  const double xmin = -xhalfgap;
  const double ymin = -yhalfgap;
  const double xmax = xhalfgap;
  const double ymax = yhalfgap;
  double       zmin = -zhalfgap;
  double       zmax = zhalfgap;

  dd4hep::printout( dd4hep::DEBUG, "DeMuonChamber",
                    "calculateHitPosInGap DEBUG: (xmin,ymin,zmin) = (%f,%f,%f) , (xmax,ymax,zmax) = (%f,%f,%f)", xmin,
                    ymin, zmin, xmax, ymax, zmax );

  bool correct = true;
  if ( xentry < xmin || xentry > xmax ) correct = false;
  if ( yentry < ymin || yentry > ymax ) correct = false;
  if ( xexit < xmin || xexit > xmax ) correct = false;
  if ( yexit < ymin || yexit > ymax ) correct = false;
  //
  if ( correct ) {
    // then x
    if ( abs( slopelocal.x() ) < std::numeric_limits<double>::epsilon() ) return false;
    double z1  = ( xmin - poslocal.x() ) / ( slopelocal.x() / slopelocal.z() );
    double z2  = ( xmax - poslocal.x() ) / ( slopelocal.x() / slopelocal.z() );
    double zz1 = std::min( z1, z2 );
    double zz2 = std::max( z1, z2 );
    zmin       = std::max( zmin, zz1 );
    zmax       = std::min( zmax, zz2 );
    // then y
    if ( abs( slopelocal.y() ) < std::numeric_limits<double>::epsilon() ) return false;
    z1   = ( xmin - poslocal.y() ) / ( slopelocal.y() / slopelocal.z() );
    z2   = ( xmax - poslocal.y() ) / ( slopelocal.y() / slopelocal.z() );
    zz1  = std::min( z1, z2 );
    zz2  = std::max( z1, z2 );
    zmin = std::max( zmin, zz1 );
    zmax = std::min( zmax, zz2 );
    if ( zmin > zhalfgap || zmax < -zhalfgap ) return false;
    xentry = poslocal.x() + ( slopelocal.x() / slopelocal.z() ) * ( zmin - zcenter );
    xexit  = poslocal.x() + ( slopelocal.x() / slopelocal.z() ) * ( zmax - zcenter );
    yentry = poslocal.y() + ( slopelocal.y() / slopelocal.z() ) * ( zmin - zcenter );
    yexit  = poslocal.y() + ( slopelocal.y() / slopelocal.z() ) * ( zmax - zcenter );
    //
    // back to the global frame
    entryGlobal = toGlobal( ROOT::Math::XYZPoint( xentry, yentry, zmin ) );
    exitGlobal  = toGlobal( ROOT::Math::XYZPoint( xexit, yexit, zmax ) );

    return true;
  }
  return false;
}
//
double DeMuonChamber::calculateAverageGap( int igapStart, int igapStop, double xpos, double ypos ) const {

  double zaverage = 0;
  //
  for ( int i = igapStart; i <= igapStop; i++ ) {

    const auto& det_gap = m_gasGaps[i];
    //
    // get 3 points to build a plane for the gap center
    ROOT::Math::XYZPoint point1 = det_gap.toGlobal( ROOT::Math::XYZPoint( 0., 0., 0. ) );
    ROOT::Math::XYZPoint point2 = det_gap.toGlobal( ROOT::Math::XYZPoint( 1., 0., 0. ) );
    ROOT::Math::XYZPoint point3 = det_gap.toGlobal( ROOT::Math::XYZPoint( 0., 1., 0. ) );

    ROOT::Math::Plane3D   plane( point1, point2, point3 );
    double                zInterceptThisPlane = 0.;
    ROOT::Math::XYZVector para                = plane.Normal();
    if ( abs( para.Z() ) > std::numeric_limits<double>::epsilon() ) {
      double a            = para.X();
      double b            = para.Y();
      double d            = plane.HesseDistance();
      zInterceptThisPlane = -( a * xpos + b * ypos + d ) / ( para.Z() );
    } else
      zInterceptThisPlane = point1.z();
    zaverage = zaverage + (float)zInterceptThisPlane;
  }
  // countGap++;
  return zaverage / ( igapStop - igapStart + 1 );
}
