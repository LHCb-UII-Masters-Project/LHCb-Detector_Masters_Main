//==========================================================================
//  AIDA Detector description implementation
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
// Author     : B.Couturier
//
//==========================================================================

#include <Core/ConditionsReader.h>
#include <DD4hep/IOV.h>
#include <GitCondDB.h>
#include <exception>
#include <fmt/format.h>
#include <optional>
#include <set>
#include <typeinfo>

/// Namespace for the AIDA detector description toolkit
namespace LHCb::Detector {

  /// Class supporting the interface of the LHCb conditions database to dd4hep
  class ConditionsGitReader final : public ConditionsReader {

    std::optional<GitCondDB::CondDB> m_condDB             = std::nullopt;
    std::string                      m_dbtag              = "";
    long                             m_validityLowerLimit = 0;
    long                             m_validityUpperLimit = dd4hep::IOV::MAX_KEY;
    // FIXME: I would prefer unordered_set, but I do not want learn how to add a new grammar
    std::set<std::string> m_limitedIovPaths;

  public:
    /// Standard constructor
    ConditionsGitReader();
    /// Read raw XML object from the database / file
    int getObject( const std::string& system_id, UserContext* ctxt, std::string& data ) override;
    /// Inform reader about a locally (e.g. by XercesC) handled source load
    void parserLoaded( const std::string& system_id ) override;
    /// Inform reader about a locally (e.g. by XercesC) handled source load
    void parserLoaded( const std::string& system_id, UserContext* ctxt ) override;
    /// Resolve a given URI to a string containing the data
    bool load( const std::string& system_id, std::string& buffer ) override;
    /// Resolve a given URI to a string containing the data
    bool load( const std::string& system_id, UserContext* ctxt, std::string& buffer ) override;
    /// Set the directory and rebuild the git cond db accordingly
    void setDirectory( const std::string& dir ) override;

    // FIXME: this should not be needed but it is a workaround for
    // https://gitlab.cern.ch/lhcb/GitCondDB/-/issues/19
    ~ConditionsGitReader() {
      if ( m_condDB ) m_condDB->disconnect();
    }
  };
} /* End namespace LHCb::Detector          */

//==========================================================================
// Framework includes
#include "DD4hep/Detector.h"
#include "DD4hep/Factories.h"
#include "DD4hep/Path.h"
#include "DD4hep/Printout.h"

// C/C++ include files
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/// Standard constructor
LHCb::Detector::ConditionsGitReader::ConditionsGitReader() : ConditionsReader() {
  declareProperty( "ValidityLower", m_context.valid_since );
  declareProperty( "ValidityUpper", m_context.valid_until );
  declareProperty( "ValidityLowerLimit", m_validityLowerLimit );
  declareProperty( "ValidityUpperLimit", m_validityUpperLimit );
  declareProperty( "DBTag", m_dbtag );
  declareProperty( "LimitedIOVPaths", m_limitedIovPaths );
}

void LHCb::Detector::ConditionsGitReader::setDirectory( const std::string& dir ) { m_directory = dir; }

int LHCb::Detector::ConditionsGitReader::getObject( const std::string& system_id, UserContext* ctxt,
                                                    std::string& buffer ) {

  if ( !m_condDB ) { m_condDB.emplace( GitCondDB::connect( m_directory ) ); }

  ConditionsReaderContext* ct = (ConditionsReaderContext*)ctxt;

  // We need to remove the first "/" if present
  std::string path       = system_id;
  auto        time_point = static_cast<GitCondDB::CondDB::time_point_t>( ct->event_time );

  // Now loading the condition
  // m_condDB->logger()->level = GitCondDB::Logger::Level::Debug;
  std::string            data{};
  GitCondDB::CondDB::IOV iov{};

  try {
    std::tie( data, iov ) = m_condDB->get( {m_dbtag, path, time_point} );
  } catch ( std::runtime_error const& e ) { return 0; }
  if ( !iov.valid() ) {
    throw std::runtime_error(
        fmt::format( "invalid IOV for {}:{}, cannot find data for point {} (probably missing 0 value)", m_dbtag, path,
                     time_point ) );
  }

  if ( !m_limitedIovPaths.empty() && m_limitedIovPaths.find( path ) != m_limitedIovPaths.end() ) {
    if ( iov.since != time_point ) {
      throw std::runtime_error(
          fmt::format( "invalid IOV for {}:{}, cannot find data for point {}", m_dbtag, path, time_point ) );
    }
    iov.until = iov.since + 1;
  }

  buffer = std::move( data );
  // GitCondDB uses uint64_t while DD4hep uses long, so we have to map one "max" to the other,
  // and make sure we do not use the part of uint64_t that long does not cover
  ct->valid_since = iov.since; // the IOV start is OK by construction
  if ( iov.until == GitCondDB::CondDB::IOV::max() ) {
    ct->valid_until = dd4hep::IOV::MAX_KEY;
  } else if ( iov.until >= static_cast<GitCondDB::CondDB::time_point_t>( dd4hep::IOV::MAX_KEY ) ) {
    throw std::runtime_error( fmt::format( "invalid IOV boundary ({} >= {}) accessing {}:{}", iov.until,
                                           dd4hep::IOV::MAX_KEY, m_dbtag, path ) );
  } else {
    // The -1 is needed because GitCondDB treats upper bounds as not included in the IOV,
    // while DD4hep includes it so we have to map [100,200) -> [100,199]
    ct->valid_until = iov.until - 1;
  }
  return 1;
}

/// Resolve a given URI to a string containing the data
bool LHCb::Detector::ConditionsGitReader::load( const std::string&, std::string& ) {
  throw std::logic_error( "ConditionsGitReader::load without contect should not be used" );
  return false;
}

/// Resolve a given URI to a string containing the data
bool LHCb::Detector::ConditionsGitReader::load( const std::string& system_id, UserContext* ctxt, std::string& buffer ) {
  // XXX TODO: Review the method to load from the git cond DB
  return getObject( system_id, ctxt, buffer );
}

/// Inform reader about a locally (e.g. by XercesC) handled source load
void LHCb::Detector::ConditionsGitReader::parserLoaded( const std::string& system_id ) {
  ConditionsReader::parserLoaded( system_id );
}

/// Inform reader about a locally (e.g. by XercesC) handled source load
void LHCb::Detector::ConditionsGitReader::parserLoaded( const std::string& /* system_id */, UserContext* ctxt ) {
  ConditionsReaderContext* ct = (ConditionsReaderContext*)ctxt;
  // Adjust IOV period according to setup (files have no IOV)
  ct->valid_since = m_context.valid_since;
  ct->valid_until = m_context.valid_until;
  // Check lower bound
  if ( ct->valid_since < m_validityLowerLimit )
    ct->valid_since = m_validityLowerLimit;
  else if ( ct->valid_since > m_validityUpperLimit )
    ct->valid_since = m_validityUpperLimit;
  // Check upper bound
  if ( ct->valid_until < m_validityLowerLimit )
    ct->valid_until = m_validityLowerLimit;
  else if ( ct->valid_until > m_validityUpperLimit )
    ct->valid_until = m_validityUpperLimit;
}

namespace {
  void* create_dddb_xml_file_reader( dd4hep::Detector&, int, char** ) {
    return new LHCb::Detector::ConditionsGitReader();
  }
} // namespace
DECLARE_DD4HEP_CONSTRUCTOR( LHCb_ConditionsGitReader, create_dddb_xml_file_reader )
