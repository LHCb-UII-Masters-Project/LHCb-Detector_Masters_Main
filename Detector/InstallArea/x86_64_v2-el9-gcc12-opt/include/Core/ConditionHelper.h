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

/* Tool to help with the configuration fo the conditions
==========================================================

The syntax expected is of the style:

<conditions_config prefix="Conditions/VP"/>
<alignment_pattern regex="Module\d\d" path="Alignment/Modules.yml"/>
<alignment_pattern regex="VP(Left|Right)?" path="Alignment/Global.yml"/>
<alignment detector="VP" path="Alignment/Global.yml" name="VPSystem"/>

<condition_pattern regex="Module\d\d" path="Condition/Modules.yml"/>
<condition_pattern regex="VP(Left|Right)?" path="Condition/Global.yml"/>
<conditions path="conditions.yml">
  <condition name="TestCond"/>
  <condition name="TestCond2"/>
</conditions>

where:
  * conditions_config: specifies the top root prefix for all the paths specified subsequently (mandatory)
  * alignment_pattern: for all subdetector which name matches the regexp, the alignment condition will be loaded from
the path specified, with a condition name equal to the name of the subdetector
  * alignment: specify the alignment for one subdetector, specify the path and name of the condition (overrides
patterns)
  * conditions: specify the path from which to load a set of conditions, specified by the name is the subsequent
condition tag

Create the helper from the plugin parsing the config, specifying the type of the condition callback,
then invoke the configure method.

```
  LHCb::Detector::ConditionConfigHelper<LHCb::Detector::DeVPConditionCall> config_helper{description, "VP", e};
  config_helper.configure();
```

*/

#include "Core/CondDBSchema.h"
#include "Core/ConditionHelper.h"
#include "Core/ConditionsRepository.h"
#include "Core/Keys.h"
#include "Core/LHCbCompactTags.h"
#include "DD4hep/DetElement.h"
#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/Printout.h"
#include "DDCond/ConditionsTags.h"
#include "XML/XMLElements.h"
#include <boost/algorithm/string.hpp>
#include <fmt/format.h>

#include <map>
#include <optional>
#include <regex>
#include <string>

/// Namespace for the LHCb Detector
namespace LHCb::Detector {

  // Containers for configuration parameters during parsing
  struct ConditionConfig {

    ConditionConfig() = default;
    ConditionConfig( std::string d, std::string l, std::string n )
        : detector{d}, condition_location{l}, condition_name{n} {}

    std::string detector;
    std::string condition_location;
    std::string condition_name;
    bool        optional = false;
  };

  // Containers for configuration regex specifications during parsing
  struct ConditionPattern {

    ConditionPattern() = default;
    ConditionPattern( std::regex r, std::string l ) : condition_regex{r}, condition_location{l}, match_path{false} {}
    ConditionPattern( std::regex r, std::string l, bool b, std::string n )
        : condition_regex{r}, condition_location{l}, match_path{b}, condition_name{n} {}

    std::regex                 condition_regex;
    std::string                condition_location;
    bool                       match_path;
    std::optional<std::string> condition_name;
    bool                       optional = false;
  };

  /**
   * @brief Helper for the configuration of the conditions on Detector elements
   *
   */
  template <class Callback>
  class ConditionConfigHelper {

  public:
    /**
     * @brief Create the helper but does not alter the description.
     * The Configure method performs all that.
     *
     */
    ConditionConfigHelper( dd4hep::Detector& description, std::string detector_name, xml_h e );

    /**
     * @brief Configure the conditions for the detector specified in the constructor
     *
     */
    void configure( std::vector<std::string> dependency_names = {} ) {
      setup_callback( dependency_names );
      configure_alignments();
      configure_conditions();
    }

    /**
     * @brief Added another key for the callback than the standard DeKey
     *
     */
    void add_callback_key( std::string condition_name );

    /**
     * @brief Acccessor method for the ConditionsRepository object (needed to add dependencies)
     *
     */
    LHCb::Detector::ConditionsRepository* conditions_repository() { return m_conditions_repository.get(); }

    /**
     * @brief Setup the callbacks for a detector and specified sub-detectors, following the parameters passes
     * within the XML element.
     *
     */
    void setup_callback( const std::vector<std::string> dependency_names = {} );

