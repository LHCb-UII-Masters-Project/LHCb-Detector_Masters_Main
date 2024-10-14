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
#include "Core/Units.h"
#include "Detector/VP/VPChannelID.h"
#include "Detector/VP/VPConstants.h"

#include <array>
#include <bitset>
#include <set>

namespace LHCb::Detector {

  namespace detail {

    /**
     *  \author  Markus Frank
     *  \date    2018-03-08
     *  \version  1.0
     */
    struct DeVPSensorObject : DeIOVObject {
      /// Cache of local x-cooordinates
      std::array<double, VP::NSensorColumns> local_x;
      /// Cache of x-pitch
      std::array<double, VP::NSensorColumns> x_pitch;

      /// Dimensions of the sensor active area
      double sizeX     = 0e0;
      double sizeY     = 0e0;
      double thickness = detail::toLHCbLengthUnits( dd4hep::_toDouble( "VP:Thickness" ) );
      /// Length of chip active area
      double chipSize = detail::toLHCbLengthUnits( dd4hep::_toDouble( "VP:ChipSize" ) );
      /// Distance between two chips
      double interChipDist = detail::toLHCbLengthUnits( dd4hep::_toDouble( "VP:InterChipDist" ) );
      /// Cell size of pixels
      double pixelSize = detail::toLHCbLengthUnits( dd4hep::_toDouble( "VP:PixelSize" ) );
      /// Cell size in column direction of elongated pixels
      double interChipPixelSize = detail::toLHCbLengthUnits( dd4hep::_toDouble( "VP:InterChipPixelSize" ) );
      /// Global Z position
      double zpos = 0e0;

      /// Sensor HV setting from CondDB/SimCond (105V as a default)
      double sensorHV = 105.;
      /// global bad chip flag, set if not readout or other issue
      std::bitset<3> chipStatusOK = {0b111}; // default good (111 = good, good, good)
      /// set of bad channels (may be empty) set from CondDB/SimCond
      std::set<VPChannelID> badChannels = {};

      /// Number of chips per ladder
      unsigned int nChips{(unsigned int)dd4hep::_toInt( "VP:NChips" )};
      /// Number of columns and rows
      unsigned int nCols{(unsigned int)dd4hep::_toInt( "VP:NColumns" )};
      unsigned int nRows{(unsigned int)dd4hep::_toInt( "VP:NRows" )};
      /// Sensor ans module number
      VPChannelID::SensorID sensorNumber{0};
      unsigned int          moduleNumber = 0;
      /// whether thie sensor is a left or a right one
      bool isLeft{true};

      /** For ref only: values are taken from the RUN-II conditions information  */
      /// Reference to time-dependent sensor information
      dd4hep::Condition info;
      /// Reference to time-dependent noise
      dd4hep::Condition noise;
      /// Reference to time-dependent readout configuration parameters
      dd4hep::Condition readout;

      /// constructor
      DeVPSensorObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt, unsigned int moduleId,
                        bool _isLeft );
      /// Printout method to stdout
      virtual void print( int indent, int flags ) const override;

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
  struct DeVPSensor : DeIOVElement<detail::DeVPSensorObject> {
    using DeIOVElement::DeIOVElement;
    /// Access the corresponding detector element
    dd4hep::DetElement detector() const { return this->access()->detector(); }
    bool               isLeft() const { return this->access()->isLeft; }
    bool               isRight() const { return !this->access()->isLeft; }
    double             activeSizeX() const { return this->access()->sizeX; }
    double             activeSizeY() const { return this->access()->sizeY; }
    /// Return sensor thickness in mm.
    double siliconThickness() const { return this->access()->thickness; }
    /// Return length of chip active area
    double chipSize() const { return this->access()->chipSize; }
    /// Return distance between two chips
    double interChipDist() const { return this->access()->interChipDist; }
    /// Return cell size in column direction of elongated pixels
    double interChipPixelSize() const { return this->access()->interChipPixelSize; }
    /// Return pixelsize
    double pixelSize() const { return this->access()->pixelSize; }
    /// Return the z position of the sensor in the global frame
    double z() const { return this->access()->zpos; }
    /// Return the number of pixel rows per chip
    unsigned int numRows() const { return this->access()->nRows; }
    /// Return the number of pixel colums per chip
    unsigned int numColumns() const { return this->access()->nCols; }
    /// Return the number of pixel colums per chip
    unsigned int numChips() const { return this->access()->nChips; }
    /// Return the sensor number
    VPChannelID::SensorID sensorNumber() const { return this->access()->sensorNumber; }
    /// Return the module number
    unsigned int module() const { return this->access()->moduleNumber; }
    /// Return station number (station comprises left and right module)
    unsigned int station() const { return module() >> 1; }
    /// Sensor size
    double sizeX() const { return this->access()->sizeX; }
    double sizeY() const { return this->access()->sizeY; }

    double sensorHV() const { return this->access()->sensorHV; };

    /// Is the pixel OK, uses cached info from CondDB/SimCond
    bool OKPixel( VPChannelID channel ) const {
      if ( !this->access()->chipStatusOK[to_unsigned( channel.chip() )] ) return false; // test chip status
      // Also not OK if the channel is in the bad strip set, if not then channel is OK.
      return this->access()->badChannels.find( channel ) == this->access()->badChannels.end();
    }

    /// Cache of local x-cooordinates
    const std::array<double, VP::NSensorColumns>& xLocal() const { return this->access()->local_x; };
    /// Cache of x-pitch
    const std::array<double, VP::NSensorColumns>& xPitch() const { return this->access()->x_pitch; };

