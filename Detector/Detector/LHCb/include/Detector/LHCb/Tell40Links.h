/*****************************************************************************\
* (c) Copyright 2023 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once

#include <cassert>
#include <cstdint>
#include <map>
#include <nlohmann/json.hpp>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace LHCb::Detector {
  struct Tell40Links {

    struct InvalidLinkIndex : std::out_of_range {
      using out_of_range::out_of_range;
    };

    Tell40Links() = default;
    Tell40Links( const nlohmann::json& obj );

    using SourceId = std::uint16_t;
    struct LinksStates {
      LinksStates() = default;
      LinksStates( const std::vector<std::uint8_t>& indexes, const std::vector<std::uint8_t>& values );
      LinksStates( const std::uint8_t firstIndex, const std::uint8_t width, const std::uint32_t bits );
      std::uint8_t  offset{};
      std::uint8_t  size{};
      std::uint32_t mask{};

      bool isEnabled( std::uint8_t idx ) const {
        if ( idx >= size ) { throw InvalidLinkIndex{"invalid Tell40 link index " + std::to_string( idx )}; }
        return mask & ( 1 << idx );
      }
    };
    std::map<SourceId, LinksStates> links;

    std::optional<LinksStates> operator[]( SourceId id ) const {
      if ( auto it = links.find( id ); it != links.end() ) { return it->second; }
      return {};
    }
  };
} // namespace LHCb::Detector
