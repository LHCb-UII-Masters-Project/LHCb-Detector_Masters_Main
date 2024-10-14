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
#include "DD4hep/Printout.h"
#include "Detector/UT/ChannelID.h"
#include "Detector/UT/DeUTSensor.h"
#include "Detector/UT/UTConstants.h"

#include <array>
#include <optional>
#include <string>
#include <vector>

#include <fmt/format.h>

namespace LHCb::Detector::UT {

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
  static const std::vector<Status> s_vec_bs = {Status::OK, Status::ReadoutProblems, Status::Dead, Status::OtherFault};
  static const std::vector<Status> s_vec_ps = {Status::NotBonded};
  std::map<unsigned int, Status>   toEnumMap( const std::map<int, int>& input );
  std::map<unsigned int, Status>   toEnumMap( const std::vector<double>& input );

  Status        toStatus( std::string_view str );
  std::string   toString( Status status );
  std::ostream& toStream( Status status, std::ostream& os );

  namespace detail {

    struct DeUTSectorObject : LHCb::Detector::detail::DeIOVObject {

      const unsigned int m_id;
      DeUTSensorObject   m_sensor;
      DeUTSectorObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );

      typedef std::map<unsigned int, Status> StatusMap;

      double                    m_pitch;
      unsigned int              m_firstBeetle{1};
      unsigned int              m_version{2};
      double                    m_capacitance{0};
      bool                      m_xInverted;
      bool                      m_yInverted;
      ChannelID                 m_channelID;
      std::string               m_type;
      std::pair<double, double> m_halfLengths;
      float                     m_thickness;

      double                m_stripLength;
      double                m_dxdy = 0.0;
      double                m_dzdy = 0.0;
      double                m_dy   = 0.0;
      ROOT::Math::XYZVector m_dp0di;
      ROOT::Math::XYZPoint  m_p0;
      double                m_angle    = 0.0;
      double                m_cosAngle = 0.0;
      double                m_sinAngle = 0.0;
      double                m_measEff  = 0.0;

      std::string  m_nickname;
      unsigned int m_column;
      unsigned int m_row;

      // status info
      Status            m_status = Status::OK;
      StatusMap         m_beetleStatus;
      StatusMap         m_stripStatus;
      std::string       m_statusString  = "Status";
      std::string       m_versionString = "DC06";
      dd4hep::Condition m_statusCondition;
      dd4hep::Condition m_noiseCondition;
      dd4hep::Condition m_readoutMapCondition;

      // Noise info
      std::string         m_noiseString = "Noise";
      std::vector<double> m_noiseValues;
      std::vector<double> m_cmModeValues;
      double              m_electronsPerADC;

      // Hit error scaling factor
      dd4hep::Condition     m_hitErrorCondition;
      std::array<double, 4> m_hitErrorFactors{1., 1., 1., 1.};

      std::string getSectorName() const {
        auto                         channelID = m_channelID;
        static constexpr const char* sides[2]  = {"C", "A"};
        static constexpr const char* layers[4] = {"aX", "aU", "bV", "bX"};
        return fmt::format( "UT{side}sideUT{layer}Stave{stave}Face{face}Module{module}Sector{sector}",
                            fmt::arg( "side", sides[channelID.side()] ), fmt::arg( "layer", layers[channelID.layer()] ),
                            fmt::arg( "stave", channelID.stave() ), fmt::arg( "face", channelID.face() ),
                            fmt::arg( "module", channelID.module() ), fmt::arg( "sector", channelID.sector() ) );
      }

      std::string getLayerName() const {
        auto                         channelID = m_channelID;
        static constexpr const char* sides[2]  = {"C", "A"};
        static constexpr const char* layers[4] = {"aX", "aU", "bV", "bX"};
        return fmt::format( "UT{side}{layer}", fmt::arg( "side", sides[channelID.side()] ),
                            fmt::arg( "layer", layers[channelID.layer()] ) );
      }

      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        func( &m_sensor );
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
  struct DeUTSector : public DeIOVElement<detail::DeUTSectorObject> {

    using DeIOVElement::DeIOVElement;

