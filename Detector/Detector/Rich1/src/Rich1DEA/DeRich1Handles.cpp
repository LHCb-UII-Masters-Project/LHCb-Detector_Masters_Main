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

// A bit of magic (from Markus , Ben et.al. ) you do not really want to know about....

// Framework include files
#include "DD4hep/detail/Handle.inl"
#include "Detector/Rich1/DetElemAccess/DeRich1.h"
#include "Detector/Rich1/DetElemAccess/DeRich1BeamPipe.h"
#include "Detector/Rich1/DetElemAccess/DeRich1Mapmt.h"
#include "Detector/Rich1/DetElemAccess/DeRich1Mirror1.h"
#include "Detector/Rich1/DetElemAccess/DeRich1Mirror2.h"
#include "Detector/Rich1/DetElemAccess/DeRich1PDModule.h"
#include "Detector/Rich1/DetElemAccess/DeRich1PhDetPanel.h"
#include "Detector/Rich1/DetElemAccess/DeRich1RadiatorGas.h"

DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeRich1Mirror1SegObject, LHCb::Detector::detail::DeIOVObject,
                                   ConditionObject );
DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeRich1Mirror1EnvObject, LHCb::Detector::detail::DeIOVObject,
                                   ConditionObject );
DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeRich1Mirror2SegObject, LHCb::Detector::detail::DeIOVObject,
                                   ConditionObject );
DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeRich1Mirror2MasterObject,
                                   LHCb::Detector::detail::DeIOVObject, ConditionObject );

DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeRich1BeamPipeObject, LHCb::Detector::detail::DeIOVObject,
                                   ConditionObject );

DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeRich1RadiatorGasObject,
                                   LHCb::Detector::detail::DeIOVObject, ConditionObject );

DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeRich1PhDetPanelObject, LHCb::Detector::detail::DeIOVObject,
                                   ConditionObject );

DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeRich1StdPDModuleObject,
                                   LHCb::Detector::detail::DeIOVObject, ConditionObject );

DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeRich1EdgePDModuleObject,
                                   LHCb::Detector::detail::DeIOVObject, ConditionObject );

DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeRich1OuterCornerPDModuleObject,
                                   LHCb::Detector::detail::DeIOVObject, ConditionObject );

DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeRich1MapmtObject, LHCb::Detector::detail::DeIOVObject,
                                   ConditionObject );

DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeRich1Object, LHCb::Detector::detail::DeIOVObject,
                                   ConditionObject );
