
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

namespace LHCb::Detector::Muon {

  static const unsigned int DAQHelper_maxTell40Number    = 22;
  static const unsigned int DAQHelper_maxNODENumber      = 144;
  static const unsigned int DAQHelper_linkNumber         = 24;
  static const unsigned int DAQHelper_frameNumber        = 4;
  static const unsigned int DAQHelper_maxTell40PCINumber = 2;
  static const unsigned int DAQHelper_ODEFrameSize       = 48;
  static const unsigned int DAQHelper_maxTell40PCINumber_linkNumber_ODEFrameSize =
      DAQHelper_maxTell40PCINumber * DAQHelper_linkNumber * DAQHelper_ODEFrameSize;
  static const unsigned int DAQHelper_linkNumber_ODEFrameSize = DAQHelper_linkNumber * DAQHelper_ODEFrameSize;

} // namespace LHCb::Detector::Muon
