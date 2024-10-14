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
#pragma once

// Framework includes
#include "XML/UriReader.h"

/// Namespace for the AIDA detector description toolkit
namespace LHCb::Detector {

  /// CondDB reader context to support intervals of validity
  /**
   *   \author  M.Frank
   *   \version 1.0
   *   \date    31/03/2016
   */
  class ConditionsReaderContext : public dd4hep::xml::UriReader::UserContext {
  public:
    long        event_time  = 0;
    long        valid_since = 0;
    long        valid_until = 0;
    std::string doc, channel, match;
    /// Standard constructor
    ConditionsReaderContext() = default;
    /// Copy constructor
    ConditionsReaderContext( const ConditionsReaderContext& c )
        : dd4hep::xml::UriReader::UserContext( c )
        , event_time( c.event_time )
        , valid_since( c.valid_since )
        , valid_until( c.valid_until )
        , channel( c.channel ) {}
    /// Default destructor
    virtual ~ConditionsReaderContext() = default;
    /// Assignment operator
    ConditionsReaderContext& operator=( const ConditionsReaderContext& c ) = default;
  };
} /* End namespace LHCb::Detector            */
