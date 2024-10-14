//==========================================================================
//  LHCb Rich Detector geometry utility class  using DD4HEP
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
//
// Author     : Sajan Easo
// Date       : 2020-11-10
//
//==========================================================================

#pragma once

#include "Core/UpgradeTags.h"
#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/OpticalSurfaces.h"
#include "DD4hep/Printout.h"
#include "Detector/Rich/Types.h"
#include "Detector/Rich/Utilities.h"
#include "XML/Utilities.h"
#include <string>
#include <type_traits>

enum class RichSurfPropType : int { Refl, Effic, Trans };

class RichSurfaceUtil {
public:
  using TabData = LHCb::Detector::XYTable;

  void InitRichGeneralSurfNames();
  void InitRich1SurfNames();
  void InitRich2SurfNames();

  void    PrintRichSurfProperty( const dd4hep::OpticalSurface& aSurf, RichSurfPropType aSprop );
  TabData RetrieveRichSurfProperty( const dd4hep::OpticalSurface& aSurf, RichSurfPropType aSprop ) const;

  template <typename S>
  constexpr auto toInTs( S f ) const noexcept {
    return static_cast<std::underlying_type_t<S>>( f );
  }

  static RichSurfaceUtil* getRichSurfaceUtilInstance();

  void setRichSurfaceUtilDebug( bool adfl ) { m_RichSurfaceUtilDebug = adfl; }

  const auto& Rich1Mirror1SurfaceName( int aIndex ) const { return m_Rich1Mirror1SurfaceNames[aIndex]; }
  const auto& Rich1Mirror2SurfaceName( int bIndex ) const { return m_Rich1Mirror2SurfaceNames[bIndex]; }

  const auto& Rich1Mirror1BorderSurfaceName( int aBIndex ) const { return m_Rich1Mirror1BorderSurfaceNames[aBIndex]; }
  const auto& Rich1Mirror2BorderSurfaceName( int bBIndex ) const { return m_Rich1Mirror2BorderSurfaceNames[bBIndex]; }

  const auto& RichSurfacePropTypeName( RichSurfPropType aS ) const { return m_RichSurfacePropTypeNames[toInTs( aS )]; }

  const auto& Rich1GasQuartzWindowSurfaceName() const { return m_Rich1GasQuartzWindowSurfaceName; }
  const auto& Rich1GasQuartzWindowBorderSurfaceName( int aSIndex ) const {
    return m_Rich1GasQuartzWindowBorderSurfaceNames[aSIndex];
  }
  const auto& Rich1GasQuartzWindowBorderBackSurfaceName( int bSIndex ) const {
    return m_Rich1GasQuartzWindowBorderBackSurfaceNames[bSIndex];
  }
  const auto& RichMapmtQWSurfaceName() const { return m_RichMapmtQWSurfaceName; }
  const auto& RichMapmtPCSurfaceName() const { return m_RichMapmtPCSurfaceName; }
  const auto& RichMapmtMTSurfaceName() const { return m_RichMapmtMTSurfaceName; }
  const auto& RichMapmtAnodeSurfaceName() const { return m_RichMapmtAnodeSurfaceName; }

  const auto& Rich2Mirror1SurfaceName( int aInd ) const { return m_Rich2Mirror1SurfaceNames[aInd]; }
  const auto& Rich2Mirror2SurfaceName( int bInd ) const { return m_Rich2Mirror2SurfaceNames[bInd]; }
  const auto& Rich2Mirror1BorderSurfaceName( int aIndex ) const { return m_Rich2Mirror1BorderSurfaceNames[aIndex]; }
  const auto& Rich2Mirror2BorderSurfaceName( int bIndex ) const { return m_Rich2Mirror2BorderSurfaceNames[bIndex]; }

  const auto& Rich2GasQuartzWindowSurfaceName() const { return m_Rich2GasQuartzWindowSurfaceName; }

  const auto& Rich2GasQuartzWindowBorderSurfaceName( int aI ) const {
    return m_Rich2GasQuartzWindowBorderSurfaceNames[aI];
  }
  const auto& Rich2GasQuartzWindowBorderBackSurfaceName( int aI ) const {
    return m_Rich2GasQuartzWindowBorderBackSurfaceNames[aI];
  }

  const auto& Rich1Mirror1NominalSurfaceName() const { return m_Rich1Mirror1NominalSurfaceName; }
  const auto& Rich1Mirror2NominalSurfaceName() const { return m_Rich1Mirror2NominalSurfaceName; }
  const auto& Rich2Mirror1NominalSurfaceName() const { return m_Rich2Mirror1NominalSurfaceName; }
  const auto& Rich2Mirror2NominalSurfaceName() const { return m_Rich2Mirror2NominalSurfaceName; }

  RichSurfaceUtil();

private:
  bool m_RichSurfaceUtilDebug{true};

  std::vector<std::string> m_RichSurfacePropTypeNames;

  std::vector<std::string> m_Rich1Mirror1SurfaceNames;
  std::vector<std::string> m_Rich1Mirror2SurfaceNames;

  std::vector<std::string> m_Rich1Mirror1BorderSurfaceNames;
  std::vector<std::string> m_Rich1Mirror2BorderSurfaceNames;

  std::string m_Rich1Mirror1NominalSurfaceName;
  std::string m_Rich1Mirror2NominalSurfaceName;

  std::string              m_Rich1GasQuartzWindowSurfaceName;
  std::vector<std::string> m_Rich1GasQuartzWindowBorderSurfaceNames;
  std::vector<std::string> m_Rich1GasQuartzWindowBorderBackSurfaceNames;

  std::vector<std::string> m_Rich2Mirror1SurfaceNames;
  std::vector<std::string> m_Rich2Mirror2SurfaceNames;

  std::vector<std::string> m_Rich2Mirror1BorderSurfaceNames;
  std::vector<std::string> m_Rich2Mirror2BorderSurfaceNames;

  std::string m_Rich2Mirror1NominalSurfaceName;
  std::string m_Rich2Mirror2NominalSurfaceName;

  std::string              m_Rich2GasQuartzWindowSurfaceName;
  std::vector<std::string> m_Rich2GasQuartzWindowBorderSurfaceNames;
  std::vector<std::string> m_Rich2GasQuartzWindowBorderBackSurfaceNames;

  std::string m_RichMapmtQWSurfaceName;
  std::string m_RichMapmtPCSurfaceName;
  std::string m_RichMapmtMTSurfaceName;
  std::string m_RichMapmtAnodeSurfaceName;

  int m_NumRich1Mirror2SurfaceNames{};
  int m_NumRich1MirrorQuad{};
  int m_NumRichSurfProp{};
};
