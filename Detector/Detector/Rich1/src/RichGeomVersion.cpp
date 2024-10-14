//==========================================================================
//  LHCb Rich Detector geometry utility class  using DD4HEP
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
//
// Author     : Sajan Easo
// Date       : 2023-08-22
//
//==========================================================================
#include "Detector/Rich1/RichGeomVersion.h"
#include <cassert>
#include <memory>
#include <mutex>

#include "DD4hep/Printout.h"

// Initializing a set of values. These are then set to the approprate values
// from Rich*_geo.cpp.
//==============================================================================//
RichGeomVersion::RichGeomVersion()
    : m_Current_RichGeomVersion( RichGeomLabel::Rich_RUN3_v2 ), m_RichGeomVersionDefined( false ) {}
//==============================================================================//
void RichGeomVersion::InitRichGeomVersion( RichGeomLabel aRG ) {
  if ( !m_RichGeomVersionDefined ) {
    setCurrent_RichGeomVersion( aRG );
    m_RichGeomVersionDefined = true;
  } else {
    // warning as attempt is made to set version multiple times in the same detector
    dd4hep::printout( dd4hep::ALWAYS,
                      "RichGeomVersion : ", "Warning  : Multiple attempts to set Geometry version New %d and Old %d  ",
                      aRG, m_Current_RichGeomVersion );
  }
}
//================================================================================//
