//==========================================================================
//  LHCb Rich1 Detector Element Dump in DD4HEP
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
//
// Author     : Sajan Easo
// Date       : 2021-10-03
//
//==========================================================================
#include "Detector/Rich1/DetElemAccess/DeRich1.h"

#include "Detector/Rich1/RichGeoUtil.h"

#include "Core/Keys.h"

#include "Core/ConditionsRepository.h"
#include "Core/DetectorDataService.h"
#include "Core/MagneticFieldExtension.h"

#include "DD4hep/Conditions.h"
#include "DD4hep/DD4hepUnits.h"
#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/Printout.h"

#include "DDCond/ConditionsDataLoader.h"
#include "DDCond/ConditionsManager.h"
#include "DDCond/ConditionsSlice.h"

#include "TTimeStamp.h"

#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

//=============================================================================//

static long test_dump_Rich1_DetElem( dd4hep::Detector& description, int argc, char** argv ) {
  // test print
  std::cout << " Printing from test_dump_Rich1_DetElem " << std::endl;
  for ( int i = 0; i < argc && argv[i]; ++i ) {
    std::cout << " Input argc argv " << i << "  " << argc << "  " << argv[i] << std::endl;
  }

  bool        help = false;
  std::string conditions, match;
  for ( int i = 0; i < argc && argv[i]; ++i ) {
    if ( argv[i][0] == '-' || argv[i][0] == '/' ) {
      if ( 0 == ::strncmp( "-help", argv[i], 4 ) ) {
        std::cout << " argv[i] help true " << i << "  " << argv[i] << std::endl;
        help = true;
      } else if ( 0 == ::strncmp( "-conditions", argv[i], 4 ) )
        conditions = argv[++i];
      else if ( 0 == ::strncmp( "-match", argv[i], 4 ) )
        match = argv[++i];
      else {
        std::cout << " help true option " << i << "  " << argv[i] << std::endl;
        help = true;
      }
    }
  }

  std::cout << "Rich1_dump_detelem help conditions " << help << "  " << conditions << std::endl;

  if ( help || conditions.empty() ) {
    /// Help printout describing the basic command line interface
    std::cout << "Usage: -plugin <name> -arg [-arg]                                   \n"
                 "     name:   factory name     LHCb_TEST_cond_content                \n"
                 "     -detector   <name>       Name of the sub-detector to analyze.  \n"
                 "     -conditions <directory>  Top-directory with conditions files.  \n"
                 "                              Fully qualified: <protocol>://<path>  \n"
                 "     -help                    Show this help.                       \n"
                 "\tArguments given: "
              << dd4hep::arguments( argc, argv ) << std::endl;
    ::exit( EINVAL );
  }

  // Show VERBOSE dd4hep printout from DetElems themselves
  dd4hep::setPrintLevel( dd4hep::DEBUG );

  dd4hep::printout( dd4hep::INFO, "Conditions", "+++ Conditions directory: %s [%s] ", conditions.c_str(),
                    match.c_str() );

  //====================================================================

  // Loading the detector and retrieving the condition with the file names for the Magnetic field service
  LHCb::Magnet::setup_magnetic_field_extension( description, "/cvmfs/lhcb.cern.ch/lib/lhcb/DBASE/FieldMap/v5r7/cdf" );
  std::vector<std::string>            detector_list{"/world", "Magnet", "Rich1"};
  LHCb::Detector::DetectorDataService dds( description, detector_list );
  dds.initialize( nlohmann::json{{"repository", conditions}} );

  int  slice1_id = 100;
  auto slice     = dds.get_slice( slice1_id );

  std::cout << "Now access the Gas radiator detelem " << std::endl;

  dd4hep::DetElement rich1Det = description.detector( "Rich1" );

  std::cout << "Now access the Gas radiator detelem object " << std::endl;

  const LHCb::Detector::DeRich1& r1De = slice->get( rich1Det, LHCb::Detector::Keys::deKey );
  //---------------------------------------------------------------------------------------------//
  // Few tests of accessing parameters. These are commented out normally just to limit the size of the
  // printout.
  // Nominal pmt qe
  /*
  auto aQETab = r1De.nominalPDQuantumEff();
  for ( const auto& elem : aQETab ) {
    std::cout << "Rich Pmt QE Energy QE " << elem.first << "  " << elem.second << std::endl;
  }
  // Nominal Sph mirr refl
  auto aR1sphMNominalRefl = r1De.nominalSphMirrorRefl();
  for ( const auto& elem : aR1sphMNominalRefl ) {
    std::cout << "Rich1 Sph Mirr Nominal Energy refl " << elem.first << "  " << elem.second << std::endl;
  }
  // Nominal sec mirr refl
  auto aR1secMNominalRefl = r1De.nominalSecMirrorRefl();
  for ( const auto& elem : aR1secMNominalRefl ) {
    std::cout << "Rich1 Sec Mirr Nominal Energy refl " << elem.first << "  " << elem.second << std::endl;
  }
  */

  //--------------------------------------------------------------------------------------------//
  // testing the access of Radiator Gas

  std::cout << "Nominal gas pressure:    " << r1De.radiatorGas().Gas_Presure_Nominal() << std::endl;
  std::cout << "Nominal gas temperature: " << r1De.radiatorGas().Gas_Temperature_Nominal() << std::endl;

  auto r1RadGasDe = r1De.radiatorGas();

  // r1RadGasDe.PrintR1GasGeneralInfoFromDB();

  // The following lines are for illustration. They print the same info as that from the
  // line above and hence line above is commented out

  std::cout << " Now retrieving and then printing ref index of Rich1 Gas " << std::endl;
  auto aR1gasRefInd = r1RadGasDe.GasRefIndex();
  for ( const auto& elem : aR1gasRefInd ) {
    std::cout << "R1 gas Energy RefIndex " << elem.first << "  " << elem.second << std::endl;
  }
  std::cout << " End of retrieving and then printing ref index of Rich1 Gas " << std::endl;
  /*
  // std::cout<<" Debug test "<<std::endl;
  //debug test
  // r1RadGasDe.RetrieveAndPrintDebugR1GasNominalRefIndexFromLocalInp();
  */
  // Example of acessing shape and locations of radiator object. Normally commented out.

  const dd4hep::Box& r1RadGasShape_BoolA = r1RadGasDe.access()->detector().solid();
  double             r1RadGas_XHalfSize  = r1RadGasShape_BoolA.x();
  double             r1RadGas_YHalfSize  = r1RadGasShape_BoolA.y();
  double             r1RadGas_ZHalfSize  = r1RadGasShape_BoolA.z();

  // const dd4hep::BooleanSolid& r1RadGasShape_BoolA = r1RadGasDe.access()->detector().solid();
  // const dd4hep::BooleanSolid& s1 = r1RadGasShape_BoolA.leftShape();
  // const dd4hep::BooleanSolid& s2 = s1.leftShape();
  // const dd4hep::Box& s3 = s2.leftShape();

  // double r1RadGas_XHalfSize = s3.x();
  // double r1RadGas_YHalfSize = s3.y();
  // double r1RadGas_ZHalfSize = s3.z();
  dd4hep::Position aRadGasGlobalPos = r1RadGasDe.access()->detectorAlignment().localToWorld( {0, 0, 0} );
  double           aR1SubMUpsEx =
      0.5 * ( dd4hep::_toDouble( "Rh1SubMasterUpstrZLim" ) - dd4hep::_toDouble( "Rh1ExtendedSubMasterUpstrZLim" ) );
  r1RadGas_ZHalfSize -= aR1SubMUpsEx;

  std::cout << " Rich1 Rad Gas Size XYZ UpsZ DnsZ " << r1RadGas_XHalfSize << "   " << r1RadGas_YHalfSize << "    "
            << r1RadGas_ZHalfSize << "  " << ( aRadGasGlobalPos.z() - r1RadGas_ZHalfSize ) << "   "
            << ( aRadGasGlobalPos.z() + r1RadGas_ZHalfSize ) << std::endl;

  std::cout << "Now accessed the Gas radiator detelem object  " << std::endl;
  //----------------------------------------------------------------------------------------//
  // Now accessing Rich1 beam pipe
  // auto r1BeamPipeDe = r1RadGasDe.BeamPipe();
  // Parameters for the beampipe assembly to be accessed.

  // End of accessing Rich1 beam pipe
  //-----------------------------------------------------------------------------------//
  // Now testing the access of Rich1 Mirror1
  for ( int iM = 0; iM < ( r1De.nPrimaryMirrors() ); iM++ ) {
    std::cout << " Current Rich1 Mirror1 Quadrant " << iM << std::endl;

    auto r1Mirror1EnvDe = r1De.Mirror1Env( iM );
    auto aCOC_Local     = r1Mirror1EnvDe.LocalCOC();
    std::cout << " R1M1 Local COC XYZ in cm  :  " << aCOC_Local.X() << " " << aCOC_Local.Y() << "  " << aCOC_Local.Z()
              << std::endl;

    auto aCOC_Global = r1Mirror1EnvDe.GlobalCOC();
    std::cout << " R1M1 Global COC XYZ in cm  :  " << aCOC_Global.X() << " " << aCOC_Global.Y() << "  "
              << aCOC_Global.Z() << std::endl;

    auto r1Mirror1SegDe = r1Mirror1EnvDe.MirrorSeg();
    std::cout << " R1M1 ROC Seg XSize YSize in cm " << r1Mirror1SegDe.ROC() << "  " << r1Mirror1SegDe.SegXSize() << "  "
              << r1Mirror1SegDe.SegYSize() << std::endl;

    auto R1M1Refl = r1Mirror1SegDe.RetrieveReflectivity();

    // The reflectivity print is limited to segment 0 as an illustration and as a measure to limit the size of the
    // printout.
    if ( iM == 0 ) {
      std::cout << " Now printing the reflectivity of R1 mirror1 segment " << iM << std::endl;
      for ( const auto& refel : R1M1Refl ) {
        std::cout << "R1M1 ener Refl " << refel.first << "  " << refel.second << std::endl;
      }

      std::cout << " End of printing the reflectivity of mirror1 segment " << iM << std::endl;
    }
  }
  // Using a vector to access the segments
  const auto ar1M1SegVect = r1De.primaryMirrors();
  for ( int ii = 0; ii < (int)ar1M1SegVect.size(); ii++ ) {
    std::cout << " R1M1Vect Quad ROC " << ii << "  " << ar1M1SegVect[ii].ROC() << std::endl;
  }
  std::cout << " End of example printouts from Rich1Mirror1" << std::endl;

  // End of testing the access of Rich1 Mirror1
  //------------------------------------------------------------------------------------//
  // Now testing access to Rich1Mirror2
  // Loop through the top and bottom sides
  for ( int iS = 0; iS < ( r1De.nSecondaryMirrors() ); iS++ ) {
    std::cout << " Current Rich1 Mirror2 Side " << iS << std::endl;

    auto r1Mirror2MasterDe = r1De.Mirror2Master( iS );

    for ( int iM = 0; iM < ( r1Mirror2MasterDe.Mirror2NumSegSide() ); iM++ ) {
      auto r1Mirror2SegDe = r1Mirror2MasterDe.Mirror2Seg( iM );

      std::cout << " R1M2 Side SegNum ROC Seg XSize YSize in cm  " << iS << "  " << iM << "    " << r1Mirror2SegDe.ROC()
                << "   " << r1Mirror2SegDe.SegXSize() << "    " << r1Mirror2SegDe.SegYSize() << std::endl;

      auto aCOC_Seg_Local  = r1Mirror2SegDe.LocalCOC();
      auto aCOC_Seg_Global = r1Mirror2SegDe.GlobalCOC();
      std::cout << " R1M2 Side SegNum LocalCOCXYZ GlobalCOCXYZ " << iS << "  " << iM << "   " << aCOC_Seg_Local.X()
                << "  " << aCOC_Seg_Local.Y() << "  " << aCOC_Seg_Local.Z() << "  " << aCOC_Seg_Global.X() << "   "
                << aCOC_Seg_Global.Y() << "  " << aCOC_Seg_Global.Z() << std::endl;

      auto R1M2Refl = r1Mirror2SegDe.RetrieveReflectivity();
      // The reflectivity print is limited to segment 0 as an illustration and as a measure to limit the size of the
      // printout.
      if ( ( iM == 0 ) && ( iS == 0 ) ) {
        std::cout << " Now printing the reflectivity of R1 mirror2 segment " << iS << "  " << iM << std::endl;
        for ( const auto& refel : R1M2Refl ) {
          std::cout << "R1M2 ener Refl " << refel.first << "  " << refel.second << std::endl;
        }

        std::cout << " End of printing the reflectivity of R1 mirror2 segment " << iM << std::endl;
      }
    }
    // Another test of accessing mirrors with vector.

    auto r1M2SegVect = r1Mirror2MasterDe.Mirror2SegVect();
    std::cout << " r1Mirror2MasterVect Size " << (int)r1M2SegVect.size() << std::endl;
    for ( int iseg = 0; iseg < (int)r1M2SegVect.size(); iseg++ ) {
      std::cout << " R1M2Vect Side SegNum ROC " << iS << "  " << r1M2SegVect[iseg].ROC() << std::endl;
    }
  }
  auto r1M2TopVect = r1De.secondaryMirrors( Rich::top );
  auto r1M2BotVect = r1De.secondaryMirrors( Rich::bottom );
  std::cout << "R1M2TopVect BotVect Size " << (int)r1M2TopVect.size() << "  " << (int)r1M2BotVect.size() << std::endl;
  for ( int ii = 0; ii < (int)r1M2TopVect.size(); ii++ ) {
    std::cout << "R1M2TopVect  seg ROC " << ii << "   " << r1M2TopVect[ii].ROC() << std::endl;
  }
  for ( int ii = 0; ii < (int)r1M2BotVect.size(); ii++ ) {
    std::cout << "R1M2BotVect  seg ROC " << ii << "   " << r1M2BotVect[ii].ROC() << std::endl;
  }

  //---------end of testing Rich1 mirror2 -----------------------------------------//
  // Now testing access to Rich1 PhDetPanel, Rich1 MapmtModules and Rich1Mapmts
  // Loop through the two panels
  // For now limiting the printout for  a few modules to limit the size of the output.
  for ( const auto iPanel : Rich::sides() ) {

    auto r1PhDetP = r1De.PhDetPanel( iPanel );
    // Now for the size of the panel
    const dd4hep::Box& aPanelShape = r1PhDetP.access()->detector().solid();
    std::cout << " Panel Num  Half size XYZ  " << iPanel << "  " << aPanelShape.x() << "    " << aPanelShape.y()
              << "   " << aPanelShape.z() << std::endl;
    // Now for Panel position
    const dd4hep::Position aPanelGlobalPos   = r1PhDetP.access()->detectorAlignment().localToWorld( {0, 0, 0} );
    const dd4hep::Position aPanelLocalPos    = r1PhDetP.access()->detector().placement().position();
    const TGeoMatrix&      aPanelOrientation = r1PhDetP.access()->detector().placement().matrix();
    std::cout << " Panel num GlobalPos LocalPos " << iPanel << "  " << aPanelGlobalPos << "  " << aPanelLocalPos << "  "
              << std::endl;
    aPanelOrientation.Print();

    // loop through the std  Modules in a Panel
    for ( int iMs = 0; iMs < ( r1PhDetP.NumMapmtStdModulesInAPanel() ); iMs++ ) {
      if ( iMs < 2 ) { // This is just to limit the size of the printout in this test.
        auto r1StdPD = r1PhDetP.StdPDModule( iMs );
        std::cout << " Rich1 Panel StdModule StdModuleCopyNum " << iPanel << "   " << iMs << "   "
                  << r1StdPD.PDModuleCopyNum() << std::endl;

        for ( int iPs = 0; iPs < ( r1StdPD.NumMapmtsInModule() ); iPs++ ) {
          auto r1StdPmt = r1StdPD.Mapmt( iPs );
          std::cout << " Rich1 StdModule Pmt Pmtcopynum " << iMs << "   " << iPs << "   " << r1StdPmt.copyNum()
                    << std::endl;
        }
      }
    }
    // loop through Edge modules in a panel
    for ( int iMe = 0; iMe < ( r1PhDetP.NumMapmtEdgeModulesInAPanel() ); iMe++ ) {
      if ( iMe < 2 ) { // This is just to limit the size of the printout in this test.

        auto r1EdgePD = r1PhDetP.EdgePDModule( iMe );
        std::cout << " Rich1 Panel EdgeModule EdgeModuleCopyNum " << iPanel << "   " << iMe << "   "
                  << r1EdgePD.PDModuleCopyNum() << std::endl;

        for ( int iPs = 0; iPs < ( r1EdgePD.NumMapmtsInModule() ); iPs++ ) {
          auto r1EdgePmt = r1EdgePD.Mapmt( iPs );
          std::cout << " Rich1 EdgeModule Pmt Pmtcopynum " << iMe << "   " << iPs << "   " << r1EdgePmt.copyNum()
                    << std::endl;
        }
      }
    }
    // loop through OuterCorner modules in a panel
    for ( int iMt = 0; iMt < ( r1PhDetP.NumMapmtOuterCornerModulesInAPanel() ); iMt++ ) {
      if ( iMt < 1 ) { // This is just to limit the size of the printout in this test.

        auto r1OuterCornerPD = r1PhDetP.OuterCornerPDModule( iMt );
        std::cout << " Rich1 Panel OuterCornerModule OuterCornerModuleCopyNum " << iPanel << "   " << iMt << "   "
                  << r1OuterCornerPD.PDModuleCopyNum() << std::endl;

        for ( int iPs = 0; iPs < ( r1OuterCornerPD.NumMapmtsInModule() ); iPs++ ) {
          auto r1OuterCornerPmt = r1OuterCornerPD.Mapmt( iPs );
          std::cout << " Rich1 OuterCornerModule Pmt Pmtcopynum " << iMt << "   " << iPs << "   "
                    << r1OuterCornerPmt.copyNum() << std::endl;
        }
      }
    }
  }
  // end of testing Rich1phdet panel and pmts
  //---------------------------------------------------------------------------------//

  return 1;
}
DECLARE_APPLY( LHCb_TEST_dump_Rich1, test_dump_Rich1_DetElem )
