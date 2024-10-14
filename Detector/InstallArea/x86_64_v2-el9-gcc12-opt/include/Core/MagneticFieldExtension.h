/*****************************************************************************\
* (c) Copyright 2021 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/

#pragma once

#include "Core/MagneticFieldGrid.h"
#include "DD4hep/DetElement.h"
#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/Detector.h"
#include "DDCond/ConditionsSlice.h"
#include <memory>
#include <string>
#include <utility>

namespace LHCb::Magnet {

  /// Nominal magnet current
  inline static constexpr double NominalCurrent = 5850.0;

  /**
   * @brief Create a DetElement extension that can load the field map files from disk
   *
   * @param description The dd4hep::Detector instance for the geometry description
   * @param field_map_path The directory containing the field map files
   */
  void setup_magnetic_field_extension( const dd4hep::Detector& description, std::string field_map_path,
                                       const double nominal_current = NominalCurrent );

  class MagneticFieldExtension {

  public:
    struct FieldData {
      bool                               useRealMap{true};
      bool                               isDown{true};
      double                             signedRelativeCurrent{1.0};
      std::shared_ptr<MagneticFieldGrid> grid;
    };

  public:
    MagneticFieldExtension( const dd4hep::Detector& description, //
                            std::string             magFieldDir, //
                            const double            nominal_current )
        : m_description{description}
        , m_magneticFieldFilesLocation{std::move( magFieldDir )}
        , m_nominalCurrent{nominal_current} {}

    // Load the magnetic field map from a specific condition
    FieldData load_magnetic_field_grid( const dd4hep::cond::ConditionUpdateContext& context );

    const auto& fieldMapLocation() const noexcept { return m_magneticFieldFilesLocation; }

    auto nominalCurrent() const noexcept { return m_nominalCurrent; }

  private:
    // Extension state
    const dd4hep::Detector& m_description;                    ///< Description
    std::string             m_magneticFieldFilesLocation;     ///< Loaded from the conditions
    double                  m_nominalCurrent{NominalCurrent}; ///< Nominal current in amps
  };

} // End namespace LHCb::Magnet
