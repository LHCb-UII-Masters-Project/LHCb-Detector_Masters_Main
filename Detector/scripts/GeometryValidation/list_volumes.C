/*****************************************************************************\
* (c) Copyright 2021 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
/*

List volumes in the geometry

*/

void list_volumes() {
  // Loading the library and geometry
  gSystem->Load( "libGeom" );
  TGeoManager::Import( "../data/Muon.gdml" );
  gGeoManager->SetVisLevel( 4 );

  // Now iterating
  TGeoIterator it( gGeoManager->GetTopVolume() );
  TGeoNode*    current;
  TString      path;
  while ( ( current = it.Next() ) ) {
    it.GetPath( path );
    std::cout << it.GetLevel() << " " << path << std::endl;
  }
}
