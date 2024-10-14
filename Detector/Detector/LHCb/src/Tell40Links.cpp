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
#include <Detector/LHCb/Tell40Links.h>
#include <string>

LHCb::Detector::Tell40Links::Tell40Links( const nlohmann::json& obj ) {
  std::vector<std::uint8_t> values;
  for ( auto item = obj.begin(); item != obj.end(); ++item ) {
    SourceId sourceId = std::stoi( item.key() );
    auto&    defs     = item.value();
    links[sourceId]   = {defs.at( "links" ).get<std::vector<std::uint8_t>>(),
                       defs.at( "enabled" ).get<std::vector<std::uint8_t>>()};
  }
}

LHCb::Detector::Tell40Links::LinksStates::LinksStates( const std::vector<std::uint8_t>& indexes,
                                                       const std::vector<std::uint8_t>& values ) {
  if ( indexes.empty() ) { throw std::length_error( "empty indexes list" ); };
  if ( indexes.size() != values.size() ) {
    throw std::length_error( "inconsistent sizes of links and enabled vectors" );
  }
  offset = indexes.front();
  // the offset can only be 0, 12 or 24
  offset = ( offset < 12 ) ? 0 : ( offset < 24 ) ? 12 : 24;
  size   = static_cast<std::uint8_t>( indexes.back() - offset ) + 1;
  // the offset can only be 0, 12 or 24
  if ( size <= 12 ) {
    size = 12;
  } else if ( size <= 24 ) {
    size = 24;
  } else {
    throw std::length_error( "invalid size of values (too big) " + std::to_string( size ) );
  }

  for ( std::size_t pos = 0; pos < indexes.size(); ++pos ) {
    if ( values[pos] != 0 ) { mask |= 1 << ( indexes[pos] - offset ); }
  }
}

LHCb::Detector::Tell40Links::LinksStates::LinksStates( const std::uint8_t firstIndex, const std::uint8_t width,
                                                       const std::uint32_t bits )
    : offset{firstIndex}, size{width}, mask{bits} {
  if ( offset != 0 && offset != 12 && offset != 24 ) {
    throw std::length_error( "invalid offset " + std::to_string( offset ) + " (only allowed values are 0, 12, 24)" );
  }
  if ( size != 12 && size != 24 ) {
    throw std::length_error( "invalid size " + std::to_string( size ) + " (only allowed values are 12, 24)" );
  }
}
