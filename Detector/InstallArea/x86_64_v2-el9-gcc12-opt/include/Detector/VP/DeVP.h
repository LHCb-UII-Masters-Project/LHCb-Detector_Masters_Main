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
#include "Core/Keys.h"
#include "Detector/VP/DeVPSensor.h"

#include <array>
#include <vector>

namespace LHCb::Detector {

  namespace detail {

    struct DeVPLadderObject : DeIOVObject {
      DeVPSensorObject sensor;
      DeVPLadderObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt, unsigned int moduleId,
                        bool _isLeft );
      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        func( &sensor );
      }
      std::size_t size() const override { return 1; }
    };

    struct DeVPModuleObject : DeIOVObject {
      std::array<DeVPLadderObject, 4> ladders;
      /// Global Z position
      double zpos{0.0};
      DeVPModuleObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt, bool _isLeft );
      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& ladder : ladders ) { func( &ladder ); };
      }
      std::size_t size() const override { return ladders.size(); }
    };

    struct DeVPSideObject : DeIOVObject {
      std::array<DeVPModuleObject, VP::NModules / 2> modules;
      DeVPSideObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt, bool _isLeft );

      std::optional<ROOT::Math::Transform3D> motionSystemTransform() const override { return m_motionSystemTransform; }

      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& module : modules ) { func( &module ); };
      }

      std::size_t size() const override { return modules.size(); }

    private:
      std::optional<ROOT::Math::Transform3D> m_motionSystemTransform;
    };

    /**
     *  \author  Markus Frank
     *  \date    2018-03-08
     *  \version  1.0
     */
    struct DeVPObject : DeIOVObject {
      /// Left/right side
      std::array<DeVPSideObject, 2> sides; // left at index 0, right at index 1;
      DeVPObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );

      // Caching of sensor values, for faster access

      /// local to global matrix cache
      std::array<std::array<float, 12>, VP::NSensors> m_ltg{{}};

      /// Cosine squared of rotation angle for each sensor.
      std::array<double, VP::NSensors> m_c2;

      /// Sine squared of rotation angle for each sensor.
      std::array<double, VP::NSensors> m_s2; /// Return the pixel size.

      // beamspot
      ROOT::Math::XYZPoint m_beamSpot{0., 0., 0.};

      // whether the velo is closed
      bool m_isVeloClosed{false};

      // find the sensor number for a given space point
      int sensitiveVolumeID( const ROOT::Math::XYZPoint& point ) const;

      // find module from a given z position
      const DeVPModuleObject* findModuleFromZ( const double z ) const;

      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& side : sides ) { func( &side ); };
      };
      std::size_t size() const override { return sides.size(); }

      /// Printout method to stdout
      void print( int indent, int flags ) const override;
    };
  } // End namespace detail

  using DeVPLadder = DeIOVElement<detail::DeVPLadderObject>;
  using DeVPModule = DeIOVElement<detail::DeVPModuleObject>;
  using DeVPSide   = DeIOVElement<detail::DeVPSideObject>;

  /**
   *  This object defines the behaviour of the objects's data
   *
   *  \author  Markus Frank
   *  \date    2018-03-08
   *  \version  1.0
   */
  struct DeVP : DeIOVElement<detail::DeVPObject> {
    using DeIOVElement::DeIOVElement;
    /// Return the number of sensors.
    size_t numberSensors() const { return VP::NSensors; }
    /// Return local x for a given sensor.
    float local_x( unsigned int n ) const {
      assert( n < VP::NSensorColumns );
      return this->access()->sides[0].modules[0].ladders[0].sensor.local_x[n];
    }
    /// Return pitch for a given sensor.
    float x_pitch( unsigned int n ) const {
      assert( n < VP::NSensorColumns );
      return this->access()->sides[0].modules[0].ladders[0].sensor.x_pitch[n];
    }
    float pixel_size() const { return this->access()->sides[0].modules[0].ladders[0].sensor.pixelSize; }
    /// Return local to global matirx for a sensor
    const std::array<float, 12>& ltg( VPChannelID::SensorID n ) const {
      return this->access()->m_ltg[to_unsigned( n )];
    }
    /// Return Cosine squared of rotation angle for a sensor
    double cos2OfSensor( VPChannelID::SensorID n ) const {
      assert( to_unsigned( n ) < VP::NSensors );
      return this->access()->m_c2[to_unsigned( n )];
    }
    /// Return Sine squared of rotation angle for a sensor
    double sin2OfSensor( VPChannelID::SensorID n ) const {
      assert( to_unsigned( n ) < VP::NSensors );
      return this->access()->m_s2[to_unsigned( n )];
    }

    /// Return sensor number for a given space point
    int sensitiveVolumeID( ROOT::Math::XYZPoint const& point ) const override {
      return this->access()->sensitiveVolumeID( point );
    }

    /// Return pointer to sensor for a given sensor number.
    const DeVPSensor sensor( VPChannelID id ) const { return sensor( id.sensor() ); }

    /// Return pointer to sensor for a given sensor number.
    const DeVPSensor sensor( VPChannelID::SensorID sensorNumber ) const {
      assert( to_unsigned( sensorNumber ) < VP::NSensors );
      auto moduleNumber = to_unsigned( sensorNumber ) / VP::NSensorsPerModule;
      return moduleNumber % 2 == 0 ? DeVPSensor( &( this->access()
                                                        ->sides[1]
                                                        .modules[moduleNumber / 2]
                                                        .ladders[to_unsigned( sensorNumber ) % VP::NSensorsPerModule]
                                                        .sensor ) )
                                   : DeVPSensor( &( this->access()
                                                        ->sides[0]
                                                        .modules[( moduleNumber - 1 ) / 2]
                                                        .ladders[to_unsigned( sensorNumber ) % VP::NSensorsPerModule]
                                                        .sensor ) );
    }
    /// Return pointer to sensor for a given sensor number.
    const DeVPModule module( VPChannelID::SensorID sensorNumber ) const {
      assert( to_unsigned( sensorNumber ) < VP::NSensors );
      auto moduleNumber = to_unsigned( sensorNumber ) / VP::NSensorsPerModule;
      return moduleNumber % 2 == 0 ? DeVPModule( &( this->access()->sides[1].modules[moduleNumber / 2] ) )
                                   : DeVPModule( &( this->access()->sides[0].modules[( moduleNumber - 1 ) / 2] ) );
    }
    template <class Operation>
    /// runs the given callable on every sensor
    void runOnAllSensors( Operation op ) const {
      for ( auto& side : this->access()->sides )
        for ( auto& module : side.modules )
          for ( auto& ladder : module.ladders ) op( DeVPSensor( &( ladder.sensor ) ) );
    }
    const DeVPSide right() const { return DeVPSide( &( this->access()->sides[1] ) ); }
    const DeVPSide left() const { return DeVPSide( &( this->access()->sides[0] ) ); }

    /// Compute the center of the moving velo and call it 'beamSpot'
    auto beamSpot() const { return this->access()->m_beamSpot; }
    /// says whether the velo is closed
    auto veloClosed() const { return this->access()->m_isVeloClosed; }
  };
} // End namespace LHCb::Detector
