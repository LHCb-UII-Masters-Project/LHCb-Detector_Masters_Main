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

  struct DeRichPDModuleObject : DeIOVObject {
    using DeIOVObject::DeIOVObject;
    std::int32_t m_PDModuleCopyNum     = -1;
    std::int32_t m_PDModuleSide        = -1;
    std::int32_t m_PDPanelLocalModuleN = -1;
    std::int32_t m_NumMapmtsInModule   = -1;
  };

  template <typename ObjectType>
  struct DeRichPDModuleElement : DeIOVElement<ObjectType> {
    using DeIOVElement<ObjectType>::DeIOVElement;
    auto PDPanelLocalModuleN() const noexcept { return this->access()->m_PDPanelLocalModuleN; }
    auto PDModuleSide() const noexcept { return this->access()->m_PDModuleSide; }
    auto PDModuleCopyNum() const noexcept { return this->access()->m_PDModuleCopyNum; }
    auto NumMapmtsInModule() const noexcept { return this->access()->m_NumMapmtsInModule; }
  };

} // namespace LHCb::Detector::detail
