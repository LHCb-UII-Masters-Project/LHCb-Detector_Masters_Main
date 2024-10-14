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

// Framework include files
#include "DD4hep/detail/Handle.inl"
#include "Detector/TV/DeTV.h"

DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeTVSensorObject, LHCb::Detector::detail::DeIOVObject,
                                   ConditionObject );
DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeTVLadderObject, LHCb::Detector::detail::DeIOVObject,
                                   ConditionObject );
DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeTVModuleObject, LHCb::Detector::detail::DeIOVObject,
                                   ConditionObject );
DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeTVSideObject, LHCb::Detector::detail::DeIOVObject,
                                   ConditionObject );
DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeTVObject, LHCb::Detector::detail::DeIOVObject,
                                   ConditionObject );
