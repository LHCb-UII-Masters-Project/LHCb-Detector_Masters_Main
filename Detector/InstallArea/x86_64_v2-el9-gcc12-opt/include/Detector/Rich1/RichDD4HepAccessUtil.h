//==========================================================================
//  LHCb Rich Detector utility class for accessing DD4HEP
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
//
// Author     : Sajan Easo
// Date       : 2021-10-03
//
//====================================================================//

#pragma once

#include "Core/UpgradeTags.h"
#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/OpticalSurfaces.h"
#include "DD4hep/Printout.h"
#include "DD4hep/PropertyTable.h"
#include "Detector/Rich/Utilities.h"
#include "Detector/Rich1/RichMatOPD.h"
#include "XML/Utilities.h"
#include <type_traits>

class RichDD4HepAccessUtil {
public:
  dd4hep::Detector& GetCurDD4HepDetectorInstance();

  TGDMLMatrix* GetGDMLTable( const std::string& aTableName );

  void PrintAMatTable( const std::string& aTableName );

  dd4hep::Material::Property GetMatTabProp( const RichMatNameWOP& aMatNameW, const RichMatPropType& aPropType );

  void PrintRichMatTabProp( const RichMatNameWOP& aMatNameW, const RichMatPropType& aPropType );

  dd4hep::OpticalSurfaceManager GetOpticalSurfManager();
  dd4hep::OpticalSurface        GetAnOpticalSurface( const std::string& aSurfName );

  static RichDD4HepAccessUtil* getRichDD4HepAccessUtilInstance();
};
