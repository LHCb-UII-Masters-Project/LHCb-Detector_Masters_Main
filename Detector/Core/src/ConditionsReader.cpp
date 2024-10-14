//==========================================================================
//  AIDA Detector description implementation
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
// Author     : M.Frank
//
//==========================================================================
//
// DDDB is a detector description convention developed by the LHCb experiment.
// For further information concerning the DTD, please see:
// http://lhcb-comp.web.cern.ch/lhcb-comp/Frameworks/DetDesc/Documents/lhcbDtd.pdf
//
//==========================================================================

// Framework includes
#include "Core/ConditionsReader.h"
#include "Core/ConditionsReaderContext.h"
#include "DD4hep/Primitives.h"
#include "DD4hep/Printout.h"

// C/C++ include files
#include <cstring>

/// Standard constructor
LHCb::Detector::ConditionsReader::ConditionsReader( const std::string& dir )
    : m_directory( dir ), m_match( "conddb:" ) {
  declareProperty( "Directory", m_directory );
  declareProperty( "Match", m_match );
  m_context.valid_since = dd4hep::detail::makeTime( 1970, 1, 1 );
  m_context.valid_until = dd4hep::detail::makeTime( 2030, 1, 1 );
  m_context.event_time  = dd4hep::detail::makeTime( 2015, 7, 1, 12, 0, 0 );
  m_context.match       = m_match;
}

/// Access to properties
dd4hep::Property& LHCb::Detector::ConditionsReader::operator[]( const std::string& property_name ) {
  return properties().property( property_name );
}

/// Access to local context
dd4hep::xml::UriReader::UserContext* LHCb::Detector::ConditionsReader::context() { return &m_context; }

/// Resolve a given URI to a string containing the data
bool LHCb::Detector::ConditionsReader::load( const std::string& system_id, std::string& buffer ) {
  return dd4hep::xml::UriReader::load( system_id, buffer );
}

/// Resolve a given URI to a string containing the data
bool LHCb::Detector::ConditionsReader::load( const std::string& system_id, UserContext* ctxt, std::string& buffer ) {
  if ( isBlocked( system_id ) ) {
    buffer = "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>  <DDDB></DDDB>";
    return true;
  } else if ( system_id.substr( 0, m_match.length() ) == m_match ) {
    std::string        mm     = m_match + "//";
    size_t             mm_len = mm.length();
    const std::string& sys    = system_id;
    std::string        id     = sys.c_str() + ( sys.substr( 0, mm_len ) == mm ? mm_len : mm_len - 2 );
    // Extract the COOL field name from the condition path
    // "conddb:/path/to/field@folder"
    std::string::size_type at_pos = id.find( '@' );
    if ( at_pos != id.npos ) {
      std::string::size_type slash_pos = id.rfind( '/', at_pos );
      // always remove '@' from the path
      id = id.substr( 0, slash_pos + 1 ) + id.substr( at_pos + 1 );
    }
    // GET: 1458055061070516000 /lhcb.xml 0 0 SUCCESS
    int ret = getObject( id, ctxt, buffer );
    if ( ret == 1 ) return true;
    dd4hep::printout( dd4hep::ERROR, "ConditionsReader", "++ Failed to resolve system id: %s [%s]", id.c_str(),
                      ::strerror( errno ) );
  }
  return false;
}

/// Inform reader about a locally (e.g. by XercesC) handled source load
void LHCb::Detector::ConditionsReader::parserLoaded( const std::string& system_id ) {
  dd4hep::xml::UriReader::parserLoaded( system_id );
}

/// Inform reader about a locally (e.g. by XercesC) handled source load
void LHCb::Detector::ConditionsReader::parserLoaded( const std::string& /* system_id */, UserContext* ctxt ) {
  ConditionsReaderContext* c = (ConditionsReaderContext*)ctxt;
  c->valid_since             = c->event_time;
  c->valid_until             = c->event_time;
}

/// Add a blocked path entry
void LHCb::Detector::ConditionsReader::blockPath( const std::string& path ) { m_blockedPathes.insert( path ); }

/// Check if a URI path is blocked
bool LHCb::Detector::ConditionsReader::isBlocked( const std::string& path ) const {
  for ( const auto& p : m_blockedPathes ) {
    if ( path.find( p ) != std::string::npos ) return true;
  }
  return false;
}

void LHCb::Detector::ConditionsReader::setDirectory( const std::string& dir ) {
  m_directory = dir;
  std::cout << "================================================ ConditionsReader SET DIRECTORY ========== " << dir
            << std::endl;
}
