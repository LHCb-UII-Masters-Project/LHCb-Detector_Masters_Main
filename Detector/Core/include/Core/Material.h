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

#include <Core/DeIOV.h>

#include <TGeoMedium.h>

using LHCb::Detector::NotImplemented;

namespace LHCb::Detector {

  /**
   * Handle class around ROOT TGeoMedium describing a Material
   *
   * Extends the Material class defined in DD4hep, so that an
   * interface backward compatible with DetDesc can be offered
   */
  struct Material : dd4hep::Material {

    using dd4hep::Material::Material;

    /// Material radiation length [cm]
    double radiationLength() const { return radLength(); };

    /// Mean excitiation energy
    double I() const { throw NotImplemented(); }

    /// Parameters for density effect correction
    double C() const { throw NotImplemented(); }
    double a() const { throw NotImplemented(); }
    double m() const { throw NotImplemented(); }
    double X1() const { throw NotImplemented(); }
    double X0() const { throw NotImplemented(); }
  };

} // namespace LHCb::Detector
