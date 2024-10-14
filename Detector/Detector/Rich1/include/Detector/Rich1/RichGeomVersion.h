//==========================================================================
//  LHCb Rich Detector geometry version within DD4HEP framework
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
//
// Author     : Sajan Easo
// Date       : 2023-08-22
//
//==========================================================================//
#pragma once

#include <type_traits>
#include <vector>

//===========================================================================//

enum class RichGeomLabel : int { Rich_RUN3_v0, Rich_RUN3_v1, Rich_RUN3_v2, Rich_NumberOfGeomVersions };

class RichGeomVersion {

private:
public:
  void setCurrent_RichGeomVersion( RichGeomLabel aVersion ) { m_Current_RichGeomVersion = aVersion; }

  RichGeomLabel Current_RichGeomVersion() const { return m_Current_RichGeomVersion; }

  void InitRichGeomVersion( RichGeomLabel aRG );

  RichGeomVersion();

private:
  RichGeomLabel m_Current_RichGeomVersion;
  bool m_RichGeomVersionDefined; // safety flag to avoid defining the version multiple times for the same detector.
};
