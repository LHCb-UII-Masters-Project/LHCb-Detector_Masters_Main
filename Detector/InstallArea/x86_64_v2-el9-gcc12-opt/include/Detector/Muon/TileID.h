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
#pragma once

#include "Detector/Muon/Base.h"
#include "Detector/Muon/Layout.h"

#include <charconv>
#include <fmt/ostream.h>
#include <ostream>
#include <string_view>
#include <utility>

namespace LHCb::Detector::Muon {

  /**
   * Universal identifier for the Muon System entities
   *
   * @author Andrei Tsaregorodtsev
   */
  class TileID final {
  public:
    constexpr TileID() = default;

    /// Constructor taking a long needed for the keyed container
    constexpr explicit TileID( unsigned int id ) : m_muonid{id} {}

    /// Constructor with all the arguments
    constexpr TileID( int station, const Layout& lay, int region, int quarter, int x, int y ) {
      setStation( station );
      setRegion( region );
      setQuarter( quarter );
      setX( x );
      setY( y );
      setLayout( Layout( lay.grid( *this ) ) );
    }

    // SV Aggiunta di Alessia
    constexpr TileID( unsigned int key, const Layout& lay ) {
      setKey( key );
      setLayout( Layout( lay.grid( *this ) ) );
    }

    /// Constructor from similar TileID
    constexpr TileID( TileID id, const unsigned int region, const unsigned int quarter, const unsigned int x,
                      const unsigned int y )
        : m_muonid{id.m_muonid} {
      setQuarter( quarter );
      setRegion( region );
      setX( x );
      setY( y );
    }

    /// Constructor from relative position in the containing MuonTile TileID
    TileID( const TileID& id, const Layout& layout, const unsigned int x, const unsigned int y );

    /// Constructor from a string representation of the TileID
    explicit TileID( std::string_view id );

    /// convert the TileID to an int for use as a key
    constexpr operator unsigned int() const { return m_muonid; }

    /// Function to extract station
    constexpr unsigned int station() const { return ( m_muonid & Base::MaskStation ) >> Base::ShiftStation; }

    /// Function to extract region
    constexpr unsigned int region() const { return ( m_muonid & Base::MaskRegion ) >> Base::ShiftRegion; }

    /// Function to extract quarter
    constexpr unsigned int quarter() const { return ( m_muonid & Base::MaskQuarter ) >> Base::ShiftQuarter; }

    /// Function to extract layout
    Layout layout() const { return {xGrid(), yGrid()}; }

    /// Check if layout is horizontal
    constexpr bool isHorizontal() const { return xGrid() > yGrid(); }

    /// Function to extract index in x
    constexpr unsigned int nX() const { return ( m_muonid & Base::MaskX ) >> Base::ShiftX; }

    /// Function to extract index in y
    constexpr unsigned int nY() const { return ( m_muonid & Base::MaskY ) >> Base::ShiftY; }

    /// Function to extract lower part of the identifier
    constexpr unsigned int index() const { return ( m_muonid & Base::MaskIndex ) >> Base::ShiftIndex; }

    /// Function to extract lower part of the identifier with station information
    constexpr unsigned int key() const { return ( m_muonid & Base::MaskKey ) >> Base::ShiftKey; }

    /// comparison operator using key.
    friend constexpr bool operator<( TileID lhs, TileID rhs ) { return lhs.key() < rhs.key(); }

    /// equality operator using key.
    friend constexpr bool operator==( TileID lhs, TileID rhs ) { return lhs.m_muonid == rhs.m_muonid; }

    /// non-equality operator using key.
    friend constexpr bool operator!=( TileID lhs, TileID rhs ) { return !( lhs == rhs ); }

    /// Find the TileID which is an interception of two TileID's
    TileID intercept( TileID id ) const;

    /// Find the TileID which is an interception of two TileID's
    TileID interceptSameRegion( TileID id ) const;

    /// Find the TileID of a Tile of a given layout containing this pad
    TileID containerID( const Layout& layout ) const;

    /// Find the TileID of a Tile which is a neighbour of this pad
    TileID neighbourID( int dirX, int dirY ) const;

    /// Get local offset X with respect to the container defined by the given Layout
    int localX( const Layout& layout ) const;

    /// Get local offset y with respect to the container defined by the given Layout
    int localY( const Layout& layout ) const;

    /// Check that the TileID is consistent in terms of its layout
    constexpr bool isValid() const;

