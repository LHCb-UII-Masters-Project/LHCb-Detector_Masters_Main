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

// Framework include files
#include "XML/XMLElements.h"

#ifndef UNICODE
#  define UNICODE( x ) extern const ::dd4hep::xml::Tag_t Unicode_##x
#endif

/// Namespace for the AIDA detector description toolkit
namespace LHCb {

  /// Namespace for the AIDA detector description toolkit supporting XML utilities
  namespace Detector {

    /// Namespace of conditions unicode tags
    namespace xml {
      UNICODE( alignment_pattern );
      UNICODE( condition_pattern );
      UNICODE( conditions_config );
      UNICODE( prefix );
      UNICODE( regex );
      UNICODE( path_regex );
      UNICODE( chambers );
      UNICODE( optional );
      // ******* Muon System Tags ******* //
      /*
      UNICODE( posXYZ );
      UNICODE( rotXYZ );
      UNICODE( sizeX );
      UNICODE( sizeY);
      UNICODE( sizeZ);
      UNICODE( rotX );
      UNICODE( rotY );
      UNICODE( rotZ );
      UNICODE( outerRadiusPZ );
      UNICODE( outerRadiusMZ);
      */
      // *** End of Muon System Tags *** //

    } // namespace xml
    // User must ensure there are no clashes. If yes, then the clashing entry is unnecessary.
    // using namespace ::dd4hep::xml::Conditions;
  } // namespace Detector
} // namespace LHCb
#undef UNICODE // Do not miss this one!

#include "XML/XMLTags.h"
#define _LBUC( x ) ::LHCb::Detector::xml::Unicode_##x
