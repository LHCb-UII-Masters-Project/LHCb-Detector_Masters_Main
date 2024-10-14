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

#include "Core/DeIOV.h"

#include "DD4hep/detail/Handle.inl"

// This is some magic you do not really want to know about....

DD4HEP_INSTANTIATE_HANDLE_UNNAMED( LHCb::Detector::detail::DeIOVObject, dd4hep::detail::ConditionObject );
