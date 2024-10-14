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
#include <DD4hep/DetFactoryHelper.h>
#include <iostream>

using namespace std;
using namespace dd4hep;

/*
 * Create the whole telescope in one one, as one detector element
 *
 *
 */
static Ref_t create_scope_all( Detector& description, xml_h e, SensitiveDetector sens ) {
  // Tell DD4hep we have a tracker: this is needed when using DDG4 to run
  // simulations with Geant4
  sens.setType( "tracker" );

  // Converting to xml_det_t in order to have useful accessor methods
  // Using them to create the main DetElement for our subdetector
  xml_det_t  x_det    = e;
  string     det_name = x_det.nameStr();
  DetElement scope_det( det_name, x_det.id() );

  // Assembly in which we will place all volumes
  Assembly scope_vol( det_name + "_assembly" );

  // Loading the box attribute and building the associated volume,
  // I.e. a Beryllium box with vacuum inside
  xml_dim_t x_box          = x_det.child( _U( box ) );
  double    box_hthickness = x_box.thickness() / 2.0;

  // we place a box of vacuum inside a box of beryllium
  Box    outbox( x_box.dx(), x_box.dy(), x_box.dz() );
  Box    intbox( x_box.dx() - box_hthickness, x_box.dy() - box_hthickness, x_box.dz() - box_hthickness );
  Volume scopebox_vol( "ScopeBox", outbox, description.material( "G4_Be" ) );
  Volume scopeboxint_vol( "ScopeBoxInt", intbox, description.material( "Vacuum" ) );
  scopebox_vol.placeVolume( scopeboxint_vol );
  scope_vol.placeVolume( scopebox_vol );

  // Now creating the sensor volume for a single sensor plane,
  // made of silicon
  xml_dim_t dim       = x_det.child( _U( dimensions ) );
  double    width     = dim.width();
  double    height    = dim.height();
  double    thickness = dim.thickness();
  double    gap       = dim.gap();
  Volume    sensor_vol( "SensorVol", Box( width / 2, height / 2, thickness / 2 ), description.material( "Silicon" ) );
  sensor_vol.setSensitiveDetector( sens );

  // Assembly gathering a set of SenvolVol in a single side
  Assembly sensor_array( "SideVol" );

  // // We need to keep track of placements in a given side
  // // to make sure we can attach the DetElements to the right ones
  vector<PlacedVolume> sensorPlacements;

  // Iterating to create individual sensors and place them
  int    sensor_count = dim.number();
  double interval     = gap + thickness;
  double box_length   = ( sensor_count - 1 ) * gap + thickness * sensor_count;

  for ( int sensor_num = 0; sensor_num < sensor_count; ++sensor_num ) {
    // As the centre of the boxes is in the middle, we offset the sensor a half length plus half the sensor itself
    double       placement_offset = -box_length / 2 + thickness / 2;
    PlacedVolume pv =
        sensor_array.placeVolume( sensor_vol, Position( 0, 0, placement_offset + sensor_num * interval ) );
    pv.addPhysVolID( "sensor", sensor_num );
    sensorPlacements.emplace_back( pv );
  }

  // Now iterating other the "side" tags to create both arrays
  int iside = 1;
  for ( xml_coll_t coll( e, _U( side ) ); coll; ++coll, ++iside ) {
    xml_dim_t pos = coll.child( _U( position ) );

    double x = pos.x();
    double y = pos.y();
    double z = pos.z();

    string     side_name = _toString( iside, "side%d" );
    DetElement side_det( scope_det, side_name, iside );

    // Iterating to create individual sensors and place them
    for ( int sensor_num = 0; sensor_num < sensor_count; ++sensor_num ) {
      string     sensor_name = _toString( sensor_num, "sensor%d" );
      DetElement sensor_det( side_det, sensor_name, sensor_num + 1 );
      sensor_det.setPlacement( sensorPlacements[sensor_num] );
    }
    PlacedVolume pv = scopeboxint_vol.placeVolume( sensor_array, Position( x, y, z ) );
    pv.addPhysVolID( "side", iside );
    side_det.setPlacement( pv );
  }

  // Finally placing the whole detector in the world
  Volume       motherVol = description.pickMotherVolume( scope_det );
  PlacedVolume phv       = motherVol.placeVolume( scope_vol, Position( 0, 0, 0 ) );
  phv.addPhysVolID( "system", x_det.id() );
  scope_det.setPlacement( phv );
  return scope_det;
}
DECLARE_DETELEMENT( TScopeAll_v1_0, create_scope_all )
