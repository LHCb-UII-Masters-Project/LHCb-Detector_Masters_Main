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

  This test exercises some functionality of ROOT/DD4hep not yet used within
  LHCb but are of interest for the future.
  It is not so much a test as a start for future investigations

*/

#include <cassert>
#include <exception>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>

#include "Core/DetectorDataService.h"
#include "Core/FloatComparison.h"
#include "Core/GeometryTools.h"

#include "DD4hep/Detector.h"
#include "DD4hep/Printout.h"

#include "Math/Vector3D.h"
#include "TApplication.h"
#include "TGeoManager.h"
#include "TGeoNode.h"
#include "TGeoVolume.h"

int test_load_2_dd4hep_detectors() {

  // THis fails for the second Detector due to a clash in the evaluator
  // (the same one is used, should be discussed with the DD4hep team)
  dd4hep::Detector& desc1 = dd4hep::Detector::getInstance( "geo1" );
  desc1.fromXML( "tests/testscope/scope.xml" );

  std::cout << "##############################" << std::endl;
  dd4hep::Detector& desc2 = dd4hep::Detector::getInstance( "geo2" );
  desc2.fromXML( "tests/testscope/scope.xml" );

  return 0;
}

TGeoPhysicalNode* test_root_geom( TGeoManager* geom ) {

  // define some materials and media
  TGeoMaterial* matVacuum = new TGeoMaterial( "Vacuum", 0, 0, 0 );
  TGeoMaterial* matAl     = new TGeoMaterial( "Al", 26.98, 13, 2.7 );
  TGeoMaterial* matSi     = new TGeoMaterial( "Si", 28.085, 14, 2.329 );

  TGeoMedium* Vacuum = new TGeoMedium( "Vacuum", 1, matVacuum );
  TGeoMedium* Al     = new TGeoMedium( "Aluminium", 2, matAl );
  TGeoMedium* Si     = new TGeoMedium( "Silicium", 3, matSi );

  //--- make the top container volume
  TGeoVolume* top = geom->MakeBox( "world", Vacuum, 1000., 1000., 1000. );
  geom->SetTopVolume( top );

  // And the box we render
  TGeoVolume* b1 = geom->MakeBox( "box", Al, 5., 5., 5. );
  b1->SetLineColor( kRed );
  TGeoVolume* sensor = geom->MakeBox( "sensor", Si, 4., 4., 1. );
  sensor->SetLineColor( kBlue );
  TGeoTranslation* trs1 = new TGeoTranslation( 0., 0, -2. );
  b1->AddNode( sensor, 1, trs1 );
  TGeoTranslation* trs2 = new TGeoTranslation( 0., 0, 2. );
  b1->AddNode( sensor, 2, trs2 );
  b1->SetLineColor( kRed );
  b1->SetVisibility( kTRUE );

  TGeoTranslation* tr1 = new TGeoTranslation( 10., 0, 0. );
  top->AddNode( b1, 1, tr1 );
  TGeoTranslation* tr2 = new TGeoTranslation( -10., 0, 0. );
  top->AddNode( b1, 2, tr2 );

  // close the geometry and draw
  geom->CloseGeometry();
  geom->GetCache()->BuildIdArray();

  // Create an alignable volume for one of the nodes
  // https://root.cern/doc/master/TGeoPhysicalNode_8cxx_source.html#l00502
  // THis requires the global state
  // gGeoManager = geom;
  // TGeoPhysicalNode* pn = new TGeoPhysicalNode("/world/box_1");

  return nullptr;
}

bool test_navigation( TGeoManager* geom, double x0, const char* node_name ) {

  // Traversing the volumes using the navigators
  std::cout << "Using the navigation to cross " << node_name << ":\n";
  TGeoNavigator* nav        = geom->GetCurrentNavigator();
  bool           found_box1 = false;
  nav->SetCurrentPoint( x0, 0, -10 );
  nav->SetCurrentDirection( 0, 0, 1 );
  nav->FindNode();
  do {
    nav->FindNextBoundaryAndStep();
    auto c = nav->GetCurrentPoint();
    if ( strcmp( nav->GetCurrentNode()->GetName(), node_name ) == 0 ) { found_box1 = true; }

    ROOT::Math::XYZPoint current( c[0], c[1], c[2] );
    std::cout << current << " - " << nav->GetCurrentNode()->GetName() << " - "
              << nav->GetCurrentNode()->GetVolume()->GetMaterial()->GetName() << '\n';

    // Exiting when too far...
    if ( c[2] >= 50 ) { break; }

  } while ( true );
  return found_box1;
}

