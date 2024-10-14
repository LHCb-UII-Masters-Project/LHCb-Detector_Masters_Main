/*****************************************************************************\
* (c) Copyright 2022 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#include <Detector/Muon/ReadoutCond.h>
#include <fmt/format.h>
#include <map>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <vector>

/* example of the expected YAML (which is internally converted to JSON)
ReadoutModules:
  M1R1:
    - ReadoutType: Cathode
      Efficiency: 0.995000
      SyncDrift: 3.000000
      ChamberNoise: 0.000000
      ElectronicsNoise: 100.000000
      MeanDeadTime: 50.000000
      RMSDeadTime: 10.000000
      TimeGateStart: 0.000000
      PadEdgeSizeX: -0.0670000
      PadEdgeSigmaX: 1.00000
      ClusterX: [{size: 1, prob: 0.980}, {size: 2, prob: 0.020}]
      PadEdgeSizeY: -0.067
      PadEdgeSigmaY: 1.000
      ClusterY: [{size: 1, prob: 0.980}, {size: 2, prob: 0.020}]
      JitterMin: 0.
      JitterMax: 58.
      JitterValues: [
        0.00001, 0.00005, 0.00022, 0.00065, 0.00136, 0.00248, 0.00373, 0.00555, 0.00760, 0.00971,
        0.01198, 0.01443, 0.01662, 0.01893, 0.02130, 0.02310, 0.02506, 0.02661, 0.02800, 0.02906,
        0.02968, 0.03050, 0.03097, 0.03100, 0.03128, 0.03093, 0.03023, 0.02991, 0.02911, 0.02826,
        0.02751, 0.02653, 0.02550, 0.02458, 0.02373, 0.02252, 0.02136, 0.02045, 0.01906, 0.01800,
        0.01688, 0.01591, 0.01478, 0.01403, 0.01299, 0.01212, 0.01137, 0.01044, 0.00967, 0.00888,
        0.00834, 0.00771, 0.00718, 0.00657, 0.00591, 0.00548, 0.00512, 0.00458, 0.00423, 0.00386,
        0.00351, 0.00323, 0.00296, 0.00265, 0.00239, 0.00222, 0.00202, 0.00182, 0.00164, 0.00152,
        0.00136, 0.00125, 0.00111, 0.00101, 0.00087, 0.00078, 0.00072, 0.00065, 0.00060, 0.00050,
        0.00047, 0.00039, 0.00038, 0.00034, 0.00031, 0.00027, 0.00022, 0.00019, 0.00018, 0.00014,
        0.00012, 0.00012, 0.00010, 0.00008, 0.00008, 0.00006, 0.00005, 0.00004, 0.00004, 0.00004,
      ]
*/

namespace LHCb::Detector::Muon {
  void from_json( const nlohmann::json& j, ReadoutCond::Parameters::Cluster& v ) {
    v.size = j.at( "size" ).get<int>();
    v.prob = j.at( "prob" ).get<double>();
  }
  void from_json( const nlohmann::json& j, ReadoutCond::Parameters& v ) {
    {
      auto value = j.at( "ReadoutType" ).get<std::string>();
      if ( value == "Anode" ) {
        v.ReadoutType = ReadoutCond::Anode;
      } else if ( value == "Cathode" ) {
        v.ReadoutType = ReadoutCond::Cathode;
      } else {
        throw std::invalid_argument(
            fmt::format( "invalid value for ReadoutCond::Parameters::ReadoutType '{}'", value ) );
      }
    }

    v.Efficiency       = j.at( "Efficiency" ).get<double>();
    v.SyncDrift        = j.at( "SyncDrift" ).get<double>();
    v.ChamberNoise     = j.at( "ChamberNoise" ).get<double>();
    v.ElectronicsNoise = j.at( "ElectronicsNoise" ).get<double>();
    v.MeanDeadTime     = j.at( "MeanDeadTime" ).get<double>();
    v.RMSDeadTime      = j.at( "RMSDeadTime" ).get<double>();
    v.TimeGateStart    = j.at( "TimeGateStart" ).get<double>();
    v.PadEdgeSizeX     = j.at( "PadEdgeSizeX" ).get<double>();
    v.PadEdgeSigmaX    = j.at( "PadEdgeSigmaX" ).get<double>();
    v.PadEdgeSizeY     = j.at( "PadEdgeSizeY" ).get<double>();
    v.PadEdgeSigmaY    = j.at( "PadEdgeSigmaY" ).get<double>();

    v.ClusterX = j.at( "ClusterX" ).get<std::vector<ReadoutCond::Parameters::Cluster>>();
    v.ClusterY = j.at( "ClusterY" ).get<std::vector<ReadoutCond::Parameters::Cluster>>();

    v.JitterMin    = j.at( "JitterMin" ).get<double>();
    v.JitterMax    = j.at( "JitterMax" ).get<double>();
    v.JitterVector = j.at( "JitterValues" ).get<std::vector<double>>();
  }
  void from_json( const nlohmann::json& j, ReadoutCond& v ) {
    v.params = j.get<std::vector<ReadoutCond::Parameters>>();

    auto calc_cum_prob = []( const auto& cluster ) {
      std::vector<double> cum_prob(
          std::max_element( cluster.begin(), cluster.end(), []( auto a, auto b ) { return a.size < b.size; } )->size );
      // fill CumProbX in position i with value p from cluster
      for ( const auto& c : cluster ) cum_prob[c.size - 1] = c.prob;
      // actually compute the partial propability
      std::partial_sum( cum_prob.begin(), cum_prob.end(), cum_prob.begin() );
      // alright paranoia here, normalise the cumlative sum
      std::transform( cum_prob.begin(), cum_prob.end(), cum_prob.begin(),
                      [&cum_prob]( auto& i ) { return i / cum_prob.back(); } );
      return cum_prob;
    };

    // Set CumProbX and CumProbY from cluserX and clusterY
    for ( auto& param : v.params ) {
      assert( param.ClusterX.size() && param.ClusterY.size() );
      param.CumProbX = calc_cum_prob( param.ClusterX );
      param.CumProbY = calc_cum_prob( param.ClusterY );
    }
  }
} // namespace LHCb::Detector::Muon
