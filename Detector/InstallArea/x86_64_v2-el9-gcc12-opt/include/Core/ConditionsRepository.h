//==========================================================================
//  AIDA Detector description implementation
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
//  \author  Markus Frank
//  \date    2016-02-02
//  \version 1.0
//
//==========================================================================
#pragma once

#include "Core/ConditionIdentifier.h"

#include "DDCond/ConditionsContent.h"

#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/key_extractors.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>

namespace {
  namespace bmi = boost::multi_index;
}

namespace LHCb::Detector {

  // Tag structs to refer to the three indices of the
  // ConditionsRepository::Locations multi-index container
  struct by_location_hash {};
  struct by_filename {};
  struct by_condition_key {};

  /// Utility function to extract the combined location hash (filename
  /// and condition name) from a ConditionIdentifier
  dd4hep::Condition::key_type location_key( const ConditionIdentifier* entry );

  /// Utility function to extract the filename hash from a const ConditionIdentifier
  inline std::string condition_filename( const ConditionIdentifier* entry ) { return entry->sys_id; };

  /// Utility function to extract the condition key from a const ConditionIdentifier
  inline dd4hep::Condition::key_type condition_key( const ConditionIdentifier* entry ) { return entry->hash; };

  struct RequestSelector;

  /**
   *  \author   M.Frank
   *  \version  1.0
   *  \ingroup  DD4HEP_GAUDI
   */
  class ConditionsRepository : public dd4hep::cond::ConditionsContent {
    friend struct RequestSelector;

  public:
    // A multi-index container that allows (fast) lookup in three
    // ways. the value is either ConditionIdentifier* or
    // const ConditionIdentifier*.
    template <bool const_pointer = false>
    using Locations = boost::multi_index_container<
        std::conditional_t<const_pointer, const ConditionIdentifier*, ConditionIdentifier*>,
        bmi::indexed_by<
            // Analogously to an `std::map` with key the combined hash
            // of the filename and the condition name
            bmi::ordered_unique<
                bmi::tag<LHCb::Detector::by_location_hash>,
                bmi::global_fun<const ConditionIdentifier*, dd4hep::Condition::key_type, &location_key>>,
            // Analogously to an `std::multimap` with key the hash of the filename
            bmi::ordered_non_unique<bmi::tag<LHCb::Detector::by_filename>,
                                    bmi::global_fun<const ConditionIdentifier*, std::string, &condition_filename>>,
            // Analogously to an `std::unordered_map` with key the
            // combined hash of the detector element and the condition
            // name
            bmi::ordered_unique<
                bmi::tag<LHCb::Detector::by_condition_key>,
                bmi::global_fun<const ConditionIdentifier*, dd4hep::Condition::key_type, &condition_key>>>>;
    Locations<> locations;

  private:
    /// Default assignment operator
    ConditionsRepository& operator=( const ConditionsRepository& copy ) = delete;
    /// Copy constructor
    ConditionsRepository( const ConditionsRepository& copy ) = delete;

  public:
    /// Default constructor
    ConditionsRepository() = default;
    /// Default destructor.
    virtual ~ConditionsRepository() = default;
    /// Access to the real condition entries to be loaded (CONST)
    const Conditions& conditions() const { return m_conditions; }
    /// Access to the derived condition entries to be computed (CONST)
    const Dependencies& derived() const { return m_derived; }
    /// Merge the content of "to_add" into the this content
    void merge( const ConditionsRepository& to_add );
    /// Clear the container. Destroys the contained stuff
    void clear();
    std::pair<dd4hep::Condition::key_type, dd4hep::cond::ConditionsLoadInfo*>
    addLocation( dd4hep::DetElement de, dd4hep::Condition::itemkey_type item, const std::string& sys_id,
                 const std::string& object, bool is_optional = false );
    std::pair<dd4hep::Condition::key_type, dd4hep::cond::ConditionsLoadInfo*>
    addGlobal( dd4hep::DetElement de, dd4hep::Condition::itemkey_type item, const std::string& sys_id,
               const std::string& object, bool is_optional = false );
    /// Add a new shared conditions dependency
    std::pair<dd4hep::Condition::key_type, dd4hep::cond::ConditionDependency*>
    addDependency( dd4hep::cond::ConditionDependency* dep );
    /// Add a new conditions dependency (Built internally from arguments)
    std::pair<dd4hep::Condition::key_type, dd4hep::cond::ConditionDependency*>
    addDependency( dd4hep::DetElement de, dd4hep::Condition::itemkey_type item,
                   std::shared_ptr<dd4hep::cond::ConditionUpdateCall> callback );
    /// Add a new global conditions dependency (Built internally from arguments)
    std::pair<dd4hep::Condition::key_type, dd4hep::cond::ConditionDependency*>
    addGlobal( dd4hep::DetElement de, dd4hep::Condition::itemkey_type item,
               std::shared_ptr<dd4hep::cond::ConditionUpdateCall> callback );
    /// Add a new global conditions dependency (Built internally from arguments)
    std::pair<dd4hep::Condition::key_type, dd4hep::cond::ConditionDependency*>
    addGlobal( dd4hep::cond::ConditionDependency* dep );
  };
} // namespace LHCb::Detector
