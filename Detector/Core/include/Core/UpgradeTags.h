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

// Framework include files
#include "XML/XMLElements.h"

#ifndef UNICODE
#  define UNICODE( x ) extern const ::dd4hep::xml::Tag_t Unicode_##x
#endif

/// Namespace for the AIDA detector description toolkit
namespace dd4hep {

  /// Namespace for the AIDA detector description toolkit supporting XML utilities
  namespace xml {

    /// Namespace of conditions unicode tags
    namespace detector {

      UNICODE( classID );
      UNICODE( paramVector );

      UNICODE( uta );
      UNICODE( utb );
      UNICODE( jacket );
      UNICODE( balcony );
      UNICODE( correction );

      // BCM
      UNICODE( wire );
      UNICODE( contact );
      UNICODE( mount );

      // Muon
      UNICODE( posXYZ );
      UNICODE( rotXYZ );
      UNICODE( sizeX );
      UNICODE( sizeY );
      UNICODE( sizeZ );
      UNICODE( rotX );
      UNICODE( rotY );
      UNICODE( rotZ );
      UNICODE( outerRadiusPZ );
      UNICODE( outerRadiusMZ );
      UNICODE( paramphysvol );
      UNICODE( paramphysvol2D );
      UNICODE( paramphysvol3D );
      UNICODE( number1 );
      UNICODE( number2 );
      UNICODE( number3 );
      UNICODE( Union );
      UNICODE( boxes );

    } // namespace detector
    // User must ensure there are no clashes. If yes, then the clashing entry is unnecessary.
  } // namespace xml
} // namespace dd4hep
#undef UNICODE // Do not miss this one!

#include "XML/XMLTags.h"
#define _LBU( x ) ::dd4hep::xml::detector::Unicode_##x
