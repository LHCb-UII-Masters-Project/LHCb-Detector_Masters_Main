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

#include "Core/MagneticFieldGrid.h"
#include <Vc/Vc>

//-----------------------------------------------------------------------------
// Implementation file for class : MagneticFieldGrid
//
// 2008-07-26 : Marco Cattaneo
// 2009-06-05 : Wouter Hulsbergen
//-----------------------------------------------------------------------------

//=============================================================================
// Return the field vector fvec at the point xyz
//=============================================================================

namespace LHCb::Magnet {

  MagneticFieldGrid::FieldVector MagneticFieldGrid::fieldVectorClosestPoint( const ROOT::Math::XYZPoint& r ) const {
    const IndexType i = ( r.x() - m_min[0] ) * m_invDxyz[0] + 0.5f;
    const IndexType j = ( r.y() - m_min[1] ) * m_invDxyz[1] + 0.5f;
    const IndexType k = ( r.z() - m_min[2] ) * m_invDxyz[2] + 0.5f;
    if ( i >= 0 && i < m_Nxyz[0] && //
         j >= 0 && j < m_Nxyz[1] && //
         k >= 0 && k < m_Nxyz[2] ) {
      const auto index = m_Nxyz[0] * ( m_Nxyz[1] * k + j ) + i;
      const auto vx    = m_B[index][0] * scaleFactor();
      const auto vy    = m_B[index][1] * scaleFactor();
      const auto vz    = m_B[index][2] * scaleFactor();
      return {vx, vy, vz};
    }
    return {0, 0, 0};
  }

  MagneticFieldGrid::FieldGradient MagneticFieldGrid::fieldGradient( const ROOT::Math::XYZPoint& r ) const {
    FieldGradient   _rc;
    const IndexType i = ( r.x() - m_min[0] ) * m_invDxyz[0];
    const IndexType j = ( r.y() - m_min[1] ) * m_invDxyz[1];
    const IndexType k = ( r.z() - m_min[2] ) * m_invDxyz[2];
    if ( i >= 0 && i < m_Nxyz[0] && //
         j >= 0 && j < m_Nxyz[1] && //
         k >= 0 && k < m_Nxyz[2] ) {
      const auto ijk000 = ( m_Nxyz[0] * ( m_Nxyz[1] * k + j ) + i );
      const auto dijk0  = 1;
      const auto dijk1  = m_Nxyz[0];
      const auto dijk2  = m_Nxyz[0] * m_Nxyz[1];
      for ( int row = 0; row < 3; ++row ) {
        _rc( row, 0 ) = ( m_B[ijk000 + dijk0][row] - m_B[ijk000][row] ) * scaleFactor() * m_invDxyz[0];
        _rc( row, 1 ) = ( m_B[ijk000 + dijk1][row] - m_B[ijk000][row] ) * scaleFactor() * m_invDxyz[1];
        _rc( row, 2 ) = ( m_B[ijk000 + dijk2][row] - m_B[ijk000][row] ) * scaleFactor() * m_invDxyz[2];
      }
    }
    return _rc;
  }

