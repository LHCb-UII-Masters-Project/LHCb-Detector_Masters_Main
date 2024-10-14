//==========================================================================
//  AIDA Detector description implementation
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
//  \author  Markus Frank
//  \date    2016-02-02
//  \version 1.0
//
//==========================================================================
#pragma once

#include "DD4hep/Conditions.h"
#include "DD4hep/DetElement.h"
#include "DDCond/ConditionsContent.h"

namespace LHCb::Detector {

  class ConditionsRepository;

  /**
   *  \author   M.Frank
   *  \version  1.0
   *  \ingroup  DD4HEP_GAUDI
   */
  class ConditionIdentifier {
  public:
    dd4hep::DetElement          detector{};
    std::string                 sys_id{};
    std::string                 object{};
    ConditionsRepository*       repository  = 0;
    dd4hep::Condition::key_type hash        = 0;
    int                         sys_id_hash = 0;
    int                         object_hash = 0;
    bool                        is_optional = false;

  public:
    ConditionIdentifier()                                  = default;
    ConditionIdentifier( ConditionIdentifier&& copy )      = default;
    ConditionIdentifier( const ConditionIdentifier& copy ) = default;
    ConditionIdentifier& operator=( ConditionIdentifier&& copy ) = default;
    ConditionIdentifier& operator=( const ConditionIdentifier& copy ) = default;
    ~ConditionIdentifier()                                            = default;
  };

  std::pair<dd4hep::Condition::key_type, dd4hep::cond::ConditionsLoadInfo*>
  add_condition_location( ConditionsRepository* content, dd4hep::DetElement de, dd4hep::Condition::itemkey_type item,
                          const std::string& file, const std::string& object );

} // namespace LHCb::Detector
