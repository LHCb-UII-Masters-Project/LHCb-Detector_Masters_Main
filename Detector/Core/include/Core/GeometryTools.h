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
#pragma once

#include "Core/Units.h"

#include "DD4hep/Printout.h"

#include "Math/Point3D.h"
#include "Math/Vector3D.h"
#include "TGeoCache.h"
#include "TGeoNavigator.h"
#include "TGeoNode.h"

#include <functional>
#include <map>
#include <sstream>
#include <string>
namespace lhcb::geometrytools {

  /**
   * struct to compare TGeoNode * by node name
   */
  struct TGeoNodeCmp {
    bool operator()( const TGeoNode* a, const TGeoNode* b ) const { return strcmp( a->GetName(), b->GetName() ) < 0; }
  };

  /**
   *  The template for our sorted node map
   */
  template <typename T>
  using nodeMap = std::map<TGeoNode*, T, TGeoNodeCmp>;

  /**
   *  Utility to derive the return type of the callable
   */
  template <typename Lambda>
  struct function_traits : function_traits<decltype( &Lambda::operator() )> {};
  template <typename R, typename C, typename... Args>
  struct function_traits<R ( C::* )( Args... ) const> {
    using result = R;
  };
  template <typename F>
  using return_type = typename function_traits<F>::result;

  /**
   * Traverse the ROOT node tree and apply the callback to each of them
   * For each node, the callback has access to:
   *  - the node pointer
   *  - the path
   *  - the level
   *  - a map with all daughters sorted by node name, with the result of the callback on each of them
   */
  template <typename C>
  return_type<C> traverse( TGeoNode* node, C& callback, std::string path = "/world_volume", int level = 0 ) {

    nodeMap<return_type<C>> daughters;
    for ( int i = 0; i < node->GetNdaughters(); i++ ) {
      TGeoNode* n = node->GetDaughter( i );
      daughters.emplace( n, traverse( n, callback, path + +"/" + n->GetName(), level + 1 ) );
    }

    return callback( node, daughters, path, level );
  }

  /**
   * Return a string representing specific a node
   */
  std::string toString( TGeoNode* node );

  /**
   * Find daughter node by name
   */
  TGeoNode* find_daughter( TGeoNode* mother, const std::string& daughter_name );

  /**
   * @brief Check if a line defined by start_point and direction intersects the
   * placement defined by target_node, and where.
   *
   * @param start_point: point from which to start the path
   * @param direction: direction of the path
   * @param entry: entry point of path in target_node
   * @param exit: exit point of path from target_node
   * @param target_node: TGeoNode to check intersection with
   * @param navigator: TGeoNavigator to use
   * @param max_dist: maximum distance after which the search should be stopped, in LHCb unit (i.e. mm)
   * @return true if path crosses the target_node
   * @return false f path does not cross the target_node
   */
  bool intersectionPoints( const ROOT::Math::XYZPoint& start_point, const ROOT::Math::XYZVector& direction,
                           ROOT::Math::XYZPoint& entry, ROOT::Math::XYZPoint& exit, TGeoNavigator* navigator,
                           TGeoNode* node, double max_dist = 20000 /* mm */ );

  /**
   * @brief Check whether a node contains the current placement
   *
   * @param cache The TGeoNodeCache corresponding to the state to check
   * @param node: the node to check
   * @return true If node is an ancestor of the the current one
   * @return false If it is not
   */
  bool is_node_ancestor( TGeoNodeCache* cache, TGeoNode* node );

  /**
   * @brief Get the navigator object
   *
   * @param detector The DDhep detector object
   * @return TGeoNavigator*
   */
  TGeoNavigator* get_navigator( dd4hep::Detector& detector );

  /**
   * @brief Discretize a length in the range of LHCb scale
   *
   * @param l the length itself
   * @return long
   */
  inline long discretize_length( double l ) {
    long dl = std::lround( 1000000 * l / dd4hep::cm ) / 10;
    return ( dl == 0 ) ? 0 : dl; // to remove signed zero
  }

  /**
   * @brief discretize an angle for checksum purposes
   *
   * @param a
   * @return long
   */
  inline long discretize_angle( double a ) {
    return std::lround( 100 * a / dd4hep::degree ); // We keep the lengths in 100th of degrees
  }

  /**
   * @brief discretize an entry from a rotation matrix for checksum purposes
   *
   * @param a
   * @return long
   */
  inline long discretize_rotmatrix( double m ) {
    auto dm = std::lround( 100000 * m );
    return ( dm == 0 ) ? 0 : dm;
  }

  /**
   * @brief discretize vector
   *
   * @param a
   * @return long
   */
  inline long discretize_vec( double m ) {
    auto dm = std::lround( 1000000 * m ) / 10;
    return ( dm == 0 ) ? 0 : dm;
  }

  /**
   * @brief transformation matrix recast
   */
  inline ROOT::Math::Transform3D toTransform3D( const TGeoHMatrix& m ) {
    const auto*               trans = m.GetTranslation();
    ROOT::Math::Translation3D t{trans[0], trans[1], trans[2]};
    const auto*               rot = m.GetRotationMatrix();
    ROOT::Math::Rotation3D    r{rot[0], rot[1], rot[2], rot[3], rot[4], rot[5], rot[6], rot[7], rot[8]};
    return {r, t};
  }
} // namespace lhcb::geometrytools
