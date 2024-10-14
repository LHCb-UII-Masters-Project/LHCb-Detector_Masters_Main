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

#include "Detector/Rich1/RichGeoUtil.h"
#include <cassert>
#include <memory>
#include <mutex>

using namespace LHCb::Detector;

//==================================================================================//

void RichGeoUtil::build_RichTransforms( const xml_h& tr_handle ) {

  printout( m_RichGeomUtilDebug ? dd4hep::DEBUG : dd4hep::DEBUG, "RiGeoUtil : ", "Now building RichTransforms " );
  // std::cout<<"RichGeoUtil: Now building RichTransforms "<<std::endl;

  for ( xml_coll_t i( tr_handle, _U( item ) ); i; ++i ) {
    // std::cout<<" Now in the loop of transform in RichTransforms "<<std::endl;
    // printout(m_RichGeomUtilDebug  ? dd4hep::ALWAYS : dd4hep::DEBUG,
    //	    "RichGeoUtil: Now in the loop of transform in RichTransforms");

    xml_comp_t c( i );
    const auto itemLabel = c.attr<std::string>( _U( name ) );
    if ( itemLabel.find( "TransformName" ) != std::string::npos ) {
      const auto trName = c.attr<std::string>( _U( value ) );
      // std::cout<<" test transform name " << trName <<std::endl;
      xml_elt_t trn = c.child( _U( transformation ) );
      printout( m_RichGeomUtilDebug ? dd4hep::ALWAYS : dd4hep::DEBUG, "RichGeoUtil: build RichTransform for %s ",
                trName.c_str() );

      const auto tr = dd4hep::xml::createTransformation( trn );

      // here there is only a cross-check that m_RichTransformNames is filled.
      // In the future one may try to store info in the m_RichTransformNames once
      // a set of specialized unicode tags are setup for storing string constants.

      // std::vector<std::string>::iterator itTr =
      //    std::find( m_RichTransformNames.begin(), m_RichTransformNames.end(), trName );
      // index = std::distance(m_RichTransformNames.begin(),itTr);

      if ( IsInRichTransformList( trName ) || IsInRich1Mirror1TransformList( trName ) ||
           IsInRich1Mirror2TransformList( trName ) ) {

        // std::cout<<" RichGeoUtil Now store the transformation for  "<< trName << std::endl;

        auto retRTrans = m_RichGeomTransforms.insert( std::make_pair( trName, tr ) );

        if ( retRTrans.second == false ) {
          printout( m_RichGeomUtilDebug ? dd4hep::ALWAYS : dd4hep::ERROR, "RichGeoUtilInfo: Error ",
                    " RichGeoUtil Transform for %s  not stored since it already existed with a value of %d ",
                    trName.c_str(), retRTrans.first->second );

          // std::cout<<" RichGeoUtil Transform for "<< trName
          //   << " not stored since it already existed with a value of  "
          //   <<retRTrans.first->second<<std::endl;
        }

        // end of test on m_RichTransformNames
      } else {

        printout( m_RichGeomUtilDebug ? dd4hep::ALWAYS : dd4hep::ERROR, "RichGeoUtilInfo: Error ",
                  "Transform name not available for %s and hence Transform not stored ", trName.c_str() );
      }
      // end test on transformName in label
    } else if ( itemLabel.find( "PhysVolName" ) != std::string::npos ) {

      auto physVolName = c.attr<std::string>( _U( value ) );

      // here there is only a cross-check that m_RichPhysVolNames is filled.
      // In the future one may try to store info in m_RichPhysVolNames once
      // a set of specialized unicode tags are setup for storing string constants.
      // Similar thing also may be done for copy numbers, just so that these xml files need to
      // be loaded only once.

      if ( std::find( m_RichPhysVolNames.begin(), m_RichPhysVolNames.end(), physVolName ) !=
           m_RichPhysVolNames.end() ) {

        printout( m_RichGeomUtilDebug ? dd4hep::ALWAYS : dd4hep::DEBUG, "RichGeoUtilInfo: ", "PhysVol name OK for  %s ",
                  physVolName.c_str() );

      } else if ( ( IsInRich1Mirror1PhysList( physVolName ) ) || ( IsInRich1Mirror2PhysList( physVolName ) ) ) {

        printout( m_RichGeomUtilDebug ? dd4hep::ALWAYS : dd4hep::DEBUG,
                  "RichGeoUtilInfo: ", "Rich1Mirror PhysVol name OK for  %s ", physVolName.c_str() );

      } else {

        printout( m_RichGeomUtilDebug ? dd4hep::ALWAYS : dd4hep::ERROR, "RichGeoUtilInfo: Error ",
                  "PhysVol name not available for %s ", physVolName.c_str() );
      }

    } // end test on physvolname

  } // end loop over transforms with trhandle
}
//==========================================================================//
const dd4hep::Transform3D& RichGeoUtil::getRichGeomTransform( const std::string& aName ) {
  const auto t = m_RichGeomTransforms.find( aName );
  if ( t != m_RichGeomTransforms.end() ) {
    return t->second;
  } else {
    printout( m_RichGeomUtilDebug ? dd4hep::ALWAYS : dd4hep::ERROR,
              "RichGeoUtil : ", "Rich Transform not found for %s ", aName.c_str() );
    static const dd4hep::Transform3D dummy;
    return dummy;
  }
}
//==========================================================================//
void RichGeoUtil::TestRichGeomParameters() {
  const auto testHBAngle = detail::dd4hep_param<double>( "Rh1TestHBAngle" );
  const auto testSAngle  = detail::dd4hep_param<double>( "Rh1TestSAngle" );
  dd4hep::printout( dd4hep::DEBUG, "RichGeoUtil :", "testHBAngle: %lf  testSAngle: %lf", testHBAngle, testSAngle );
}
//==========================================================================//
void RichGeoUtil::InitRichTransformsAndNames() {
  m_RichMaxNumGeomTransforms = 100;

  m_RichGeomTransforms.clear();
  m_RichTransformNames.clear();
  m_RichPhysVolNames.clear();
  m_RichPhysVolCopyNumbers.clear();
  m_Rich1MagShLVNames.clear();
  m_Rich1PhDetSupFrameLVNames.clear();
  m_Rich1PhDetSupFrameDetNames.clear();
  m_Rich1BeamPipeSegmentLVNames.clear();

  detail::init_param( m_Rich1NumSides, "Rh1NumSides" );
  detail::init_param( m_Rich1NumBeamPipeSeg, "Rh1NumBeamPipeSegments" );

  m_Rich1MasterLVName         = "lvRich1Master";
  m_Rich1SubMasterLVName      = "lvRich1SubMaster";
  m_Rich1MagShLVNames         = {"lvRich1MagShH0", "lvRich1MagShH1"};
  m_Rich1PhDetSupFrameLVNames = {"lvRich1PhDetSupFrameH0", "lvRich1PhDetSupFrameH1"};
  m_Rich1GasQuartzWLVName     = "lvRich1GQuartzW";
  m_Rich1GasQuartzWMatName    = "Rich:Rich1GasWindowQuartz";
  m_RichMasterMatName         = "Rich:Air";
  m_Rich1NitrogenMatName      = "Rich:Rich1Nitrogen";
  m_Rich1C4F10MatName         = "Rich:C4F10";
  m_Rich1RadiatorGasMatName   = "Rich:R1RadiatorGas";

  // Intialize names for Det elements
  m_Rich1MasterDetName    = "Rich1Master";
  m_Rich1SubMasterDetName = "Rich1SubMaster";

  m_Rich1PhDetSupFrameDetNames  = {"Rich1PmtPanel0", "Rich1PmtPanel1"};
  m_Rich1BeamPipeSegmentLVNames = {"lvUX851InRich1SubMaster", "lvUX851InRich1BeforeSubM", "lvUX851InRich1AfterSubM"};
  m_Rich1BeamPipeInSubMDetName  = "Rich1BeamPipe";

  // Initialize with dummy names

  m_RichTransformNames.assign( m_RichMaxNumGeomTransforms, "DummyRichTransform" );
  m_RichPhysVolNames.assign( m_RichMaxNumGeomTransforms, "DummyRichPhysvolName" );
  m_RichPhysVolCopyNumbers.assign( m_RichMaxNumGeomTransforms, 0 );

  //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    //

  const Rich::Detector::NamedVector<int> Rich1GeneralPvI{
      {"Rich1SubMaster", toInTe( RichGeomTransformLabel::pvRich1SubMasterTN )},
      {"Rich1PhDetSupFrameH0", toInTe( RichGeomTransformLabel::pvRich1PhDetSupFrameH0TN )},
      {"Rich1PhDetSupFrameH1", toInTe( RichGeomTransformLabel::pvRich1PhDetSupFrameH1TN )},
      {"Rich1GQuartzWH0", toInTe( RichGeomTransformLabel::pvRich1GQuartzWH0TN )},
      {"Rich1GQuartzWH1", toInTe( RichGeomTransformLabel::pvRich1GQuartzWH1TN )},
      {"Rich1MagShH0", toInTe( RichGeomTransformLabel::pvRich1MagShH0TN )},
      {"Rich1MagShH1", toInTe( RichGeomTransformLabel::pvRich1MagShH1TN )}};
  InitRich1FillTN( Rich1GeneralPvI );

  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

  const Rich::Detector::NamedVector<int> Rich1MirrorPvI{
      {"Rich1Mirror1Master", toInTe( RichGeomTransformLabel::pvRich1Mirror1MasterTN )},
      {"Rich1Mirror2MasterTop", toInTe( RichGeomTransformLabel::pvRich1Mirror2MasterTopTN )},
      {"Rich1Mirror2MasterBot", toInTe( RichGeomTransformLabel::pvRich1Mirror2MasterBotTN )},
      {"Rich1Mirror2SupportTop", toInTe( RichGeomTransformLabel::pvRich1Mirror2SupportTopTN )},
      {"Rich1Mirror2SupportBot", toInTe( RichGeomTransformLabel::pvRich1Mirror2SupportBotTN )}};

  InitRich1FillTN( Rich1MirrorPvI );

  if ( ( m_Cur_Rich1GeomVersion ) != ( RichGeomLabel::Rich_RUN3_v0 ) ) {
    const Rich::Detector::NamedVector<int> Rich1Mirror2QuadPvI{
        {"Rich1Mirror2Q0Master", toInTe( RichGeomTransformLabel::pvRich1Mirror2Q0MasterTN )},
        {"Rich1Mirror2Q1Master", toInTe( RichGeomTransformLabel::pvRich1Mirror2Q1MasterTN )},
        {"Rich1Mirror2Q2Master", toInTe( RichGeomTransformLabel::pvRich1Mirror2Q2MasterTN )},
        {"Rich1Mirror2Q3Master", toInTe( RichGeomTransformLabel::pvRich1Mirror2Q3MasterTN )}};

    InitRich1FillTN( Rich1Mirror2QuadPvI );
  }

  InitRich1MirrorTransformsAndNames();
  InitRich1MirrorCaFiCyl();

  //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    //
  const Rich::Detector::NamedVector<int> Rich1MgsPvI{
      {"Rich1MgsOuterH0", toInTe( RichGeomTransformLabel::pvRich1MgsOuterH0TN )},
      {"Rich1MgsOuterH1", toInTe( RichGeomTransformLabel::pvRich1MgsOuterH1TN )},
      {"Rich1MgsUpstrH0", toInTe( RichGeomTransformLabel::pvRich1MgsUpstrH0TN )},
      {"Rich1MgsUpstrH1", toInTe( RichGeomTransformLabel::pvRich1MgsUpstrH1TN )},
      {"Rich1MgsSideQ0", toInTe( RichGeomTransformLabel::pvRich1MgsSideQ0TN )},
      {"Rich1MgsSideQ1", toInTe( RichGeomTransformLabel::pvRich1MgsSideQ1TN )},
      {"Rich1MgsSideQ2", toInTe( RichGeomTransformLabel::pvRich1MgsSideQ2TN )},
      {"Rich1MgsSideQ3", toInTe( RichGeomTransformLabel::pvRich1MgsSideQ3TN )},
      {"Rich1MgsUpstrCornerH0", toInTe( RichGeomTransformLabel::pvRich1MgsUpstrCornerH0TN )},
      {"Rich1MgsUpstrCornerH1", toInTe( RichGeomTransformLabel::pvRich1MgsUpstrCornerH1TN )},
      {"Rich1MgsTeethQ0", toInTe( RichGeomTransformLabel::pvRich1MgsTeethQ0TN )},
      {"Rich1MgsTeethQ1", toInTe( RichGeomTransformLabel::pvRich1MgsTeethQ1TN )},
      {"Rich1MgsTeethQ2", toInTe( RichGeomTransformLabel::pvRich1MgsTeethQ2TN )},
      {"Rich1MgsTeethQ3", toInTe( RichGeomTransformLabel::pvRich1MgsTeethQ3TN )},
      {"Rich1MgsMidH0", toInTe( RichGeomTransformLabel::pvRich1MgsMidH0TN )},
      {"Rich1MgsMidH1", toInTe( RichGeomTransformLabel::pvRich1MgsMidH1TN )},
      {"Rich1MgsDnstrUTH0", toInTe( RichGeomTransformLabel::pvRich1MgsDnstrUTH0TN )},
      {"Rich1MgsDnstrUTH1", toInTe( RichGeomTransformLabel::pvRich1MgsDnstrUTH1TN )}};
  InitRich1FillTN( Rich1MgsPvI );

  //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    //
  const Rich::Detector::NamedVector<int> Rich1ExWPvI{
      {"Rich1ExitWallMaster", toInTe( RichGeomTransformLabel::pvRich1ExitWallMasterTN )},
      {"Rich1ExitDiaphramCentralUps", toInTe( RichGeomTransformLabel::pvRich1ExitDiaphramCentralUpsTN )},
      {"Rich1ExitDiaphramCentralDns", toInTe( RichGeomTransformLabel::pvRich1ExitDiaphramCentralDnsTN )},
      {"Rich1ExitDiaphramMiddle", toInTe( RichGeomTransformLabel::pvRich1ExitDiaphramMiddleTN )},
      {"Rich1ExitDiaphramPeriphery", toInTe( RichGeomTransformLabel::pvRich1ExitDiaphramPeripheryTN )},
      {"Rich1ExitDiaphramCentralPlugUps", toInTe( RichGeomTransformLabel::pvRich1ExitDiaphramCentralPlugUpsTN )},
      {"Rich1ExitDiaphramCentralPlugDns", toInTe( RichGeomTransformLabel::pvRich1ExitDiaphramCentralPlugDnsTN )},
      {"Rich1ExitG10Upstr", toInTe( RichGeomTransformLabel::pvRich1ExitG10UpstrTN )},
      {"Rich1ExitG10Dnstr", toInTe( RichGeomTransformLabel::pvRich1ExitG10DnstrTN )},
      {"Rich1ExitPMI", toInTe( RichGeomTransformLabel::pvRich1ExitPMITN )}};
  InitRich1FillTN( Rich1ExWPvI );

  //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    //

  const Rich::Detector::NamedVector<int> Rich1BeamPipePvI{
      {"UX851InRich1SubMaster", toInTe( RichGeomTransformLabel::pvUX851InRich1SubMasterTN )},
      {"UX851InRich1BeforeSubM", toInTe( RichGeomTransformLabel::pvUX851InRich1BeforeSubMTN )},
      {"UX851InRich1AfterSubM", toInTe( RichGeomTransformLabel::pvUX851InRich1AfterSubMTN )}};
  InitRich1FillTN( Rich1BeamPipePvI );
}
//========================================================================== //

