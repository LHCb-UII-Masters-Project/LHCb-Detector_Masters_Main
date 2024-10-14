/*****************************************************************************\
* (c) Copyright 2000-2018 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once

#include "Core/TaggedBool.h"

#include "Math/Point3D.h"
#include "Math/Vector3D.h"

#include <boost/optional.hpp>
#include <memory>
#include <utility>

namespace LHCb::Detector {
  /**
   * This is the base class for the trajectory classes. 'mu' is the expansion
   * parameter. For the existing trajectories it is either the arclength or z.
   *
   * @author Edwin Bos, Jeroen van Tilburg, Eduardo Rodrigues
   * @date   01/12/2005
   *
   */
  template <typename FTYPE = double>
  class Trajectory {
  public:
    using DirNormalized = tagged_bool<struct DirNormalized_tag>;
    using Range         = std::pair<FTYPE, FTYPE>;
    using Point         = typename ROOT::Math::PositionVector3D<ROOT::Math::Cartesian3D<FTYPE>,
                                                        ROOT::Math::DefaultCoordinateSystemTag>; ///< 3D cartesian point
    using Vector =
        typename ROOT::Math::DisplacementVector3D<ROOT::Math::Cartesian3D<FTYPE>,
                                                  ROOT::Math::DefaultCoordinateSystemTag>; ///< Cartesian 3D vector

    /// Clone a trajectory...
    virtual std::unique_ptr<Trajectory<FTYPE>> clone() const = 0;

    Trajectory() = default;

    /// Constructor taking the values of mu that defined the valid range of the trajectory
    Trajectory( FTYPE begin, FTYPE end ) : m_range( Range( begin, end ) ) {}

    /// constructer taking a range
    Trajectory( const Range& range ) : m_range( range ) {}

    /// destructor
    virtual ~Trajectory() = default;

    /// Point on the trajectory at position mu from the starting point
    virtual Point position( FTYPE mu ) const = 0;

    /// Beginpoint of the Trajectory
    Point beginPoint() const { return position( beginRange() ); }

    /// Endpoint of the Trajectory
    Point endPoint() const { return position( endRange() ); }

    /// First derivative of the position to mu. Note: not normalized!
    virtual Vector direction( FTYPE mu ) const = 0;

    /// Second derivative of the position to mu. Note: not normalized!
    virtual Vector curvature( FTYPE mu ) const = 0;

    /// Create a parabolic approximation to the trajectory at position mu
    virtual void expansion( FTYPE mu, Point& p, Vector& dp, Vector& ddp ) const = 0;

    /// Estimate the value mu that corresponds to the closest point on the
    /// trajectory to a given point. The position corresponding to mu may differ
    /// from the exact point of closest approach for trajectories with non-zero
    /// curvature.
    virtual FTYPE muEstimate( const Point& ) const = 0;

    /// Distance in mu until the deviation from the linear approximation differs
    /// from this trajectory by a given tolerance.
    virtual FTYPE distTo1stError( FTYPE mu, FTYPE tolerance, int pathDirection = +1 ) const = 0;

    /// Distance in mu until the deviation from the quadratic approximation differs
    /// from this trajectory by a given tolerance.
    virtual FTYPE distTo2ndError( FTYPE mu, FTYPE tolerance, int pathDirection = +1 ) const = 0;

    /// Range of expansion parameter over which the trajectory is defined
    Range range() const { return m_range; }

    /// Maximum value of mu for which the trajectory is defined
    FTYPE endRange() const { return range().second; }

    /// Minimum value of mu for which the trajectory is defined
    FTYPE beginRange() const { return range().first; }

    /// Distance, along the Trajectory, between Trajectory::position(mu1) and
    /// Trajectory::position(mu2)
    virtual FTYPE arclength( FTYPE mu1, FTYPE mu2 ) const = 0;

    // Arclength of valid range
    virtual FTYPE arclength() const { return arclength( beginRange(), endRange() ); }

    /// Set the range
    void setRange( FTYPE begin, FTYPE end ) {
      m_range.first  = begin;
      m_range.second = end;
    }

    /// obsolete, must be removed/renamed in inherited classes/clients
    FTYPE length() const { return arclength(); };
    /// obsolete, must be removed/renamed in inherited classes/clients
    FTYPE arclength( const Point& point ) const { return muEstimate( point ); }

  protected:
    Range m_range;

  }; // class Trajectory

} // namespace LHCb::Detector
