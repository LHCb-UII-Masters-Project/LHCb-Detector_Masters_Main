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

// Framework include files
#include "XML/XMLElements.h"
#include "XML/XMLTags.h"

// Define unicode tags
#define UNICODE( x ) extern const ::dd4hep::xml::Tag_t Unicode_##x( #x )
#include "Core/UpgradeTags.h"
