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
/*
 Example of navigation through a subtraction solid
 */

#include "Math/Point3D.h"
#include "Math/Vector3D.h"
#include "TApplication.h"
#include "TGeoBBox.h"
#include "TGeoCompositeShape.h"
#include "TGeoManager.h"
#include "TGeoNode.h"
#include "TGeoTube.h"
#include "TGeoVolume.h"

#include <string>

void test_root_geom( TGeoManager* geom ) {

  // define some materials and media
  TGeoMaterial* matVacuum = new TGeoMaterial( "Vacuum", 0, 0, 0 );
  TGeoMaterial* matAl     = new TGeoMaterial( "Al", 26.98, 13, 2.7 );

  TGeoMedium* Vacuum = new TGeoMedium( "Vacuum", 1, matVacuum );
  TGeoMedium* Al     = new TGeoMedium( "Aluminium", 2, matAl );

  // make the top container volume
  TGeoVolume* top = geom->MakeBox( "world", Vacuum, 1000., 1000., 1000. );
  geom->SetTopVolume( top );

  // Making our subtraction volume
  [[maybe_unused]] TGeoBBox* boxShape  = new TGeoBBox( "box", 50, 50, 100 );
  [[maybe_unused]] TGeoTube* tubeShape = new TGeoTube( "tube", 0., 10., 100.0 + 1e-8 );
  TGeoCompositeShape*        compShape = new TGeoCompositeShape( "cs", "box-tube" );
  TGeoVolume*                compVol   = new TGeoVolume( "compVol", compShape, Al );

  top->AddNode( compVol, 1 );

  // close the geometry and draw
  geom->CloseGeometry();
  geom->GetCache()->BuildIdArray();
}

void test_navigation( TGeoManager* geom, ROOT::Math::XYZPoint initial, ROOT::Math::XYZPoint final ) {
  // Traversing the volumes using the navigators
  std::cout << "\nChecking the materials found between " << initial << " and " << final << '\n';
  TGeoNavigator* nav = geom->GetCurrentNavigator();
  nav->SetCurrentPoint( initial.X(), initial.Y(), initial.Z() );
  auto direction = ( final - initial ).Unit();
  nav->SetCurrentDirection( direction.X(), direction.Y(), direction.Z() );
  nav->FindNode( initial.X(), initial.Y(), initial.Z() );
  std::cout << initial << " - " << nav->GetCurrentNode()->GetName() << " - "
            << nav->GetCurrentNode()->GetVolume()->GetMaterial()->GetName() << '\n';

  double dist2_final = ( final - initial ).Mag2();
  double dist2       = 0;
  do {
    nav->FindNextBoundaryAndStep();
    auto c = nav->GetCurrentPoint();

    ROOT::Math::XYZPoint current( c[0], c[1], c[2] );
    std::cout << current << " - " << nav->GetCurrentNode()->GetName() << " - "
              << nav->GetCurrentNode()->GetVolume()->GetMaterial()->GetName() << '\n';

    dist2 = ( current - final ).Mag2();
  } while ( dist2 < dist2_final );
}

int main( int argc, char** argv ) {
  TApplication app( "app", &argc, argv );

  //  Creating a first TGeoManager
  // gGeoManager = 0;
  TGeoManager* geom1 = new TGeoManager( "geom1", "geom1" );
  test_root_geom( geom1 );
  test_navigation( geom1, ROOT::Math::XYZPoint( -100, 0, 0 ), ROOT::Math::XYZPoint( 100, 0, 0 ) );
  test_navigation( geom1, ROOT::Math::XYZPoint( -60, -60, 0 ), ROOT::Math::XYZPoint( 60, 60, 0 ) );
  test_navigation( geom1, ROOT::Math::XYZPoint( 0, 0, -100 ), ROOT::Math::XYZPoint( 0, 0, 100 ) );
  /// To display just uncomment the following lines...
  // geom1->SetVisLevel( 4 );
  // geom1->GetTopVolume()->Draw( "ogle" );
  // app.Run();
  return 0;
}
