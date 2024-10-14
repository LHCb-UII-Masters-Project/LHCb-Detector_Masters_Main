/*****************************************************************************\
* (c) Copyright 2000-2018 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <iomanip>
#include <string>
#include <vector>

namespace LHCb::Detector::Calo::CellCode {

  // FIXME this should not be needed, but some tests (LHCb/loaddddb ones)
  // are relying on it !
  const std::string s_BadName = "????";

  /// The actual enumeration for the calorimeter areas
  enum CaloArea {
    UndefinedArea = -1,
    Outer         = 0,
    Middle, // NB: Inner for Hcal
    Inner,
    PinArea,         // some code lines explicitly rely on this value
    CaloAreaNums = 4 // total number of Calorimeter areas
  };

  namespace {

    /// The array of calorimeter area names
    static const std::array<std::string, CaloAreaNums> s_AreaName = {{"Outer", "Middle", "Inner", "PinArea"}};

    /// get the area index from name
    inline int _area( std::string_view area ) {
      auto it = std::find( s_AreaName.begin(), s_AreaName.end(), area );
      return it == s_AreaName.end() ? -1 : std::distance( s_AreaName.begin(), it );
    }

  } // namespace

  // ==========================================================================
  // Coding of CaloCellID objects : Lengths, basic
  // ==========================================================================
  constexpr unsigned int BitsCol  = 6;
  constexpr unsigned int BitsRow  = 6;
  constexpr unsigned int BitsArea = 2;
  constexpr unsigned int BitsCalo = 2;
  // ==========================================================================
  // Coding of CaloCellID objects : Lengths, extra
  // ==========================================================================
  constexpr unsigned int BitsIndex = BitsCol + BitsRow + BitsArea;
  constexpr unsigned int BitsAll   = BitsCol + BitsRow + BitsArea + BitsCalo;
  constexpr unsigned int BitsTotal = 32;
  constexpr unsigned int BitsRest  = BitsTotal - BitsAll;
  // ==========================================================================
  // Coding of CaloCellID objects : Shifts, basic
  // ==========================================================================
  constexpr unsigned int ShiftCol  = 0;
  constexpr unsigned int ShiftRow  = ShiftCol + BitsCol;
  constexpr unsigned int ShiftArea = ShiftRow + BitsRow;
  constexpr unsigned int ShiftCalo = ShiftArea + BitsArea;
  // ==========================================================================
  // Coding of CaloCellID objects : Shifts, extra
  // ==========================================================================
  constexpr unsigned int ShiftIndex = ShiftCol;
  constexpr unsigned int ShiftAll   = ShiftCol;
  constexpr unsigned int ShiftRest  = ShiftCalo + BitsCalo;
  // ==========================================================================
  // Coding of CaloCellID objects : Masks, basic
  // ==========================================================================
  constexpr unsigned int MaskCol  = ( ( ( (unsigned int)1 ) << BitsCol ) - 1 ) << ShiftCol;
  constexpr unsigned int MaskRow  = ( ( ( (unsigned int)1 ) << BitsRow ) - 1 ) << ShiftRow;
  constexpr unsigned int MaskArea = ( ( ( (unsigned int)1 ) << BitsArea ) - 1 ) << ShiftArea;
  constexpr unsigned int MaskCalo = ( ( ( (unsigned int)1 ) << BitsCalo ) - 1 ) << ShiftCalo;
  // ==========================================================================
  // Coding of CaloCellID objects : Masks, extra
  // ==========================================================================
  constexpr unsigned int MaskIndex = ( ( ( (unsigned int)1 ) << BitsIndex ) - 1 ) << ShiftIndex;
  constexpr unsigned int MaskAll   = ( ( ( (unsigned int)1 ) << BitsAll ) - 1 ) << ShiftAll;
  constexpr unsigned int MaskRest  = ( ( ( (unsigned int)1 ) << BitsRest ) - 1 ) << ShiftRest;

  /** set the active fields
   *  @param Where (INPUT) the initial value
   *  @param Value (INPUT) value to be set
   *  @param Shift (INPUT) shift
   *  @param Mask  (INPUT) mask
   *  @return new Field
   */
  inline constexpr unsigned int setField( const unsigned int Field, const unsigned int Value, const unsigned int Shift,
                                          const unsigned int Mask ) {
    unsigned int tmp1 = ( Value << Shift ) & Mask;
    unsigned int tmp2 = Field & ~Mask;
    return tmp1 | tmp2;
  }

  /// the indices for  Calorimeter Detectors (to extract the coding of the "calo" )
  enum Index {
    Undefined = -1,
    // Spd and Prs are gone
    EcalCalo = 2,
    HcalCalo,
    /// total number of NAMED Calorimeters
    CaloNums = 4 // total number of NAMED Calorimeters (form the beginning)
  };

  /// The actual list of Calorimter names
  inline const std::array<std::string, CaloNums> CaloNames = {{"Spd", "Prs", "Ecal", "Hcal"}};

  /// the calorimeter names:
  inline const std::string_view EcalName = {CaloNames[EcalCalo]}; // Ecal
  inline const std::string_view HcalName = {CaloNames[HcalCalo]}; // Hcal

  /** simple function to get the calorimeter name from number
   *  @param num (INPUT) calorimeter index
   *  @return calorimeter name
   */
  inline const std::string& caloName( Index num ) {
    // FIXME this should be
    //   assert( num >= 0 && num < CaloNums );
    //   return CaloNames[num];
    // but some tests (LHCb/loaddddb ones) are relying on it !
    return num < 0 ? s_BadName : num < CaloNums ? CaloNames[num] : s_BadName;
  }

  inline std::string   toString( const Index& i ) { return caloName( i ); }
  inline std::ostream& toStream( const Index& ci, std::ostream& os ) {
    return os << std::quoted( toString( ci ), '\'' );
  }

  /** get the calorimeter index from name, returns -1 for wrong name!
   *  @param name (INPUT) the calorimeter name (can be long string)
   *  @return calorimeter index from name, returns -1 for wrong name!
   */
  inline Index caloNum( std::string_view name ) {
    auto begin = CaloNames.begin();
    auto end   = CaloNames.end();
    auto m     = std::find_if( begin, end, [=]( const auto& i ) { return name.find( i ) != std::string_view::npos; } );
    return Index( m != end ? Index( m - begin ) : Index::Undefined );
  }

  /** get the calorimeter index from name, returns -1 for wrong name!
   *  @paran name (INPUT) the calorimeter name (can be long string)
   *  @return calorimeter index from name, returns -1 for wrong name!
   */
  inline Index CaloNumFromName( std::string_view name ) { return caloNum( name ); }

  /** get the area name from calorimeter index and number
   *  @attention function make heavy use of hadcoded structure of Calorimeter!
   *  @warning   the different convention for Hcal
   *  @param  calo (INPUT) calorimeter index
   *  @param  area (INPUT) area index
   *  @return name for the area
   */
  inline const std::string& caloArea( const int calo, const int area ) {
    switch ( calo ) {
    case EcalCalo:
      switch ( area ) {
      case 0:
        return s_AreaName[Outer]; // Outer
      case 1:
        return s_AreaName[Middle]; // Middle
      case 2:
        return s_AreaName[Inner]; // Inner
      case 3:
        return s_AreaName[PinArea]; // Pin-Area
      default:
        throw std::string( "Unknown calorimeter area" );
      }
    case HcalCalo:
      switch ( area ) {
      case 0:
        return s_AreaName[Outer]; // Outer
      case 1:
        return s_AreaName[Inner]; // Middle, return as Inner ! ATTENTION!
      case 3:
        return s_AreaName[PinArea]; // Pin-Area
      default:
        throw std::string( "Unknown calorimeter area" );
      }
    default:
      throw std::string( "Unknown calorimeter number" );
    }
  }

  /** get the area index from calorimeter index and name
   *  @attention function make heavy use of hadcoded structure of Calorimeter!
   *  @warning   the different convention for Hcal
   *  @param  calo (INPUT) calorimeter index
   *  @param  area (INPUT) area name
   *  @return indx for the area
   */
  inline int caloArea( const int calo, std::string_view area ) {
    switch ( calo ) {
    case HcalCalo:
      switch ( _area( area ) ) {
      case 0:
        return Outer; // 0
      case 1:
        return -1; // ATTENTION!!!
      case 2:
        return Middle; // 1        // ATTENTION!!!
      case 3:
        return PinArea; // 3
      default:
        return -1;
      }
    case EcalCalo:
      return _area( area );
    default:
      return -1;
    }
  }

  /** Is the given area is Pin-diod area ?
   *  @attention It must be coherent with caloArea
   *  @see caloArea
   *  @param calo (INPUT) calorimeetr index
   *  @param area (INPUT) calorimeter index
   *  @reutrn true if the area is Pin-diod area
   */
  inline bool isPinArea( Index calo, const int area ) {
    return (int)PinArea == area && ( Index::EcalCalo == calo || Index::HcalCalo == calo );
  }

} // namespace LHCb::Detector::Calo::CellCode