    /**
     * @brief Setup the callbacks for a detector and specified sub-detectors, following the parameters passes
     * within the XML element.
     *
     */
    void configure_alignments();

    /**
     * @brief Setup the conditions mappings for the standard (i.e. non alignments) conditions.
     *
     */
    void configure_conditions();

    /// If the CondDB schema is available, check if the condition path and key are accessible. Assume true if no schema.
    bool condition_exists( const std::string& path, const std::string& key ) const {
      return !m_conddb_schema || m_conddb_schema->has( path, key );
    }

    /// If the CondDB schema is available, make sure that the condition path and key are accessible.
    void assert_condition_exists( const dd4hep::DetElement& det, const std::string& path,
                                  const std::string& key ) const {
      if ( !condition_exists( path, key ) )
        throw std::runtime_error(
            fmt::format( "detector {} requested a condition not in the database: {}[{}]", det.name(), path, key ) );
    }

  private:
    /// Utility to iterate detector tree and map the DetElement names with their entries
    /// in the conditions
    void _iterate_detector_tree_alignments( dd4hep::DetElement                    de,
                                            LHCb::Detector::ConditionsRepository& conditions_repository,
                                            dd4hep::cond::ConditionDependency*    top_det_element );

    void _iterate_detector_tree_conditions( dd4hep::DetElement                    de,
                                            LHCb::Detector::ConditionsRepository& conditions_repository,
                                            dd4hep::cond::ConditionDependency*    top_det_element );

    // General info about the detector
    dd4hep::Detector&  m_detector_description;
    std::string        m_detector_name;
    dd4hep::DetElement m_detelement;
    CondDBSchema*      m_conddb_schema = nullptr;

    // DD4hep container for the repository config
    std::shared_ptr<LHCb::Detector::ConditionsRepository> m_conditions_repository;

    // Pointer to the dependency
    dd4hep::cond::ConditionDependency* m_condition_dependency = nullptr;

    // Various data parsed from the XML configuration
    std::string                             m_prefix;
    std::map<std::string, ConditionConfig>  m_condition_locations;
    std::map<std::string, ConditionPattern> m_condition_patterns;
    std::map<std::string, ConditionConfig>  m_alignment_locations;
    std::map<std::string, ConditionPattern> m_alignment_patterns;

  }; // ConditionConfigHelper

