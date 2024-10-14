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
#include "Core/GeometryTools.h"
#include "Core/ShapeUtilities.h"
#include "Core/Units.h"

#include "TClass.h"
#include "TGeoBBox.h"
#include "TGeoManager.h"
#include "TGeoMatrix.h"
#include "TGeoNode.h"

#include "DD4hep/Detector.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

using LHCb::Detector::detail::DD4hepToLHCbCm;
using LHCb::Detector::detail::toDD4hepUnits;
/**
 *  Utilities to serialize the various ROOT geometry classes
 */
namespace {

  void serialize( TGeoMatrix& m, std::stringstream& stream ) {

    stream << ":mx:" << std::setprecision( 8 );
    const Double_t* rot = m.GetRotationMatrix();
    const Double_t* tr  = m.GetTranslation();

    stream << " rot:";
    for ( int i = 0; i < 9; i++ ) { stream << lhcb::Detector::shapeutils::discretize_rotmatrix( rot[i] ) << "|"; }
    stream << " trans:";
    for ( int i = 0; i < 3; i++ ) { stream << lhcb::Detector::shapeutils::discretize_length( tr[i] ) << "|"; }

    if ( m.IsScale() ) {
      stream << " scale:";
      const Double_t* scl = m.GetScale();
      for ( int i = 0; i < 3; i++ ) { stream << scl[i] << "|"; }
    }
  }

  void serialize( TGeoMaterial& m, std::stringstream& stream ) { stream << ":mat:" << m.GetName(); }

  void serialize( TGeoMedium& m, std::stringstream& stream ) {
    stream << ":med:" << m.GetName();
    serialize( *( m.GetMaterial() ), stream );
  }

  void serialize( TGeoShape& n, std::stringstream& stream ) {
    stream << ":shape:" << n.IsA()->GetName() << std::setprecision( 8 );
    std::vector<double> dimensions = lhcb::Detector::shapeutils::get_shape_dimensions( &n );
    for ( auto& d : dimensions ) { stream << d << "|"; }
  }

  void serialize( TGeoNode& n, std::stringstream& stream ) {
    stream << n.GetName();
    serialize( *( n.GetMatrix() ), stream );
    serialize( *( n.GetMedium() ), stream );
    serialize( *( n.GetVolume()->GetShape() ), stream );
  }

} // namespace

std::string lhcb::geometrytools::toString( TGeoNode* node ) {
  std::stringstream nodestr;
  serialize( *node, nodestr );
  return nodestr.str();
}

TGeoNode* lhcb::geometrytools::find_daughter( TGeoNode* mother, const std::string& daughter_name ) {
  TGeoIterator next( mother->GetVolume() );
  next.SetType( 1 ); // iterate just on children
  TGeoNode* res = nullptr;
  TGeoNode* c   = nullptr;
  while ( ( c = next() ) ) {
    if ( strncmp( c->GetName(), daughter_name.c_str(), strlen( daughter_name.c_str() ) ) == 0 ) {
      res = c;
      break;
    }
  }
  return res;
}

