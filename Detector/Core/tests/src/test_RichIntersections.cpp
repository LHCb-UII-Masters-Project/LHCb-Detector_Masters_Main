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
#include "Detector/Rich1/DetElemAccess/DeRich1.h"
#include "Detector/Rich2/DetElemAccess/DeRich2.h"

#include "Core/DetectorDataService.h"
#include "Core/GeometryTools.h"
#include "Core/Keys.h"
#include "Core/MagneticFieldExtension.h"

#include "Detector/Rich/Types.h"

#include "DD4hep/Detector.h"
#include "DD4hep/Printout.h"

#include "Math/Vector3D.h"

#include <array>
#include <iostream>
#include <vector>

int main( int argc, char* argv[] ) {

  using TrajDataWithResult =
      std::tuple<ROOT::Math::XYZPoint, ROOT::Math::XYZVector, ROOT::Math::XYZPoint, ROOT::Math::XYZPoint>;

  const std::vector<TrajDataWithResult> r1TrajRes = {
      {{4.8339, -29.1795, 990}, {0.00492716, -0.0340466, 1}, {4.8339, -29.1795, 990}, {10.6233, -69.1843, 2165}},
      {{-98.3335, 17.5605, 990}, {-0.160749, 0.0358936, 1}, {-98.3335, 17.5605, 990}, {-287.214, 59.7355, 2165}},
      {{2.9279, -9.9783, 990}, {0.00322774, -0.0103524, 1}, {2.9279, -9.9783, 990}, {6.72049, -22.1424, 2165}},
      {{-40.8416, 14.2797, 990}, {-0.0587384, 0.00969204, 1}, {-40.8416, 14.2797, 990}, {-109.859, 25.6678, 2165}},
      {{197.9, -182.212, 990}, {0.204637, -0.188133, 1}, {197.9, -182.212, 990}, {438.348, -403.268, 2165}}};
  const std::vector<TrajDataWithResult> r2TrajRes = {
      {{-211.711, -52.4182, 9450},
       {-0.0398052, -0.00558938, 1},
       {-213.701, -52.6977, 9500},
       {-306.527, -65.7321, 11832}},
      {{877.05, 91.0958, 9500}, {0.285066, 0.00889497, 1}, {877.05, 91.0958, 9500}, {1541.82, 111.839, 11832}},
      {{2933.82, -1818.83, 9450}, {0.44724, -0.189439, 1}, {2956.18, -1828.3, 9500}, {3999.15, -2270.07, 11832}},
      {{1567.06, -230.439, 9500}, {0.280539, -0.0236485, 1}, {1567.06, -230.439, 9500}, {2221.28, -285.587, 11832}},
      {{-671.092, -96.1549, 9450}, {-0.133261, -0.0101545, 1}, {-677.755, -96.6626, 9500}, {-988.52, -120.343, 11832}}};
  const auto trajRes = std::array{&r1TrajRes, &r2TrajRes};

  // Loading the Rich1 and Rich2 DetectorElements
  // --------------------------------------------------------------------------

  // This script expects 3 arguments:
  // - the path to the geometry(compact/run3/trunk/LHCb.xml)
  // - conditions URL (git:/cvmfs/lhcb.cern.ch/lib/lhcb/git-conddb/lhcb-conditions-database.git@master)
  // - the magnetic field (/cvmfs/lhcb.cern.ch/lib/lhcb/DBASE/FieldMap/v5r7/cdf)
  const char* desc_xml       = "compact/run3/trunk/LHCb.xml";
  const char* conditions     = "git:/cvmfs/lhcb.cern.ch/lib/lhcb/git-conddb/lhcb-conditions-database.git@master";
  const char* field_map_path = "/cvmfs/lhcb.cern.ch/lib/lhcb/DBASE/FieldMap/v5r7/cdf";
  // If LHCB_CONDITION_DB is set respect its value
  const char* db_env = getenv( "LHCB_CONDITION_DB" );
  if ( db_env ) { conditions = db_env; }
  if ( argc > 3 ) {
    desc_xml       = argv[1];
    conditions     = argv[2];
    field_map_path = argv[3];
  }

  auto& desc = dd4hep::Detector::getInstance();
  desc.fromXML( desc_xml );

  LHCb::Magnet::setup_magnetic_field_extension( desc, field_map_path );
  std::vector<std::string>            detector_list{"/world", "Magnet", "Rich1", "Rich2"};
  LHCb::Detector::DetectorDataService dds( desc, detector_list );
  dds.initialize( nlohmann::json{{"repository", conditions}} );

  int                            slice1_id = 100;
  auto                           slice     = dds.get_slice( slice1_id );
  dd4hep::DetElement             rich1det  = desc.detector( "Rich1" );
  const LHCb::Detector::DeRich1& rich1     = slice->get( rich1det, LHCb::Detector::Keys::deKey );
  dd4hep::DetElement             rich2det  = desc.detector( "Rich2" );
  const LHCb::Detector::DeRich2& rich2     = slice->get( rich2det, LHCb::Detector::Keys::deKey );

  // Now using the geomnetry as taken from the DetectorElements
  // --------------------------------------------------------------------------
  // auto geom = rich1det.geometry();
  dd4hep::printout( dd4hep::INFO, "test_RichIntersect", "=> Rich1 path: %s", rich1.detector().path().c_str() );
  dd4hep::printout( dd4hep::INFO, "test_RichIntersect", "=> Rich2 path: %s", rich2.detector().path().c_str() );

  // Looking for Rich1
  auto rg1   = rich1.radiatorGas();
  auto r1gas = rg1.placement().ptr();
  dd4hep::printout( dd4hep::INFO, "test_RichIntersect", "Rich1gas node name: %s", r1gas->GetName() );
  dd4hep::printout( dd4hep::INFO, "test_RichIntersect", "Rich1gas: %s %d", r1gas->GetVolume()->GetShape()->GetName(),
                    r1gas->GetIndex() );

  // Looking for Rich2
  auto rg2   = rich2.radiatorGas();
  auto r2gas = rg2.placement().ptr();
  dd4hep::printout( dd4hep::INFO, "test_RichIntersect", "Rich2gas node name: %s", r2gas->GetName() );

  auto navigator = lhcb::geometrytools::get_navigator( desc );

  auto testIntersects = [&]( const Rich::DetectorType rich ) {
    auto node = ( Rich::Rich1 == rich ? r1gas : r2gas );
    for ( const auto& t : *trajRes[rich] ) {
      ROOT::Math::XYZPoint entry, exit;
      const auto           ok =
          lhcb::geometrytools::intersectionPoints( std::get<0>( t ), std::get<1>( t ), entry, exit, navigator, node );
      std::cout << "Trajectory " << std::get<0>( t ) << " " << std::get<1>( t ) << '\n';
      std::cout << " -> intersects = " << ok << '\n';
      std::cout << " -> entry      = " << entry << " diff: " << ( entry - std::get<2>( t ) ) << '\n';
      std::cout << " -> exit       = " << exit << " diff: " << ( exit - std::get<3>( t ) ) << '\n';
      std::cout << '\n';
    }
  };

  dd4hep::setPrintLevel( dd4hep::DEBUG );

  testIntersects( Rich::Rich1 );
  testIntersects( Rich::Rich2 );
}