void RichGeoUtil::InitRich1FillTN( const Rich::Detector::NamedVector<int>& aRich1CompPvI ) {

  const std::string TNSuf  = "Transform";
  const std::string CNSuf  = "CopyNumber";
  const std::string pvPref = "pv";

  for ( const auto& G : aRich1CompPvI ) {
    const auto& gIndex           = G.second;
    const auto& gLabel           = G.first;
    m_RichTransformNames[gIndex] = pvPref + gLabel + TNSuf;
    m_RichPhysVolNames[gIndex]   = pvPref + gLabel;
    detail::init_param( m_RichPhysVolCopyNumbers[gIndex], pvPref + gLabel + CNSuf );
  }
}
//========================================================================== //

void RichGeoUtil::InitRich1MirrorTransformsAndNames() {

  m_Rich1Mirror1TransformNames.clear();
  m_Rich1Mirror1PhysVolNames.clear();
  m_Rich1Mirror1CopyNumbers.clear();
  m_Rich1Mirror1LogVolNames.clear();
  m_Rich1Mirror1DetNames.clear();

  m_Rich1Mirror2TransformNames.clear();
  m_Rich1Mirror2PhysVolNames.clear();
  m_Rich1Mirror2CopyNumbers.clear();
  m_Rich1Mirror2DetNames.clear();

  m_Rich1Mirror1QuadrantNames.clear();
  m_Rich1Mirror2SegNames.clear();
  m_Rich1Mirror2MasterLVNames.clear();
  m_Rich1Mirror2SupportLVNames.clear();
  m_Rich1Mirror2MasterDetNames.clear();
  m_Rich1Mirror2QuadrantMasterLVNames.clear();
  m_Rich1Mirror2QuadrantMasterDetNames.clear();
  m_Rich2Mirror2QuadInM2Sys.clear();

  m_Rich1Mirror1MasterLVName = "lvRich1Mirror1Master";
  m_Rich1Mirror1CaFiLVName   = "lvRich1Mirror1CarbonFibreCylinder";

  m_Rich1Mirror2MasterLVNames          = {"lvRich1Mirror2MasterTop", "lvRich1Mirror2MasterBot"};
  m_Rich1Mirror2SupportLVNames         = {"lvRich1Mirror2SupportTop", "lvRich1Mirror2SupportBot"};
  m_Rich1Mirror2MasterDetNames         = {"Rich1Mirror2MasterTop", "Rich1Mirror2MasterBot"};
  m_Rich1Mirror2QuadrantMasterLVNames  = {"lvRich1Mirror2Q0Master", "lvRich1Mirror2Q1Master", "lvRich1Mirror2Q2Master",
                                         "lvRich1Mirror2Q3Master"};
  m_Rich1Mirror2QuadrantMasterDetNames = {"Rich1Mirror2Q0Master", "Rich1Mirror2Q1Master", "Rich1Mirror2Q2Master",
                                          "Rich1Mirror2Q3Master"};

  m_Rich1Mirror1QuadrantNames = {"Q0", "Q1", "Q2", "Q3"};
  m_Rich1Mirror2SegNames      = {"00", "01", "02", "03", "04", "05", "06", "07",
                            "08", "09", "10", "11", "12", "13", "14", "15"};

  // The spherical and almostFlat mirror systems constitute two different systems.

  detail::init_param( m_Rich1NumMirror1Seg, "Rh1NumberofMirror1Segments" );
  detail::init_param( m_Rich1NumMirror2SegInAHalf, "Rh1NumberOfMirror2SegmentsInaHalf" );

  detail::init_param( m_Rich1NumMirror2Systems, "Rh1NumberOfMirror2Systems" ); // for the top and bottom systems
  detail::init_param( m_Rich1NumMirror2Seg, "Rh1NumberOfMirror2Segments" );
  detail::init_param( m_Rich1NumMirror1VolTypes, "Rh1NumberOfMirror1SubSystems" );
  detail::init_param( m_Rich1NumMirror1DetTypes, "Rh1NumberOfMirror1DetSubSystems" );

  if ( ( m_Cur_Rich1GeomVersion ) != ( RichGeomLabel::Rich_RUN3_v0 ) ) {
    detail::init_param( m_Rich1NumMirror2Quadrants, "Rh1NumberOfMirror2Quadrants" );
  } else {
    m_Rich1NumMirror2Quadrants = 4;
  }

  m_Rich1NumMirror2QuadrantsInHalf = m_Rich1NumMirror2Quadrants / 2;

  m_Rich2Mirror2QuadInM2Sys = {0, 0, 1, 1};

  m_Rich1NumMirror2SegInAQuad = m_Rich1NumMirror2SegInAHalf / 2;

  m_Rich1Mirror1TransformNames.resize(
      m_Rich1NumMirror1VolTypes, std::vector<std::string>( m_Rich1NumMirror1Seg, "Rich1Mirror1DummyTransformName" ) );
  m_Rich1Mirror1PhysVolNames.resize( m_Rich1NumMirror1VolTypes,
                                     std::vector<std::string>( m_Rich1NumMirror1Seg, "Rich1Mirror1DummyPhysVolName" ) );
  m_Rich1Mirror1CopyNumbers.resize( m_Rich1NumMirror1VolTypes, std::vector<int>( m_Rich1NumMirror1Seg, 0 ) );
  m_Rich1Mirror1LogVolNames.resize( m_Rich1NumMirror1VolTypes,
                                    std::vector<std::string>( m_Rich1NumMirror1Seg, "Rich1Mirror1DummyLogVolName" ) );
  m_Rich1Mirror1DetNames.resize( m_Rich1NumMirror1DetTypes,
                                 std::vector<std::string>( m_Rich1NumMirror1Seg, "Rich1Mirror1DummyDetName" ) );
  // Rich1Mirror1Det.resize(m_Rich1NumMirror1DetTypes,
  //			 std::vector<dd4hep::DetElement>(m_Rich1NumMirror1Seg, aDummyDet );

  m_Rich1Mirror2TransformNames.resize( m_Rich1NumMirror2Seg, "Rich1Mirror2DummyTransformName" );
  m_Rich1Mirror2PhysVolNames.resize( m_Rich1NumMirror2Seg, "Rich1Mirror2DummyPhysVolName" );
  m_Rich1Mirror2CopyNumbers.resize( m_Rich1NumMirror2Seg, 0 );
  m_Rich1Mirror2LogVolNames.resize( m_Rich1NumMirror2Seg, "Rich1Mirror2DummyLogVolName" );
  m_Rich1Mirror2DetNames.resize( m_Rich1NumMirror2Seg, "Rich1Mirror2DummyDetName" );

  const std::string lvPref = "lv";
  const std::string pvPref = "pv";

  const std::string aM1LQPrefix    = "Rich1Mirror1QuadrantModule";
  const std::string aM1CFInnPreFix = "Rich1Mirror1CarbonFibreInnerLayer";
  const std::string aM1CFOutPreFix = "Rich1Mirror1CarbonFibreOuterLayer";

  m_Rich1Mirror1VolTypeNames.clear();
  m_Rich1Mirror1VolTypeNames = {aM1LQPrefix, aM1CFInnPreFix, aM1CFOutPreFix};

  const std::string aTS = "Transform";
  const std::string aCN = "CopyNumber";

  // Now for Rich1 Mirror1  structure

  for ( int itN = 0; itN < m_Rich1NumMirror1VolTypes; itN++ ) {
    for ( int iQ = 0; iQ < m_Rich1NumMirror1Seg; iQ++ ) {

      const auto aTypeQ   = m_Rich1Mirror1VolTypeNames[itN] + m_Rich1Mirror1QuadrantNames[iQ];
      const auto apvTypeQ = pvPref + aTypeQ;

      m_Rich1Mirror1TransformNames[itN][iQ] = apvTypeQ + aTS;
      m_Rich1Mirror1PhysVolNames[itN][iQ]   = apvTypeQ;
      detail::init_param( m_Rich1Mirror1CopyNumbers[itN][iQ], apvTypeQ + aCN );
      m_Rich1Mirror1LogVolNames[itN][iQ] = lvPref + aTypeQ;

      if ( itN < m_Rich1NumMirror1DetTypes ) { m_Rich1Mirror1DetNames[itN][iQ] = aTypeQ; }
    }
  }

  // Now for Rich1 Mirror2 structure
  const std::string aM2SegPrefix = "Rich1Mirror2Seg";

  for ( int is = 0; is < m_Rich1NumMirror2Seg; is++ ) {
    const auto aTypeS   = aM2SegPrefix + m_Rich1Mirror2SegNames[is];
    const auto apvTypeS = pvPref + aTypeS;

    m_Rich1Mirror2TransformNames[is] = apvTypeS + aTS;
    m_Rich1Mirror2PhysVolNames[is]   = apvTypeS;
    detail::init_param( m_Rich1Mirror2CopyNumbers[is], apvTypeS + aCN );
    m_Rich1Mirror2LogVolNames[is] = lvPref + aTypeS;
    m_Rich1Mirror2DetNames[is]    = aTypeS;
  }
}
//========================================================================== //
void RichGeoUtil::InitRich1MirrorCaFiCyl() {

  const auto aNumR1CylX          = detail::dd4hep_param<int>( "Rh1Mirror1NumCylAlongX" );
  const auto aNumR1CylY          = detail::dd4hep_param<int>( "Rh1Mirror1NumCylAlongY" );
  m_Rich1Mirror1NumCaFiCylAlongX = aNumR1CylX + 1;
  m_Rich1Mirror1NumCaFiCylAlongY = aNumR1CylY + 1;
  m_Rich1Mirror1NumCaFiCylInASeg = m_Rich1Mirror1NumCaFiCylAlongX * m_Rich1Mirror1NumCaFiCylAlongY;
  m_Rich1Mirror1CaFiCylinderPhysVolNames.clear();
  m_Rich1Mirror1CaFiCylinderCopyNumbers.clear();
  m_Rich1Mirror1CaFiCylinderTransforms.clear();

  m_Rich1Mirror1CaFiCylinderPhysVolNames.resize(
      m_Rich1NumMirror1Seg,
      std::vector<std::string>( m_Rich1Mirror1NumCaFiCylInASeg, "Rich1Mirror1CaFiDummyPhysVolName" ) );

  m_Rich1Mirror1CaFiCylinderCopyNumbers.resize( m_Rich1NumMirror1Seg,
                                                std::vector<int>( m_Rich1Mirror1NumCaFiCylInASeg, 0 ) );

  m_Rich1Mirror1CaFiCylinderTransforms.resize(
      m_Rich1NumMirror1Seg, std::vector<dd4hep::Transform3D>( m_Rich1Mirror1NumCaFiCylInASeg, dd4hep::Transform3D{} ) );

  // Now fill the actual values
  std::vector<double> a_Rh1Mirror1CylThetaStart( m_Rich1NumMirror1Seg, 0.0 );
  std::vector<double> a_Rh1Mirror1CylPhiStart( m_Rich1NumMirror1Seg, 0.0 );
  std::vector<double> a_Rh1Mirror1CylRadialLocation( m_Rich1NumMirror1Seg, 0.0 );
  std::vector<double> a_Rh1Mirror1CylDeltaTheta( m_Rich1NumMirror1Seg, 0.0 );
  std::vector<double> a_Rh1Mirror1CylDeltaPhi( m_Rich1NumMirror1Seg, 0.0 );

  for ( int iq = 0; iq < m_Rich1NumMirror1Seg; iq++ ) {

    a_Rh1Mirror1CylThetaStart[iq] =
        detail::dd4hep_param<double>( "Rh1Mirror1CylThetaStart" + m_Rich1Mirror1QuadrantNames[iq] );
    a_Rh1Mirror1CylPhiStart[iq] =
        detail::dd4hep_param<double>( "Rh1Mirror1CylPhiStart" + m_Rich1Mirror1QuadrantNames[iq] );
    a_Rh1Mirror1CylRadialLocation[iq] =
        detail::dd4hep_param<double>( "Rh1Mirror1CylRadialLocation" + m_Rich1Mirror1QuadrantNames[iq] );
    a_Rh1Mirror1CylDeltaTheta[iq] =
        detail::dd4hep_param<double>( "Rh1Mirror1CylDeltaTheta" + m_Rich1Mirror1QuadrantNames[iq] );
    a_Rh1Mirror1CylDeltaPhi[iq] =
        detail::dd4hep_param<double>( "Rh1Mirror1CylDeltaPhi" + m_Rich1Mirror1QuadrantNames[iq] );
  }

  const std::string apvCaFiCylName = "pvRich1Mirror1CarbonFibreCylinder";
  const std::string aS             = "In";

  auto aCaFiPosA = dd4hep::Position( 0, 0, 0 );
  auto aCaFiRotB = dd4hep::RotationZYX( 0, 0, 0 );

  for ( int iQ = 0; iQ < m_Rich1NumMirror1Seg; iQ++ ) {
    auto aSQ = aS + m_Rich1Mirror1QuadrantNames[iQ];

    for ( int iY = 0; iY < m_Rich1Mirror1NumCaFiCylAlongY; iY++ ) {
      auto aCaFiRotZ = a_Rh1Mirror1CylPhiStart[iQ] + iY * a_Rh1Mirror1CylDeltaPhi[iQ];

      for ( int iX = 0; iX < m_Rich1Mirror1NumCaFiCylAlongX; iX++ ) {
        const auto aCaFiRotY = a_Rh1Mirror1CylThetaStart[iQ] + iX * a_Rh1Mirror1CylDeltaTheta[iQ];

        const auto iCylInQ                                  = iY * m_Rich1Mirror1NumCaFiCylAlongX + iX;
        const auto iStrCylInQ                               = std::to_string( iCylInQ );
        m_Rich1Mirror1CaFiCylinderPhysVolNames[iQ][iCylInQ] = apvCaFiCylName + iStrCylInQ + aSQ;
        m_Rich1Mirror1CaFiCylinderCopyNumbers[iQ][iCylInQ]  = 1000 + iCylInQ;

        const auto aCaFiRotA = dd4hep::RotationZYX( aCaFiRotZ, aCaFiRotY, 0 );
        const auto aCaFiPosB = dd4hep::PositionPolar( a_Rh1Mirror1CylRadialLocation[iQ], aCaFiRotY, aCaFiRotZ );

        m_Rich1Mirror1CaFiCylinderTransforms[iQ][iCylInQ] =
            ( dd4hep::Transform3D( aCaFiRotB, aCaFiPosB ) ) * ( dd4hep::Transform3D( aCaFiRotA, aCaFiPosA ) );
      }
    }
  }
}
//========================================================================== //
bool RichGeoUtil::IsInRich1Mirror1PhysList( const std::string& aPhysVolName ) {
  for ( auto row = m_Rich1Mirror1PhysVolNames.begin(); row != m_Rich1Mirror1PhysVolNames.end(); row++ ) {
    if ( std::find( row->begin(), row->end(), aPhysVolName ) != row->end() ) { return true; }
  }
  return false;
}
//========================================================================== //
bool RichGeoUtil::IsInRich1Mirror1TransformList( const std::string& aTransformName ) {
  for ( auto row = m_Rich1Mirror1TransformNames.begin(); row != m_Rich1Mirror1TransformNames.end(); row++ ) {
    if ( std::find( row->begin(), row->end(), aTransformName ) != row->end() ) { return true; }
  }
  return false;
}
//========================================================================== //
bool RichGeoUtil::IsInRich1Mirror2PhysList( const std::string& aPhysVolName ) {
  return ( std::find( m_Rich1Mirror2PhysVolNames.begin(), m_Rich1Mirror2PhysVolNames.end(), aPhysVolName ) !=
           m_Rich1Mirror2PhysVolNames.end() );
}
//========================================================================== //
bool RichGeoUtil::IsInRich1Mirror2TransformList( const std::string& aTransformName ) {
  return ( std::find( m_Rich1Mirror2TransformNames.begin(), m_Rich1Mirror2TransformNames.end(), aTransformName ) !=
           m_Rich1Mirror2TransformNames.end() );
}
//========================================================================== //
bool RichGeoUtil::IsInRichTransformList( const std::string& aTransformName ) {
  return ( std::find( m_RichTransformNames.begin(), m_RichTransformNames.end(), aTransformName ) !=
           m_RichTransformNames.end() );
}
//===========================================================================//
RichGeoUtil::ListDE //
RichGeoUtil::Rich1Mirror2ListDE( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt, int aSide ) {
  const auto aRich1Mirror2NumSegSide = Rich1NumMirror2SegInAHalf();
  ListDE     aDep;
  aDep.reserve( aRich1Mirror2NumSegSide );
  for ( int iSeg = 0; iSeg < aRich1Mirror2NumSegSide; ++iSeg ) {
    const auto iSegCopyNum = ( aSide == 0 ? iSeg : ( iSeg + aRich1Mirror2NumSegSide ) );
    aDep.emplace_back( de.child( Rich1Mirror2DetName( iSegCopyNum ) ), ctxt );
  }
  return aDep;
}
//==================================================================================//
RichGeoUtil* RichGeoUtil::getRichGeoUtilInstance() {
  static std::once_flag               alloc_instance_once;
  static std::unique_ptr<RichGeoUtil> RichGeoUtilInstance;
  std::call_once( alloc_instance_once, []() { RichGeoUtilInstance = std::make_unique<RichGeoUtil>(); } );
  assert( RichGeoUtilInstance.get() );
  return RichGeoUtilInstance.get();
}
//==================================================================================//
