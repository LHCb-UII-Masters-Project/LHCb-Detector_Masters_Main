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
#include <map>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

/* example of the expected YAML
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

namespace YAML {
  template <>
  struct convert<LHCb::Detector::Muon::ReadoutCond::Parameters::Cluster> {
    static bool decode( const Node& node, LHCb::Detector::Muon::ReadoutCond::Parameters::Cluster& rhs ) {
      if ( !node.IsMap() || node.size() != 2 ) { return false; }
      rhs.size = node["size"].as<int>();
      rhs.prob = node["prob"].as<double>();
      return true;
    }
  };

  template <>
  struct convert<LHCb::Detector::Muon::ReadoutCond::Parameters> {
    // static Node encode( const ReadoutCond::Parameters& rhs ) {
    //   Node node;
    //   return node;
    // }

    static bool decode( const Node& node, LHCb::Detector::Muon::ReadoutCond::Parameters& rhs ) {
      using LHCb::Detector::Muon::ReadoutCond;
      using Cluster = LHCb::Detector::Muon::ReadoutCond::Parameters::Cluster;
      if ( !node.IsMap() || node.size() != 17 ) { return false; }

      {
        auto value = node["ReadoutType"].as<std::string>();
        if ( value == "Anode" ) {
          rhs.ReadoutType = ReadoutCond::Anode;
        } else if ( value == "Cathode" ) {
          rhs.ReadoutType = ReadoutCond::Cathode;
        } else {
          return false;
        }
      }

      rhs.Efficiency       = node["Efficiency"].as<double>();
      rhs.SyncDrift        = node["SyncDrift"].as<double>();
      rhs.ChamberNoise     = node["ChamberNoise"].as<double>();
      rhs.ElectronicsNoise = node["ElectronicsNoise"].as<double>();
      rhs.MeanDeadTime     = node["MeanDeadTime"].as<double>();
      rhs.RMSDeadTime      = node["RMSDeadTime"].as<double>();
      rhs.TimeGateStart    = node["TimeGateStart"].as<double>();
      rhs.PadEdgeSizeX     = node["PadEdgeSizeX"].as<double>();
      rhs.PadEdgeSigmaX    = node["PadEdgeSigmaX"].as<double>();
      rhs.PadEdgeSizeY     = node["PadEdgeSizeY"].as<double>();
      rhs.PadEdgeSigmaY    = node["PadEdgeSigmaY"].as<double>();

      rhs.ClusterX = node["ClusterX"].as<std::vector<Cluster>>();
      rhs.ClusterY = node["ClusterY"].as<std::vector<Cluster>>();

      rhs.JitterMin    = node["JitterMin"].as<double>();
      rhs.JitterMax    = node["JitterMax"].as<double>();
      rhs.JitterVector = node["JitterValues"].as<std::vector<double>>();

      return true;
    }
  };
  template <>
  struct convert<LHCb::Detector::Muon::ReadoutCond> {
    static bool decode( const Node& node, LHCb::Detector::Muon::ReadoutCond& rhs ) {
      rhs = LHCb::Detector::Muon::ReadoutCond( node );
      return true;
    }
  };
} // namespace YAML