  template <class Callback>
  ConditionConfigHelper<Callback>::ConditionConfigHelper( dd4hep::Detector& description, std::string detector_name,
                                                          xml_h e )
      : m_detector_description{description}
      , m_detector_name{detector_name}
      , m_detelement{description.detector( detector_name )}
      , m_conddb_schema{description.extension<CondDBSchema>( false )} {

    // XXX This shall be reviewed later, we should use the attached to the Description instead
    m_conditions_repository = std::make_shared<LHCb::Detector::ConditionsRepository>();

    // Add the ConditionsRepository extension to the Detector element instead
    using Ext_t = std::shared_ptr<LHCb::Detector::ConditionsRepository>;
    m_detelement.addExtension(
        new dd4hep::detail::DeleteExtension<Ext_t, Ext_t>( new Ext_t( m_conditions_repository ) ) );

    // Looking for the config entry with the prefix
    // that will be used to derive the default path to the alignment conditions
    // <config path="conditions/VP"/>
    xml_comp_t config = e.child( _LBUC( conditions_config ) );
    m_prefix          = config.attr<std::string>( _LBUC( prefix ) );
    if ( m_prefix.back() != '/' ) { m_prefix.push_back( '/' ); }

    // Parsing alignment patterns
    //  <alignment_pattern regex="Module\d\d" path="Alignment/Modules.yml"/>
    for ( xml_coll_t i( e, _LBUC( alignment_pattern ) ); i; ++i ) {
      xml_elt_t c = i;

      // the tag should have one of regex and path_regex
      if ( c.hasAttr( _LBUC( path_regex ) ) && !c.hasAttr( _LBUC( regex ) ) ) {
        std::string txt_path_re{c.attr<std::string>( _LBUC( path_regex ), "" )};
        std::regex  re{txt_path_re};
        // If not specified we choose the file "Alignment.yml"
        std::string loc = c.attr<std::string>( _U( path ), "Alignment.yml" );
        if ( c.hasAttr( _U( name ) ) ) {
          m_alignment_patterns[txt_path_re] =
              ConditionPattern{re, m_prefix + loc, true, c.attr<std::string>( _U( name ) )};
        } else {
          throw std::runtime_error(
              "Attribute 'name' has to be specified when 'path_regex' is used with <alignment_pattern>" );
        }

      } else if ( !c.hasAttr( _LBUC( path_regex ) ) && c.hasAttr( _LBUC( regex ) ) ) {
        std::string txt_re{c.attr<std::string>( _LBUC( regex ) )};
        std::regex  re{txt_re};
        // If not specified we choose the file "Alignment.yml"
        std::string loc              = c.attr<std::string>( _U( path ), "Alignment.yml" );
        m_alignment_patterns[txt_re] = ConditionPattern{re, m_prefix + loc};
      } else {
        throw std::runtime_error( "One and only one of 'regex' or 'path_regex' should be used in <alignment_pattern>" );
      }
    }

    // Parsing the specific alignments
    //  <alignment detector="VP" ref="Alignment/Global.yml" name="VPSystem"/>
    for ( xml_coll_t i( e, _UC( alignment ) ); i; ++i ) {
      xml_comp_t      c = i;
      ConditionConfig ac{c.attr<std::string>( _U( detector ) ), m_prefix + c.attr<std::string>( _U( path ) ),
                         c.attr<std::string>( _U( name ) )};
      m_alignment_locations[c.attr<std::string>( _U( detector ) )] = ac;
    }

    // Parsing the generic condition locations
    // <conditions path="conditions.yml">
    //   <condition name="TestCond"/>
    // </conditions>
    for ( xml_coll_t i( e, _UC( conditions ) ); i; ++i ) {
      xml_elt_t   c         = i;
      std::string cond_path = c.attr<std::string>( _U( path ) );
      for ( xml_coll_t cc( c, _UC( condition ) ); cc; ++cc ) {
        xml_elt_t       cond = cc;
        ConditionConfig ac{detector_name, m_prefix + c.attr<std::string>( _U( path ) ),
                           cc.attr<std::string>( _U( name ) )};
        // Checking whether we have optional conditions signalled by optional="1"
        if ( cc.hasAttr( _LBUC( optional ) ) ) {
          int val = cc.attr<int>( _LBUC( optional ) );
          if ( val != 0 ) { ac.optional = true; }
        }
        m_condition_locations[cond.attr<std::string>( _U( name ) )] = ac;
      }
    }

    // Parsing the condition location patterns
    // <condition_pattern path_regex="MAPMTP(\d)InECR(\d)InModule(\d+)$" path="Conditions/mapmt.yml" name="${0}"/>
    for ( xml_coll_t i( e, _LBUC( condition_pattern ) ); i; ++i ) {
      xml_elt_t c = i;

      // the tag should have one of regex and path_regex
      if ( c.hasAttr( _LBUC( path_regex ) ) && !c.hasAttr( _LBUC( regex ) ) ) {
        std::string txt_path_re{c.attr<std::string>( _LBUC( path_regex ), "" )};
        std::regex  re{txt_path_re};
        // If not specified we choose the file "Condition.yml"
        std::string loc = c.attr<std::string>( _U( path ), "Condition.yml" );
        if ( c.hasAttr( _U( name ) ) ) {
          auto cp = ConditionPattern{re, m_prefix + loc, true, c.attr<std::string>( _U( name ) )};
          if ( c.hasAttr( _LBUC( optional ) ) ) {
            int val = c.attr<int>( _LBUC( optional ) );
            if ( val != 0 ) { cp.optional = true; }
          }
          m_condition_patterns[txt_path_re] = cp;

        } else {
          throw std::runtime_error(
              "Attribute 'name' has to be specified when 'path_regex' is used with <condition_pattern>" );
        }

      } else if ( !c.hasAttr( _LBUC( path_regex ) ) && c.hasAttr( _LBUC( regex ) ) ) {
        std::string txt_re{c.attr<std::string>( _LBUC( regex ) )};
        std::regex  re{txt_re};
        // If not specified we choose the file "Condition.yml"
        std::string loc = c.attr<std::string>( _U( path ), "Condition.yml" );
        auto        cp  = ConditionPattern{re, m_prefix + loc};
        if ( c.hasAttr( _LBUC( optional ) ) ) {
          int val = c.attr<int>( _LBUC( optional ) );
          if ( val != 0 ) { cp.optional = true; }
        }
        m_condition_patterns[txt_re] = cp;
      } else {
        throw std::runtime_error( "One and only one of 'regex' or 'path_regex' should be used in <condition_pattern>" );
      }
    }
  }

