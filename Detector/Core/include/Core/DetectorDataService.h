/*****************************************************************************\
* (c) Copyright 2020-2023 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once

#include <Core/ConditionsRepository.h>
#include <Core/DeIOV.h>
#include <DDCond/ConditionsContent.h>
#include <DDCond/ConditionsDataLoader.h>
#include <DDCond/ConditionsManager.h>
#include <DDCond/ConditionsManagerObject.h>
#include <DDCond/ConditionsSlice.h>
#include <deque>
#include <filesystem>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <utility>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace LHCb::Detector {

  /**
   * Parse the identifier for a DD4hep condition, which should be of the form:
   *  <detector name>:<condition path>
   * If no ":" is specified, it is assumed that the detector is "/world"
   */
  std::tuple<std::string_view, std::string_view> parseConditionIdentifier( std::string_view condIdentifier );

  class ConditionsOverlay;

  class DetectorDataService {

  public:
    using ConditionsSlicePtr   = std::shared_ptr<dd4hep::cond::ConditionsSlice>;
    using ConditionsSliceCache = std::deque<ConditionsSlicePtr>;

    DetectorDataService( dd4hep::Detector& description, std::vector<std::string> detectorNames );

    virtual ~DetectorDataService() = default;

    /// Initialize the DetectorDataService instance from a configuration map in JSON.
    ///
    /// The configuration must contain the fields:
    /// - `repository`
    /// - `tag` (unless `repository` is in the format `<URL>@<tag>`)
    ///
    /// Optional fields are:
    /// - `overlay`: a boolean or mapping `{"path": "<path to initialization directory>"}` to initialize the overlay
    /// from files
    /// - `overrides`: mapping string to string
    /// - `print_level`: output level for DD4hep as an integer or a string  (default to INFO)
    /// - `online_conditions`: list of paths to conditions that must have IOV span limited
    ///   to exactly one run (used to ensure synchronization of special online conditions)
    void initialize( const nlohmann::json& configuration );

    void finalize();

    ConditionsSlicePtr get_slice( size_t iov );
    ConditionsSlicePtr load_slice( size_t iov );
    void               drop_slice( size_t iov );
    void               clear_slice_cache();

    /// Update a condition value in the conditions overlay.
    /// The DetectorDataService must be initialized with the overlay enabled when invokind this function,
    /// otherwise an exception is thrown.
    void update_condition( const std::string& path, const std::string& name, const YAML::Node& data );
    /// Update a condition value in the conditions overlay.
    /// The DetectorDataService must be initialized with the overlay enabled when invokind this function,
    /// otherwise an exception is thrown.
    void update_condition( const std::string& path, const std::string& name, const nlohmann::json& data );
    /// Dump the conditions in the overlay to a directory on disk.
    /// The DetectorDataService must be initialized with the overlay enabled when invokind this function,
    /// otherwise an exception is thrown.
    void dump_conditions( std::filesystem::path root_dir, bool only_dirty = true ) const;
    /// Fill the overlay from the content of a dump.
    void load_conditions( std::filesystem::path root_dir );
    /// Helper to update in the conditions overlay a condition bound to a DetElement.
    void update_condition( const dd4hep::DetElement& de, const std::string& cond, const YAML::Node& data );
    /// Helper to update in the conditions overlay a condition bound to a DetElement.
    void update_condition( const dd4hep::DetElement& de, const std::string& cond, const nlohmann::json& data );
    /// Helper to update in the conditions overlay the alignment condition of a given DetElement.
    void update_alignment( const dd4hep::DetElement& de, const dd4hep::Delta& delta );
    /// Helper to update in the conditions overlay the alignment condition of a given DeIOV.
    void update_alignment( const LHCb::Detector::DeIOV& de, const dd4hep::Delta& delta ) {
      update_alignment( de.detector(), delta );
    }

    dd4hep::IOV make_iov( size_t point ) const;
    dd4hep::IOV make_iov( size_t since, size_t until ) const;

  protected:
    dd4hep::Detector&                     m_description;
    dd4hep::cond::ConditionsManager       m_manager;
    std::vector<std::string>              m_detectorNames;
    const dd4hep::IOVType*                m_iov_typ{nullptr};
    std::shared_ptr<ConditionsRepository> m_all_conditions = std::make_shared<ConditionsRepository>();

    ConditionsSliceCache m_cache;
    std::uint64_t        m_max_cache_entries = 3;
    std::mutex           m_cache_mutex;

    LHCb::Detector::ConditionsOverlay* m_condOverlay{nullptr};
  };

  inline const std::type_info& get_condition_type( const dd4hep::Condition& condition ) {
    if ( condition.is_bound() )
      return condition.typeInfo();
    else {
      // assign the result of operator* to avoid clang's -Wpotentially-evaluated-expression
      auto& held_object = *condition;
      return typeid( held_object );
    }
  }

} // End namespace LHCb::Detector
