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
#include "DD4hep/ConditionDerived.h"
#include "DD4hep/DetElement.h"
#include "DD4hep/Detector.h"
#include "DD4hep/Printout.h"
#include "Detector/Magnet/DeMagnet.h"

#include "Core/ConditionHelper.h"
#include "Core/DeConditionCall.h"
#include "Core/MagneticFieldExtension.h"

#include <yaml-cpp/yaml.h>

#include <memory>

// Callback to setup  the DeMagnet object
namespace LHCb::Detector {

  /// Condition derivation call to build the top level VP DetElement condition information
  // It can process two keys
  struct DeMagnetConditionCall : DeConditionCall {
    using DeConditionCall::DeConditionCall;
    virtual dd4hep::Condition operator()( const dd4hep::ConditionKey&           key,
                                          dd4hep::cond::ConditionUpdateContext& context ) override final {

      if ( key.item_key() == LHCb::Detector::Keys::deKey ) {

        // This is the pointer we need to return at the end of the callback
        auto magnetdet = dd4hep::Detector::getInstance().detector( "Magnet" );

        // Loading the Magnetic field
        auto mfhelper = magnetdet.extension<LHCb::Magnet::MagneticFieldExtension>();
        dd4hep::printout( dd4hep::INFO, "DeMagnetConditionCall", "Loading mag field from %s",
                          mfhelper->fieldMapLocation().c_str() );

        // Creating the Detector element with the desired field
        const auto data  = mfhelper->load_magnetic_field_grid( context );
        auto demagnetobj = std::make_unique<detail::DeMagnetObject>( magnetdet, context, data.grid, data.useRealMap,
                                                                     data.isDown, data.signedRelativeCurrent );

        return DeIOV( demagnetobj.release() );
      }
      throw std::logic_error( "Key unknown to DeMagnetConditionCall" );
    }
  };

} // namespace LHCb::Detector

// DD4hep plugin to parse the Magnet_cond.xml and set up the conditions
static long create_conditions_recipes( dd4hep::Detector& description, xml_h e ) {

  // Use the helper to load the XML, setup the callback according
  LHCb::Detector::ConditionConfigHelper<LHCb::Detector::DeMagnetConditionCall> config_helper{description, "Magnet", e};
  config_helper.configure();

  return 1;
}

DECLARE_XML_DOC_READER( LHCb_Magnet_cond, create_conditions_recipes )