  template <class Callback>
  void ConditionConfigHelper<Callback>::setup_callback( const std::vector<std::string> dependency_names ) {

    // Setting up the callback as specified by the template type
    auto                            callback = std::make_shared<Callback>( m_conditions_repository );
    dd4hep::cond::DependencyBuilder depbuilder( m_detelement, LHCb::Detector::Keys::deKey, callback );
    for ( const auto& n : dependency_names ) { depbuilder.add( dd4hep::ConditionKey( m_detelement, n ) ); }
    auto cond_dep          = m_conditions_repository->addDependency( depbuilder.release() );
    m_condition_dependency = cond_dep.second;
    // cond_dep is std::pair<dd4hep::Condition::key_type, dd4hep::cond::ConditionDependency*>
    // We keep the pointer to this as we will add the new conditions to that list to that they are updated
    // when needed
  }

  template <class Callback>
  void ConditionConfigHelper<Callback>::add_callback_key( std::string condition_name ) {
    // Adding another key that will be filled up by the callback
    auto callback = std::make_shared<Callback>( m_conditions_repository );
    auto cond_dep =
        m_conditions_repository->addDependency( m_detelement, LHCb::Detector::item_key( condition_name ), callback );
    m_condition_dependency = cond_dep.second;
  }

  /**
   * Iterate over the detector tree to setup alignments
   */
  template <class Callback>
  void ConditionConfigHelper<Callback>::_iterate_detector_tree_alignments(
      dd4hep::DetElement det, LHCb::Detector::ConditionsRepository& conditions_repository,
      dd4hep::cond::ConditionDependency* condition_dependency ) {

    bool        configured = false;
    std::string cond_location, cond_name;

    // First checking whether we match one of the specific alignments specified as an alignment tag
    for ( const auto& [txt_re, p] : m_alignment_patterns ) {

      std::string det_str = det.name();
      if ( p.match_path ) { det_str = det.path(); }

      std::smatch match;
      if ( std::regex_match( det_str, match, p.condition_regex ) ) {
        configured    = true;
        cond_location = p.condition_location;
        if ( p.condition_name.has_value() ) {
          std::string cond_tmp = p.condition_name.value();
          // If the condition name was specified, replace ${i}
          // by the corresponding match
          // CAREFUL, this is space sensitive, we need a library to do this
          for ( size_t i = 0; i < match.size(); ++i ) {
            std::stringstream ss;
            ss << "${" << i << "}";

            std::string match_str = match[i].str();
            boost::replace_all( cond_tmp, ss.str(), match_str );
          }
          cond_name = cond_tmp;
        } else {
          cond_name = det.name();
        }
        dd4hep::printout( dd4hep::DEBUG, "ConditionConfigHelper", "Regex match: %s:%s matches %s",
                          cond_location.c_str(), cond_name.c_str(), det.path().c_str() );
      }
    }

    // Checking whether we have a specific config for a detector
    auto entry = m_alignment_locations.find( det.name() );
    if ( entry != m_alignment_locations.end() ) {
      configured    = true;
      cond_location = entry->second.condition_location;
      cond_name     = entry->second.condition_name;
      dd4hep::printout( dd4hep::DEBUG, "ConditionConfigHelper", "%s has specific config", det.name() );
    }

    // This detector is configured
    if ( configured ) {
      dd4hep::printout( dd4hep::DEBUG, "ConditionConfigHelper", "%s fetching conditions from %s:%s", det.name(),
                        cond_location.c_str(), cond_name.c_str() );

      assert_condition_exists( det, cond_location, cond_name );
      auto addr = conditions_repository.addLocation( det, LHCb::Detector::Keys::deltaKey, cond_location, cond_name );
      condition_dependency->dependencies.push_back( addr.first );
    }

    // Iterating on children
    for ( const auto& [name, child_det] : det.children() ) {
      _iterate_detector_tree_alignments( child_det, conditions_repository, condition_dependency );
    }
  }

