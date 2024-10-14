//==========================================================================
//  AIDA Detector description implementation
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
// Author     : M.Frank
//
//==========================================================================

/*
 * This is a copy of the shape utilities available in DD4hep, temporarily copied
 * here to deal with the change of units.
 * This should be replaced by a fix or the implementation in DD4hep.
 */
#pragma once
#include "DD4hep/DD4hepUnits.h"
#include <DD4hep/Handle.h>
#include <DD4hep/Printout.h> // for except....
#include <TGeoShape.h>
#include <vector>

/// Namespace for the AIDA detector description toolkit
namespace lhcb::Detector::shapeutils {

  using dd4hep::Handle;

  template <typename SOLID>
  std::vector<double> dimensions( const Handle<TGeoShape>& solid );

  std::vector<double> get_shape_dimensions( TGeoShape* shape );

  std::vector<double> get_shape_dimensions( const Handle<TGeoShape>& shape );

  inline double discretize_length( double l ) {
    long dl = std::lround( 1000000 * l / dd4hep::cm ) / 10;
    return ( dl == 0 ) ? 0 : dl; // to remove signed zero
  }

  inline double discretize_angle( double a ) {
    return std::round( 100 * a / dd4hep::degree ); // We keep the lengths in 100th of degrees
  }

  inline double discretize_rotmatrix( double m ) { return std::round( 100000 * m ); }

} // namespace lhcb::Detector::shapeutils
