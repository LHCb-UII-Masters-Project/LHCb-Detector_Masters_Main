//==========================================================================
//  LHCb Rich Detector geometry transform Auxiliary class  using DD4HEP
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
//
// Author     : Sajan Easo
// Date       : 2020-12-01
//
//==========================================================================

#pragma once

#include "Core/UpgradeTags.h"
#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/Printout.h"
#include "Detector/Rich/Utilities.h"
#include "XML/Utilities.h"

class RichGeoTransAux {
public:
  size_t loadRichTransforms( xml_h element, const std::string& tag, const std::string& RichDetectorName );

  static RichGeoTransAux* getRichGeoTransAuxInstance();

  void setRichGeoTransDebug( bool aVa ) { m_RichGeoTransDebug = aVa; }

private:
  auto getDD4hepPrintLevel() const noexcept { return m_RichGeoTransDebug ? dd4hep::DEBUG : dd4hep::VERBOSE; }

private:
  bool m_RichGeoTransDebug{false};
};
