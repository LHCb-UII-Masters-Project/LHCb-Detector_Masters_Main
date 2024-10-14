/*****************************************************************************\
* (c) Copyright 2000-2021 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once

#include <exception>
#include <nlohmann/json.hpp>
#include <vector>

// forward declaration to allow conversion from YAML
namespace YAML {
  class Node;
  template <typename T>
  struct convert;
} // namespace YAML

namespace LHCb::Detector::Muon {

  struct ReadoutCond final {
    enum Type : int { Anode = 0, Cathode = 1 };

    ReadoutCond() = default;
    ReadoutCond( const YAML::Node& node );

    ReadoutCond( const ReadoutCond& ) = default;
    ReadoutCond( ReadoutCond&& )      = default;

    ReadoutCond& operator=( const ReadoutCond& ) = default;
    ReadoutCond& operator=( ReadoutCond&& ) = default;

    /// get number of readouts associated to this chamber
    std::size_t size() const { return params.size(); }

    /// get readoutType (Anode=0, Cathode=1)
    Type readoutType( int i ) const { return params[i].ReadoutType; }

    /// get Efficiency as a fraction (mean value)
    double efficiency( int i ) const { return params[i].Efficiency; }
    /// get (maximum) synchronization imprecision (ns)
    double syncDrift( int i ) const { return params[i].SyncDrift; }
    /// get Chamber Noise rate (counts/sec/cm2)
    double chamberNoise( int i ) const { return params[i].ChamberNoise; }
    /// get Electronics noise rates (counts/sec/channel)
    double electronicsNoise( int i ) const { return params[i].ElectronicsNoise; }
    /// get average dead time of a channel (ns)
    double meanDeadTime( int i ) const { return params[i].MeanDeadTime; }
    /// get RMS of the dead time (ns)
    double rmsDeadTime( int i ) const { return params[i].RMSDeadTime; }

    int singleGapClusterX( double randomNumber, double xDistPadEdge, int i ) const {
      return singleGapCluster( 0, randomNumber, xDistPadEdge, i );
    }
    int singleGapClusterY( double randomNumber, double xDistPadEdge, int i ) const {
      return singleGapCluster( 1, randomNumber, xDistPadEdge, i );
    }

    std::vector<double> timeJitter( double& min, double& max, int i ) const {
      min = params[i].JitterMin;
      max = params[i].JitterMax;
      return params[i].JitterVector;
    }

  private:
    struct Parameters {
      struct Cluster {
        int    size;
        double prob;
      };

      Type                 ReadoutType;
      double               Efficiency;
      double               SyncDrift;
      double               ChamberNoise;
      double               ElectronicsNoise;
      double               MeanDeadTime;
      double               RMSDeadTime;
      double               TimeGateStart;
      double               PadEdgeSizeX;
      double               PadEdgeSigmaX;
      double               PadEdgeSizeY;
      double               PadEdgeSigmaY;
      std::vector<Cluster> ClusterX;
      std::vector<Cluster> ClusterY;
      std::vector<double>  CumProbX;
      std::vector<double>  CumProbY;
      std::vector<double>  JitterVector;
      double               JitterMin;
      double               JitterMax;
    };

    std::vector<Parameters> params;

    int singleGapCluster( int xy, double randomNumber, double xpos, int i ) const;

    // allow YAML converters to access private types and members
    template <typename T>
    friend struct YAML::convert;

    // allow JSON converters to access private types and members
    friend void from_json( const nlohmann::json&, ReadoutCond& );
    friend void from_json( const nlohmann::json&, Parameters& );
    friend void from_json( const nlohmann::json&, Parameters::Cluster& );
  };
} // namespace LHCb::Detector::Muon
