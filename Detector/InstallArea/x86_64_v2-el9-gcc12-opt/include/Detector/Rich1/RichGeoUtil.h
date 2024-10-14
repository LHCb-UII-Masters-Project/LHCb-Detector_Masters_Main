//==========================================================================
//  LHCb Rich Detector geometry utility class  using DD4HEP
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
//
// Author     : Sajan Easo
// Date       : 2020-08-03
//
//==========================================================================

#pragma once

#include "Core/DeIOV.h"
#include "Core/UpgradeTags.h"
#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/Printout.h"
#include "Detector/Rich/Types.h"
#include "Detector/Rich/Utilities.h"
#include "Detector/Rich1/RichGeomVersion.h"
#include "XML/Utilities.h"
#include <type_traits>
#include <vector>

enum class RichGeomTransformLabel : int {
  pvRich1SubMasterTN,
  pvRich1PhDetSupFrameH0TN,
  pvRich1PhDetSupFrameH1TN,
  pvRich1GQuartzWH0TN,
  pvRich1GQuartzWH1TN,
  pvRich1MagShH0TN,
  pvRich1MagShH1TN,
  pvRich1Mirror1MasterTN,
  pvRich1Mirror2MasterTopTN,
  pvRich1Mirror2MasterBotTN,
  pvRich1Mirror2SupportTopTN,
  pvRich1Mirror2SupportBotTN,
  pvRich1Mirror2Q0MasterTN,
  pvRich1Mirror2Q1MasterTN,
  pvRich1Mirror2Q2MasterTN,
  pvRich1Mirror2Q3MasterTN,
  pvRich1MgsOuterH0TN,
  pvRich1MgsOuterH1TN,
  pvRich1MgsUpstrH0TN,
  pvRich1MgsUpstrH1TN,
  pvRich1MgsSideQ0TN,
  pvRich1MgsSideQ1TN,
  pvRich1MgsSideQ2TN,
  pvRich1MgsSideQ3TN,
  pvRich1MgsUpstrCornerH0TN,
  pvRich1MgsUpstrCornerH1TN,
  pvRich1MgsTeethQ0TN,
  pvRich1MgsTeethQ1TN,
  pvRich1MgsTeethQ2TN,
  pvRich1MgsTeethQ3TN,
  pvRich1MgsMidH0TN,
  pvRich1MgsMidH1TN,
  pvRich1MgsDnstrUTH0TN,
  pvRich1MgsDnstrUTH1TN,
  pvRich1ExitWallMasterTN,
  pvRich1ExitDiaphramCentralUpsTN,
  pvRich1ExitDiaphramCentralDnsTN,
  pvRich1ExitDiaphramMiddleTN,
  pvRich1ExitDiaphramPeripheryTN,
  pvRich1ExitDiaphramCentralPlugUpsTN,
  pvRich1ExitDiaphramCentralPlugDnsTN,
  pvRich1ExitG10UpstrTN,
  pvRich1ExitG10DnstrTN,
  pvRich1ExitPMITN,
  pvUX851InRich1SubMasterTN,
  pvUX851InRich1BeforeSubMTN,
  pvUX851InRich1AfterSubMTN
};

/// Utility class for build the Rich detectors of LHCb
class RichGeoUtil {

private:
  template <typename G>
  constexpr auto toInTe( const G g ) const noexcept {
    return static_cast<std::underlying_type_t<G>>( g );
  }

public:
  void                       build_RichTransforms( const xml_h& tr_handle );
  void                       InitRichTransformsAndNames();
  void                       InitRich1MirrorTransformsAndNames();
  void                       InitRich1MirrorCaFiCyl();
  void                       InitRich1FillTN( const Rich::Detector::NamedVector<int>& aRich1CompPvI );
  const dd4hep::Transform3D& getRichGeomTransform( const std::string& aName );
  void                       TestRichGeomParameters();
  bool                       IsInRich1Mirror1PhysList( const std::string& aPhysName );
  bool                       IsInRich1Mirror1TransformList( const std::string& aTransformName );
  bool                       IsInRich1Mirror2PhysList( const std::string& aPhysName );
  bool                       IsInRich1Mirror2TransformList( const std::string& aTransformName );
  bool                       IsInRichTransformList( const std::string& aTransformName );

  const auto& getRichTransformName( RichGeomTransformLabel aLabel ) { return m_RichTransformNames[toInTe( aLabel )]; }
  const auto& getRichPhysVolName( RichGeomTransformLabel aLabel ) { return m_RichPhysVolNames[toInTe( aLabel )]; }
  auto getPhysVolCopyNumber( RichGeomTransformLabel aLabel ) { return m_RichPhysVolCopyNumbers[toInTe( aLabel )]; }

  static RichGeoUtil* getRichGeoUtilInstance();