    /// Check that the TileID is defined. It means that its code is not 0
    constexpr bool isDefined() const { return m_muonid != 0; }

    /// presents the TileID as a readable string
    std::string toString() const;

    /// update station identifier
    constexpr TileID& setStation( const unsigned int station ) {
      return set( station, Base::ShiftStation, Base::MaskStation );
    }

    /// update region identifier
    constexpr TileID& setRegion( const unsigned int region ) {
      return set( region, Base::ShiftRegion, Base::MaskRegion );
    }

    /// update quarter identifier
    constexpr TileID& setQuarter( const unsigned int quarter ) {
      return set( quarter, Base::ShiftQuarter, Base::MaskQuarter );
    }

    /// update index in x
    constexpr TileID& setX( const unsigned int x ) { return set( x, Base::ShiftX, Base::MaskX ); }

    /// update index in y
    constexpr TileID& setY( const unsigned int y ) { return set( y, Base::ShiftY, Base::MaskY ); }

    /// SV aggiunta da Alessia
    /// update index
    constexpr TileID& setKey( const unsigned int key ) { return set( key, Base::ShiftKey, Base::MaskKey ); }

    /// update layout identifier
    constexpr TileID& setLayout( const Layout& layout ) { return setLayout( layout.xGrid(), layout.yGrid() ); }

    /// modify index in x
    constexpr TileID& deltaX( int dx ) { return setX( nX() + dx ); }

    /// modify index in y
    constexpr TileID& deltaY( int dy ) { return setY( nY() + dy ); }

    /// Print this TileID in a human readable way
    std::ostream& fillStream( std::ostream& s ) const;

    friend std::ostream& operator<<( std::ostream& str, const TileID& obj ) { return obj.fillStream( str ); }

  private:
    constexpr TileID& set( unsigned int Value, unsigned int Shift, unsigned int Mask ) {
      m_muonid = ( ( Value << Shift ) & Mask ) | ( m_muonid & ~Mask );
      return *this;
    }
    constexpr unsigned xGrid() const { return ( m_muonid & Base::MaskLayoutX ) >> Base::ShiftLayoutX; }
    constexpr unsigned yGrid() const { return ( m_muonid & Base::MaskLayoutY ) >> Base::ShiftLayoutY; }

    /// update layout identifier
    constexpr TileID& setLayout( unsigned lx, unsigned ly ) {
      set( ( ly << Base::BitsLayoutX ) | lx, Base::ShiftLayoutX, Base::MaskLayoutY | Base::MaskLayoutX );
      return *this;
    }

    unsigned int m_muonid{0}; ///< muon tile id

  }; // class TileID

} // namespace LHCb::Detector::Muon

// -----------------------------------------------------------------------------
// end of class
// -----------------------------------------------------------------------------

inline LHCb::Detector::Muon::TileID::TileID( std::string_view s ) {

  // assumed format of strid:  "S{}({},{})Q{},R{},{},{}"

  constexpr auto process = []( std::string_view prefix, std::string_view sv ) {
    if ( sv.substr( 0, prefix.size() ) != prefix ) throw std::invalid_argument{"MuonTileID: bad format"};
    sv.remove_prefix( prefix.size() );
    long l;
    auto [ptr, ec] = std::from_chars( sv.begin(), sv.end(), l );
    if ( ec != std::errc{} ) throw std::invalid_argument{"MuonTileID: bad format"};
    return std::pair{sv.substr( ptr - sv.begin() ), l};
  };

  auto p = process( "S", s );
  setStation( p.second );
  p            = process( "(", p.first );
  unsigned mlx = p.second;
  p            = process( ",", p.first );
  unsigned mly = p.second;
  setLayout( Layout{mlx, mly} );
  p = process( ")Q", p.first );
  setQuarter( p.second );
  p = process( ",R", p.first );
  setRegion( p.second );
  p = process( ",", p.first );
  setX( p.second );
  p = process( ",", p.first );
  setY( p.second );
}

constexpr bool LHCb::Detector::Muon::TileID::isValid() const {

  if ( !isDefined() ) return false;
  auto nx = nX();
  auto ny = nY();
  auto xg = xGrid();
  auto yg = yGrid();
  return ( ( ny >= yg && ny < 2 * yg ) || ( nx >= xg && ny < yg ) ) && nx < 2 * xg;
}

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<LHCb::Detector::Muon::TileID> : ostream_formatter {};
#endif
