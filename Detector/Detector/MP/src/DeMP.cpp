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
#include "Detector/MP/DeMP.h"
#include "Core/DeIOV.h"
#include "Core/Units.h"
#include "Core/yaml_converters.h"
#include "DD4hep/Detector.h"
#include "DD4hep/Printout.h"
#include <TGeoNavigator.h>
#include <array>
#include <iostream>
#include <regex>
#include <string>

LHCb::Detector::detail::DeMPObject::DeMPObject( dd4hep::DetElement de, dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeIOVObject( de, ctxt ) {}
