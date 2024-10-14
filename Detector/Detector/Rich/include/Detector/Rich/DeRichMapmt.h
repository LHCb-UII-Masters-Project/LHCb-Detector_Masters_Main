/*****************************************************************************\
* (c) Copyright 2000-2019 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\******************************************************************************/

#pragma once

#include "Core/DeIOV.h"
#include "Detector/Rich/Types.h"
#include "Detector/Rich/Utilities.h"
#include <cstdint>

namespace LHCb::Detector::detail {

  struct DeRichMapmtObject : DeIOVObject {
  public:
    std::int32_t m_MapmtCopyNum           = -1;
    std::int32_t m_MapmtNumInModule       = -1;
    std::int32_t m_MapmtPanelLocalModuleN = -1;
    std::int32_t m_MapmtModuleCopyNum     = -1;
    std::int32_t m_numPixels              = -1;
    std::int32_t m_MapmtSide              = -1;
    float        m_effPixelArea           = -1;
    bool         m_isHType                = false;
    XYTable      m_PmtQE;

  public:
    DeRichMapmtObject( const dd4hep::DetElement&             de, //
                       dd4hep::cond::ConditionUpdateContext& ctxt );

    void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& ) const override {
      // no children
    }
  };

  template <typename ObjectType>
  struct DeRichMapmtElement : DeIOVElement<ObjectType> {
    using DeIOVElement<ObjectType>::DeIOVElement;
    auto copyNum() const noexcept { return this->access()->m_MapmtCopyNum; }
    auto side() const noexcept { return this->access()->m_MapmtSide; }
    auto numInModule() const noexcept { return this->access()->m_MapmtNumInModule; }
    auto parentModule() const noexcept { return this->access()->detector.parent(); }
    auto parentModuleCopyNum() const noexcept { return parentModule().id(); }
    auto panelLocalModuleN() const noexcept { return this->access()->m_MapmtPanelLocalModuleN; }
    auto moduleCopyNum() const noexcept { return this->access()->m_MapmtModuleCopyNum; }
    auto isHType() const noexcept { return this->access()->m_isHType; }
    auto QE() const noexcept { return scalePhotonEnergies( this->access()->m_PmtQE ); }
    auto effPixelArea() const noexcept { return this->access()->m_effPixelArea; }
    auto numPixels() const noexcept { return this->access()->m_numPixels; }
  };

} // namespace LHCb::Detector::detail
