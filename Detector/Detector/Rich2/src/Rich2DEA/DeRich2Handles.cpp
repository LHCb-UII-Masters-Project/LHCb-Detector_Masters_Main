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
#include "Detector/Rich2/DetElemAccess/DeRich2.h"
#include "Detector/Rich2/DetElemAccess/DeRich2BeamPipe.h"
#include "Detector/Rich2/DetElemAccess/DeRich2Mapmt.h"
#include "Detector/Rich2/DetElemAccess/DeRich2PDModule.h"
#include "Detector/Rich2/DetElemAccess/DeRich2PhDetPanel.h"
#include "Detector/Rich2/DetElemAccess/DeRich2RadiatorGas.h"
#include "Detector/Rich2/DetElemAccess/DeRich2SecMirror.h"
#include "Detector/Rich2/DetElemAccess/DeRich2SphMirror.h"

DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeRich2RadiatorGasObject,
                                   LHCb::Detector::detail::DeIOVObject, ConditionObject );

DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeRich2SphMirrorSegObject,
                                   LHCb::Detector::detail::DeIOVObject, ConditionObject );

DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeRich2SphMirrorMasterObject,
                                   LHCb::Detector::detail::DeIOVObject, ConditionObject );

DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeRich2SecMirrorSegObject,
                                   LHCb::Detector::detail::DeIOVObject, ConditionObject );

DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeRich2SecMirrorMasterObject,
                                   LHCb::Detector::detail::DeIOVObject, ConditionObject );

DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeRich2StdMapmtObject, LHCb::Detector::detail::DeIOVObject,
                                   ConditionObject );

DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeRich2GrandMapmtObject, LHCb::Detector::detail::DeIOVObject,
                                   ConditionObject );

DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeRich2StdPDModuleObject,
                                   LHCb::Detector::detail::DeIOVObject, ConditionObject );

DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeRich2GrandPDModuleObject,
                                   LHCb::Detector::detail::DeIOVObject, ConditionObject );

DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeRich2PhDetPanelObject, LHCb::Detector::detail::DeIOVObject,
                                   ConditionObject );

DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeRich2BeamPipeObject, LHCb::Detector::detail::DeIOVObject,
                                   ConditionObject );

DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeRich2Object, LHCb::Detector::detail::DeIOVObject,
                                   ConditionObject );
