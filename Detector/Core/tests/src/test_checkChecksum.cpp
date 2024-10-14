/*****************************************************************************\
* (c) Copyright 2020 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#include <exception>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>

#include "Core/Checksum.h"
#include "Core/DetectorDataService.h"
#include "Core/GeometryTools.h"

#include "DD4hep/Detector.h"
#include "DD4hep/Printout.h"

#include "Math/Vector3D.h"
#include "TGeoManager.h"
#include "TGeoNode.h"
#include "TGeoVolume.h"

int main() {

  dd4hep::Detector& desc = dd4hep::Detector::getInstance();
  desc.fromXML( "tests/testscope/scope.xml" );

  std::map<std::string, lhcb::Detector::checksum::AccumulatorCrc::ret> checksums;
  checksums = lhcb::Detector::checksum::geometry_crc( gGeoManager->GetTopNode(), 7 );

  for ( auto& e : checksums ) { std::cout << e.first << ":" << e.second << std::endl; }

  int                                           retval   = 0;
  lhcb::Detector::checksum::AccumulatorCrc::ret expected = 0x5154ccca;
  if ( checksums["/world_volume"] != expected ) {
    std::cout << "Expected Checksum " << std::hex << expected << " computed: " << checksums["/world_volume"] << '\n';
    retval = 1;
  }
  return retval;
}
