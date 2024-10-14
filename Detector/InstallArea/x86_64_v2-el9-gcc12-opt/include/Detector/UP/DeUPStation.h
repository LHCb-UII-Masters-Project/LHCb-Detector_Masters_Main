/*****************************************************************************\
* (c) Copyright 2000-2021 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once

#include "Core/DeIOV.h"

#include "Detector/UP/DeUPLayer.h"
#include "Detector/UP/UPChannelID.h"

#include <vector>

namespace LHCb::Detector {

  namespace detail {

    struct DeUPStationObject : LHCb::Detector::detail::DeIOVObject {
      DeUPStationObject( dd4hep::DetElement const& de, dd4hep::cond::ConditionUpdateContext& ctxt );
    };

  } // namespace detail

  template <typename ObjectType>
  class DeUPStationElement : public DeIOVElement<ObjectType> {
  public:
    using DeIOVElement<ObjectType>::DeIOVElement;

    /// check contains channel
    bool contains( const ChannelID aChannel ) const { return false; }

    /// locate the layer based on a channel id
    const DeUPLayer& findLayer( const ChannelID aChannel ) const { return m_noLayer; }

    /// locate layer based on a point
    const std::optional<DeUPLayer> findLayer( const ROOT::Math::XYZPoint& point ) const { return {}; }

    /// vector of children
    const std::vector<DeUPLayer>& layers() const { return m_emptyLayers; }

    /// fraction active channels
    double fractionActive() const { return 0; }

  private:
    std::vector<DeUPLayer> m_emptyLayers;
    DeUPLayer              m_noLayer;
  };
  using DeUPStation = DeUPStationElement<detail::DeUPStationObject>;

} // End namespace LHCb::Detector
