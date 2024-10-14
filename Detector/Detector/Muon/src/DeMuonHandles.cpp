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

// A bit of magic you do not really want to know about....
// This has to be kept in its own compilation unit

// Framework include files
#include "DD4hep/detail/Handle.inl"
#include "Detector/Muon/DeMuon.h"

DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeMuonObject, LHCb::Detector::detail::DeIOVObject,
                                   ConditionObject );
DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeMuonStationObject, LHCb::Detector::detail::DeIOVObject,
                                   ConditionObject );
DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeMuonSideObject, LHCb::Detector::detail::DeIOVObject,
                                   ConditionObject );
DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeMuonRegionObject, LHCb::Detector::detail::DeIOVObject,
                                   ConditionObject );
DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeMuonChamberObject, LHCb::Detector::detail::DeIOVObject,
                                   ConditionObject );
DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeMuonGasGapObject, LHCb::Detector::detail::DeIOVObject,
                                   ConditionObject );
