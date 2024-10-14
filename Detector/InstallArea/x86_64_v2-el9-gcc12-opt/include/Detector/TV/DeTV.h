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
#include "Detector/TV/DeTVSensor.h"

#include <array>
#include <vector>

namespace LHCb::Detector {

  namespace detail {

    struct DeTVLadderObject : DeIOVObject {
      DeTVSensorObject sensor;
      DeTVLadderObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt, unsigned int moduleId,
                        bool _isLeft );
      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        func( &sensor );
      }
      std::size_t size() const override { return 1; }
    };

    struct DeTVModuleObject : DeIOVObject {
      std::array<DeTVLadderObject, 4> ladders;
      /// Global Z position
      double zpos{0.0};
      DeTVModuleObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt, bool _isLeft );
      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& ladder : ladders ) { func( &ladder ); };
      }
      std::size_t size() const override { return ladders.size(); }
    };

    struct DeTVSideObject : DeIOVObject {
      std::array<DeTVModuleObject, TV::NModules / 2> modules;
      DeTVSideObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt, bool _isLeft );

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
    struct DeTVObject : DeIOVObject {
      /// Left/right side
      std::array<DeTVSideObject, 2> sides; // left at index 0, right at index 1;
      DeTVObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );

      // Caching of sensor values, for faster access

      /// local to global matrix cache
      std::array<std::array<float, 12>, TV::NSensors> m_ltg{{}};

      /// Cosine squared of rotation angle for each sensor.
      std::array<double, TV::NSensors> m_c2;

      /// Sine squared of rotation angle for each sensor.
      std::array<double, TV::NSensors> m_s2; /// Return the pixel size.

      // beamspot
      ROOT::Math::XYZPoint m_beamSpot{0., 0., 0.};

      // whether the velo is closed
      bool m_isVeloClosed{false};

      // find the sensor number for a given space point
      int sensitiveVolumeID( const ROOT::Math::XYZPoint& point ) const;

      // find module from a given z position
      const DeTVModuleObject* findModuleFromZ( const double z ) const;

      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& side : sides ) { func( &side ); };
      };
      std::size_t size() const override { return sides.size(); }

      /// Printout method to stdout
      void print( int indent, int flags ) const override;
    };
  } // End namespace detail

  using DeTVLadder = DeIOVElement<detail::DeTVLadderObject>;
  using DeTVModule = DeIOVElement<detail::DeTVModuleObject>;
  using DeTVSide   = DeIOVElement<detail::DeTVSideObject>;

  /**
   *  This object defines the behaviour of the objects's data
   *
   *  \author  Markus Frank
   *  \date    2018-03-08
   *  \version  1.0
   */
  struct DeTV : DeIOVElement<detail::DeTVObject> {
    using DeIOVElement::DeIOVElement;
    /// Return the number of sensors.
    size_t numberSensors() const { return TV::NSensors; }
    /// Return local x for a given sensor.
    float local_x( unsigned int n ) const {
      assert( n < TV::NSensorColumns );
      return this->access()->sides[0].modules[0].ladders[0].sensor.local_x[n];
    }
    /// Return pitch for a given sensor.
    float x_pitch( unsigned int n ) const {
      assert( n < TV::NSensorColumns );
      return this->access()->sides[0].modules[0].ladders[0].sensor.x_pitch[n];
    }
    float pixel_size() const { return this->access()->sides[0].modules[0].ladders[0].sensor.pixelSize; }
    /// Return local to global matirx for a sensor
    const std::array<float, 12>& ltg( TVChannelID::SensorID n ) const {
      return this->access()->m_ltg[to_unsigned( n )];
    }
    /// Return Cosine squared of rotation angle for a sensor
    double cos2OfSensor( TVChannelID::SensorID n ) const {
      assert( to_unsigned( n ) < TV::NSensors );
      return this->access()->m_c2[to_unsigned( n )];
    }
    /// Return Sine squared of rotation angle for a sensor
    double sin2OfSensor( TVChannelID::SensorID n ) const {
      assert( to_unsigned( n ) < TV::NSensors );
      return this->access()->m_s2[to_unsigned( n )];
    }

    /// Return sensor number for a given space point
    int sensitiveVolumeID( ROOT::Math::XYZPoint const& point ) const override {
      return this->access()->sensitiveVolumeID( point );
    }

    /// Return pointer to sensor for a given sensor number.
    const DeTVSensor sensor( TVChannelID id ) const { return sensor( id.sensor() ); }

    /// Return pointer to sensor for a given sensor number.
    const DeTVSensor sensor( TVChannelID::SensorID sensorNumber ) const {
      assert( to_unsigned( sensorNumber ) < TV::NSensors );
      auto moduleNumber = to_unsigned( sensorNumber ) / TV::NSensorsPerModule;
      return moduleNumber % 2 == 0 ? DeTVSensor( &( this->access()
                                                        ->sides[1]
                                                        .modules[moduleNumber / 2]
                                                        .ladders[to_unsigned( sensorNumber ) % TV::NSensorsPerModule]
                                                        .sensor ) )
                                   : DeTVSensor( &( this->access()
                                                        ->sides[0]
                                                        .modules[( moduleNumber - 1 ) / 2]
                                                        .ladders[to_unsigned( sensorNumber ) % TV::NSensorsPerModule]
                                                        .sensor ) );
    }
    /// Return pointer to sensor for a given sensor number.
    const DeTVModule module( TVChannelID::SensorID sensorNumber ) const {
      assert( to_unsigned( sensorNumber ) < TV::NSensors );
      auto moduleNumber = to_unsigned( sensorNumber ) / TV::NSensorsPerModule;
      return moduleNumber % 2 == 0 ? DeTVModule( &( this->access()->sides[1].modules[moduleNumber / 2] ) )
                                   : DeTVModule( &( this->access()->sides[0].modules[( moduleNumber - 1 ) / 2] ) );
    }
    template <class Operation>
    /// runs the given callable on every sensor
    void runOnAllSensors( Operation op ) const {
      for ( auto& side : this->access()->sides )
        for ( auto& module : side.modules )
          for ( auto& ladder : module.ladders ) op( DeTVSensor( &( ladder.sensor ) ) );
    }
    const DeTVSide right() const { return DeTVSide( &( this->access()->sides[1] ) ); }
    const DeTVSide left() const { return DeTVSide( &( this->access()->sides[0] ) ); }

    /// Compute the center of the moving velo and call it 'beamSpot'
    auto beamSpot() const { return this->access()->m_beamSpot; }
    /// says whether the velo is closed
    auto veloClosed() const { return this->access()->m_isVeloClosed; }
  };
} // End namespace LHCb::Detector
