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
#include <Core/ConditionHelper.h>
#include <Core/DeConditionCall.h>
#include <DD4hep/ConditionDerived.h>
#include <DD4hep/DetElement.h>
#include <DD4hep/Detector.h>
#include <DD4hep/Printout.h>
#include <Detector/Calo/DeCalorimeter.h>
#include <yaml-cpp/yaml.h>

namespace {
  struct EcalConditionCall : LHCb::Detector::DeConditionCall {
    using DeConditionCall::DeConditionCall;
    virtual dd4hep::Condition operator()( const dd4hep::ConditionKey&           key,
                                          dd4hep::cond::ConditionUpdateContext& context ) override final {
      using namespace LHCb::Detector;
      if ( key.item_key() == Keys::deKey ) {
        // This is the pointer we need to return at the end of the callback
        auto det   = dd4hep::Detector::getInstance().detector( "Ecal" );
        auto deobj = new Calo::detail::DeCaloObject( det, context );
        return DeIOV( deobj );
      }
      throw std::logic_error( "Key unknown to EcalConditionCall" );
    }
  };
} // namespace

// DD4hep plugin to parse the Magnet_cond.xml and set up the conditions
static long create_conditions_recipes( dd4hep::Detector& description, xml_h e ) {

  // Use the helper to load the XML, setup the callback according
  LHCb::Detector::ConditionConfigHelper<EcalConditionCall> config_helper{description, "Ecal", e};
  config_helper.configure();

  return 1;
}
DECLARE_XML_DOC_READER( LHCb_Ecal_cond, create_conditions_recipes )
