/*****************************************************************************\
* (c) Copyright 2023 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#include "DD4hep/detail/Handle.inl"

#include "Detector/MP/DeMP.h"

DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeMPObject, LHCb::Detector::detail::DeIOVObject,
                                   ConditionObject );
