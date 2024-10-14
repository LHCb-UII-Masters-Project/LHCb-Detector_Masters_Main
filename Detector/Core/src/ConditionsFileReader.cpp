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

// Framework includes
#include "Core/ConditionsReader.h"
#include "DD4hep/IOV.h"

/// Namespace for the AIDA detector description toolkit
namespace LHCb::Detector {

  /// Class supporting the interface of the LHCb conditions database to dd4hep
  /**
   *
   *  \author   M.Frank
   *  \version  1.0
   *  \ingroup DD4HEP_XML
   */
  class ConditionsFileReader : public ConditionsReader {

    long m_validityLowerLimit = 0;
    long m_validityUpperLimit = dd4hep::IOV::MAX_KEY;

  public:
    /// Standard constructor
    ConditionsFileReader();
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
  };
} // namespace LHCb::Detector

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
LHCb::Detector::ConditionsFileReader::ConditionsFileReader() : ConditionsReader() {
  declareProperty( "ValidityLower", m_context.valid_since );
  declareProperty( "ValidityUpper", m_context.valid_until );
  declareProperty( "ValidityLowerLimit", m_validityLowerLimit );
  declareProperty( "ValidityUpperLimit", m_validityUpperLimit );
}

int LHCb::Detector::ConditionsFileReader::getObject( const std::string& system_id, UserContext* ctxt,
                                                     std::string& buffer ) {
  std::string              path = system_id;
  int                      fid  = ::open( path.c_str(), O_RDONLY );
  ConditionsReaderContext* ct   = (ConditionsReaderContext*)ctxt;

  if ( fid == -1 ) {
    std::string p = m_directory + system_id;
    if ( p.substr( 0, 7 ) == "file://" )
      path = p.substr( 6 );
    else if ( p.substr( 0, 5 ) == "file:" )
      path = p.substr( 5 );
    else
      path = p;
    fid = ::open( path.c_str(), O_RDONLY );
  }
  if ( fid != -1 ) {
    struct stat buff;
    if ( 0 == ::fstat( fid, &buff ) ) {
      int   done = 0, len = buff.st_size;
      char* b = new char[len + 1];
      b[0]    = 0;
      while ( done < len ) {
        int sc = ::read( fid, b + done, buff.st_size - done );
        if ( sc >= 0 ) {
          done += sc;
          continue;
        }
        break;
      }
      ::close( fid );
      b[done] = 0;
      buffer  = b;
      delete[] b;
      if ( done >= len ) {
        ct->valid_since = ct->event_time;
        ct->valid_until = ct->event_time;
        return 1;
      }
      return 0;
    }
    ::close( fid );
  }
  return 0;
}

/// Resolve a given URI to a string containing the data
bool LHCb::Detector::ConditionsFileReader::load( const std::string& system_id, std::string& buffer ) {
  return dd4hep::xml::UriReader::load( system_id, buffer );
}
namespace {
  std::string _replace( const std::string& val, const std::string& sear, const std::string& rep ) {
    std::string ret = val;
    size_t      idx;
    while ( ( idx = ret.find( sear ) ) != std::string::npos ) { ret.replace( idx, idx + sear.length(), rep ); }
    return ret;
  }
} // namespace

/// Resolve a given URI to a string containing the data
bool LHCb::Detector::ConditionsFileReader::load( const std::string& system_id, UserContext* ctxt,
                                                 std::string& buffer ) {
  std::string val = m_directory + _replace( system_id, "git:", "" );
  return ConditionsReader::load( val, ctxt, buffer );
}

/// Inform reader about a locally (e.g. by XercesC) handled source load
void LHCb::Detector::ConditionsFileReader::parserLoaded( const std::string& system_id ) {
  ConditionsReader::parserLoaded( system_id );
}

/// Inform reader about a locally (e.g. by XercesC) handled source load
void LHCb::Detector::ConditionsFileReader::parserLoaded( const std::string& /* system_id */, UserContext* ctxt ) {
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
    return new LHCb::Detector::ConditionsFileReader();
  }
} // namespace
DECLARE_DD4HEP_CONSTRUCTOR( LHCb_ConditionsFileReader, create_dddb_xml_file_reader )
