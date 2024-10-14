/*****************************************************************************\
* (c) Copyright 2000-2019 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/

#include "Detector/VP/DeVPSensor.h"
#include "Core/PrintHelpers.h"

#include "DD4hep/Primitives.h"
#include "DD4hep/Printout.h"

#include <nlohmann/json.hpp>

namespace {
  const auto key_info       = dd4hep::ConditionKey::itemCode( "StripInfo" );
  const auto key_noise      = dd4hep::ConditionKey::itemCode( "StripNoise" );
  const auto key_readout    = dd4hep::ConditionKey::itemCode( "StripReadout" );
  const auto key_sensorHV   = dd4hep::ConditionKey::itemCode( "SensorHV" );
  const auto key_chipStatus = dd4hep::ConditionKey::itemCode( "ChipStatus" );
  const auto key_badPixel   = dd4hep::ConditionKey::itemCode( "BadPixelList" );
} // namespace

LHCb::Detector::detail::DeVPSensorObject::DeVPSensorObject( const dd4hep::DetElement&             de,
                                                            dd4hep::cond::ConditionUpdateContext& ctxt,
                                                            unsigned int moduleId, bool _isLeft )
    : DeIOVObject( de, ctxt, 1008106 ) // matches DetDesc CLID_VeloAlignCond
    , sensorNumber( VPChannelID::SensorID{de.id() + VP::NSensorsPerModule * moduleId} )
    , moduleNumber( moduleId )
    , isLeft( _isLeft )
    , info( ctxt.condition( key_info, false ) )
    , noise( ctxt.condition( key_noise, false ) )
    , readout( ctxt.condition( key_readout, false ) ) {
  // get sensorHV, chipStatus and badPixel conditions
  auto condsh = ctxt.condition( key_sensorHV, false );
  if ( condsh.isValid() ) {
    auto& node = condsh.get<nlohmann::json>();
    sensorHV   = node.get<double>();
  }
  auto condcs = ctxt.condition( key_chipStatus, false );
  if ( condcs.isValid() ) {
    auto& node = condcs.get<nlohmann::json>();
    if ( node.size() != 3 ) {
      dd4hep::printout( dd4hep::WARNING, "DeVPSensorObject",
                        "Can not set chip status as not three values in the vector" );
    } else {
      for ( auto const& item : node ) { chipStatusOK = item.get<bool>(); }
    }
  }
  auto condbp = ctxt.condition( key_badPixel, false );
  if ( condbp.isValid() ) {
    auto& node = condbp.get<nlohmann::json>();
    for ( auto const& item : node ) { badChannels.emplace( item.get<int>() ); }
  }
  sizeX = nChips * chipSize + ( nChips - 1 ) * interChipDist;
  sizeY = chipSize;
  zpos  = toGlobal( ROOT::Math::XYZPoint{0.0, 0.0, 0.0} ).z();
  for ( unsigned int col = 0; col < VP::NSensorColumns; ++col ) {
    // Calculate the x-coordinate of the pixel centre and the pitch.
    double x0    = ( col / VP::NColumns ) * ( chipSize + interChipDist );
    double x     = x0 + ( col % VP::NColumns + 0.5 ) * pixelSize;
    double pitch = pixelSize;
    switch ( col ) {
    case 256:
    case 512:
      // right of chip border
      x -= 0.5 * ( interChipPixelSize - pixelSize );
      pitch = 0.5 * ( interChipPixelSize + pixelSize );
      break;
    case 255:
    case 511:
      // left of chip border
      x += 0.5 * ( interChipPixelSize - pixelSize );
      pitch = interChipPixelSize;
      break;
    case 254:
    case 510:
      // two left of chip border
      pitch = 0.5 * ( interChipPixelSize + pixelSize );
      break;
    }
    local_x[col] = x;
    x_pitch[col] = pitch;
  }
}

void LHCb::Detector::detail::DeVPSensorObject::print( int indent, int flg ) const {
  std::string prefix = getIndentation( indent );
  DeIOVObject::print( indent, flg );
  if ( flg & SPECIFIC ) {
    dd4hep::printout( dd4hep::INFO, "DeVPSensor", "%s+  >> Module:%d Sensor:%d %s %d Chips Rows:%d Cols:%d",
                      prefix.c_str(), moduleNumber, sensorNumber, isLeft ? "Left" : "Right", nChips, nCols, nRows );
  }
  if ( flg & DETAIL ) {
    dd4hep::printout( dd4hep::INFO, "DeVPSensor", "%s+  >> Thickness:%g ChipSize:%g Dist:%g Pix-Size:%g Dist:%g",
                      prefix.c_str(), thickness, chipSize, interChipDist, pixelSize, interChipPixelSize );
    dd4hep::printout( dd4hep::INFO, "DeVPSensor", "%s+  >> SizeX: %g SizeY: %g local:%ld pitch:%ld", prefix.c_str(),
                      sizeX, sizeY, local_x.size(), x_pitch.size() );
  }
  dd4hep::printout( dd4hep::INFO, "DeVPSensor", "%s+  >> Info: %s Noise:%s Readout:%s", prefix.c_str(),
                    dd4hep::yes_no( info.isValid() ), dd4hep::yes_no( noise.isValid() ),
                    dd4hep::yes_no( readout.isValid() ) );
}
