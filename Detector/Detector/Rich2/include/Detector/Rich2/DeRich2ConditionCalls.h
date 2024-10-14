/*****************************************************************************\
* (c) Copyright 2000-2019 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
// Author   Sajan Easo  2022-01-10
//
#pragma once

#include "Core/DeConditionCall.h"
#include "DD4hep/ConditionDerived.h"
#include "Detector/Rich2/DetElemAccess/DeRich2.h"

namespace LHCb::Detector {

  /// Condition derivation call to build the top level Rich2 DetElement condition information
  struct DeRich2ConditionCall : DeConditionCall {
    using DeConditionCall::DeConditionCall;
    virtual dd4hep::Condition operator()( const dd4hep::ConditionKey&           key,
                                          dd4hep::cond::ConditionUpdateContext& context ) override final;
  };

} // namespace LHCb::Detector
