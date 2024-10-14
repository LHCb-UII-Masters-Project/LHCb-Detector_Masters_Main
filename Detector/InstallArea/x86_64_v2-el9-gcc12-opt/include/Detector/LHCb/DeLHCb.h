/*****************************************************************************\
* (c) Copyright 2000-2023 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once

#include <Core/ConditionsRepository.h>
#include <Core/DeIOV.h>
#include <Core/Keys.h>
#include <Detector/LHCb/InteractionRegion.h>
#include <Detector/LHCb/LHCInfo.h>
#include <Detector/LHCb/SMOGInfo.h>
#include <Detector/LHCb/Tell40Links.h>

#include <fmt/core.h>

#include <memory>
#include <optional>

namespace LHCb::Detector {

  namespace detail {

    struct DeLHCbObject : DeIOVObject {
      DeLHCbObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );
      void               applyToAllChildren( const std::function<void( DeIOVElement<DeIOVObject> )>& ) const override;
      std::vector<DeIOV> children;
      Tell40Links        m_tell40links;
      std::optional<InteractionRegion> m_interactionRegion;
      std::optional<LHCInfo>           m_lhcinfo;
      std::optional<SMOGInfo>          m_SMOG;
    };

    // Utility method to lookup DeIOV object from the condition slice
    DeIOV get_DeIOV_from_context( dd4hep::cond::ConditionUpdateContext& context, std::string name );

  } // End namespace detail

  struct DeLHCb : DeIOVElement<detail::DeLHCbObject> {
    using DeIOVElement::DeIOVElement;

    const Tell40Links& tell40links() const { return access()->m_tell40links; }
    auto               tell40links( Tell40Links::SourceId id ) const { return access()->m_tell40links[id]; }

    /// Provide the position, spread, and tilt of the interaction region
    std::optional<InteractionRegion> interactionRegion() const { return this->access()->m_interactionRegion; }
    std::optional<LHCInfo>           lhcInfo() const { return this->access()->m_lhcinfo; }
    // Provides the SMOG injection type, the injected gas and the injection stability
    const std::optional<SMOGInfo> SMOG() const { return this->access()->m_SMOG; }
  };

  // Utility method to setup DeLHCb
  void setup_DeLHCb_callback( dd4hep::Detector& description );

} // End namespace LHCb::Detector