  void setDebugLevel( bool aLevel ) { m_RichGeomUtilDebug = aLevel; }

  const auto& Rich1Mirror1TransformName( int aType, int aQ ) const { return m_Rich1Mirror1TransformNames[aType][aQ]; }
  const auto& Rich1Mirror1PhysVolName( int aType, int aQ ) const { return m_Rich1Mirror1PhysVolNames[aType][aQ]; }
  auto        Rich1Mirror1CopyNumber( int aType, int aQ ) const { return m_Rich1Mirror1CopyNumbers[aType][aQ]; }
  const auto& Rich1Mirror1LogVolName( int aType, int aQ ) const { return m_Rich1Mirror1LogVolNames[aType][aQ]; }
  const auto& Rich1Mirror1DetName( int aType, int aQ ) const { return m_Rich1Mirror1DetNames[aType][aQ]; }
  auto Rich1Mirror1FullDetName( int aType, int aQ ) const { return ( m_Rich1Mirror1DetNames[aType][aQ] + "Det" ); }

  // const auto& Rich1Mirror1Det (int aType, int aQ ) const {return m_Rich1Mirror1Det [aType][aQ]; }
  // void setRich1Mirror1Det (int aType, int aQ, dd4hep::DetElement aDet ) {
  //   m_Rich1Mirror1Det [aType][aQ] = aDet ; }

  const auto& Rich1Mirror2TransformName( int aSeg ) const { return m_Rich1Mirror2TransformNames[aSeg]; }
  const auto& Rich1Mirror2PhysVolName( int aSeg ) const { return m_Rich1Mirror2PhysVolNames[aSeg]; }
  auto        Rich1Mirror2CopyNumber( int aSeg ) const { return m_Rich1Mirror2CopyNumbers[aSeg]; }
  const auto& Rich1Mirror2LogVolName( int aSeg ) const { return m_Rich1Mirror2LogVolNames[aSeg]; }
  const auto& Rich1Mirror2DetName( int aSeg ) const { return m_Rich1Mirror2DetNames[aSeg]; }
  auto        Rich1Mirror2FullDetName( int aSeg ) const { return ( m_Rich1Mirror2DetNames[aSeg] + "Det" ); }

  auto Rich1NumMirror1Seg() const { return m_Rich1NumMirror1Seg; }

  auto Rich1NumMirror1VolTypes() const { return m_Rich1NumMirror1VolTypes; }
  auto Rich1NumMirror1DetTypes() const { return m_Rich1NumMirror1DetTypes; }

  const auto& Rich1Mirror1QuadrantName( int iQ ) { return m_Rich1Mirror1QuadrantNames[iQ]; }

  const auto& Rich1Mirror1QuadrantNames() const { return m_Rich1Mirror1QuadrantNames; }
  const auto& Rich1Mirror2SegNames() const { return m_Rich1Mirror2SegNames; }

  const auto& Rich1Mirror1MasterLVName() const { return m_Rich1Mirror1MasterLVName; }
  const auto& Rich1Mirror1CaFiLVName() const { return m_Rich1Mirror1CaFiLVName; }
  const auto& Rich1Mirror2MasterLVName( int aH ) const { return m_Rich1Mirror2MasterLVNames[aH]; }
  const auto& Rich1Mirror2SupportLVName( int aG ) const { return m_Rich1Mirror2SupportLVNames[aG]; }
  const auto& Rich1Mirror2MasterDetName( int aH ) const { return m_Rich1Mirror2MasterDetNames[aH]; }

  const auto& Rich1Mirror2QuadrantMasterLVName( int aQ ) const { return m_Rich1Mirror2QuadrantMasterLVNames[aQ]; }
  const auto& Rich1Mirror2QuadrantMasterDetName( int aD ) const { return m_Rich1Mirror2QuadrantMasterDetNames[aD]; }
  auto        getR1M2QuadrantMasterTN( int aQu ) const {
    if ( aQu == 0 ) {
      return RichGeomTransformLabel::pvRich1Mirror2Q0MasterTN;
    } else if ( aQu == 1 ) {
      return RichGeomTransformLabel::pvRich1Mirror2Q1MasterTN;
    } else if ( aQu == 2 ) {
      return RichGeomTransformLabel::pvRich1Mirror2Q2MasterTN;
    } else {
      return RichGeomTransformLabel::pvRich1Mirror2Q3MasterTN;
    }
  }

  auto Rich1NumMirror2Seg() const { return m_Rich1NumMirror2Seg; }
  auto Rich1NumMirror2SegInAHalf() const { return m_Rich1NumMirror2SegInAHalf; }
  auto Rich1NumMirror2SegInAQuad() const { return m_Rich1NumMirror2SegInAQuad; }

