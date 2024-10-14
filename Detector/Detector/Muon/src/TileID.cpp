/*****************************************************************************\
* (c) Copyright 2000-2021 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/

#include "Detector/Muon/TileID.h"
#include "Detector/Muon/Layout.h"

#include "fmt/format.h"

#include <numeric>

using namespace LHCb::Detector::Muon;

TileID::TileID( const TileID& id, const Layout& lay, const unsigned int x, const unsigned int y ) {

  m_muonid = id.m_muonid;
  Layout this_layout( lay.grid( id ) );
  setLayout( this_layout );
  setX( this_layout.xGrid() / id.layout().xGrid() * id.nX() + x );
  setY( this_layout.yGrid() / id.layout().yGrid() * id.nY() + y );
}

TileID TileID::neighbourID( int dirX, int dirY ) const {
  TileID result( *this );
  switch ( dirX ) {
  case Base::RIGHT:
    result.setX( nX() + 1 );
    break;
  case Base::LEFT:
    if ( nX() == 0 ) break;
    result.setX( nX() - 1 );
    break;
  }
  switch ( dirY ) {
  case Base::UP:
    result.setY( nY() + 1 );
    break;
  case Base::DOWN:
    if ( nY() == 0 ) break;
    result.setY( nY() - 1 );
    break;
  }
  return result;
}

TileID TileID::intercept( TileID otherID ) const {

  // check first that the two strips are really intercepting

  if ( station() != otherID.station() || quarter() != otherID.quarter() ) return TileID();

  int thisGridX = layout().xGrid();
  int thisGridY = layout().yGrid();

  int otherGridX = otherID.layout().xGrid();
  int otherGridY = otherID.layout().yGrid();
  if ( otherID.region() > region() ) {
    int rfactor = ( 1 << otherID.region() ) / ( 1 << region() );
    otherGridX  = otherID.layout().xGrid() / rfactor;
    otherGridY  = otherID.layout().yGrid() / rfactor;
  } else if ( otherID.region() < region() ) {
    int rfactor = ( 1 << region() ) / ( 1 << otherID.region() );
    otherGridX  = otherID.layout().xGrid() * rfactor;
    otherGridY  = otherID.layout().yGrid() * rfactor;
  }

  if ( thisGridX > otherGridX ) {
    unsigned int calcX = nX() * otherGridX / thisGridX;
    if ( calcX != otherID.nX() ) return TileID();
  } else {
    unsigned int calcX = otherID.nX() * thisGridX / otherGridX;
    if ( calcX != nX() ) return TileID();
  }
  if ( thisGridY > otherGridY ) {
    unsigned int calcY = nY() * otherGridY / thisGridY;
    if ( calcY != otherID.nY() ) return TileID();
  } else {
    unsigned int calcY = otherID.nY() * thisGridY / otherGridY;
    if ( calcY != nY() ) return TileID();
  }

  // Now the strips are intercepting - get it !

  int indX = thisGridX < otherGridX ? otherID.nX() : nX();
  int indY = thisGridY < otherGridY ? otherID.nY() : nY();

  TileID resultID( *this );
  resultID.setX( indX );
  resultID.setY( indY );
  int lx = std::max( thisGridX, otherGridX );
  int ly = std::max( thisGridY, otherGridY );
  resultID.setLayout( Layout( lx, ly ) );

  return resultID;
}

TileID TileID::interceptSameRegion( TileID otherID ) const {

  // check first that the two strips are really intercepting

  if ( station() != otherID.station() || quarter() != otherID.quarter() ) return TileID();

  int thisGridX = layout().xGrid();
  int thisGridY = layout().yGrid();

  int          otherGridX = otherID.layout().xGrid();
  int          otherGridY = otherID.layout().yGrid();
  unsigned int calcX      = nX() * otherGridX / thisGridX;
  if ( calcX != otherID.nX() ) return TileID();
  if ( thisGridY > otherGridY ) {
    unsigned int calcY = nY() * otherGridY / thisGridY;
    if ( calcY != otherID.nY() ) return TileID();
  } else {
    unsigned int calcY = otherID.nY() * thisGridY / otherGridY;
    if ( calcY != nY() ) return TileID();
  }

  // Now the strips are intercepting - get it !

  int indX = thisGridX < otherGridX ? otherID.nX() : nX();
  int indY = thisGridY < otherGridY ? otherID.nY() : nY();

  TileID resultID( *this );
  resultID.setX( indX );
  resultID.setY( indY );
  int lx = std::max( thisGridX, otherGridX );
  int ly = std::max( thisGridY, otherGridY );
  resultID.setLayout( Layout( lx, ly ) );

  return resultID;
}

TileID TileID::containerID( const Layout& lay ) const {

  Layout containerLayout( lay.grid( *this ) );
  TileID containerID( *this );
  containerID.setX( nX() * containerLayout.xGrid() / layout().xGrid() );
  containerID.setY( nY() * containerLayout.yGrid() / layout().yGrid() );
  containerID.setLayout( containerLayout );

  return containerID;
}

int TileID::localX( const Layout& lay ) const {

  Layout padLayout( lay.grid( *this ) );
  return nX() % padLayout.xGrid();
}

int TileID::localY( const Layout& lay ) const {

  Layout padLayout( lay.grid( *this ) );
  return nY() % padLayout.yGrid();
}

std::string TileID::toString() const {
  return fmt::format( "S{}({},{})Q{},R{},{},{}", station(), layout().xGrid(), layout().yGrid(), quarter(), region(),
                      nX(), nY() );
}

std::ostream& TileID::fillStream( std::ostream& s ) const {
  s << "{ ";
  if ( !isValid() ) {
    s << "WARNING : Unvalid MuonTileID";
  } else if ( !isDefined() ) {
    s << "WARNING : Undefined MuonTileID";
  } else {
    s << "MuonTileID : " << (int)( *this ) << " : station=" << station() << " region=" << region()
      << " quarter=" << quarter() << " nX=" << nX() << " nY=" << nY();
  }
  return s << " }";
}
