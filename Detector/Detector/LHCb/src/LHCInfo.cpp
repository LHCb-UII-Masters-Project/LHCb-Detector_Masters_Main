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
#include <Detector/LHCb/LHCInfo.h>

#include <Core/Utils.h>

#include <DD4hep/DD4hepUnits.h>
#include <DD4hep/Handle.h>

#include <nlohmann/json.hpp>

LHCb::Detector::LHCInfo::LHCInfo( const nlohmann::json& obj ) {
  if ( !obj.contains( "FillNumber" ) || !obj.contains( "LHCState" ) || !obj.contains( "LHCEnergy" ) ||
       !obj.contains( "LHCbClockPhase" ) ) {
    throw std::runtime_error{"LHC condition is empty"};
  }

  fillnumber     = obj.at( "FillNumber" );
  lhcstate       = obj.at( "LHCState" );
  lhcenergy      = obj.at( "LHCEnergy" );
  lhcbclockphase = obj.at( "LHCbClockPhase" );
  if ( obj.contains( "XAngleH" ) ) xangleh = obj.at( "XAngleH" );
  if ( obj.contains( "XAngleV" ) ) xanglev = obj.at( "XAngleV" );
  if ( obj.contains( "BeamType" ) ) beamtype = obj.at( "BeamType" );
}
