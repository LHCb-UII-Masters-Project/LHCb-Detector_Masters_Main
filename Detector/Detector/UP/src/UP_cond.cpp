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
// Specialized generic detector constructor
//
//==========================================================================
#include "Core/ConditionHelper.h"
#include "Detector/UP/DeUPConditionCalls.h"

static long create_conditions_recipes( dd4hep::Detector& description, xml_h e ) {

  // Use the helper to load the XML, setup the callback according
  LHCb::Detector::ConditionConfigHelper<LHCb::Detector::DeUPConditionCall> config_helper{description, "UP", e};
  config_helper.configure();

  return 1;
}
DECLARE_XML_DOC_READER( LHCb_UP_cond, create_conditions_recipes )