bool lhcb::geometrytools::intersectionPoints( const ROOT::Math::XYZPoint&  start_point,
                                              const ROOT::Math::XYZVector& direction, ROOT::Math::XYZPoint& entry,
                                              ROOT::Math::XYZPoint& exit, TGeoNavigator* navigator, TGeoNode* node,
                                              double max_dist ) {

  // Convert to ROOT units...
  max_dist = toDD4hepUnits( max_dist ); // max dist is expressed in LHCb units
  ROOT::Math::XYZPoint  r_start_point{toDD4hepUnits( start_point )};
  ROOT::Math::XYZVector r_direction = direction.Unit();
  double                max_dist2   = max_dist * max_dist;
  bool                  found       = false;

  auto findPoints = [&]( auto& firstPtn, auto& secondPtn, const bool forward ) {
    // set the direction of search
    if ( forward ) {
      dd4hep::printout( dd4hep::DEBUG, "intersectionPoints", "Forward search" );
      navigator->SetCurrentDirection( r_direction.X(), r_direction.Y(), r_direction.Z() );
    } else {
      dd4hep::printout( dd4hep::DEBUG, "intersectionPoints", "Backward search" );
      navigator->SetCurrentDirection( -r_direction.X(), -r_direction.Y(), -r_direction.Z() );
    }

    // are we already inside the target volume ?
    const bool startsInNode = ( node == navigator->GetCurrentNode() );
    if ( startsInNode ) {
      dd4hep::printout( dd4hep::DEBUG, "intersectionPoints", " -> Trajectory reference point is inside target volume" );
    }

    // search caches
    double               distancesq = 0;
    ROOT::Math::XYZPoint current{};

    auto log_position = [&navigator, &current]( const auto& msg ) {
      if ( dd4hep::isActivePrintLevel( dd4hep::DEBUG ) ) {
        std::ostringstream os;
        os << navigator->GetCurrentPoint() << " - " << ( current * DD4hepToLHCbCm ) << " - "
           << navigator->GetCurrentNode()->GetName() << " - "
           << navigator->GetCurrentNode()->GetVolume()->GetMaterial()->GetName() << "|" << navigator->GetPath();
        dd4hep::printout( dd4hep::DEBUG, "intersectionPoints", msg, os.str().c_str() );
      }
    };

    const auto maxSteps = 99999u; // sanity check against infinite loop
    auto       iStep    = 0u;
    do {
      navigator->FindNextBoundaryAndStep();
      auto c  = navigator->GetCurrentPoint();
      current = {c[0], c[1], c[2]};

      // Basic check that we are strictly in the node itself,
      // i.e. not a subvolume
      // navigator->GetCurrentNode() == node
      // To check whether are in the node or a daughter:
      // is_node_ancestor(navigator->GetCache(), node);

      bool in_target = is_node_ancestor( navigator->GetCache(), node );
      if ( in_target ) {
        log_position( " -> IN  %s" );
        // only set first point if the stat point was not inside the volume
        if ( !found && !startsInNode ) {
          found = true;
          firstPtn.SetX( c[0] * DD4hepToLHCbCm );
          firstPtn.SetY( c[1] * DD4hepToLHCbCm );
          firstPtn.SetZ( c[2] * DD4hepToLHCbCm );
        }
      } else {
        log_position( " -> OUT %s" );
        if ( found || startsInNode ) {
          // We were in the volume, not anymore so we have exited and are on the outside boundary
          secondPtn.SetX( c[0] * DD4hepToLHCbCm );
          secondPtn.SetY( c[1] * DD4hepToLHCbCm );
          secondPtn.SetZ( c[2] * DD4hepToLHCbCm );
          break;
        }
      }
      distancesq = ( current - r_start_point ).mag2();

    } while ( ( distancesq < max_dist2 ) && ( ++iStep <= maxSteps ) );
    if ( iStep > maxSteps ) {
      dd4hep::printout( dd4hep::WARNING, "intersectionPoints", "Max intersection steps exceeded" );
    }
  };

  // Navigating to the reference point
  navigator->SetCurrentPoint( r_start_point.X(), r_start_point.Y(), r_start_point.Z() );
  const auto startNode = navigator->FindNode();

  // is the start reference point inside the volume of interest
  const bool startsInNode = startNode == node;

  // find points forwards
  findPoints( entry, exit, true );

  // if a) search starts off inside volume of interest or b) no intersections found,
  // perform search in reverse direction.
  if ( startsInNode || !found ) {
    // reset to reference point
    navigator->SetCurrentPoint( r_start_point.X(), r_start_point.Y(), r_start_point.Z() );
    navigator->FindNode();
    // find points bcakwards
    findPoints( exit, entry, false );
  }

  return found;
}

bool lhcb::geometrytools::is_node_ancestor( TGeoNodeCache* cache, TGeoNode* node ) {
  bool found = false;
  if ( cache->GetNode() == node ) {
    found = true;
  } else {
    TGeoNode** ancestors = (TGeoNode**)cache->GetBranch();
    for ( int i = 0; i < cache->GetLevel(); i++ ) {
      if ( ancestors[i] == node ) {
        found = true;
        break;
      }
    }
  }
  return found;
}

TGeoNavigator* lhcb::geometrytools::get_navigator( dd4hep::Detector& detector ) {
  TGeoNavigator* nav = detector.manager().GetCurrentNavigator();
  if ( nav == nullptr ) { nav = detector.manager().AddNavigator(); }
  return nav;
}
