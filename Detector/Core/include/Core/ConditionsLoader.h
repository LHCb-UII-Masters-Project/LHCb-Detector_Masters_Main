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

// Framework include files
#include "Core/ConditionsOverlay.h"
#include "Core/ConditionsReader.h"
#include "DD4hep/ConditionsListener.h"
#include "DD4hep/Printout.h"
#include "DDCond/ConditionsDataLoader.h"
#include "XML/XML.h"

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

/// Namespace for the Gaudi framework
namespace LHCb::Detector {
  struct ConditionOverrideEntry {
    std::string path;
    std::string name;
    YAML::Node  value;
  };
  using ConditionsOverrides = std::vector<ConditionOverrideEntry>;

  class ConditionsLoader : public dd4hep::cond::ConditionsDataLoader {
    using Key    = std::pair<std::string, std::string>;
    using KeyMap = std::map<dd4hep::Condition::key_type, Key>;

    std::unique_ptr<ConditionsReader> m_reader;
    std::string                       m_readerType, m_directory, m_match, m_dbtag;
    std::string                       m_iovTypeName;
    const dd4hep::IOVType*            m_iovType = 0;

    // Optional conditions overlay
    //
    // If the overlay is enabled, we keep a copy of the data last read from the database,
    // that can then be updated to override values of conditions. When a condition is overriden
    // (dirty flag set to true) future reads from the disk will get the data from the overlay
    // ignoring the disk content.
    bool m_useOverlay = false;
    // If set and m_useOverlay is true, the content of the overlay is initialized form the files
    // in that directory.
    std::string         m_overlayInitPath;
    ConditionsOverlay   m_overlay;
    ConditionsOverrides m_conditionsOverride;

  public:
    /// Default constructor
    ConditionsLoader( dd4hep::Detector& description, dd4hep::cond::ConditionsManager mgr, const std::string& nam );
    /// Initialize loader according to user information
    void initialize() override;
    /// Optimized update using conditions slice data
    size_t load_many( const dd4hep::IOV& req_validity, RequiredItems& work, LoadedItems& loaded,
                      dd4hep::IOV& conditions_validity ) override;

    /// Accessor for the ConditionsOverlay internal instance.
    ConditionsOverlay& overlay() { return m_overlay; }
    /// Set the overrides for the conditions
    void set_conditions_overrides( ConditionsOverrides o ) { m_conditionsOverride = std::move( o ); }
    /// Declare the condition paths which IOV length must be limited to 1 unit
    template <typename CONT>
    void set_limited_iov_paths( CONT&& paths ) {
      if ( m_reader ) {
        ( *m_reader )["LimitedIOVPaths"] = std::forward<CONT>( paths );
      } else {
        throw std::logic_error( "ConditionsLoader::set_limited_iov_paths invoked before initialize" );
      }
    }
  };
} // namespace LHCb::Detector
