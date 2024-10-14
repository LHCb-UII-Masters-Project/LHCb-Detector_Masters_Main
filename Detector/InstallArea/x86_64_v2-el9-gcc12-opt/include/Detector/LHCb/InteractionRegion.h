/*****************************************************************************\
* (c) Copyright 2023 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once

#include <nlohmann/json_fwd.hpp>

#include <Math/Point3D.h>
#include <Math/SMatrix.h>
#include <Math/SMatrixDfwd.h>

namespace LHCb::Detector {
  struct InteractionRegion {

    InteractionRegion() = default;
    InteractionRegion( const nlohmann::json& obj );

    double tX() const { return ( spread( 2, 2 ) != 0.0 ) ? spread( 0, 2 ) / spread( 2, 2 ) : 0.0; }
    double tY() const { return ( spread( 2, 2 ) != 0.0 ) ? spread( 1, 2 ) / spread( 2, 2 ) : 0.0; }

    ROOT::Math::XYZPoint     avgPosition;
    ROOT::Math::SMatrixSym3D spread;
  };
} // namespace LHCb::Detector