    unsigned int firstStrip() const { return this->access()->m_sensor.m_firstStrip; }
    unsigned int nStrip() const { return this->access()->m_sensor.m_nStrip; }
    bool         stripflip() const { return this->access()->m_sensor.m_stripflip; }
    double       thickness() const { return this->access()->m_sensor.m_thickness; }
    std::string  nickname() const { return this->access()->m_nickname; }
    unsigned int id() const { return this->access()->m_id; }

    DeUTSensor   sensor() const { return DeUTSensor( &( this->access()->m_sensor ) ); }
    unsigned int nSensors() const { return 1; }
    bool         xInverted() const { return this->access()->m_sensor.m_xInverted; }
    bool         yInverted() const { return this->access()->m_sensor.m_yInverted; }
    ChannelID    channelID() const { return this->access()->m_channelID; }
    ChannelID    elementID() const { return this->access()->m_channelID; }
    unsigned int staveID() const { return channelID().stave(); }
    // column and row are old channel id fields and should never be used in DD4HEP
    unsigned int column() const { return this->access()->m_column; }
    unsigned int row() const { return this->access()->m_row; }
    unsigned int nBeetle() const { return nStrip() / nStripsInBeetle; }

    LineTraj<double> createTraj( unsigned int strip, double offset ) const {
      return sensor().trajectory( strip, offset );
    }
    LineTraj<double> trajectoryFirstStrip() const { return createTraj( firstStrip(), 0. ); }
    LineTraj<double> trajectoryLastStrip() const { return createTraj( nStrip() - ( firstStrip() + 1 ) % 2, 0. ); }

    ROOT::Math::XYZPoint globalCentre() const { return this->toGlobal( ROOT::Math::XYZPoint( 0.0, 0.0, 0.0 ) ); }

    std::pair<double, double> halfLengths() const { return this->access()->m_halfLengths; }
    std::string const&        type() const { return this->access()->m_type; }

    /// beetle corresponding to channel  1-3 (IT) 1-4 (TT)
    unsigned int beetle( const unsigned int strip ) const { return ( ( strip - 1u ) / nStripsInBeetle ) + 1u; };
    unsigned int beetle( const ChannelID& chan ) const { return beetle( chan.strip() + ( firstStrip() + 1 ) % 2 ); }

    ROOT::Math::XYZVector normalY() {
      return this->toGlobal( ROOT::Math::XYZPoint( 0., 1., 0. ) ) -
             this->toGlobal( ROOT::Math::XYZPoint( 0., 0., 0. ) );
    }
    double pitch() const { return this->access()->m_pitch; }

    std::string m_conditionPathName = "..to be filled";
    std::string conditionsPathName() const { return m_conditionPathName; }

    /** direct access to the status condition, for experts only */
    // note: alternatively, can define a member variable to store dd4hep::condition data type
    dd4hep::Condition* statusCondition() const { return &( this->access()->m_statusCondition ); }

    void setStatusCondition( const std::string& type, unsigned int entry, const Status& newStatus ) const {
      // Set the condition
      dd4hep::Condition* aCon = statusCondition();
      if ( !aCon ) {
        dd4hep::printout( dd4hep::ERROR, "DeUTSector::setStatusCondition", "Failed to find status condition" );
      } else {
        auto& params = aCon->get<nlohmann::json>();
        if ( type == "SectorStatus" ) {
          dd4hep::printout( dd4hep::DEBUG, "DeUTSector::setStatusCondition",
                            "Ignore input argument `entry' for SectorStatus" );
          params[nickname()][type] = int( newStatus );
        } else {
          params[nickname()][type][entry] = int( newStatus );
        }
      }
    }

    // Status of beetle
    std::vector<Status> beetleStatus() const {
      std::vector<Status> vec;
      vec.resize( nBeetle() );
      for ( unsigned int iBeetle = this->access()->m_firstBeetle; iBeetle <= nBeetle(); ++iBeetle ) {
        if ( sectorStatus() != Status::OK ) {
          vec[iBeetle - 1] = sectorStatus();
          continue;
        }
        auto iter = this->access()->m_beetleStatus.find( iBeetle );
        if ( iter != this->access()->m_beetleStatus.end() ) {
          vec[iBeetle - 1] = iter->second;
        } else {
          vec[iBeetle - 1] = Status::OK;
        }
      } // nBeetle
      return vec;
    }

