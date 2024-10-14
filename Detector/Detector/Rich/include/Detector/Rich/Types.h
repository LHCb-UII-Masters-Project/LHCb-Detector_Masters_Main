/*****************************************************************************\
* (c) Copyright 2000-2022 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/

#pragma once

// Boost
#include <boost/container/small_vector.hpp>
#include <boost/version.hpp>

// ROOT
#include "Math/Plane3D.h"
#include "Math/Point3D.h"
#include "Math/Vector3D.h"

// STL files
#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

// Hack to work around cling issue
#define N_SIDE_TYPES 2
#define N_DETECTOR_TYPES 2
#define N_RADIATOR_TYPES 3

namespace Rich {

  /// Number of RICH detectors
  inline constexpr std::uint16_t NRiches = N_DETECTOR_TYPES;
  /// Number of PD panels per RICH detector
  inline constexpr uint16_t NPDPanelsPerRICH = N_SIDE_TYPES;
  /// Total number of PD panels
  inline constexpr uint16_t NTotalPDPanels = NRiches * NPDPanelsPerRICH;
  /// Number of RICH radiators
  inline constexpr std::uint16_t NRadiatorTypes = N_RADIATOR_TYPES;

  /// RICH enum
  enum DetectorType : std::int8_t {
    InvalidDetector = -1, ///< Unspecified Detector
    Rich1           = 0,  ///< RICH1 detector
    Rich2           = 1,  ///< RICH2 detector
    Rich            = 1   ///< Single RICH detector
  };

  /// Detector side enum
  enum Side : int32_t {
    InvalidSide = -1, ///< Invalid side
                      // RICH1
    top    = 0,       ///< Upper panel in RICH1
    bottom = 1,       ///< Lower panel in RICH1
                      // RICH2
    left  = 0,        ///< Left panel in RICH2
    right = 1,        ///< Right panel in RICH2
    aside = 0,        ///< A-Side panel in RICH2
    cside = 1,        ///< C-Side panel in RICH2
                      // Generic
    firstSide  = 0,   ///< Upper panel in RICH1 or Left panel in RICH2
    secondSide = 1    ///< Lower panel in RICH1 or Right panel in RICH2
  };

  /// Radiator enum
  enum RadiatorType : std::int8_t {
    InvalidRadiator = -1, ///< Unspecified radiator type
    Aerogel         = 0,  ///< Aerogel in RICH1
    Rich1Gas        = 1,  ///< Gaseous RICH1 radiator
    Rich2Gas        = 2,  ///< Gaseous RICH2 radiator
    C4F10           = 1,  ///< Gaseous RICH1 radiator (to be removed)
    CF4             = 2,  ///< Gaseous RICH2 radiator (to be removed)
    // background types
    GasQuartzWin  = 3, ///< Quartz windows to the gas radiator volumes
    HPDQuartzWin  = 4, ///< HPD Quartz windows
    Nitrogen      = 5, ///< Nitrogen volume
    AerogelFilter = 6, ///< Aerogel filter material
    CO2           = 7, ///< Carbon dioxide
    PMTQuartzWin  = 8  ///< MAPMT Quartz windows
  };

  /// Type for fixed size arrays for data for each RICH
  template <typename TYPE>
  using DetectorArray = std::array<TYPE, NRiches>;
  /// Type for fixed size arrays with RICH panel information
  template <typename TYPE>
  using PanelArray = std::array<TYPE, NPDPanelsPerRICH>;
  /// Type for fixed size arrays with radiator information
  template <typename TYPE>
  using RadiatorArray = std::array<TYPE, NRadiatorTypes>;

  /// Type for container of detector types
  using Detectors = boost::container::small_vector<DetectorType, N_DETECTOR_TYPES>;
  /// Type for container of side types
  using Sides = boost::container::small_vector<Side, N_SIDE_TYPES>;
  /// Type for container of radiator types
  using Radiators = boost::container::small_vector<RadiatorType, N_RADIATOR_TYPES>;

  /// Access all valid detector types
  inline Detectors detectors() noexcept { return {Rich::Rich1, Rich::Rich2}; }
  /// Access all valid sides
  inline Sides sides() noexcept { return {Rich::firstSide, Rich::secondSide}; }
  /// Access all valid radiator types
  inline Radiators radiators() noexcept { return {Rich::Aerogel, Rich::Rich1Gas, Rich::Rich2Gas}; }

} // namespace Rich

// remove clang hack
#undef N_SIDE_TYPES
#undef N_DETECTOR_TYPES
#undef N_RADIATOR_TYPES

namespace Rich::Detector {

  // Geometrical Types
  using XYZVector = ROOT::Math::XYZVector;
  using XYZPoint  = ROOT::Math::XYZPoint;
  using Plane3D   = ROOT::Math::Plane3D;

  /// Type for maps with string keys
  template <typename TYPE>
  using StringMap = std::map<std::string, TYPE>;

  /// Typ for storage of types with added name
  template <typename TYPE>
  using NamedVector = std::vector<std::pair<const char*, TYPE>>;

} // namespace Rich::Detector

namespace LHCb::Detector {

  /// Type for interpolator (x,y) data points.
  using XYTable = std::map<double, double>;

} // namespace LHCb::Detector
