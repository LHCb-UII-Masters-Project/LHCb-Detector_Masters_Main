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
#include "DD4hep/DD4hepUnits.h"
#include "Math/Point3D.h"
#include "Math/SMatrix.h"
#include "Math/Vector3D.h"
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

namespace LHCb::Magnet {

  class MagneticFieldGridReader;

  /** @class MagneticFieldGrid MagneticFieldGrid.h
   *  Simple class to hold a magnetic field grid.
   *
   *  @author Marco Cattaneo, Wouter Hulsbergen
   *  @date   2008-07-26
   */
  class MagneticFieldGrid final {

  public:
    /// typedefs etc
    using FieldVector   = ROOT::Math::XYZVector;
    using FieldGradient = ROOT::Math::SMatrix<double, 3, 3>;

    /// declare the field reader as friend so that we don't have to make a constructor
    friend class MagneticFieldGridReader;

    /// Return the field vector fvec at the point xyz by interpolation
    /// on the grid.
    FieldVector fieldVectorLinearInterpolation( const ROOT::Math::XYZPoint& xyz ) const;

    /// Return the field vector fvec at the point xyz by interpolation
    /// on the grid.
    FieldGradient fieldGradient( const ROOT::Math::XYZPoint& xyz ) const;

    /// Return the field vector fvec at the point xyz by choosing the
    /// closest point on the grid.
    FieldVector fieldVectorClosestPoint( const ROOT::Math::XYZPoint& xyz ) const;

    /// Return the magnetic field scale factor
    auto scaleFactor() const noexcept { return m_scaleFactor[0]; }

    /// Update the scale factor
    void setScaleFactor( const float s ) { m_scaleFactor = {s, s, s, 0}; }

  public:
    using IndexType = std::int32_t;

  private:
    /// Vector version of the scale factor, padded to 4 elements
    std::array<float, 4> m_scaleFactor = {1, 1, 1, 0};
    /// Vectorised Field map, padded to 4 elements per vector
    std::vector<std::array<float, 4>> m_B;
    /// Offset in x, y and z, padded to 4 elements
    std::array<float, 4> m_min = {0., 0., 0., 0.};
    /// Inverse of steps in x, y and z (cached for speed), padded to 4 elements
    std::array<float, 4> m_invDxyz = {0., 0., 0., 0.};
    /// Number of steps in x, y and z, padded to 4 elements
    std::array<IndexType, 4> m_Nxyz = {0, 0, 0, 0};
  };

} // namespace LHCb::Magnet
