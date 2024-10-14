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

#include <iostream>
#include <vector>

namespace LHCb::Detector::Muon {

  class TileID;

  /**
   *  Defines a Muon station single plane logical layout. Layouts in
   *  all the regions of the station are the same with the scaling factor 2
   *  when passing from a region to a larger one. The class also implements
   *  various layout/tiles manipulation functions.
   *
   *  @author A.Tsaregorodtsev
   *  @date 6 April 2001
   */
  class Layout final {

  public:
    constexpr Layout() = default;
    /**
     * Constructor taking X and Y granularities
     * @param   xgrid  granularity in X
     * @param   ygrid  granularity in Y
     */
    constexpr Layout( unsigned int xgrid, unsigned int ygrid ) : m_xgrid{xgrid}, m_ygrid{ygrid} {}
    /**
     * Constructor taking X and Y granularities as std::pair
     * @param   xygrid  granularity in X
     */
    constexpr Layout( std::pair<unsigned int, unsigned int> xygrid ) : Layout{xygrid.first, xygrid.second} {}

    /// Accessor to Layout grid corresponding to the given TileID
    constexpr std::pair<unsigned int, unsigned int> grid( const TileID& ) const { return {xGrid(), yGrid()}; }

    /// Accessor to X granularity
    constexpr unsigned int xGrid() const { return m_xgrid; }
    /// Accessor to Y granularity
    constexpr unsigned int yGrid() const { return m_ygrid; }

    /**
     * find a vector of TileID's defined in terms of this Layout
     * which are touched by an area around a given TileID defined
     * in its own Layout
     *
     * @param pad   :  tile defining the area covered
     */
    std::vector<TileID> tiles( const TileID& pad ) const;

    /**
     * find a vector of TileID's defined in terms of this Layout
     * which are touched by an area around a given TileID defined
     * in its own Layout
     *
     * @param pad   : central for the search
     * @param areaX : limits of the search area in X
     * @param areaY : limits of the search area in Y
     */
    std::vector<TileID> tilesInArea( const TileID& pad, int areaX, int areaY ) const;

    /// returns a vector of its TileID's.
    std::vector<TileID> tiles() const;

    /**
     * returns a vector of its TileID's in a given quarter
     *
     * @param quarter : the quarter number to look into
     */
    std::vector<TileID> tiles( int quarter ) const;

    /**
     * returns a vector of its TileID's in a given quarter and region
     *
     * @param quarter : the quarter number to look into
     * @param region : the region number to look into
     */
    std::vector<TileID> tiles( int quarter, int region ) const;

    /// find a tile containing the argument tile
    TileID contains( const TileID& pad ) const;

    /// Get tiles touched by pad defined in terms of pregion region number
    std::vector<TileID> tilesInRegion( const TileID& pad, int pregion ) const;

    /// find all the TileID's which are neighbours of the argument tile
    std::vector<TileID> neighbours( const TileID& pad ) const;

    /**
     * find all the TileID's which are neighbours of the argument tile
     * in the specified direction.
     * @param   pad   find the neighbours of this tile
     * @param   dirX  horizontal direction in which to look for neighbours
     * @param   dirY  vertical direction in which to look for neighbours
     */
    std::vector<TileID> neighbours( const TileID& pad, int dirX, int dirY ) const;

    /**
     * find all the TileID's which are neighbours of the argument tile
     * in the specified direction.
     * @param   pad   find the neighbours of this tile
     * @param   dirX  horizontal direction in which to look for neighbours
     * @param   dirY  vertical direction in which to look for neighbours
     * @param   depth depth of the band in which to look for neighbours
     */
    std::vector<TileID> neighbours( const TileID& pad, int dirX, int dirY, int depth ) const;

    /**
     * find all the TileID's which are neighbours of the argument tile
     * in the specified direction. This version takes into account that
     * area to look in can take two parameters to specify the depth of the
     * search in cases of neighbours at the corners, i.e. LEFT-UP.
     * @param   pad   find the neighbours of this tile
     * @param   dirX  horizontal direction in which to look for neighbours
     * @param   dirY  vertical direction in which to look for neighbours
     * @param   depthX depth of the band in which to look for neighbours
     * @param   depthY depth of the band in which to look for neighbours
     */
    std::vector<TileID> neighboursInArea( const TileID& pad, int dirX, int dirY, int depthX, int depthY ) const;

    /// check if the given TileID is valid for this layout
    bool isValidID( const TileID& mt ) const;

    /// check if the layout itself is defined
    bool isDefined() const;

    /// printout to std::ostream
    inline std::ostream& printOut( std::ostream& ) const;

  private:
    /// find magnification factor of pads in the given region
    int rfactor( unsigned int nr ) const { return 1 << nr; }

    /**
     * find region for the given pad indices. The pad indices
     * are given in terms of the most inner region. If the pad indices
     * are not fitting into the layout, the result returned is -1
     */
    int region( unsigned int bx, unsigned int by ) const;

    // output to std::ostream
    friend inline std::ostream& operator<<( std::ostream& os, const Layout& id ) { return id.printOut( os ); }

    friend constexpr bool operator==( const Layout& ml1, const Layout& ml2 ) {
      return ml1.xGrid() == ml2.xGrid() && ml1.yGrid() == ml2.yGrid();
    }
    friend constexpr bool operator!=( const Layout& ml1, const Layout& ml2 ) { return !( ml1 == ml2 ); }

  private:
    unsigned int m_xgrid = 0;
    unsigned int m_ygrid = 0;
  };

} // namespace LHCb::Detector::Muon

inline std::ostream& LHCb::Detector::Muon::Layout::printOut( std::ostream& os ) const {
  os << "(";
  return os << xGrid() << "," << yGrid() << ")";
}
