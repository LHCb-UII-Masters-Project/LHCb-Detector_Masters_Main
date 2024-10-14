/*****************************************************************************\
* (c) Copyright 2000-2019 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/

#pragma once

#include <Core/FloatComparison.h>

#include "DD4hep/AlignmentData.h"
#include "DD4hep/DD4hepUnits.h"

#include "Math/Point3D.h"
#include "Math/Transform3D.h"
#include "Math/Vector3D.h"

namespace LHCb::Detector::detail {

  inline static constexpr double GaudiCentimeter    = 10.0; // Gaudi/G4 units are mm
  inline static constexpr double DD4hepToLHCbCm     = GaudiCentimeter / dd4hep::centimeter;
  inline static constexpr double LHCbToDD4hepCm     = dd4hep::centimeter / GaudiCentimeter;
  inline static constexpr double GaudiTeV           = 1.0e6; // Gaudi/G4 units are MeV
  inline static constexpr double DD4hepToLHCbEnergy = GaudiTeV / dd4hep::TeV;

  template <typename TYPE>
  inline TYPE toLHCbLengthUnits( const TYPE& length ) {
    return DD4hepToLHCbCm * length;
  }
  inline auto toLHCbLengthUnits( const TGeoHMatrix& length ) {
    return TGeoScale( DD4hepToLHCbCm, DD4hepToLHCbCm, DD4hepToLHCbCm ) * length;
  }
  template <typename TYPE>
  inline TYPE toLHCbAreaUnits( const TYPE& area ) {
    return DD4hepToLHCbCm * DD4hepToLHCbCm * area;
  }
  template <typename TYPE>
  inline TYPE toLHCbEnergyUnits( const TYPE& energy ) {
    return DD4hepToLHCbEnergy * energy;
  }
  inline auto toLHCbLengthUnits( const ROOT::Math::Transform3D& t ) {
    if constexpr ( !essentiallyEqual( 1.0, DD4hepToLHCbCm ) ) {
      ROOT::Math::Transform3D::Scalar xx{}, xy{}, xz{}, dx{}, yx{}, yy{}, yz{}, dy{}, zx{}, zy{}, zz{}, dz{};
      t.GetComponents( xx, xy, xz, dx, yx, yy, yz, dy, zx, zy, zz, dz );
      return ROOT::Math::Transform3D{xx, xy, xz, toLHCbLengthUnits( dx ), //
                                     yx, yy, yz, toLHCbLengthUnits( dy ), //
                                     zx, zy, zz, toLHCbLengthUnits( dz )};
    } else {
      return t;
    }
  }
  inline auto toLHCbLengthUnits( const ROOT::Math::Translation3D& t ) {
    if constexpr ( !essentiallyEqual( 1.0, DD4hepToLHCbCm ) ) {
      ROOT::Math::Translation3D::Scalar dx{}, dy{}, dz{};
      t.GetComponents( dx, dy, dz );
      return ROOT::Math::Translation3D{toLHCbLengthUnits( dx ), toLHCbLengthUnits( dy ), toLHCbLengthUnits( dz )};
    } else {
      return t;
    }
  }
  template <typename T>
  inline auto toDD4hepUnits( const T& d ) {
    return LHCbToDD4hepCm * d;
  }
  inline auto toDD4hepUnits( const TGeoHMatrix& m ) {
    if constexpr ( !essentiallyEqual( 1.0, DD4hepToLHCbCm ) ) {
      return TGeoScale( LHCbToDD4hepCm, LHCbToDD4hepCm, LHCbToDD4hepCm ) * m;
    } else {
      return m;
    }
  }
  inline auto toLHCbLengthUnits( const dd4hep::Delta& d ) {
    if constexpr ( !essentiallyEqual( 1.0, DD4hepToLHCbCm ) ) {
      // transform all components without checking which ones are used
      auto res = dd4hep::Delta{toLHCbLengthUnits( d.translation ), toLHCbLengthUnits( d.pivot ), d.rotation};
      // use flags to ignore what needs to be ignored
      res.setFlag( d.flags );
      return res;
    } else {
      return d;
    }
  }
  inline auto toDD4hepUnits( const ROOT::Math::Translation3D& t ) {
    if constexpr ( !essentiallyEqual( 1.0, DD4hepToLHCbCm ) ) {
      ROOT::Math::Translation3D::Scalar dx{}, dy{}, dz{};
      t.GetComponents( dx, dy, dz );
      return ROOT::Math::Translation3D{toDD4hepUnits( dx ), toDD4hepUnits( dy ), toDD4hepUnits( dz )};
    } else {
      return t;
    }
  }
  inline auto toDD4hepUnits( const dd4hep::Delta& d ) {
    if constexpr ( !essentiallyEqual( 1.0, DD4hepToLHCbCm ) ) {
      // transform all components without checking which ones are used
      auto res = dd4hep::Delta{toDD4hepUnits( d.translation ), toDD4hepUnits( d.pivot ), d.rotation};
      // use flags to ignore what needs to be ignored
      res.setFlag( d.flags );
      return res;
    } else {
      return d;
    }
  }

} // End namespace LHCb::Detector::detail
