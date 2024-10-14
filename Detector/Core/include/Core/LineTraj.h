/*****************************************************************************\
* (c) Copyright 2000-2020 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once

#include "Core/Trajectory.h"

#include "DD4hep/DD4hepUnits.h"

namespace LHCb::Detector {

  template <typename FTYPE, typename VECTYPE>
  inline auto updir( bool dnorm, VECTYPE dir ) ->
      typename std::enable_if<not std::is_floating_point<FTYPE>::value, VECTYPE>::type {
    // Unit() function from ROOT not compatible with VCL
    if ( dnorm ) return dir;

    auto tot = dir.R();
    tot      = select( tot == 0, 1, tot );
    return dir * ( 1. / tot );
  }

  template <typename FTYPE, typename VECTYPE>
  inline auto updir( bool dnorm, VECTYPE dir ) ->
      typename std::enable_if<std::is_floating_point<FTYPE>::value, VECTYPE>::type {
    return dnorm ? dir : dir.Unit();
  }

  /** @class LineTraj LineTraj.h Kernel/LineTraj.h
   *
   * Trajectory parameterized as a straight line
   *
   * @author Edwin Bos, Jeroen van Tilburg, Eduardo Rodrigues
   * @date   17/01/2005
   *
   */

  template <typename FTYPE>
  class LineTraj : public Trajectory<FTYPE> {
  public:
    using DirNormalized = typename Trajectory<FTYPE>::DirNormalized;
    using Trajectory<FTYPE>::Trajectory;

    using Vector = typename Trajectory<FTYPE>::Vector;
    using Point  = typename Trajectory<FTYPE>::Point;
    using Range  = typename Trajectory<FTYPE>::Range;

    // clone thyself...
    std::unique_ptr<Trajectory<FTYPE>> clone() const override { return std::make_unique<LineTraj<FTYPE>>( *this ); };

    // Constructor from the middle point and a direction vector
    // and which takes either a normalized direction vector or not....
    LineTraj( const Point& middle, const Vector& dir, const Range& range, DirNormalized dnorm = DirNormalized{true} )
        : Trajectory<FTYPE>( range )
        ,
        // m_dir(dnorm ? dir : dir.Unit()),
        m_pos( middle ) {
      m_dir = updir<FTYPE, Vector>( dnorm == DirNormalized{true}, dir );
      // Raw unit
      // m_dir = dnorm == DirNormalized{true} ? dir : dir.Unit();
    }

    /// Constructor from a begin and an end point
    LineTraj( const Point& beg, const Point& end );

    /// Point on the trajectory at arclength from the starting point
    Point position( FTYPE arclength ) const override;

    /// First derivative of the trajectory at arclength from the starting point
    Vector direction( FTYPE arclength ) const override;

    /// Second derivative of the trajectory at arclength from the starting point
    Vector curvature( FTYPE arclength ) const override;

    /// Create a parabolic approximation to the trajectory
    /// at arclength from the starting point
    void expansion( FTYPE arclength, Point& p, Vector& dp, Vector& ddp ) const override;

    /// Determine the distance in arclenghts to the
    /// closest point on the trajectory to a given point
    FTYPE muEstimate( const Point& point ) const override;

    /// Number of arclengths until deviation of the trajectory from the
    /// expansion reaches the given tolerance.
    FTYPE distTo1stError( FTYPE arclength, FTYPE tolerance, int pathDirection = +1 ) const override;

    /// Number of arclengths until deviation of the trajectory from the
    /// expansion reaches the given tolerance.
    FTYPE distTo2ndError( FTYPE arclength, FTYPE tolerance, int pathDirection = +1 ) const override;

    /// Distance, along the Trajectory, between position(mu1) and
    /// position(mu2). Trivial because LineTraj is parameterized in
    /// arclength.
    using Trajectory<FTYPE>::arclength;
    FTYPE arclength( FTYPE mu1, FTYPE mu2 ) const override { return mu2 - mu1; }

  private:
    Vector m_dir;
    Point  m_pos;

  }; // class LineTraj

  /// Constructor from a begin and an end point
  template <typename FTYPE>
  inline LineTraj<FTYPE>::LineTraj( const typename Trajectory<FTYPE>::Point& beg,
                                    const typename Trajectory<FTYPE>::Point& end )
      : m_dir( end - beg ), m_pos( beg + 0.5 * m_dir ) {
    Vector p   = end - beg;
    FTYPE  val = sqrt( p.Mag2() ) / 2.;
    this->setRange( -1. * val, val );
    m_dir = m_dir.Unit();
  }

  /// Point on the trajectory at arclength from the starting point
  template <typename FTYPE>
  inline typename Trajectory<FTYPE>::Point LineTraj<FTYPE>::position( FTYPE arclength ) const {
    return m_pos + arclength * m_dir;
  }

  /// First derivative of the trajectory at arclength from the starting point
  template <typename FTYPE>
  inline typename Trajectory<FTYPE>::Vector LineTraj<FTYPE>::direction( FTYPE /* arclength*/ ) const {
    return m_dir;
  }

  /// Second derivative of the trajectory at arclength from the starting point
  template <typename FTYPE>
  inline typename Trajectory<FTYPE>::Vector LineTraj<FTYPE>::curvature( FTYPE /* arclength */ ) const {
    return {0, 0, 0};
  }

  /// Create a parabolic approximation to the trajectory
  /// at arclength from the starting point
  template <typename FTYPE>
  inline void LineTraj<FTYPE>::expansion( FTYPE arclength, typename Trajectory<FTYPE>::Point& p,
                                          typename Trajectory<FTYPE>::Vector& dp,
                                          typename Trajectory<FTYPE>::Vector& ddp ) const {
    ddp = {0, 0, 0};
    dp  = m_dir;
    p   = m_pos + arclength * m_dir;
  }

  /// Determine the distance in arclenghts to the
  /// closest point on the trajectory to a given point
  template <typename FTYPE>
  inline FTYPE LineTraj<FTYPE>::muEstimate( const typename Trajectory<FTYPE>::Point& point ) const {
    return m_dir.Dot( point - m_pos );
  }

  // 1st order approx OK everywhere
  template <typename FTYPE>
  inline FTYPE LineTraj<FTYPE>::distTo1stError( FTYPE, FTYPE, int ) const {
    return 10 * dd4hep::km * 10; // 10 km in LHCb units as dd4hep works with cm while LHCb uses mm
  }

  // 2nd order approx OK everywhere
  template <typename FTYPE>
  inline FTYPE LineTraj<FTYPE>::distTo2ndError( FTYPE, FTYPE, int ) const {
    return 10 * dd4hep::km * 10; // 10 km in LHCb units as dd4hep works with cm while LHCb uses mm
  }

} // namespace LHCb::Detector