    Status beetleStatus( unsigned int id ) const {
      Status theStatus = sectorStatus();
      if ( theStatus == Status::OK ) {
        if ( auto iter = this->access()->m_beetleStatus.find( id ); iter != this->access()->m_beetleStatus.end() )
          theStatus = iter->second;
      }
      return theStatus;
    }

    Status beetleStatus( ChannelID chan ) const { return beetleStatus( beetle( chan ) ); }

    std::vector<Status> stripStatus() const {
      std::vector<Status> vec;
      vec.resize( nStrip() );
      for ( unsigned int iStrip = 0; iStrip < nStrip(); ++iStrip ) {
        if ( sectorStatus() != Status::OK ) {
          vec[iStrip] = sectorStatus();
          continue;
        }
        LHCb::Detector::UT::ChannelID chan = stripToChan( iStrip );
        if ( beetleStatus( chan ) != Status::OK ) {
          vec[iStrip] = beetleStatus( chan );
          continue;
        }
        auto iter   = this->access()->m_stripStatus.find( iStrip );
        vec[iStrip] = ( iter != this->access()->m_stripStatus.end() ? iter->second : Status::OK );
      } // nStrip
      return vec;
    }

    Status stripStatus( ChannelID chan ) const {
      Status theStatus = beetleStatus( chan );
      if ( theStatus == Status::OK ) {
        if ( auto iter = this->access()->m_stripStatus.find( chan.strip() );
             iter != this->access()->m_stripStatus.end() )
          theStatus = iter->second;
      }
      return theStatus;
    }

    std::vector<Status> validBeetleStates() const { return LHCb::Detector::UT::s_vec_bs; }
    std::vector<Status> protectedStates() const { return LHCb::Detector::UT::s_vec_ps; }

    void setBeetleStatus( unsigned int beetle, const Status& newStatus ) const {
      if ( sectorStatus() != Status::OK ) {
        // if the sector is not ok nothing to be done
        dd4hep::printout( dd4hep::DEBUG, "DeUTSector::setBeetleStatus", "Sector is off anyway: set request ignored" );
      } else {
        if ( newStatus == Status::OK ) {
          // Lazarus walks...if we have an entry in the map delete it
          this->access()->m_beetleStatus.erase( beetle );
          setStatusCondition( "SALTStatus", beetle, newStatus );
        } else {
          // death comes to this beetle, update the map
          if ( std::find( validBeetleStates().begin(), validBeetleStates().end(), newStatus ) !=
               validBeetleStates().end() ) {
            this->access()->m_beetleStatus[beetle] = newStatus;
            setStatusCondition( "SALTStatus", beetle, newStatus );
          } // check is valid state
          else {
            dd4hep::printout( dd4hep::ERROR, "DeUTSector::setBeetleStatus",
                              "Not a valid Beetle state: set request ignored " );
          }
        }
      }
    }
    void setBeetleStatus( ChannelID chan, const Status& newStatus ) { setBeetleStatus( beetle( chan ), newStatus ); }

    void setStripStatus( unsigned int strip, const Status& newStatus ) {
      if ( sectorStatus() != Status::OK || beetleStatus( strip ) != Status::OK ) {
        // if the sector is not ok nothing to be done
        dd4hep::printout( dd4hep::DEBUG, "DeUTSector::setStripStatus",
                          "Sector/Beetle is off anyway: set request ignored " );
      } else {
        if ( newStatus == Status::OK ) {
          // Lazarus walks...if we have an entry in the map delete it
          this->access()->m_stripStatus.erase( strip );
        } else {
          // death comes to this strip, update the map
          Status oldStatus = this->access()->m_stripStatus.find( strip )->second;
          if ( std::find( protectedStates().begin(), protectedStates().end(), oldStatus ) == protectedStates().end() ) {
            this->access()->m_stripStatus[strip] = newStatus;
            setStatusCondition( "StripStatus", strip, newStatus );
          } else {
            dd4hep::printout( dd4hep::DEBUG, "DeUTSector::setStripStatus",
                              "Strip in protected state: set request ignored" );
          }
        }
      }
    }

