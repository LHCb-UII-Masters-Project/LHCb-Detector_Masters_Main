/*****************************************************************************\
* (c) Copyright 2021 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#include "Core/ConditionHelper.h"
#include "Core/ConditionsRepository.h"
#include "Core/Keys.h"
#include "DD4hep/Conditions.h"
#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/ExtensionEntry.h"
#include "DD4hep/Printout.h"
#include "DDCond/ConditionsSlice.h"
#include "Detector/LHCb/DeLHCb.h"

// Fake version  of the plugin kept for compatibility with old XML descriptions
// It now does nothing as the DeLHCb is setup by the DetectorDataService anyway
static long create_conditions_recipes( dd4hep::Detector& /*description*/, xml_h /* e */ ) { return 1; }
DECLARE_XML_DOC_READER( LHCb_LHCb_cond, create_conditions_recipes )
