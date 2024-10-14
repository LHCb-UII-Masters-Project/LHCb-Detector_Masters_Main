/*****************************************************************************\
* (c) Copyright 2000-2019 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once

#include "Core/DeIOV.h"
#include "Core/LineTraj.h"
#include "Detector/UT/ChannelID.h"

#include "DD4hep/DD4hepUnits.h"

#include <array>
#include <vector>

namespace LHCb::Detector::UT {

  namespace detail {

    struct DeUTSensorObject : LHCb::Detector::detail::DeIOVObject {
      DeUTSensorObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );
      unsigned int m_id = 0u;

      float        m_pitch      = 0.0;
      unsigned int m_nStrip     = 512;
      float        m_deadWidth  = 0.0;
      bool         m_stripflip  = true;
      unsigned int m_firstStrip = 0u;
      unsigned int m_version    = 2;
      ChannelID    m_channelID;
      char         m_sensorType;

      float m_uMaxLocal   = 0.0;
      float m_uMinLocal   = 0.0;
      float m_vMaxLocal   = 0.0;
      float m_vMinLocal   = 0.0;
      float m_stripLength = 0.0;
      float m_thickness   = 0.0;
      bool  m_xInverted   = false;
      bool  m_yInverted   = false;

      ROOT::Math::Plane3D             m_plane;
      ROOT::Math::Plane3D             m_entryPlane;
      ROOT::Math::Plane3D             m_exitPlane;
      ROOT::Math::XYZVector           m_direction;
      std::pair<double, double>       m_range;
      std::optional<LineTraj<double>> m_midTraj;

      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& ) const override {
        // no children
      }
    };
  } // End namespace detail

  /**
   *  This object defines the behaviour of the objects's data
   *
   *  \author  Markus Frank
   *  \date    2018-03-08
   *  \version  1.0
   */
  struct DeUTSensor : public DeIOVElement<detail::DeUTSensorObject> {

    ChannelID           channelID() const { return this->access()->m_channelID; }
    unsigned int        id() const { return this->access()->m_id; };
    void                setID( const unsigned int id ) { this->access()->m_id = id; }
    float               pitch() const { return this->access()->m_pitch; }
    char                sensorType() const { return this->access()->m_sensorType; }
    unsigned int        nStrip() const { return this->access()->m_nStrip; }
    float               uMaxLocal() const { return this->access()->m_uMaxLocal; }
    float               uMinLocal() const { return this->access()->m_uMinLocal; }
    float               vMaxLocal() const { return this->access()->m_vMaxLocal; }
    float               vMinLocal() const { return this->access()->m_vMinLocal; }
    bool                yInverted() const { return this->access()->m_yInverted; }
    bool                xInverted() const { return this->access()->m_xInverted; }
    ROOT::Math::Plane3D plane() const { return this->access()->m_plane; }
    ROOT::Math::Plane3D entryPlane() const { return this->access()->m_entryPlane; }
    ROOT::Math::Plane3D exitPlane() const { return this->access()->m_exitPlane; }
    float               thickness() const { return this->access()->m_thickness; }
    float        activeWidth() const { return fabs( this->access()->m_uMinLocal - this->access()->m_uMaxLocal ); }
    float        activeHeight() const { return fabs( this->access()->m_vMinLocal - this->access()->m_vMaxLocal ); }
    bool         getStripflip() const { return this->access()->m_stripflip; }
    unsigned int firstStrip() const { return this->access()->m_firstStrip; };
    unsigned int version() const { return this->access()->m_version; };

    using DeIOVElement::DeIOVElement;

    bool contains( const ChannelID aChannel ) const { return ( aChannel == channelID() ); }

    bool isStrip( const unsigned int strip ) const {
      return ( strip >= firstStrip() ) && strip < nStrip() + firstStrip();
    }
    LineTraj<double> trajectory( unsigned int strip, double offset ) const {
      const double arclen =
          ( ( xInverted() && getStripflip() ) ? ( nStrip() - offset - strip - ( firstStrip() + 1 ) % 2 )
                                              : ( offset + strip - firstStrip() ) ) *
          pitch();
      ROOT::Math::XYZPoint midPoint =
          this->access()->m_midTraj->position( arclen + this->access()->m_midTraj->beginRange() );
      return {midPoint, this->access()->m_direction, this->access()->m_range, Trajectory<double>::DirNormalized{true}};
    }
    bool localInActive( const ROOT::Math::XYZPoint& point, ROOT::Math::XYZPoint tol = {0., 0., 0.} ) const {
      const double u = point.x();
      const double v = point.y();
      return ( u - tol.X() ) < ( uMaxLocal() + ( 0.5f * pitch() ) ) &&
             ( u + tol.X() ) > ( uMinLocal() - ( 0.5f * pitch() ) ) && ( ( v - tol.Y() ) < vMaxLocal() ) &&
             ( ( v + tol.Y() ) > vMinLocal() );
    }
    double localU( const unsigned int strip, const double offset = 0.0 ) const {
      // strip to local
      double tStrip = strip + offset + ( firstStrip() + 1 ) % 2;
      if ( !getStripflip() && xInverted() ) {
        return uMaxLocal() + pitch() * ( 0.5 - tStrip );
      } else {
        return uMinLocal() + pitch() * ( tStrip - 0.5 );
      }
    }
    unsigned int localUToStrip( const double u ) const {
      // convert local u to a strip
      unsigned int strip = ( ( !getStripflip() && xInverted() )
                                 ? static_cast<unsigned int>( floor( ( ( uMaxLocal() - u ) / pitch() ) + 0.5 ) ) -
                                       ( firstStrip() + 1 ) % 2
                                 : static_cast<unsigned int>( floor( ( ( u - uMinLocal() ) / pitch() ) + 0.5 ) ) ) -
                           ( firstStrip() + 1 ) % 2;
      return isStrip( strip ) ? strip : 0u;
    }
    bool localInBondGap( float v, float tol ) const {
      return ( ( v + tol > uMinLocal() ) && ( v - tol < uMaxLocal() ) );
    }
    bool globalInBondGap( const ROOT::Math::XYZPoint& gpoint, float tol ) const {
      ROOT::Math::XYZPoint lPoint = this->toLocal( gpoint );
      return localInBondGap( lPoint.Y(), tol );
    }
    bool globalInActive( const ROOT::Math::XYZPoint& gpoint, ROOT::Math::XYZPoint tol = {0., 0., 0.} ) const {
      ROOT::Math::XYZPoint lPoint = this->toLocal( gpoint );
      return localInActive( lPoint, tol );
    }

    double capacitance() const {
      // by chance the CMS and LHCb sensors have same capacitance
      constexpr float rho = 1.4 * dd4hep::picofarad / dd4hep::cm * 10.f; // * 10 to convert back to Gaudi units
      return rho * activeHeight();
    }

    ROOT::Math::XYZPoint globalPoint( double x, double y, double z ) const {
      return this->toGlobal( ROOT::Math::XYZPoint( x, y, z ) );
    }
  };

} // namespace LHCb::Detector::UT