    void setSectorStatus( const Status& newStatus ) const {
      this->access()->m_status = newStatus;
      // Set the condition
      dd4hep::Condition* aCon = statusCondition();
      if ( aCon == nullptr ) {
        dd4hep::printout( dd4hep::ERROR, "DeUTSector::setSectorStatus", "Failed to find status condition" );
      } else {
        auto& params                       = aCon->get<nlohmann::json>();
        params[nickname()]["SectorStatus"] = int( newStatus );
      }
    }

    Status sectorStatus() const { return this->access()->m_status; }
    /// measured efficiency
    double measEff() const { return this->access()->m_measEff; }
    void   setMeasEff( const double newMeasEff ) { this->access()->m_measEff = newMeasEff; }
    double fractionActive() const {
      // fraction of the sector that works
      const auto& statusVector = stripStatus();
      return std::count( statusVector.begin(), statusVector.end(), Status::OK ) / double( nStrip() );
    }

    /** direct access to the noise condition, for experts only */
    dd4hep::Condition* noiseCondition() const { return &( this->access()->m_noiseCondition ); }

    bool globalInBondGap( const ROOT::Math::XYZPoint& point, double tol ) const {
      return sensor().isInside( point ) ? sensor().globalInBondGap( point, tol ) : false;
    }
    bool globalInActive( const ROOT::Math::XYZPoint& point,
                         ROOT::Math::XYZPoint        tol = ROOT::Math::XYZPoint{0.0, 0.0, 0.0} ) const {
      return sensor().globalInActive( point, tol );
    }

    void applyToAllSensors( const std::function<void( const DeUTSensor& )>& func ) const { func( sensor() ); }

    const DeUTSensor findSensor( const ROOT::Math::XYZPoint& aPoint ) const {
      const auto& sensor = this->access()->m_sensor;
      return DeUTSensor{&sensor}.isInside( aPoint ) ? &sensor : nullptr;
    }

    bool updateNoiseCondition() const {
      dd4hep::Condition* aCon = noiseCondition();
      if ( !aCon ) {
        dd4hep::printout( dd4hep::ERROR, "DeUTSector::updateNoiseCondition", "failed to find noise condition" );
        return false;
      }
      auto&                      params   = aCon->get<nlohmann::json>();
      const std::vector<double>& tmpNoise = params[nickname()]["SectorNoise"];

      if ( tmpNoise.size() == nBeetle() )
        this->access()->m_noiseValues = tmpNoise;
      else {
        dd4hep::printout( dd4hep::ERROR, "DeUTSector::updateNoiseCondition",
                          fmt::format( "Size mismatch for SectorNoise: {new} vs. {ref}",
                                       fmt::arg( "new", tmpNoise.size() ), fmt::arg( "ref", nBeetle() ) ) );
        return false;
      }

      const double& tmpElectrons = params[nickname()]["electronsPerADC"];

      if ( tmpElectrons )
        this->access()->m_electronsPerADC = tmpElectrons;
      else {
        dd4hep::printout( dd4hep::ERROR, "DeUTSector::updateNoiseCondition", "Missing electronsPerADC" );
        return false;
      }

      if ( params[nickname()].contains( "cmNoise" ) ) {
        const std::vector<double>& tmpCM = params[nickname()]["cmNoise"];
        if ( tmpCM.size() == nBeetle() )
          this->access()->m_cmModeValues = tmpCM;
        else {
          dd4hep::printout( dd4hep::ERROR, "DeUTSector::updateNoiseCondition", "Size mismatch for cmNoise" );
          return false;
        }
      } else {
        // doesn't exists (MC early databases...)
        this->access()->m_cmModeValues.assign( nBeetle(), 0. );
        params[nickname()]["cmNoise"] = this->access()->m_cmModeValues;
      }
      return true;
    }
    bool updateStatusCondition() const {
      dd4hep::Condition* aCon = statusCondition();
      if ( !aCon ) {
        dd4hep::printout( dd4hep::ERROR, "DeUTSector::updateStatusCondition", "failed to find status condition" );
        return false;
      }
      auto& params = aCon->get<nlohmann::json>();

      const int& tStatus       = params[nickname()]["SectorStatus"];
      this->access()->m_status = Status( tStatus );

      this->access()->m_beetleStatus =
          toEnumMap( static_cast<std::vector<double>>( params[nickname()]["SALTStatus"] ) );

      this->access()->m_stripStatus =
          toEnumMap( static_cast<std::vector<double>>( params[nickname()]["StripStatus"] ) );

      if ( params[nickname()].contains( "measuredEff" ) ) {
        this->access()->m_measEff = params[nickname()]["measuredEff"];
      } else {
        this->access()->m_measEff         = 1.0;
        params[nickname()]["measuredEff"] = this->access()->m_measEff;
      }

      dd4hep::Condition* bCon = &( this->access()->m_readoutMapCondition );
      if ( !bCon ) {
        dd4hep::printout( dd4hep::ERROR, "DeUTSector::updateStatusCondition", "failed to find Readout condition" );
        return false;
      }
      this->access()->m_sensor.m_stripflip = bCon->get<nlohmann::json>().contains( "nTell40InUT" );

      return true;
    }

