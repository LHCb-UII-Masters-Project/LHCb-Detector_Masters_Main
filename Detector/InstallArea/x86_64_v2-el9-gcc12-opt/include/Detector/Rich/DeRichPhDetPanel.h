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
#include <cstdint>

namespace LHCb::Detector::detail {

  struct DeRichPhDetPanelObject : DeIOVObject {
    using DeIOVObject::DeIOVObject;
    std::int32_t m_PhDetPanelCopyIdRef        = -1;
    std::int32_t m_PhDetPanelN                = -1;
    std::int32_t m_NumMapmtModulesInAPanel    = -1;
    std::int32_t m_NumMapmtStdModulesInAPanel = -1;
  };

  template <typename ObjectType>
  struct DeRichPhDetPanelElement : DeIOVElement<ObjectType> {
    using DeIOVElement<ObjectType>::DeIOVElement;
    auto PhDetPanelN() const noexcept { return this->access()->m_PhDetPanelN; }
    auto PhDetPanelCopyIdRef() const noexcept { return this->access()->m_PhDetPanelCopyIdRef; }
    auto NumMapmtStdModulesInAPanel() const noexcept { return this->access()->m_NumMapmtStdModulesInAPanel; }
    auto NumMapmtModulesInAPanel() const noexcept { return this->access()->m_NumMapmtModulesInAPanel; }
  };

} // namespace LHCb::Detector::detail
