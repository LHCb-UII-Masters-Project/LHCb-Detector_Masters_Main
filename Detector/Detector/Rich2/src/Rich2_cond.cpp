//==========================================================================
//  LHCb Rich1 Detector condition implementation using DD4HEP
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
//
// Author     : Sajan Easo
// Date       : 2022-01-10
//
//==========================================================================
#include "Core/ConditionHelper.h"
#include "Detector/Rich2/DeRich2ConditionCalls.h"

static long create_Rich2_conditions_recipes( dd4hep::Detector& description, xml_h e ) {

  // Use the helper to load the XML, setup the callback according
  LHCb::Detector::ConditionConfigHelper<LHCb::Detector::DeRich2ConditionCall> config_helper{description, "Rich2", e};
  config_helper.configure();

  return 1;
}
DECLARE_XML_DOC_READER( LHCb_Rich2_cond, create_Rich2_conditions_recipes )
