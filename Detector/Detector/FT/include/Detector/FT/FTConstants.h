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
#include <array>
#include <cstdint>

namespace LHCb::Detector {
  namespace FT {
    enum DetElementTypes {
      TOP     = 1 << 0,
      STATION = 1 << 1,
      LAYER   = 1 << 2,
      QUARTER = 1 << 3,
      MODULE  = 1 << 4,
      MAT     = 1 << 5
    };
    constexpr uint32_t nStations      = 3;
    constexpr uint32_t nLayers        = 4;
    constexpr uint32_t nLayersTotal   = nStations * nLayers;
    constexpr uint32_t nXLayersTotal  = nStations * 2;
    constexpr uint32_t nUVLayersTotal = nStations * 2;

    constexpr uint32_t nHalfLayers            = 2;
    constexpr uint32_t nQuarters              = 4;
    constexpr uint32_t nQuartersPerHalfLayer  = 2;
    constexpr uint32_t nModulesMax            = 6;
    constexpr uint32_t nMats                  = 4;
    constexpr uint32_t nHalfRobs              = 2;
    constexpr uint32_t nLinksPerClusterBoard  = 2;
    constexpr uint32_t nSiPM                  = 4;
    constexpr uint32_t nChannels              = 128;
    constexpr uint32_t nDie                   = 2;
    constexpr uint32_t nChannelsInDie         = nChannels / nDie;
    constexpr uint32_t nSiPMsPerModule        = nSiPM * nMats;
    constexpr uint32_t nChannelsPerModule     = nSiPMsPerModule * nChannels;
    constexpr uint32_t nQuartersStation       = nQuarters * nLayers;
    constexpr uint32_t nQuartersTotal         = nQuartersStation * nStations;
    constexpr uint32_t nZonesTotal            = nQuartersTotal / 2;
    constexpr uint32_t nMatsMax               = ( nStations + 1 ) * ( nQuartersStation ) * ( nModulesMax ) * ( nMats );
    constexpr uint32_t nModulesTotal          = nQuartersTotal * 5 + nLayers * nQuarters;
    constexpr uint32_t nMatsTotal             = nMats * nModulesTotal;
    constexpr uint16_t nSiPMsTotal            = nSiPMsPerModule * nModulesTotal;
    constexpr uint16_t nMaxMatsPerQuarter     = nMats * nModulesMax;
    constexpr uint16_t nMaxSiPMsPerQuarter    = nSiPM * nMaxMatsPerQuarter;
    constexpr uint16_t nMaxChannelsPerQuarter = nChannels * nMaxSiPMsPerQuarter;

    constexpr inline std::size_t maxNumberMats  = 2 * nLayersTotal * nModulesMax * nSiPMsPerModule; // 2304
    constexpr inline float       triangleHeight = 22.7f;                                            // mm

    // FIXME: Hardcoded
    constexpr inline auto xZonesUpper = std::array{1, 7, 9, 15, 17, 23};
    constexpr inline auto xZonesLower = std::array{0, 6, 8, 14, 16, 22};

    constexpr inline auto uvZonesUpper = std::array{3, 5, 11, 13, 19, 21};
    constexpr inline auto uvZonesLower = std::array{2, 4, 10, 12, 18, 20};

    constexpr inline auto ZonesLower = std::array{0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22};

    constexpr inline auto stereoZones = std::array{2, 3, 4, 5, 10, 11, 12, 13, 18, 19, 20, 21};

    constexpr inline auto xLayers       = std::array{0, 3, 4, 7, 8, 11};
    constexpr inline auto stereoLayers  = std::array{1, 2, 5, 6, 9, 10};
    constexpr inline auto xStereoLayers = std::array{0, 3, 4, 7, 8, 11, 1, 2, 5, 6, 9, 10};
    constexpr inline auto isXLayer =
        std::array{true, false, false, true, true, false, false, true, true, false, false, true};

    enum UpperZones {
      T1X1 = xZonesUpper[0],
      T1U  = uvZonesUpper[0],
      T1V  = uvZonesUpper[1],
      T1X2 = xZonesUpper[1],
      T2X1 = xZonesUpper[2],
      T2U  = uvZonesUpper[2],
      T2V  = uvZonesUpper[3],
      T2X2 = xZonesUpper[3],
      T3X1 = xZonesUpper[4],
      T3U  = uvZonesUpper[4],
      T3V  = uvZonesUpper[5],
      T3X2 = xZonesUpper[5],
    };

    // Shifts
    constexpr uint16_t stationShift = 1;
    constexpr uint16_t layerShift   = stationShift * nLayers;
    constexpr uint16_t quarterShift = layerShift * nQuarters;

    /**
     * the maximum number of possible hits in the SciFiTracker is a hardware limit determined by the
     * output bandwidth of the TELL40. The max number is 45568, round it up to 50k.
     * (for more details ask Sevda Esen and Olivier Le Dortz)
     */
    constexpr inline std::size_t maxNumberHits{50000};

    // Others
    constexpr uint32_t clusterLength = 9; // Channel (7) Frac (1) Size (1)
    constexpr uint16_t nModulesPerQuarter( const uint16_t station ) { return station < 3 ? 5u : 6u; }
    constexpr uint16_t nModulesPerStation( const uint16_t station ) {
      return nModulesPerQuarter( station ) * nQuarters * nLayers;
    }

    // Raw bank params
    namespace RawBank {
      // From https://edms.cern.ch/ui/file/2100937/5/edms_2100937_raw_data_format_run3.pdf
      constexpr uint32_t leftHeader  = 0b00111;
      constexpr uint32_t rightHeader = 0b01000;

      enum shifts {
        linkShift     = 9,
        cellShift     = 2,
        fractionShift = 1,
        sizeShift     = 0,
      };

      enum masks {
        linkMask = 63,
        cellMask = 127,
      };

      inline constexpr uint16_t maxClusterWidth = 4;
      inline constexpr uint16_t nbClusMaximum   = 31;  // 5 bits
      inline constexpr uint16_t nbDigitMaximum  = 128; // 128 digits per link
      inline constexpr uint16_t nbClusFFMaximum = 10;  //
      inline constexpr uint16_t fractionMaximum = 1;   // 1 bits allocted
      inline constexpr uint16_t cellMaximum     = 127; // 0 to 127; coded on 7 bits
      inline constexpr uint16_t sizeMaximum     = 1;   // 1 bits allocated
      inline constexpr uint16_t maxPseudoSize   = 16;
      inline constexpr uint16_t maxAdcCharge    = 4;

      enum BankProperties {
        NbBanksMax        = 240,
        NbBanksPerQuarter = 5,
        NbLinksPerBank    = 24,
        NbLinksMax        = NbLinksPerBank * NbBanksMax
      };

    } // namespace RawBank

  } // namespace FT

} // End namespace LHCb::Detector