  auto Rich1NumMirror2Systems() const { return m_Rich1NumMirror2Systems; }
  auto Rich1NumMirror2Quadrants() const { return m_Rich1NumMirror2Quadrants; }
  auto Rich1NumMirror2QuadrantsInHalf() const { return m_Rich1NumMirror2QuadrantsInHalf; }

  auto Rich2Mirror2QuadInM2Sys( int aQu ) const { return m_Rich2Mirror2QuadInM2Sys[aQu]; }

  const auto& Rich1Mirror1CaFiCylinderPhysVolName( int iQ, int iC ) const {
    return m_Rich1Mirror1CaFiCylinderPhysVolNames[iQ][iC];
  }

  auto Rich1Mirror1CaFiCylinderCopyNumber( int iQ, int iC ) const {
    return m_Rich1Mirror1CaFiCylinderCopyNumbers[iQ][iC];
  }

  const auto& Rich1Mirror1CaFiCylinderTransform( int iQ, int iC ) const {
    return m_Rich1Mirror1CaFiCylinderTransforms[iQ][iC];
  }

  auto Rich1Mirror1NumCaFiCylAlongX() const { return m_Rich1Mirror1NumCaFiCylAlongX; }
  auto Rich1Mirror1NumCaFiCylAlongY() const { return m_Rich1Mirror1NumCaFiCylAlongY; }
  auto Rich1Mirror1NumCaFiCylInASeg() const { return m_Rich1Mirror1NumCaFiCylInASeg; }

  const auto& Rich1SubMasterLVName() const { return m_Rich1SubMasterLVName; }
  const auto& Rich1MasterLVName() const { return m_Rich1MasterLVName; }

  const auto& Rich1MasterDetName() const { return m_Rich1MasterDetName; }
  const auto& Rich1SubMasterDetName() const { return m_Rich1SubMasterDetName; }
  auto        Rich1SubMasterFullDetName() const { return ( m_Rich1SubMasterDetName + "Det" ); }
  // const auto& Rich1SubMasterDet() const { return m_Rich1SubMasterDet; }
  // void setRich1SubMasterDet(dd4hep::DetElement aDet ) {m_Rich1SubMasterDet = aDet ;}

  const auto& Rich1MagShLVName( int aT ) const { return m_Rich1MagShLVNames[aT]; }

  const auto& Rich1PhDetSupFrameLVName( int aB ) const { return m_Rich1PhDetSupFrameLVNames[aB]; }

  const auto& Rich1PhDetSupFrameDetName( int aB ) const { return m_Rich1PhDetSupFrameDetNames[aB]; }
  auto        Rich1PhDetSupFrameFullDetName( int aB ) const { return ( m_Rich1PhDetSupFrameDetNames[aB] + "Det" ); }

  const auto& Rich1GasQuartzWLVName() const { return m_Rich1GasQuartzWLVName; }

  auto        Rich1NumSides() const { return m_Rich1NumSides; }
  auto        Rich1NumBeamPipeSeg() const { return m_Rich1NumBeamPipeSeg; }
  const auto& Rich1BeamPipeSegmentLVName( int aB ) const { return m_Rich1BeamPipeSegmentLVNames[aB]; }
  const auto& Rich1BeamPipeInSubMDetName() const { return m_Rich1BeamPipeInSubMDetName; }

  const auto& Rich1GasQuartzWMatName() const { return m_Rich1GasQuartzWMatName; }

  const auto& RichMasterMatName() const { return m_RichMasterMatName; }
  const auto& Rich1NitrogenMatName() const { return m_Rich1NitrogenMatName; }
  const auto& Rich1C4F10MatName() const { return m_Rich1C4F10MatName; }
  const auto& Rich1RadiatorGasMatName() const { return m_Rich1RadiatorGasMatName; }
  void        setRich1GeometryVersion( RichGeomLabel aRG ) { m_Cur_Rich1GeomVersion = aRG; }
  const auto& Cur_Rich1GeomVersion() { return m_Cur_Rich1GeomVersion; }

  using ListDE = std::vector<std::pair<dd4hep::DetElement, dd4hep::cond::ConditionUpdateContext&>>;
  ListDE Rich1Mirror2ListDE( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt, int aSide );

  // RichGeoUtil() { InitRichTransformsAndNames(); }
  RichGeoUtil() {}

private:
  int m_RichMaxNumGeomTransforms{0};

  std::vector<std::string>                       m_RichTransformNames;
  Rich::Detector::StringMap<dd4hep::Transform3D> m_RichGeomTransforms;
  std::vector<std::string>                       m_RichPhysVolNames;
  std::vector<int>                               m_RichPhysVolCopyNumbers;

  std::string m_Rich1Mirror1MasterLVName;
  std::string m_Rich1Mirror1CaFiLVName;

