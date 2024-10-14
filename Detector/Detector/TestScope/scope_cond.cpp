/*****************************************************************************\
* (c) Copyright 2022 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/

#include "Core/ConditionHelper.h"
#include "Core/DeConditionCall.h"
#include "Core/DeIOV.h"
#include "DD4hep/ConditionDerived.h"
namespace {
  struct EmptyDeIOVObject : LHCb::Detector::detail::DeIOVObject {
    using DeIOVObject::DeIOVObject;
    void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& ) const override {}
  };
} // namespace

namespace LHCb::Detector {

  /// Condition derivation call to build the top level VP DetElement condition information
  struct TestScopeConditionCall : DeConditionCall {
    using DeConditionCall::DeConditionCall;
    virtual dd4hep::Condition operator()( const dd4hep::ConditionKey& /*   key */,
                                          dd4hep::cond::ConditionUpdateContext& context ) override final {

      context.condition( Keys::alignmentsComputedKey );
      return DeIOV( new EmptyDeIOVObject( dd4hep::Detector::getInstance().detector( "scope" ), context ) );
    }
  };
} // namespace LHCb::Detector

static long create_conditions_recipes( dd4hep::Detector& description, xml_h e ) {

  // Use the helper to load the XML, setup the callback according
  LHCb::Detector::ConditionConfigHelper<LHCb::Detector::TestScopeConditionCall> config_helper{description, "scope", e};
  config_helper.configure();

  return 1;
}
DECLARE_XML_DOC_READER( LHCb_TestScope_cond, create_conditions_recipes )

// this part is needed to generate the implementation of safe_cast methods
#include "DD4hep/detail/Handle.inl"
DD4HEP_INSTANTIATE_HANDLE_UNNAMED( EmptyDeIOVObject, LHCb::Detector::detail::DeIOVObject, ConditionObject );