  /**
   * Iterate over the detector tree to setup conditions
   * This is a separate method as the handling is different
   */
  template <class Callback>
  void ConditionConfigHelper<Callback>::_iterate_detector_tree_conditions(
      dd4hep::DetElement det, LHCb::Detector::ConditionsRepository& conditions_repository,
      dd4hep::cond::ConditionDependency* condition_dependency ) {

    bool        configured = false;
    std::string cond_location, cond_name;
    bool        cond_optional = false;

    // Iterating on all condition patterns
    for ( const auto& [txt_re, p] : m_condition_patterns ) {

      std::string det_str = det.name();
      if ( p.match_path ) { det_str = det.path(); }

      std::smatch match;
      if ( std::regex_match( det_str, match, p.condition_regex ) ) {
        configured    = true;
        cond_location = p.condition_location;
        if ( p.condition_name.has_value() ) {
          std::string cond_tmp = p.condition_name.value();
          // If the condition name was specified, replace ${i}
          // by the corresponding match
          // CAREFUL, this is space sensitive, we need a library to do this
          for ( size_t i = 0; i < match.size(); ++i ) {
            std::stringstream ss;
            ss << "${" << i << "}";

            std::string match_str = match[i].str();
            boost::replace_all( cond_tmp, ss.str(), match_str );
          }
          cond_name = cond_tmp;
        } else {
          cond_name = det.name();
        }
        dd4hep::printout( dd4hep::DEBUG, "ConditionConfigHelper", "Regex match: %s:%s matches %s",
                          cond_location.c_str(), cond_name.c_str(), det.path().c_str() );
        cond_optional = p.optional;
      }
    }

    // This detector is configured
    if ( configured ) {
      dd4hep::printout( dd4hep::DEBUG, "ConditionConfigHelper", "%s fetching conditions from %s:%s", det.name(),
                        cond_location.c_str(), cond_name.c_str() );
      if ( !cond_optional ) { assert_condition_exists( det, cond_location, cond_name ); }
      auto addr = conditions_repository.addLocation( det, LHCb::Detector::item_key( cond_name ), cond_location,
                                                     cond_name, cond_optional );
      condition_dependency->dependencies.push_back( addr.first );
    }

    // Iterating on children
    for ( const auto& [name, child_det] : det.children() ) {
      _iterate_detector_tree_conditions( child_det, conditions_repository, condition_dependency );
    }
  }

  template <class Callback>
  void ConditionConfigHelper<Callback>::configure_alignments() {

    dd4hep::printout( dd4hep::DEBUG, "ConditionConfigHelper", "Configuring alignments for %s, default path: %s",
                      m_detector_name.c_str(), m_prefix.c_str() );
    _iterate_detector_tree_alignments( m_detelement, *m_conditions_repository, m_condition_dependency );
  }

  template <class Callback>
  void ConditionConfigHelper<Callback>::configure_conditions() {

    // Iterate over the detector elements to check if we have matching conditions
    dd4hep::printout( dd4hep::DEBUG, "ConditionConfigHelper", "Configuring conditions for %s, default path: %s",
                      m_detector_name.c_str(), m_prefix.c_str() );
    _iterate_detector_tree_conditions( m_detelement, *m_conditions_repository, m_condition_dependency );

    // Iterating through the conditions defined in the XML for the detector and adding them all
    for ( const auto& [condition_name, condition_config] : m_condition_locations ) {
      if ( !condition_config.optional ) {
        assert_condition_exists( m_detelement, condition_config.condition_location, condition_config.condition_name );
      }
      m_conditions_repository->addLocation( m_detelement, LHCb::Detector::item_key( condition_name ),
                                            condition_config.condition_location, condition_config.condition_name,
                                            condition_config.optional );
      dd4hep::printout( dd4hep::DEBUG, "ConditionConfigHelper", "Defined condition %s:%s",
                        condition_config.condition_location.c_str(), condition_name.c_str() );
    }
  }

} // namespace LHCb::Detector
