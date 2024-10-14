/*****************************************************************************\
* (c) Copyright 2000-2019 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*******************************************************************************/

#pragma once

#include "Core/DeIOV.h"
#include "Core/GeometryTools.h"
#include "Detector/Rich/DeRich.h"
#include "Detector/Rich/Types.h"
#include "Detector/Rich/Utilities.h"

#include "DD4hep/Detector.h"

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace LHCb::Detector::detail {

  struct DeRichRadiatorObject : DeIOVObject {

  public:
    using TabData = XYTable;

    Rich::RadiatorType m_gasName             = Rich::InvalidRadiator;
    double             m_Presure_Nominal     = -1;
    double             m_Temperature_Nominal = -1;
    double             m_PhotonMinEnergy     = -1;
    double             m_PhotonMaxEnergy     = -1;
    TGeoNavigator*     m_navigator           = nullptr;
    TabData            m_GasRefIndex;

    dd4hep::Condition m_refScalefactor;
    dd4hep::Condition m_gasParameters;

    // For simple implementation of intersectionPoints
    double              m_z_entry{-1};      ///< z position of gas volume entry plane
    double              m_z_exit{-1};       ///< z position of gas volume exit plane
    double              m_x_size_entry{-1}; ///< rough gas volume size in x at entry plane
    double              m_y_size_entry{-1}; ///< rough gas volume size in y at entry plane
    double              m_x_size_exit{-1};  ///< rough gas volume size in x at exit plane
    double              m_y_size_exit{-1};  ///< rough gas volume size in y at exit plane
    ROOT::Math::Plane3D m_entryPlane;       ///< Entry plane
    ROOT::Math::Plane3D m_exitPlane;        ///< Exit plane

  protected:
    // determine on the fly a representation of the entry and exit planes
    // formed from three points in each plane, found via the full intersection calls.
    void formPlanes() {

      using namespace LHCb::Detector::detail;

      // Note lhcb::geometrytools::intersectionPoints expects the input coordinates to be in 'gaudi' units...
      std::array<ROOT::Math::XYZPoint, 3> entry, exit;
      const ROOT::Math::XYZVector         v{0.0, 0.0, 1.0};
      const double                        xxx{( Rich::Rich1Gas == m_gasName ? 25.0 : 35.0 ) * GaudiCentimeter};
      const double                        zzz{( Rich::Rich1Gas == m_gasName ? 80.0 : 900.0 ) * GaudiCentimeter};

      auto fullInter = [&]( const ROOT::Math::XYZPoint p ) {
        ROOT::Math::XYZPoint in, out;
        const bool           ok = intersectionPoints_full( p, v, in, out );
        std::ostringstream   mess;
        mess << "Radiator plane intersects | ptn=" << p << " dir=" << v << " | entry=" << in << " exit=" << out;
        if ( !ok ) {
          dd4hep::printout( dd4hep::ERROR, "DeRichRadiatorObject", ( "FAILED to find " + mess.str() ).c_str() );
          throw std::runtime_error( mess.str() );
        } else {
          dd4hep::printout( dd4hep::DEBUG, "DeRichRadiatorObject", mess.str().c_str() );
        }
        return std::make_pair( in, out );
      };

      std::tie( entry[0], exit[0] ) = fullInter( {0.0, xxx, zzz} );
      std::tie( entry[1], exit[1] ) = fullInter( {xxx, 0.0, zzz} );
      std::tie( entry[2], exit[2] ) = fullInter( {xxx, xxx, zzz} );

      assert( std::all_of( entry.begin(), entry.end(), [&]( const auto& p ) { return p.z() > zzz; } ) );
      assert( std::all_of( exit.begin(), exit.end(), [&]( const auto& p ) { return p.z() > zzz; } ) );

      m_entryPlane = ROOT::Math::Plane3D( entry[0], entry[1], entry[2] );
      m_exitPlane  = ROOT::Math::Plane3D( exit[0], exit[1], exit[2] );
    }

  public:
    inline auto CurrentRefIndexScaleFactor() const {
      auto& scale = m_refScalefactor.get<nlohmann::json>();
      return scale["CurrentScaleFactor"].get<double>();
    }

    TabData CurrentScaledRefIndexInObjTab();

    TabData CurrentScaledRefIndexInObjTab( const double aPressure, const double aTemperature,
                                           const double ScaleF = 1.0 ) const;

    TabData RetrieveNominalGasRefIndexInDeObjectFromDB() const;

    DeRichRadiatorObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt )
        : DeIOVObject( de, ctxt ) {
      auto& desc  = dd4hep::Detector::getInstance();
      m_navigator = lhcb::geometrytools::get_navigator( desc );
      assert( m_navigator );
      dd4hep::printout( dd4hep::DEBUG, "DeRichRadiatorObject", "Loaded navigator | Name='%s'",
                        m_navigator->GetCurrentNode()->GetName() );
    }

    /** Determine the intersections with this radiator volume.
        Returns true if intersections where found, false otherwise.
        Perform a basic plane intersection. Entry/exit planes are hardcoded to be vertical in
        z, which is correct for RICH1 but ever so slightly wrong for RICH2 as that detector
        is aligned with gravity and not the beam line.
        Means a small mrad tilt in the beam access through RICH2. */
    template <typename VECTOR, typename POINT>
    inline bool intersectionPoints_simple( const POINT&  gP,    ///< A point on the trajectory
                                           const VECTOR& gV,    ///< The direction through the trajectory point
                                           POINT&        entry, ///< The determined entry point
                                           POINT&        exit   ///< The determined exit point
                                           ) const {
      // creates a point at a given z position along the trajectory
      auto propagate = [&]( const auto z ) {
        // Distance in z
        const auto zDiff = z - gP.z();
        // form new point at requested z
        const auto p = gP + ( ( zDiff / gV.z() ) * gV );
        assert( fabs( z - p.z() ) < 1e-5 );
        return p;
      };
      // intersect a given plane
      auto intersectPlane = [&]( const auto& plane ) {
        const auto scalar   = gV.Dot( plane.Normal() );
        const auto distance = plane.Distance( gP ) / scalar;
        return gP - ( distance * gV );
      };
      // form the entry/exit points
      entry = intersectPlane( m_entryPlane );
      // FIXME For now use fixed z for RICH1 exit as for some reason plane object gives intersects
      //       that seem a little too far downstream... This is actually not so bad as RICH1 is
      //       vertical in the LHCb global frame, so the planes are indeed at fixed z.
      exit = ( Rich::Rich1Gas == m_gasName ? propagate( m_z_exit ) : intersectPlane( m_exitPlane ) );
      // very crude (x,y) acceptance cut
      // require track is in *either* the entry or exit acceptance
      return ( ( fabs( entry.x() ) <= m_x_size_entry && //
                 fabs( entry.y() ) <= m_y_size_entry ) ||
               ( fabs( exit.x() ) <= m_x_size_exit && //
                 fabs( exit.y() ) <= m_y_size_exit ) );
    }

    /** Determine the intersections with this radiator volume.
        Returns true if intersections where found, false otherwise.
        Full version using DD4HEP geometry. */
    template <typename VECTOR, typename POINT>
    inline bool intersectionPoints_full( const POINT&  gP,    ///< A point on the trajectory
                                         const VECTOR& gV,    ///< The direction through the trajectory point
                                         POINT&        entry, ///< The determined entry point
                                         POINT&        exit   ///< The determined exit poit
                                         ) const {
      assert( m_navigator );
      return lhcb::geometrytools::intersectionPoints( gP, gV, entry, exit, m_navigator, placement().ptr() );
    }

    template <typename... ARGS>
    inline auto intersectionPoints( ARGS&&... args ) const {
      // Intersection with simple representation of gas volume
      return intersectionPoints_simple( std::forward<ARGS>( args )... );
      // Peform full dd4hep/ROOT intersection
      // return intersectionPoints_full( std::forward<ARGS>( args )... );
    }

    bool isInside( const ROOT::Math::XYZPoint& gPoint ) const override {
      // scale to dd4hep units :(((
      return DeIOVObject::isInside( toDD4hepUnits( gPoint ) );
    }
  };

  template <typename ObjectType>
  struct DeRichRadiatorElement : DeIOVElement<ObjectType> {
    using DeIOVElement<ObjectType>::DeIOVElement;
    auto CurrentRefIndexScaleFactor() const noexcept { return this->access()->CurrentRefIndexScaleFactor(); }
    auto Gas_Presure_Nominal() const noexcept { return this->access()->m_Presure_Nominal; }
    auto Gas_Temperature_Nominal() const noexcept { return this->access()->m_Temperature_Nominal; }
    auto Gas_PhotonMinEnergy() const noexcept { return toLHCbEnergyUnits( this->access()->m_PhotonMinEnergy ); }
    auto Gas_PhotonMaxEnergy() const noexcept { return toLHCbEnergyUnits( this->access()->m_PhotonMaxEnergy ); }
    auto GasRefIndex() const noexcept { return scalePhotonEnergies( this->access()->CurrentScaledRefIndexInObjTab() ); }
    auto GasRefIndex( const double aP, const double aT ) const noexcept {
      return scalePhotonEnergies( this->access()->CurrentScaledRefIndexInObjTab( aP, aT ) );
    }
  };

} // namespace LHCb::Detector::detail
