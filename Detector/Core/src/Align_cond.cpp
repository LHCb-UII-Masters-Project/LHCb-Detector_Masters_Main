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
#include "Core/ConditionsRepository.h"
#include "Core/DeAlignmentCall.h"
#include "Core/Keys.h"
#include "DD4hep/ConditionDerived.h"
#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/ExtensionEntry.h"
#include "DD4hep/Printout.h"

// C/C++ include files
#include <memory>

static long create_conditions_recipe( dd4hep::Detector& description, xml_h /* e */ ) {
  using Ext_t = std::shared_ptr<LHCb::Detector::ConditionsRepository>;
  auto world  = description.world();
  auto req    = world.extension<Ext_t>( false );
  if ( !req ) {
    req = new Ext_t( new LHCb::Detector::ConditionsRepository() );
    world.addExtension( new dd4hep::detail::DeleteExtension<Ext_t, Ext_t>( req ) );
  }
  dd4hep::cond::DependencyBuilder builder( world, LHCb::Detector::Keys::alignmentsComputedKey,
                                           std::make_shared<LHCb::Detector::DeAlignmentCall>( world ) );
  auto*                           dep = builder.release();
  dep->target.hash                    = LHCb::Detector::Keys::alignmentsComputedKey;
  ( *req )->addGlobal( dep );
  dd4hep::printout( dd4hep::INFO, "Alignments", "++ Add dependency: alignmentsComputedKey: %s %016llX",
                    world.path().c_str(), dep->target.hash );
  return 1;
}
DECLARE_XML_DOC_READER( LHCb_Align_cond, create_conditions_recipe )
