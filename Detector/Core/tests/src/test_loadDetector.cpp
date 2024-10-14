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
#include <iostream>

#include "Core/DetectorDataService.h"
#include "DD4hep/Detector.h"
#include "DD4hep/Printout.h"

#include "Math/Vector3D.h"

int main() {

  dd4hep::Detector& desc = dd4hep::Detector::getInstance();
  desc.fromXML( "compact/run3/trunk/LHCb.xml" );

  ROOT::Math::XYZPoint start_point{0, 0, 0};
  ROOT::Math::XYZPoint end_point{10, 10, 50};

  auto                  nav       = gGeoManager->GetCurrentNavigator();
  ROOT::Math::XYZVector direction = ( end_point - start_point ).Unit();

  nav->SetCurrentPoint( start_point.X(), start_point.Y(), start_point.Z() );
  nav->SetCurrentDirection( direction.X(), direction.Y(), direction.Z() );
  double distancesq = ( end_point - start_point ).mag2();
  do {
    nav->FindNextBoundaryAndStep( std::sqrt( distancesq ) );
    auto                 c = nav->GetCurrentPoint();
    ROOT::Math::XYZPoint current( c[0], c[1], c[2] );

    std::ostringstream os;
    os << current << " - " << nav->GetCurrentNode()->GetName() << " - "
       << nav->GetCurrentNode()->GetVolume()->GetMaterial()->GetName();
    dd4hep::printout( dd4hep::INFO, "find_intersec", "%s", os.str().c_str() );
    distancesq = ( end_point - current ).mag2();

  } while ( distancesq > 0.1 );
}