    /// Convert local position to global position
    ROOT::Math::XYZPoint localToGlobal( const ROOT::Math::XYZPoint& point ) const { return toGlobal( point ); }
    /// Convert global position to local position
    ROOT::Math::XYZPoint globalToLocal( const ROOT::Math::XYZPoint& point ) const { return toLocal( point ); }
    /// Convert local vector to global vector
    ROOT::Math::XYZVector localToGlobal( const ROOT::Math::XYZVector& vector ) const { return toGlobal( vector ); }
    /// Convert global vector to local vector
    ROOT::Math::XYZVector globalToLocal( const ROOT::Math::XYZVector& vector ) const { return toLocal( vector ); }
    /// Calculate the local position of a given pixel.
    ROOT::Math::XYZPoint channelToLocalPoint( const VPChannelID channel ) const {
      const double x = xLocal()[channel.scol()];
      const double y = ( to_unsigned( channel.row() ) + 0.5 ) * pixelSize();
      return ROOT::Math::XYZPoint( x, y, 0.0 );
    }
    /// Calculate the global position of a given pixel.
    ROOT::Math::XYZPoint channelToGlobalPoint( const VPChannelID channel ) const {
      return localToGlobal( channelToLocalPoint( channel ) );
    }
    /// Check if a local point is inside the active area of the sensor.
    bool isInActiveArea( const ROOT::Math::XYZPoint& point ) const {
      if ( point.x() < 0. || point.x() > activeSizeX() ) return false;
      if ( point.y() < 0. || point.y() > activeSizeY() ) return false;
      return true;
    }
    /// Determine whether a point is within the sensor
    bool isInsideSensor( const double x, const double y ) const {
      return isInActiveArea( globalToLocal( ROOT::Math::XYZPoint{x, y, z()} ) );
    }
    /// Determine closest distance to sensor with respect to global point
    ROOT::Math::XYZVector closestDistanceToSensor( const ROOT::Math::XYZPoint& point ) const {
      auto   ref       = globalToLocal( point );
      double dx        = ref.x() > activeSizeX() ? ref.x() - activeSizeX() : ( ref.x() < 0. ? ref.x() : 0. );
      double dy        = ref.y() > activeSizeY() ? ref.y() - activeSizeY() : ( ref.y() < 0. ? ref.y() : 0. );
      auto   halfthick = 0.5 * siliconThickness();
      double dz = ref.z() > halfthick ? ref.z() - halfthick : ( ref.z() < -halfthick ? ref.z() + halfthick : 0. );
      return localToGlobal( ROOT::Math::XYZPoint( dx, dy, dz ) ) - localToGlobal( ROOT::Math::XYZPoint( 0., 0., 0. ) );
    }
    static void getGlobalMatrixDecomposition( const detail::DeVPSensorObject& sensor, ROOT::Math::Rotation3D& ltg_rot,
                                              ROOT::Math::XYZVector& ltg_trans ) {

      auto          localToGlobalRoot = detail::toLHCbLengthUnits( sensor.detectorAlignment().worldTransformation() );
      const double* r                 = localToGlobalRoot.GetRotationMatrix();
      ltg_rot.SetComponents( r, r + 9 );
      const double* t = localToGlobalRoot.GetTranslation();
      ltg_trans.SetXYZ( *t, *( t + 1 ), *( t + 2 ) );
    }
    void getGlobalMatrixDecomposition( ROOT::Math::Rotation3D& ltg_rot, ROOT::Math::XYZVector& ltg_trans ) const {
      return getGlobalMatrixDecomposition( *this->access(), ltg_rot, ltg_trans );
    }
    bool isLong( const VPChannelID channel ) const {
      const unsigned int chip = to_unsigned( channel.chip() );
      const unsigned int col  = to_unsigned( channel.col() );
      return ( col == 0 && chip > 0 ) || ( col == numColumns() - 1 && chip < numChips() - 1 );
    }

    /// Calculate the nearest channel to a given point.
    bool pointToChannel( ROOT::Math::XYZPoint const& point, bool local, VPChannelID& channel ) const;
    /// Calculate the position of a given pixel.
    ROOT::Math::XYZPoint channelToPoint( const VPChannelID& channel, const bool local ) const {
      const double               x = xLocal()[channel.scol()];
      const double               y = ( to_unsigned( channel.row() ) + 0.5 ) * pixelSize();
      const ROOT::Math::XYZPoint point( x, y, 0.0 );
      return ( local ? point : localToGlobal( point ) );
    }
  };
} // End namespace LHCb::Detector

inline bool LHCb::Detector::DeVPSensor::pointToChannel( ROOT::Math::XYZPoint const& point, bool local,
                                                        VPChannelID& channel ) const {
  ROOT::Math::XYZPoint localPoint = local ? point : globalToLocal( point );
  // Check if the point is in the active area of the sensor.
  if ( !isInActiveArea( localPoint ) ) return false;
  // Set the sensor number.
  channel.setSensor( sensorNumber() );
  double x0 = 0.;
  auto   dx = chipSize() + interChipDist();
  for ( unsigned int i = 0; i < numChips(); ++i, x0 += dx ) {
    const double x = localPoint.x() - x0;
    if ( x < chipSize() + 0.5 * interChipDist() ) {
      // Set the chip number.
      channel.setChip( VPChannelID::ChipID{i} );
      // Set the row and column.
      unsigned int col = 0;
      unsigned int row = 0;
      if ( x > 0. ) {
        col = int( x / pixelSize() );
        if ( col >= numColumns() ) col = numColumns() - 1;
      }
      if ( localPoint.y() > 0. ) {
        row = int( localPoint.y() / pixelSize() );
        if ( row >= numRows() ) row = numRows() - 1;
      }
      channel.setCol( VPChannelID::ColumnID{col} );
      channel.setRow( VPChannelID::RowID{row} );
      break;
    }
  }
  return true;
}