  MagneticFieldGrid::FieldVector
  MagneticFieldGrid::fieldVectorLinearInterpolation( const ROOT::Math::XYZPoint& r ) const {
#ifdef __x86_64__
    // for x86_64, we use sse vectorization

    // Linear interpolate the field on a cube
    const float abc0 = ( (float)r.x() - m_min[0] ) * m_invDxyz[0];
    const float abc1 = ( (float)r.y() - m_min[1] ) * m_invDxyz[1];
    const float abc2 = ( (float)r.z() - m_min[2] ) * m_invDxyz[2];
    using float_4v   = Vc::Vector<float, Vc::VectorAbi::Sse>;
    float_4v abc( abc0 );
    abc[1] = abc1;
    abc[2] = abc2;

    // get field map indices
    const auto _i = static_cast<IndexType>( abc0 );
    const auto _j = static_cast<IndexType>( abc1 );
    const auto _k = static_cast<IndexType>( abc2 );

    // bounds check
    if ( _i >= 0 && _i < m_Nxyz[0] - 1 && //
         _j >= 0 && _j < m_Nxyz[1] - 1 && //
         _k >= 0 && _k < m_Nxyz[2] - 1 ) {

      // auxiliary variables defined at the vertices of the cube that
      // contains the (x, y, z) point where the field is interpolated
      const auto idx    = [&]( auto i, auto j, auto k ) { return m_Nxyz[0] * ( m_Nxyz[1] * k + j ) + i; };
      const auto ijk000 = idx( _i, _j, _k );
      const auto ijk001 = idx( _i, _j, _k + 1 );
      const auto ijk010 = idx( _i, _j + 1, _k );
      const auto ijk011 = idx( _i, _j + 1, _k + 1 );

      float_4v ijk( _i );
      ijk[1]        = _j;
      ijk[2]        = _k;
      const auto h1 = abc - ijk;
      const auto h0 = float_4v( 1.0f ) - h1;

      const auto h1x( h1[0] ), h1y( h1[1] ), h1z( h1[2] );
      const auto h0x( h0[0] ), h0y( h0[1] ), h0z( h0[2] );

      const float_4v h00 = h0x * h0y;
      const float_4v h01 = h0x * h1y;
      const float_4v h10 = h1x * h0y;
      const float_4v h11 = h1x * h1y;

      const float_4v bf_V = //
          float_4v( m_scaleFactor.data() ) *
          ( h0z * ( ( h00 * float_4v( m_B[ijk000].data() ) + h10 * float_4v( m_B[ijk000 + 1].data() ) ) +   //
                    ( h01 * float_4v( m_B[ijk010].data() ) + h11 * float_4v( m_B[ijk010 + 1].data() ) ) ) + //
            h1z * ( ( h00 * float_4v( m_B[ijk001].data() ) + h10 * float_4v( m_B[ijk001 + 1].data() ) ) +   //
                    ( h01 * float_4v( m_B[ijk011].data() ) + h11 * float_4v( m_B[ijk011 + 1].data() ) ) ) );
      return {bf_V[0], bf_V[1], bf_V[2]};
    }

#else
    // for other platforms, we fall back to scalar implementation

    // Linear interpolate the field on a cube
    const std::array<float, 3> abc = {(float)( r.x() - m_min[0] ) * m_invDxyz[0],
                                      (float)( r.y() - m_min[1] ) * m_invDxyz[1],
                                      (float)( r.z() - m_min[2] ) * m_invDxyz[2]};

    const auto i = static_cast<IndexType>( abc[0] );
    const auto j = static_cast<IndexType>( abc[1] );
    const auto k = static_cast<IndexType>( abc[2] );

    if ( i >= 0 && i < m_Nxyz[0] - 1 && //
         j >= 0 && j < m_Nxyz[1] - 1 && //
         k >= 0 && k < m_Nxyz[2] - 1 ) {

      // auxiliary variables defined at the vertices of the cube that
      // contains the (x, y, z) point where the field is interpolated
      const auto ijk000 = m_Nxyz[0] * ( m_Nxyz[1] * k + j ) + i;
      const auto ijk001 = m_Nxyz[0] * ( m_Nxyz[1] * ( k + 1 ) + j ) + i;
      const auto ijk010 = m_Nxyz[0] * ( m_Nxyz[1] * k + j + 1 ) + i;
      const auto ijk011 = m_Nxyz[0] * ( m_Nxyz[1] * ( k + 1 ) + j + 1 ) + i;

      const auto h1x = abc[0] - i;
      const auto h1y = abc[1] - j;
      const auto h1z = abc[2] - k;

      const auto h0x( 1.0f - h1x );
      const auto h0y( 1.0f - h1y );
      const auto h0z( 1.0f - h1z );

      const auto h00 = h0x * h0y;
      const auto h01 = h0x * h1y;
      const auto h10 = h1x * h0y;
      const auto h11 = h1x * h1y;

      return {m_scaleFactor[0] * ( h0z * ( h00 * m_B[ijk000][0] + h10 * m_B[ijk000 + 1][0] + h01 * m_B[ijk010][0] +
                                           h11 * m_B[ijk010 + 1][0] ) +
                                   h1z * ( h00 * m_B[ijk001][0] + h10 * m_B[ijk001 + 1][0] + h01 * m_B[ijk011][0] +
                                           h11 * m_B[ijk011 + 1][0] ) ),
              m_scaleFactor[1] * ( h0z * ( h00 * m_B[ijk000][1] + h10 * m_B[ijk000 + 1][1] + h01 * m_B[ijk010][1] +
                                           h11 * m_B[ijk010 + 1][1] ) +
                                   h1z * ( h00 * m_B[ijk001][1] + h10 * m_B[ijk001 + 1][1] + h01 * m_B[ijk011][1] +
                                           h11 * m_B[ijk011 + 1][1] ) ),
              m_scaleFactor[2] * ( h0z * ( h00 * m_B[ijk000][2] + h10 * m_B[ijk000 + 1][2] + h01 * m_B[ijk010][2] +
                                           h11 * m_B[ijk010 + 1][2] ) +
                                   h1z * ( h00 * m_B[ijk001][2] + h10 * m_B[ijk001 + 1][2] + h01 * m_B[ijk011][2] +
                                           h11 * m_B[ijk011 + 1][2] ) )};
    }
#endif // __x86_64__

    // if get here return null vector
    return {0, 0, 0};
  }

} // namespace LHCb::Magnet