    bool isStrip( const unsigned int strip ) const { return sensor().isStrip( strip ); }
    bool contains( const ChannelID aChannel ) const { return aChannel.uniqueSector() == channelID().uniqueSector(); }
    ChannelID nextLeft( const ChannelID testChan ) const {
      if ( ( contains( testChan ) ) && ( testChan.strip() > 0 ) && ( isStrip( testChan.strip() - 1u ) ) ) {
        return ChannelID( testChan.channelID() - 1u );
      } else {
        return ChannelID( 0u );
      }
    }
    ChannelID nextRight( const ChannelID testChan ) const {
      if ( ( contains( testChan ) ) && ( isStrip( testChan.strip() + 1u ) ) ) {
        return ChannelID( testChan.channelID() + 1u );
      } else {
        return ChannelID( 0u );
      }
    }
    LineTraj<double> trajectory( ChannelID aChan, double offset ) const {
      if ( !contains( aChan ) ) {
        dd4hep::printout( dd4hep::ERROR, "DeUTSector::trajectory",
                          "Failed to link to sector " + nickname() + " test strip number " + aChan.strip() + " strip" );
        throw std::runtime_error( "DeUTSector.cpp: Failed to make trajectory" );
      }
      return createTraj( aChan.strip(), offset );
    }

    std::optional<float> cmPortNoise( unsigned int beetle, unsigned int port ) const {
      if ( beetle < this->access()->m_firstBeetle || beetle > this->access()->m_firstBeetle + nBeetle() - 1 ) {
        throw std::out_of_range( std::string( "DeUTSector::cmPortNoise: beetle out of range, beelte start with " ) +
                                 std::to_string( this->access()->m_firstBeetle ) );
      }

      if ( port == 0 || port > 4 ) { throw std::out_of_range( "DeUTSector::cmPortNoise: port out of range" ); }
      const std::vector<Status> statusVector = stripStatus();
      float                     sum( 0 ), number( 0 );
      for ( unsigned int chan( ( beetle - 1 ) * LHCb::Detector::UT::nStripsInBeetle +
                               ( port - 1 ) * LHCb::Detector::UT::nStripsInPort );
            chan < ( beetle - 1 ) * LHCb::Detector::UT::nStripsInBeetle + port * LHCb::Detector::UT::nStripsInPort;
            chan++ ) {
        if ( statusVector[chan] == Status::OK || statusVector[chan] == Status::Pinhole ) {
          sum += this->access()->m_cmModeValues[chan];
          number += 1;
        }
      }
      dd4hep::printout( dd4hep::DEBUG, "DeUTSector::cmPortNoise",
                        std::to_string( number ) + " strips out of " +
                            std::to_string( LHCb::Detector::UT::nStripsInPort ) + " are not taken into account" );
      return number > 0 ? std::optional( sum / number ) : std::nullopt;
    }
    double toElectron( double val ) const { return val * this->access()->m_electronsPerADC; }
    double toADC( double e ) const { return e / this->access()->m_electronsPerADC; }
    [[deprecated( "use 'toADC( double )' instead" )]] double toADC( double e, ChannelID ) const { return toADC( e ); }
    std::optional<float>                                     cmBeetleNoise( unsigned int beetle ) const {
      if ( beetle < this->access()->m_firstBeetle || beetle > this->access()->m_firstBeetle + nBeetle() - 1 ) {
        throw std::out_of_range( std::string( "DeUTSector::cmBeetleNoise: beetle out of range, beelte start with " ) +
                                 std::to_string( this->access()->m_firstBeetle ) );
      }
      auto number = this->access()->m_cmModeValues[beetle - this->access()->m_firstBeetle];
      return number > 0 ? std::optional( number ) : std::nullopt;
    }
    std::optional<float> cmSectorNoise() const {
      const std::vector<Status> statusVector = beetleStatus();
      float                     sum( 0 ), number( 0 );
      for ( unsigned int beetle( 0 ); beetle < nBeetle(); ++beetle ) {
        if ( statusVector[beetle] == Status::OK || statusVector[beetle] == Status::Pinhole ) {
          sum += this->access()->m_cmModeValues[beetle];
          number += 1;
        }
      }
      dd4hep::printout( dd4hep::DEBUG, "DeUTSector::cmSectorNoise",
                        std::to_string( number ) + " strips out of " + std::to_string( nBeetle() ) +
                            " are not taken into account" );
      return number > 0 ? std::optional( sum / number ) : std::nullopt;
    }
    std::optional<float> cmNoise( ChannelID aChannel ) const {
      // check strip is valid
      if ( !isStrip( aChannel.strip() ) ) return {};
      const Status theStatus = stripStatus( aChannel );
      // take ok strips and pinholes...
      if ( theStatus == Status::OK || theStatus == Status::Pinhole )
        return this->access()->m_cmModeValues[aChannel.strip() - 1u];
      return {};
    }

