/***************************************************************************** \
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

#include <array>
#include <bitset>
#include <set>

namespace LHCb::Detector {
  namespace Muon {
    constexpr unsigned int nStations      = 4;
    constexpr unsigned int nSides         = 2;
    constexpr unsigned int nRegions       = 4;
    constexpr unsigned int nQuadrants     = 4;
    constexpr unsigned int nPartitions    = nStations * nRegions;
    constexpr unsigned int nGaps          = 4;
    constexpr unsigned int maxReadoutType = 2;
    constexpr unsigned int firstStation   = 2;
    constexpr unsigned int firstRegion    = 1;
    constexpr unsigned int firstChamber   = 1;
    //
    static constexpr std::size_t N_OF_STATIONS = 4;
    static constexpr std::size_t N_OF_REGIONS  = 4;
    static constexpr std::size_t N_OF_TotSides = 8;  //  nStations * nSides
    static constexpr std::size_t N_OF_TotReg   = 32; //  nStations * nSides * nRegions
    //
    static constexpr std::array<std::pair<unsigned int, unsigned int>, N_OF_REGIONS> grid{
        {{1, 1}, {1, 2}, {1, 4}, {2, 8}}};
    static constexpr std::array<unsigned int, N_OF_REGIONS> nChambers{12, 24, 48, 192};
    //
    // Initialize station names
    const std::string m_stationName[Muon::N_OF_REGIONS] = {"M2", "M3", "M4", "M5"};

  } // namespace Muon

} // End namespace LHCb::Detector
