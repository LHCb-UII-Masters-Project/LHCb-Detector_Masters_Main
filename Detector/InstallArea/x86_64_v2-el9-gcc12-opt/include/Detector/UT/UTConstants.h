/*****************************************************************************\
* (c) Copyright 2020 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once

namespace LHCb::Detector {

  namespace UT {

    static const unsigned int nStripsInBeetle = 128; // FIXME: beetle is a misnomer of ASIC
    static const unsigned int nStripsInSector = 512;
    static const unsigned int nStripsInPort   = 32;

    enum UserFlags {
      LEFT     = 0,
      RIGHT    = 1 << 0,
      TOP      = 1 << 1,
      SIDE     = 1 << 2,
      MODULE   = 1 << 3,
      ASSEMBLY = 1 << 4,
      SENSOR   = 1 << 5,
      ASIC     = 1 << 6
    };

    enum class DetectorNumbers : unsigned {
      Sectors     = 98,
      Regions     = 3,
      Layers      = 2,
      Stations    = 2,
      TotalLayers = 4,
      SubSectors  = 2, // FIXME: sector is misnamed as subsector here
      Modules     = 8,
      Faces       = 2,
      Staves      = 9,
      HalfLayers  = 4,
      Sides       = 2
    };

    constexpr const unsigned int nSides          = static_cast<unsigned int>( DetectorNumbers::Sides );
    constexpr const unsigned int nHalfLayers     = static_cast<unsigned int>( DetectorNumbers::HalfLayers );
    constexpr const unsigned int nStavesPerLayer = static_cast<unsigned int>( DetectorNumbers::Staves );
    constexpr const unsigned int nStavesTotal =
        static_cast<unsigned int>( DetectorNumbers::Staves ) * static_cast<unsigned int>( DetectorNumbers::HalfLayers );

    constexpr const unsigned int nFaces      = static_cast<unsigned int>( DetectorNumbers::Faces );
    constexpr const unsigned int nModules    = static_cast<unsigned int>( DetectorNumbers::Modules );
    constexpr const unsigned int nSubSectors = static_cast<unsigned int>( DetectorNumbers::SubSectors );

    constexpr const unsigned int MaxNumberOfSectors =
        static_cast<unsigned int>( DetectorNumbers::Sides ) * static_cast<unsigned int>( DetectorNumbers::HalfLayers ) *
        static_cast<unsigned int>( DetectorNumbers::Staves ) * static_cast<unsigned int>( DetectorNumbers::Faces ) *
        static_cast<unsigned int>( DetectorNumbers::Modules ) *
        static_cast<unsigned int>( DetectorNumbers::SubSectors );

  } // namespace UT

} // namespace LHCb::Detector