    std::optional<float> portNoise( unsigned int beetle, unsigned int port ) const {
      if ( beetle < this->access()->m_firstBeetle || beetle > this->access()->m_firstBeetle + nBeetle() - 1 ) {
        throw std::out_of_range( std::string( "DeUTSector::portNoise: beetle out of range, beelte start with " ) +
                                 std::to_string( this->access()->m_firstBeetle ) );
      }
      if ( port == 0 || port > 4 ) { throw std::out_of_range( "DeUTSector::portNoise: port out of range" ); }
      const std::vector<Status> statusVector = stripStatus();
      float                     sum( 0. ), number( 0. );
      for ( unsigned int chan( ( beetle - 1 ) * LHCb::Detector::UT::nStripsInBeetle +
                               ( port - 1 ) * LHCb::Detector::UT::nStripsInPort );
            chan < ( beetle - 1 ) * LHCb::Detector::UT::nStripsInBeetle + port * LHCb::Detector::UT::nStripsInPort;
            chan++ ) {
        if ( statusVector[chan] == Status::OK || statusVector[chan] == Status::Pinhole ) {
          sum += this->access()->m_noiseValues[chan];
          number += 1;
        }
      }
      dd4hep::printout( dd4hep::DEBUG, "DeUTSector::cmPortNoise", "%d strips out of %d are not taken into account",
                        number, LHCb::Detector::UT::nStripsInPort );
      return number > 0 ? std::optional( sum / number ) : std::nullopt;
    }

    std::optional<float> beetleNoise( unsigned int beetle ) const {
      if ( beetle < this->access()->m_firstBeetle || beetle > this->access()->m_firstBeetle + nBeetle() - 1 ) {
        throw std::out_of_range( std::string( "DeUTSector::beetleNoise: beetle out of range, beelte start with " ) +
                                 std::to_string( this->access()->m_firstBeetle ) );
      }
      float number = this->access()->m_noiseValues[beetle - this->access()->m_firstBeetle];
      return number > 0 ? std::optional( number ) : std::nullopt;
    }

    std::optional<float> sectorNoise() const {
      const std::vector<Status> statusVector = beetleStatus();
      float                     sum( 0 ), number( 0 );
      for ( unsigned int beetle( 0 ); beetle < nBeetle(); ++beetle ) {
        if ( statusVector[beetle] == Status::OK || statusVector[beetle] == Status::Pinhole ) {
          sum += this->access()->m_noiseValues[beetle];
          number += 1;
        }
      }
      dd4hep::printout( dd4hep::DEBUG, "DeUTSector::beetleNoise", "%d strips out of %d are not taken into account",
                        number, nBeetle() );
      return number > 0 ? std::optional( sum / number ) : std::nullopt;
    }

