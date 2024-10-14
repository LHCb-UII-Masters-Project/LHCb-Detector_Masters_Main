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

namespace UP {

  static const unsigned int NModules          = 52;
  static const unsigned int NSensorsPerModule = 4;
  static const unsigned int NSensors          = NModules * NSensorsPerModule;
  static const unsigned int NChipsPerSensor   = 3;
  static const unsigned int NRows             = 256;
  static const unsigned int NColumns          = 256;
  static const unsigned int NSensorColumns    = NColumns * NChipsPerSensor;
  static const unsigned int NPixelsPerSensor  = NSensorColumns * NRows;

  static const double Pitch = 0.055;

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

} // namespace UP