  std::vector<std::vector<std::string>> m_Rich1Mirror1TransformNames;
  std::vector<std::vector<std::string>> m_Rich1Mirror1PhysVolNames;
  std::vector<std::vector<int>>         m_Rich1Mirror1CopyNumbers;
  std::vector<std::vector<std::string>> m_Rich1Mirror1LogVolNames;
  std::vector<std::vector<std::string>> m_Rich1Mirror1DetNames;
  // std::vector<std::vector<dd4hep::DetElement>> m_Rich1Mirror1Det;

  std::vector<std::string> m_Rich1Mirror2MasterLVNames;
  std::vector<std::string> m_Rich1Mirror2SupportLVNames;
  std::vector<std::string> m_Rich1Mirror2MasterDetNames;
  std::vector<std::string> m_Rich1Mirror2QuadrantMasterLVNames;
  std::vector<std::string> m_Rich1Mirror2QuadrantMasterDetNames;

  std::vector<std::string> m_Rich1Mirror2TransformNames;
  std::vector<std::string> m_Rich1Mirror2PhysVolNames;
  std::vector<int>         m_Rich1Mirror2CopyNumbers;
  std::vector<std::string> m_Rich1Mirror2LogVolNames;
  std::vector<std::string> m_Rich1Mirror2DetNames;

  std::vector<std::string> m_Rich1Mirror1QuadrantNames;
  std::vector<std::string> m_Rich1Mirror2SegNames;
  std::vector<std::string> m_Rich1Mirror1VolTypeNames;

  int m_Rich1NumMirror1Seg;             // Number of Mirror segments in Rich1 spherical   mirror system
  int m_Rich1NumMirror2Seg;             // Number of Mirror segments in Rich1 almostFlat  mirror system
  int m_Rich1NumMirror2SegInAHalf;      // Number of Mirror segments in top (or bottom) half in Rich1 almostFlat mirror
                                        // system
  int m_Rich1NumMirror2SegInAQuad;      // Number of Mirror segments in a quadrant in top (or bottom) half
                                        // in Rich1 almostFlat mirror  system
  int m_Rich1NumMirror2Systems;         // For the top and bottom almostFlat mirror system
  int m_Rich1NumMirror2Quadrants;       // Number of quadrants for the Rich1 mirror2 system.
  int m_Rich1NumMirror2QuadrantsInHalf; // Number of quadrants in top or bottom half of the Rich1 mirror2 system.
  int m_Rich1NumMirror1VolTypes;        // Number of different Volume types in Rich1Mirror1 system.
  int m_Rich1NumMirror1DetTypes;        // Number of different det element types in  Rich1Mirror1 system.
  std::vector<int> m_Rich2Mirror2QuadInM2Sys; // Indicates a quadrant in top (0) or bottom (1) half of the mirror2
                                              // system
  int m_Rich1Mirror1NumCaFiCylAlongX;         // total number of CaFi Cyl along X in a quadrant segment.
  int m_Rich1Mirror1NumCaFiCylAlongY;         // total number of CaFi Cyl along Y in a quadrant segment.
  int m_Rich1Mirror1NumCaFiCylInASeg;         // total number of CaFi Cyl in a quadrant segment.

  std::vector<std::vector<std::string>> m_Rich1Mirror1CaFiCylinderPhysVolNames;
  std::vector<std::vector<int>>         m_Rich1Mirror1CaFiCylinderCopyNumbers;

  std::vector<std::vector<dd4hep::Transform3D>> m_Rich1Mirror1CaFiCylinderTransforms;

  int                      m_Rich1NumSides;       // Refering to the two sides Rich1 Top and Rich1 Bottom
  int                      m_Rich1NumBeamPipeSeg; // The three beampipe segments in Rich1
  std::string              m_Rich1SubMasterLVName;
  std::string              m_Rich1MasterLVName;
  std::vector<std::string> m_Rich1MagShLVNames;
  std::vector<std::string> m_Rich1PhDetSupFrameLVNames;
  std::string              m_Rich1GasQuartzWLVName;
  std::vector<std::string> m_Rich1PhDetSupFrameDetNames;
  std::vector<std::string> m_Rich1BeamPipeSegmentLVNames;
  std::string              m_Rich1BeamPipeInSubMDetName;

  std::string m_Rich1MasterDetName;
  std::string m_Rich1SubMasterDetName;

  // Parameters used in Simulation with Gaussino

  std::string   m_Rich1GasQuartzWMatName;
  std::string   m_RichMasterMatName;
  std::string   m_Rich1NitrogenMatName;
  std::string   m_Rich1C4F10MatName;
  std::string   m_Rich1RadiatorGasMatName;
  RichGeomLabel m_Cur_Rich1GeomVersion;
  bool          m_RichGeomUtilDebug{true};
};
