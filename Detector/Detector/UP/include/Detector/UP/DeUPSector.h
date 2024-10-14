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
#include "Detector/UP/DeUPSensor.h"
#include "Detector/UP/UPChannelID.h"

#include <array>
#include <vector>

namespace LHCb::Detector {

  namespace detail {

    struct DeUPSectorObject : LHCb::Detector::detail::DeIOVObject {
      DeUPSectorObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );
      std::vector<DeUPSensorObject> m_sensors;

      double       m_pitch, m_pixelWidth, m_pixelLength, m_deadWidth, m_capacitance;
      double       m_version{1};
      unsigned int m_nPixel;
      unsigned int m_firstPixel{0};
      bool         m_pixelflip;
      bool         m_xInverted;
      bool         m_yInverted;
      ChannelID    m_channelID;

      double                m_dxdy, m_dzdy, m_dy, m_angle, m_cosAngle, m_sinAngle;
      ROOT::Math::XYZVector m_dp0di;
      ROOT::Math::XYZPoint  m_p0;

      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& sensor : m_sensors ) { func( &sensor ); };
      }
    };
  } // End namespace detail

  enum class Status {
    OK              = 0,
    Open            = 1,
    Short           = 2,
    Pinhole         = 3,
    ReadoutProblems = 4,
    NotBonded       = 5,
    LowGain         = 6,
    Noisy           = 7,
    OtherFault      = 9,
    Dead            = 10,
    UnknownStatus   = 100
  };

  static const std::map<Status, std::string> s_map = {{Status::OK, "OK"},
                                                      {Status::Open, "Open"},
                                                      {Status::Short, "Short"},
                                                      {Status::Pinhole, "Pinhole"},
                                                      {Status::NotBonded, "NotBonded"},
                                                      {Status::LowGain, "LowGain"},
                                                      {Status::Noisy, "Noisy"},
                                                      {Status::ReadoutProblems, "ReadoutProblems"},
                                                      {Status::OtherFault, "OtherFault"},
                                                      {Status::Dead, "Dead"},
                                                      {Status::UnknownStatus, "Unknown"}};

  std::string          toString( Status status );
  Status               toStatus( std::string_view str );
  inline std::ostream& operator<<( std::ostream& os, Status const& s ) { return os << s_map.at( s ); }
  inline std::ostream& toStream( Status status, std::ostream& os ) {
    return os << std::quoted( toString( status ), '\'' );
  }

  /**
   *  This object defines the behaviour of the objects's data
   *
   *  \author  Markus Frank
   *  \date    2018-03-08
   *  \version  1.0
   */
  template <typename ObjectType>
  class DeUPSectorElement : public DeIOVElement<ObjectType> {
  private:
    auto const& sensors() const { return this->access()->m_sensors; }

  public:
    using DeIOVElement<ObjectType>::DeIOVElement;

    unsigned int firstPixel() const { return this->access()->m_firstPixel; }
    unsigned int nPixel() const { return this->access()->m_nPixel; }
    bool         pixelflip() { return this->access()->m_pixelflip; }
    double       thickness() const { throw NotImplemented(); }

    DeUPSensor   sensor( unsigned int index ) const { return &this->access()->m_sensors[index]; }
    unsigned int nSensors() const { return sensors().size(); }
    bool         xInverted() const { throw NotImplemented(); }
    bool         yInverted() const { throw NotImplemented(); }
    ChannelID    channelID() const { return this->access()->m_channelID; }

    LineTraj<double> createTraj( unsigned int, double ) const { throw NotImplemented(); }
    LineTraj<double> trajectoryFirstPixel() const { return createTraj( firstPixel(), 0. ); }
    LineTraj<double> trajectoryLastPixel() const { return createTraj( nPixel(), 0. ); }

    ChannelID                 elementID() const { throw NotImplemented(); }
    unsigned int              column() const { throw NotImplemented(); }
    unsigned int              row() const { throw NotImplemented(); }
    unsigned int              id() const { throw NotImplemented(); }
    std::string const&        type() const { throw NotImplemented(); }
    ROOT::Math::XYZPoint      globalCentre() const { throw NotImplemented(); }
    std::pair<double, double> halfLengths() const { throw NotImplemented(); }
    const std::string&        nickname() const { throw NotImplemented(); }

    /// beetle corresponding to channel  1-3 (IT) 1-4 (TT)
    unsigned int beetle( const ChannelID& ) const { throw NotImplemented(); }

    ROOT::Math::XYZVector normalY() { throw NotImplemented(); }
    double                pitch() const { throw NotImplemented(); }
    double                pixelWidth() const { throw NotImplemented(); }
    double                pixelLength() const { throw NotImplemented(); }

    std::string m_conditionPathName = "..to be filled";
    std::string conditionsPathName() const { return m_conditionPathName; }

    void setStatusCondition( const std::string&, unsigned int, const Status& ) { throw NotImplemented(); }
    /// Status of channel
    Status pixelStatus( ChannelID ) const { throw NotImplemented(); }
    void   setPixelStatus( unsigned int, const Status& ) { throw NotImplemented(); }
    void   setBeetleStatus( unsigned int, const Status& ) { throw NotImplemented(); }
    void   setSectorStatus( const Status& ) { throw NotImplemented(); }
    Status sectorStatus() const { throw NotImplemented(); }
    /// measured efficiency
    double measEff() const { throw NotImplemented(); }
    void   setMeasEff( const double ) { throw NotImplemented(); }
    double fractionActive() const { throw NotImplemented(); }
    bool   globalInBondGap( const ROOT::Math::XYZPoint&, double ) const { throw NotImplemented(); }
    bool   globalInActive( const ROOT::Math::XYZPoint&, ROOT::Math::XYZPoint = {0., 0., 0.} ) const {
      throw NotImplemented();
    }
    const std::optional<DeUPSensor> findSensor( const ROOT::Math::XYZPoint& ) const { throw NotImplemented(); }

    /// apply given callable to all sensors
    void applyToAllSensors( const std::function<void( DeUPSensor const& )>& ) const { throw NotImplemented(); }

    bool        updateNoiseCondition() { throw NotImplemented(); }
    bool        updateStatusCondition() { throw NotImplemented(); }
    bool        registerConditionsCallbacks() { throw NotImplemented(); }
    inline bool isPixel( const unsigned int pixel ) const {
      return pixel >= firstPixel() && pixel < firstPixel() + nPixel();
    }
    inline bool contains( const ChannelID aChannel ) const {
      return aChannel.uniqueSector() == channelID().uniqueSector();
    }
    ChannelID nextLeft( const ChannelID testChan ) const {
      if ( ( contains( testChan ) ) && ( testChan.pixel() > 0 ) && ( isPixel( testChan.pixel() - 1u ) ) ) {
        return ChannelID( testChan.channelID() - 1u );
      } else {
        return ChannelID( 0u );
      }
    }
    ChannelID nextRight( const ChannelID testChan ) const {
      if ( ( contains( testChan ) ) && ( isPixel( testChan.pixel() + 1u ) ) ) {
        return ChannelID( testChan.channelID() + 1u );
      } else {
        return ChannelID( 0u );
      }
    }
    LineTraj<double> trajectory( ChannelID aChan, double offset ) const {
      if ( !contains( aChan ) ) {
        // MsgStream msg( msgSvc(), name() );
        // msg << MSG::ERROR << "Failed to link to sector " << nickname() << " test pixel  number " << aChan.pixel()
        // << " pixel " << endmsg;
        // throw GaudiException( "Failed to make trajectory", "DeUPSector.cpp", StatusCode::FAILURE );
        // throw error;
      }
      return createTraj( aChan.pixel(), offset );
    }
    double toElectron( double, unsigned int ) const { throw NotImplemented(); }
    double toADC( double, unsigned int ) const { throw NotImplemented(); }
    float  cmPortNoise( unsigned int, unsigned int ) const { throw NotImplemented(); }
    float  cmBeetleNoise( unsigned int ) const { throw NotImplemented(); }
    float  cmSectorNoise() const { throw NotImplemented(); }
    float  cmNoise( ChannelID ) const { throw NotImplemented(); }
    void   setADCConversion( std::vector<double> ) { throw NotImplemented(); }
    void   setCMNoise( std::vector<double> ) { throw NotImplemented(); }
    void   setCMNoise( unsigned int, double ) { throw NotImplemented(); }
    void   setNoise( std::vector<double> ) { throw NotImplemented(); }
    void   setNoise( unsigned int, double ) { throw NotImplemented(); }
    float  portNoise( unsigned int, unsigned int ) const { throw NotImplemented(); }
    float  beetleNoise( unsigned int ) const { throw NotImplemented(); }
    float  sectorNoise() const { throw NotImplemented(); }
    float  rawSectorNoise() const {
      return std::sqrt( sectorNoise() * sectorNoise() + cmSectorNoise() * cmSectorNoise() );
    }
    float rawBeetleNoise( unsigned int beetle ) const {
      return std::sqrt( beetleNoise( beetle ) * beetleNoise( beetle ) +
                        cmBeetleNoise( beetle ) * cmBeetleNoise( beetle ) );
    }
    float rawPortNoise( unsigned int beetle, unsigned int port ) const {
      return std::sqrt( portNoise( beetle, port ) * portNoise( beetle, port ) +
                        cmPortNoise( beetle, port ) * cmPortNoise( beetle, port ) );
    }
    float rawNoise( ChannelID aChannel ) const {
      return std::sqrt( noise( aChannel ) * noise( aChannel ) + cmNoise( aChannel ) * cmNoise( aChannel ) );
    }
    float       noise( ChannelID ) const { throw NotImplemented(); }
    std::string m_hybridType;
    std::string hybridType() const { return m_hybridType; }

    ChannelID pixelToChan( unsigned int ) const { throw NotImplemented(); }
    bool      isOKPixel( ChannelID ) const { throw NotImplemented(); }
    void      trajectory( unsigned int, double, double&, double&, double&, double&, double&, double& ) const {
      throw NotImplemented();
    }
    double                cosAngle() const { throw NotImplemented(); }
    double                capacitance() const { throw NotImplemented(); }
    ROOT::Math::XYZPoint  get_p0() const { throw NotImplemented(); }
    ROOT::Math::XYZVector get_dp0di() const { throw NotImplemented(); }
    double                get_dy() const { throw NotImplemented(); }
    bool                  getPixelflip() const { throw NotImplemented(); }

    friend std::ostream& operator<<( std::ostream&, const DeUPSectorElement& ) { throw NotImplemented(); }
  };

  using DeUPSector = DeUPSectorElement<detail::DeUPSectorObject>;

} // namespace LHCb::Detector
