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
#include <Detector/LHCb/InteractionRegion.h>

#include <Core/Utils.h>

#include <DD4hep/DD4hepUnits.h>
#include <DD4hep/Handle.h>

#include <nlohmann/json.hpp>

template <std::size_t Size>
std::array<double, Size> arrayFromJson( const nlohmann::json& jsonArray, const double unit ) {
  if ( jsonArray.size() != Size ) { throw std::runtime_error{"InteractionRegion condition array is wrong size"}; }

  std::array<double, Size> values;

  for ( std::size_t i{0}; i < Size; ++i ) {
    if ( jsonArray[i].is_string() ) {
      values[i] = dd4hep::_toDouble( jsonArray[i].get<std::string>() );
    } else if ( jsonArray[i].is_number() ) {
      values[i] = jsonArray[i].get<double>() * unit;
    } else {
      throw std::runtime_error{"InteractionRegion condition contains unexpected types"};
    }
  }

  return values;
}

LHCb::Detector::InteractionRegion::InteractionRegion( const nlohmann::json& obj ) {
  if ( !obj.contains( "position" ) || !obj.contains( "spread" ) ) {
    throw std::runtime_error{"InteractionRegion condition is empty"};
  }

  auto&                       pos = obj.at( "position" );
  constexpr std::size_t       posSize{3};
  std::array<double, posSize> posVals{arrayFromJson<posSize>( pos, dd4hep::mm )};

  auto&                       spr = obj.at( "spread" );
  constexpr std::size_t       sprSize{6};
  std::array<double, sprSize> sprVals{arrayFromJson<sprSize>( spr, dd4hep::mm2 )};

  avgPosition.SetCoordinates( posVals.begin(), posVals.end() );

  spread.SetElements( sprVals.begin(), sprVals.end(), true );
}
