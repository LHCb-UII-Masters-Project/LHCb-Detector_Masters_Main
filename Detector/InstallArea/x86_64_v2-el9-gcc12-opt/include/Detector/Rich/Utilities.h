/*****************************************************************************\
* (c) Copyright 2000-2019 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*******************************************************************************/

#pragma once

#include "Core/DeIOV.h"
#include "Core/Utils.h"
#include "DD4hep/Printout.h"
#include "boost/algorithm/string.hpp"
#include <cstdint>
#include <exception>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace LHCb::Detector::detail {

  /// More flexible API to access DD4HEP parameters
  template <typename TYPE>
  inline TYPE dd4hep_param( std::string name, const bool scaleToLHCb = true ) {
    // temporary work around for different parameter names between DetDesc and dd4hep.
    // To be removed once DetDesc is gone, and then the names passed should be updated
    // to be the dd4hep ones.
    using NameMap                 = std::unordered_map<std::string, std::string>;
    static const NameMap name_map = //
        {{"Rich1GasQuartzWindowThickness", "Rh1QuartzWindowZSize"},
         {"Rich2GasQuartzWindowThickness", "Rh2QuartzWindowSizeZ"},
         {"SellC4F10F1Param", "SellC4F10F1"},
         {"SellC4F10F2Param", "SellC4F10F2"},
         {"SellC4F10E1Param", "SellC4F10E1"},
         {"SellC4F10E2Param", "SellC4F10E2"},
         {"SellCF4F1Param", "SellCF4F1"},
         {"SellCF4F2Param", "SellCF4F2"},
         {"SellCF4E1Param", "SellCF4E1"},
         {"SellCF4E2Param", "SellCF4E2"},
         {"GasMolWeightC4F10Param", "GasMolWeightC4F10"},
         {"GasMolWeightCF4Param", "GasMolWeightCF4"},
         {"RhoEffectiveSellC4F10Param", "RhoEffectiveSellC4F10"},
         {"RhoEffectiveSellCF4Param", "GasRhoCF4atSTP"}, // double check this one
         {"SellLorGasFacParam", "SellLorGasFac"},
         {"RichPmtNumPixelCol", "RhPMTNumPixelsInCol"},
         {"RichPmtNumPixelRow", "RhPMTNumPixelsInRow"},
         {"RichPmtPixelXSize", "RhPMTPixelXSize"},
         {"RichPmtPixelYSize", "RhPMTPixelYSize"},
         {"RichGrandPmtPixelXSize", "RhGrandPMTPixelXSize"},
         {"RichGrandPmtPixelYSize", "RhGrandPMTPixelYSize"},
         {"RichPmtAnodeXSize", "RhPMTAnodeXSize"},
         {"RichPmtAnodeYSize", "RhPMTAnodeYSize"},
         {"RichGrandPmtAnodeXSize", "RhGrandPMTAnodeXSize"},
         {"RichGrandPmtAnodeYSize", "RhGrandPMTAnodeYSize"},
         {"Rich1PmtDetPlaneZInPmtPanel", "Rh1PMTDetPlaneZLocationInPhDetSupFrame"},
         {"Rich2PmtDetPlaneZInPmtPanel", "Rh2MixedPMTDetPlaneZLocationInPMTPanel"}};
    const auto mapped_name = name_map.find( name );
    if ( mapped_name != name_map.end() ) { name = mapped_name->second; }
    // Convert inconvenient DD4HEP API to dispatch via template type
    try {
      if constexpr ( std::is_arithmetic_v<TYPE> ) {
        // deal with really (really) annoying units mis-match between dd4hep and everyone else.
        // To be removed once the units are corrected 'at source' to be the expected Gaudi ones.
        auto F = TYPE( 1 );
        if ( scaleToLHCb ) {
          using ScaleMap                  = std::unordered_map<std::string, TYPE>;
          const TYPE            cmTOmm    = LHCb::Detector::detail::DD4hepToLHCbCm; // cm -> mm
          static const ScaleMap scale_map = {{"Rh1QuartzWindowZSize", cmTOmm},
                                             {"Rh2QuartzWindowSizeZ", cmTOmm},
                                             {"RhPMTPixelXSize", cmTOmm},
                                             {"RhPMTPixelYSize", cmTOmm},
                                             {"RhGrandPMTPixelXSize", cmTOmm},
                                             {"RhGrandPMTPixelYSize", cmTOmm},
                                             {"RhPMTAnodeXSize", cmTOmm},
                                             {"RhPMTAnodeYSize", cmTOmm},
                                             {"RhGrandPMTAnodeXSize", cmTOmm},
                                             {"RhGrandPMTAnodeYSize", cmTOmm},
                                             {"Rh1PMTDetPlaneZLocationInPhDetSupFrame", cmTOmm},
                                             {"Rh2MixedPMTDetPlaneZLocationInPMTPanel", cmTOmm},
                                             {"Rh1PhDetSupXSize", cmTOmm},
                                             {"Rh1PhDetSupYSize", cmTOmm},
                                             {"Rh2PDPanelSizeX", cmTOmm},
                                             {"Rh2PDPanelSizeY", cmTOmm},
                                             {"Rh1SubMasterUpstrZLim", cmTOmm},
                                             {"Rh1SubMasterDnstrZLim", cmTOmm},
                                             {"Rh1SubMasterXSize", cmTOmm},
                                             {"Rh1SubMasterYSize", cmTOmm},
                                             {"Rich2MasterZ", cmTOmm},
                                             {"Rich2MasterZSize", cmTOmm},
                                             {"Rh2MainGasSizeX1", cmTOmm},
                                             {"Rh2MainGasSizeY1", cmTOmm},
                                             {"Rh2MainGasSizeX2", cmTOmm},
                                             {"Rh2MainGasSizeY2", cmTOmm},
                                             {"Rh1BeamPipeConeBeginInLHCbZ", cmTOmm},
                                             {"Rh1BeamPipeConeEndInLHCbZ", cmTOmm},
                                             {"Rh1BeamPipeConeOuterRad1", cmTOmm},
                                             {"Rh1BeamPipeConeOuterRad2", cmTOmm},
                                             {"Rh2BeamHoleConeZSize", cmTOmm},
                                             {"Rh2BeamHoleConeZBeginInLHCb", cmTOmm},
                                             {"Rh2MirrBeamHoleConeUpstreamRadius", cmTOmm},
                                             {"Rh2MirrBeamHoleConeDownstreamRadius", cmTOmm},
                                             {"RhPMTPhCathodeZPos", cmTOmm},
                                             {"RhPMTPhCathodeZThickness", cmTOmm},
                                             {"RhPMTQuartzThickness", cmTOmm},
                                             {"Rh1PMTModulePlaneXEdgeTop", cmTOmm},
                                             {"Rh1PMTModulePlaneYEdgeTop", cmTOmm},
                                             {"Rh2MixedPMTModulePlaneXEdgeLeft", cmTOmm},
                                             {"Rh2MixedPMTModulePlaneYEdgeLeft", cmTOmm}};
          const auto            scale     = scale_map.find( name );
          F                               = ( scale == scale_map.end() ? TYPE( 1 ) : scale->second );
        }
        // floating point types
        if constexpr ( std::is_same_v<double, TYPE> ) {
          return F * static_cast<TYPE>( dd4hep::_toDouble( name.c_str() ) );
        }
        if constexpr ( std::is_same_v<float, TYPE> ) {
          return F * static_cast<TYPE>( dd4hep::_toFloat( name.c_str() ) );
        }
        // signed ints
        if constexpr ( std::is_same_v<std::int8_t, TYPE> ) {
          return F * static_cast<TYPE>( dd4hep::_toShort( name.c_str() ) );
        }
        if constexpr ( std::is_same_v<std::int16_t, TYPE> ) {
          return F * static_cast<TYPE>( dd4hep::_toShort( name.c_str() ) );
        }
        if constexpr ( std::is_same_v<std::int32_t, TYPE> ) {
          return F * static_cast<TYPE>( dd4hep::_toInt( name.c_str() ) );
        }
        if constexpr ( std::is_same_v<std::int64_t, TYPE> ) {
          return F * static_cast<TYPE>( dd4hep::_toLong( name.c_str() ) );
        }
        // unsigned ints
        if constexpr ( std::is_same_v<std::uint8_t, TYPE> ) {
          return F * static_cast<TYPE>( dd4hep::_toShort( name.c_str() ) );
        }
        if constexpr ( std::is_same_v<std::uint16_t, TYPE> ) {
          return F * static_cast<TYPE>( dd4hep::_toShort( name.c_str() ) );
        }
        if constexpr ( std::is_same_v<std::uint32_t, TYPE> ) {
          return F * static_cast<TYPE>( dd4hep::_toInt( name.c_str() ) );
        }
        if constexpr ( std::is_same_v<std::uint64_t, TYPE> ) {
          return F * static_cast<TYPE>( dd4hep::_toLong( name.c_str() ) );
        }
      }
      // strings
      if constexpr ( std::is_same_v<std::string, TYPE> ) { return dd4hep::_toString( name.c_str() ); }
      // bool
      if constexpr ( std::is_same_v<bool, TYPE> ) { return dd4hep::_toBool( name.c_str() ); }
      // compiler error if get here.
    } catch ( const std::exception& e ) {
      std::cerr << "ERROR : Cannot load parameter '" << name << "' as type '" << dd4hep::typeName( typeid( TYPE ) )
                << "' Exception='" << e.what() << "'" << std::endl;
      throw e;
    }
  }

  /// Initialise a given variable with the given DD4HEP Parameter
  template <typename TYPE>
  void init_param( TYPE& v, std::string name, const bool scaleToLHCb = true ) {
    v = dd4hep_param<TYPE>( std::move( name ), scaleToLHCb );
  }

  /// Scales values in dd4hep interpolator data to correct photon energy units
  /// to account for different units used in dd4hep w.r.t. Gaudi
  template <typename XYDATAMAP>
  auto scalePhotonEnergies( const XYDATAMAP& data ) {
    if constexpr ( !essentiallyEqual( 1.0, detail::DD4hepToLHCbEnergy ) ) {
      XYDATAMAP scaled_data;
      for ( const auto [x, y] : data ) { scaled_data[detail::toLHCbEnergyUnits( x )] = y; }
      return scaled_data;
    } else {
      return data;
    }
  }

  /// Scales values in dd4hep interpolator data to correct length units
  /// to account for different units used in dd4hep w.r.t. Gaudi
  template <typename XYDATAMAP>
  auto scaleLengths( const XYDATAMAP& data ) {
    if constexpr ( !essentiallyEqual( 1.0, detail::DD4hepToLHCbCm ) ) {
      XYDATAMAP scaled_data;
      for ( const auto [x, y] : data ) { scaled_data[x] = detail::toLHCbLengthUnits( y ); }
      return scaled_data;
    } else {
      return data;
    }
  }

  // forward declarations
  struct DeRichPDModuleObject;
  struct DeRichMapmtObject;
  struct DeRichPhDetPanelObject;
  struct DeRichMirrorSegObject;
  struct DeRichRadiatorObject;
  struct DeRichObject;

  template <typename OBJ>
  inline void print( const OBJ& obj, const dd4hep::PrintLevel level = dd4hep::DEBUG ) {
    // form name from full type
    auto name = dd4hep::typeName( typeid( obj ) );
    boost::erase_all( name, "LHCb::Detector::" );
    boost::erase_all( name, "detail::" );
    // Dispatch print format by base type
    if constexpr ( std::is_base_of_v<DeRichPDModuleObject, OBJ> ) {
      dd4hep::printout( level, name,                                                     //
                        "  Module : CopyN=%i Side=%i PanelLocalModN=%i NumPMTsInMod=%i", //
                        obj.m_PDModuleCopyNum, obj.m_PDModuleSide, obj.m_PDPanelLocalModuleN, obj.m_NumMapmtsInModule );
    }
    if constexpr ( std::is_base_of_v<DeRichMapmtObject, OBJ> ) {
      dd4hep::printout( level, name,                                                                    //
                        "     PMT : CopyN=%i Side=%i NumInMod=%i IsHType=%i PixelArea=%f NumPixels=%i", //
                        obj.m_MapmtCopyNum, obj.m_MapmtSide, obj.m_MapmtNumInModule, obj.m_isHType, obj.m_effPixelArea,
                        obj.m_numPixels );
    }
    if constexpr ( std::is_base_of_v<DeRichPhDetPanelObject, OBJ> ) {
      dd4hep::printout( level, name,                   //
                        " PDPanel : Num=%i CopyID=%i", //
                        obj.m_PhDetPanelN, obj.m_PhDetPanelCopyIdRef );
    }
    if constexpr ( std::is_base_of_v<DeRichMirrorSegObject, OBJ> ) {
      dd4hep::printout( level, name,                                                       //
                        "  Mirror : Num=%i Side=%i RoC=%f CoC=(%f,%f,%f) Norm=(%f,%f,%f)", //
                        obj.m_mirrorNumber, obj.m_mirrorSide, obj.m_SegmentROC,            //
                        obj.m_SegmentGlobalCOC.X(), obj.m_SegmentGlobalCOC.Y(), obj.m_SegmentGlobalCOC.Z(),
                        obj.m_MirrorSurfCentreNormal.X(), obj.m_MirrorSurfCentreNormal.Y(),
                        obj.m_MirrorSurfCentreNormal.Z() );
    }
    if constexpr ( std::is_base_of_v<DeRichRadiatorObject, OBJ> ) {
      dd4hep::printout( level, name,                                                                                 //
                        "Radiator : NomP=%f NomT=%f Z(min,max)=(%f,%f) (X,Y)SizeEntry=(%f,%f) (X,Y)SizeExit=(%f,%f)" //
                        " EntPlane=(%f,%f,%f,%f) ExtPlane=(%f,%f,%f,%f)",                                            //
                        obj.m_Presure_Nominal, obj.m_Temperature_Nominal, obj.m_z_entry, obj.m_z_exit,               //
                        obj.m_x_size_entry, obj.m_y_size_entry, obj.m_x_size_exit, obj.m_y_size_exit,                //
                        obj.m_entryPlane.A(), obj.m_entryPlane.B(), obj.m_entryPlane.C(), obj.m_entryPlane.D(),      //
                        obj.m_exitPlane.A(), obj.m_exitPlane.B(), obj.m_exitPlane.C(), obj.m_exitPlane.D() );
    }
    if constexpr ( std::is_base_of_v<DeRichObject, OBJ> ) {
      dd4hep::printout( level, name,                                                                              //
                        "    RICH : NPrimMirrRows=%i NPrimMirrCols=%i NSecMirrRows=%i NSecMirrCols=%i NomRoC=%d", //
                        obj.m_nSphMirrorSegRows, obj.m_nSphMirrorSegCols, obj.m_nSecMirrorSegRows,
                        obj.m_nSecMirrorSegCols, obj.m_sphMirrorNominalROC );
    }
  }

} // namespace LHCb::Detector::detail