int main( int argc, char** argv ) {
  TApplication app( "app", &argc, argv );

  //  Creating a first TGeoManager
  // gGeoManager = 0;
  TGeoManager* geom1 = new TGeoManager( "geom1", "geom1" );
  test_root_geom( geom1 );
  auto* pn1  = new TGeoPhysicalNode( "/world_1/box_1" );
  auto* ps11 = new TGeoPhysicalNode( "/world_1/box_1/sensor_1" );
  // auto* ps12 = new TGeoPhysicalNode( "/world_1/box_1/sensor_2" );
  // auto* ps21 = new TGeoPhysicalNode( "/world_1/box_2/sensor_1" );
  // auto* ps22 = new TGeoPhysicalNode( "/world_1/box_2/sensor_2" );

  // And the second
  gGeoManager        = 0;
  TGeoManager* geom2 = new TGeoManager( "geom2", "geom2" );
  test_root_geom( geom2 );
  auto* pn2 = new TGeoPhysicalNode( "/world_1/box_1" );

  //  Iterating over the nodes of the first one
  TGeoIterator i1( geom1->GetTopVolume() );
  TGeoNode*    n1;
  TString      path;
  std::cout << "Listing the nodes in geom1:\n";
  while ( ( n1 = i1() ) ) {
    i1.GetPath( path );
    std::cout << path << '\n';
  }

  // Now aligning...
  TGeoTranslation* a1 = new TGeoTranslation( 10, 0, 0. );
  gGeoManager         = geom1;
  pn1->Align( a1 );

  TGeoTranslation* a2 = new TGeoTranslation( 15., 0, 0. );
  gGeoManager         = geom2;
  pn2->Align( a2 );

  // gGeoManager = nullptr;
  //  Now checking that we can see the effect of the different alignments
  geom1->GetCurrentNavigator()->cd( "/world_1/box_1" );
  std::cout << "Geom1 path:" << geom1->GetCurrentNavigator()->GetPath() << '\n';
  const TGeoHMatrix* tmp = geom1->GetCurrentNavigator()->GetCurrentMatrix();
  TGeoHMatrix*       h1  = new TGeoHMatrix( *tmp );
  h1->Print();

  // Now checking that we can see the effect of the different alignments
  geom2->GetCurrentNavigator()->cd( "/world_1/box_1" );
  tmp             = geom2->GetCurrentNavigator()->GetCurrentMatrix();
  TGeoHMatrix* h2 = new TGeoHMatrix( *tmp );
  h2->Print();

  // Checking navigation in both geometries
  gGeoManager = 0;
  assert( test_navigation( geom1, 10, "box_1" ) == true );
  assert( test_navigation( geom2, 15, "box_1" ) == true );

  // Now moving the volumes by 30cm and check if we find them at the right place....
  // careful we set the full matrix, not just and alignment...
  TGeoTranslation* a12 = new TGeoTranslation( 10. + 30., 0, 0. );
  gGeoManager          = geom1;
  pn1->Align( a12 );

  TGeoTranslation* a22 = new TGeoTranslation( 15. + 30., 0, 0. );
  gGeoManager          = geom2;
  pn2->Align( a22 );

  // We should not need the geGeoManager at that point
  gGeoManager = 0;
  assert( test_navigation( geom1, 10, "box_1" ) == false );
  assert( test_navigation( geom2, 15, "box_1" ) == false );
  assert( test_navigation( geom1, 40, "box_1" ) == true );
  assert( test_navigation( geom2, 45, "box_1" ) == true );

  // Now checking the independence of the sensors
  gGeoManager             = geom1;
  TGeoTranslation* ps11v2 = new TGeoTranslation( 0., 0, -3. );
  ps11->Align( ps11v2 );
  gGeoManager = nullptr;

  // Checking that this last allignment was applied correctly
  TGeoNavigator* nav = geom1->GetCurrentNavigator();
  nav->cd( "/world_1/box_1/sensor_1" );
  double lp[]  = {0, 0, 0};
  double gp1[] = {0, 0, 0};
  nav->LocalToMaster( lp, gp1 );
  std::cout << "Position: " << gp1[0] << ", " << gp1[1] << ", " << gp1[2] << '\n';
  nav->cd( "/world_1/box_2/sensor_1" );
  double gp2[] = {0, 0, 0};
  nav->LocalToMaster( lp, gp2 );
  std::cout << "Position: " << gp2[0] << ", " << gp2[1] << ", " << gp2[2] << '\n';
  // Here we check that the alignment of "/world_1/box_1/sensor_1"
  // did not affect the same placement in box_2
  assert( LHCb::essentiallyEqual( gp1[2], -3. ) );
  assert( LHCb::essentiallyEqual( gp2[2], -2. ) );

  /// To display just uncomment the following lines...
  // geom1->SetVisLevel( 4 );
  // geom1->GetTopVolume()->Draw( "ogle" );
  // app.Run();
  return 0;
}
