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
#include "Detector/UP/UPChannelID.h"

#include "DD4hep/DD4hepUnits.h"

#include <array>
#include <vector>

namespace LHCb::Detector {

  namespace detail {

    struct DeUPSensorObject : LHCb::Detector::detail::DeIOVObject {
      DeUPSensorObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );
      unsigned int m_id;

      double       m_pitch;
      double       m_nPixel;
      double       m_pixelWidth;
      double       m_pixelLength;
      double       m_deadWidth;
      bool         m_pixelflip;
      unsigned int m_firstPixel;
      unsigned int m_version;
      ChannelID    m_channelID;

      unsigned int m_nx;
      unsigned int m_ny;

      float m_uMaxLocal;
      float m_uMinLocal;
      float m_vMaxLocal;
      float m_vMinLocal;
      float m_thickness;
      bool  m_xInverted;
      bool  m_yInverted;

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
  template <typename ObjectType>
  struct DeUPSensorElement : DeIOVElement<ObjectType> {

    using DeIOVElement<ObjectType>::DeIOVElement;

    bool contains( const ChannelID ) const { throw NotImplemented(); }

    ChannelID    channelID() const { return this->access()->m_channelID; }
    unsigned int id() const { return this->access()->m_id; };
    void         setID( const unsigned int id ) { this->access()->m_id = id; }
    double       pitch() const { return this->access()->m_pitch; }
    double       pixelWidth() const { return this->access()->m_pixelWidth; }
    double       pixelLength() const { return this->access()->m_pixelLength; }

    unsigned int        nPixel() const { return this->access()->m_nPixel; }
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
    float activeWidth() const { return fabs( this->access()->m_uMinLocal - this->access()->m_uMaxLocal ); }
    float activeHeight() const { return fabs( this->access()->m_vMinLocal - this->access()->m_vMaxLocal ); }

    bool isPixel( const unsigned int pixel ) const {
      return ( pixel >= firstPixel() && pixel < firstPixel() + nPixel() );
    }
    bool             getPixelflip() const { return this->access()->m_pixelflip; }
    unsigned int     firstPixel() const { return this->access()->m_firstPixel; };
    LineTraj<double> trajectory( unsigned int, double ) const { throw NotImplemented(); }
    bool             localInActive( const ROOT::Math::XYZPoint& point, ROOT::Math::XYZPoint tol = {0., 0., 0.} ) const {
      const double u = point.x();
      const double v = point.y();
      return ( u - tol.X() ) < ( uMaxLocal() + ( 0.5f * pitch() ) ) &&
             ( u + tol.X() ) > ( uMinLocal() - ( 0.5f * pitch() ) ) && ( ( v - tol.Y() ) < vMaxLocal() ) &&
             ( ( v + tol.Y() ) > vMinLocal() );
    }

    double localU( [[maybe_unused]] const unsigned int pixel, [[maybe_unused]] const double offset ) const {
      throw NotImplemented();
    }

    double localU( [[maybe_unused]] int pixel ) const { throw NotImplemented(); }

    double localV( [[maybe_unused]] const unsigned int pixel, [[maybe_unused]] const double offset ) const {
      throw NotImplemented();
    }

    int localUPoColumn( [[maybe_unused]] const double u ) const { throw NotImplemented(); }

    int localVToRow( [[maybe_unused]] const double v ) const { throw NotImplemented(); }

    int localToPixel( [[maybe_unused]] const double u, [[maybe_unused]] const double v ) { throw NotImplemented(); }

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

    ROOT::Math::XYZPoint globalPoint( double, double, double ) const { throw NotImplemented(); }
  };

  using DeUPSensor = DeUPSensorElement<detail::DeUPSensorObject>;

} // namespace LHCb::Detector