    std::optional<float> rawSectorNoise() const {
      auto sn   = sectorNoise();
      auto cmsn = cmSectorNoise();
      return sn && cmsn ? std::optional( std::sqrt( *sn * *sn + *cmsn * *cmsn ) ) : std::nullopt;
    }

    std::optional<float> rawBeetleNoise( unsigned int beetle ) const {
      auto bn   = beetleNoise( beetle );
      auto cmbn = cmBeetleNoise( beetle );
      return bn && cmbn ? std::optional( std::sqrt( *bn * *bn + *cmbn * *cmbn ) ) : std::nullopt;
    }

    std::optional<float> rawPortNoise( unsigned int beetle, unsigned int port ) const {
      auto pn   = portNoise( beetle, port );
      auto cmpn = cmPortNoise( beetle, port );
      return pn && cmpn ? std::optional( std::sqrt( *pn * *pn + *cmpn * *cmpn ) ) : std::nullopt;
      ;
    }

    std::optional<float> rawNoise( ChannelID aChannel ) const {
      auto cmn = cmNoise( aChannel );
      auto n   = noise( aChannel );
      return cmn && n ? std::optional( std::sqrt( *n * *n + *cmn * *cmn ) ) : std::nullopt;
    }

    std::optional<float> noise( ChannelID aChannel ) const {
      // check strip is valid
      if ( !isStrip( aChannel.strip() + firstStrip() ) ) return {};
      const Status theStatus = stripStatus( aChannel );
      // take ok strips and pinholes...
      if ( theStatus == Status::OK || theStatus == Status::Pinhole )
        return this->access()->m_noiseValues[aChannel.strip()];
      return {};
    }
    std::string m_hybridType;
    std::string hybridType() const { return m_hybridType; }

    ChannelID stripToChan( const unsigned int strip ) const {
      return isStrip( strip ) ? LHCb::Detector::UT::ChannelID( channelID().channelID() + strip - firstStrip() )
                              : LHCb::Detector::UT::ChannelID( 0 );
    }
    bool isOKStrip( ChannelID chan ) const { return stripStatus( chan ) == Status::OK; }

    // return hit error factors
    std::array<double, 4> hitErrorFactors() const { return this->access()->m_hitErrorFactors; }
    double                hitErrorFactor( unsigned int clusterSize ) const {
      // support up to clusterSize=4
      return hitErrorFactors()[std::min( static_cast<int>( clusterSize ) - 1, 3 )];
    }

    void trajectory( unsigned int strip, double offset, double& dxdy, double& dzdy, double& xAtYEq0, double& zAtYEq0,
                     double& ybegin, double& yend ) const {
      auto i         = offset + strip;
      auto numstrips = ( ( stripflip() && xInverted() ) ? ( nStrip() - i ) : i );

      dxdy    = this->access()->m_dxdy;
      dzdy    = this->access()->m_dzdy;
      xAtYEq0 = this->access()->m_p0.x() + numstrips * this->access()->m_dp0di.x();
      zAtYEq0 = this->access()->m_p0.z() + numstrips * this->access()->m_dp0di.z();
      ybegin  = this->access()->m_p0.y() + numstrips * this->access()->m_dp0di.y();
      yend    = ybegin + this->access()->m_dy;
    }
    double                cosAngle() const { return this->access()->m_cosAngle; }
    double                capacitance() const { return this->access()->m_capacitance; }
    double                sensorCapacitance() const { return sensor().capacitance(); }
    ROOT::Math::XYZPoint  get_p0() const { return this->access()->m_p0; }
    ROOT::Math::XYZVector get_dp0di() const { return this->access()->m_dp0di; }
    double                get_dy() const { return this->access()->m_dy; }
    double                get_dxdy() const { return this->access()->m_dxdy; }
    bool                  getStripflip() const { return stripflip(); }
  };

} // namespace LHCb::Detector::UT
