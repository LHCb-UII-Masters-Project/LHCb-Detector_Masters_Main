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
#include "Detector/UT/DeUT.h"

DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::UT::detail::DeUTObject, LHCb::Detector::detail::DeIOVObject,
                                   ConditionObject );
DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::UT::detail::DeUTSideObject, LHCb::Detector::detail::DeIOVObject,
                                   ConditionObject );
DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::UT::detail::DeUTLayerObject, LHCb::Detector::detail::DeIOVObject,
                                   ConditionObject );
DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::UT::detail::DeUTStaveObject, LHCb::Detector::detail::DeIOVObject,
                                   ConditionObject );
DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::UT::detail::DeUTFaceObject, LHCb::Detector::detail::DeIOVObject,
                                   ConditionObject );
DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::UT::detail::DeUTModuleObject, LHCb::Detector::detail::DeIOVObject,
                                   ConditionObject );
DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::UT::detail::DeUTSectorObject, LHCb::Detector::detail::DeIOVObject,
                                   ConditionObject );
DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::UT::detail::DeUTSensorObject, LHCb::Detector::detail::DeIOVObject,
                                   ConditionObject );
