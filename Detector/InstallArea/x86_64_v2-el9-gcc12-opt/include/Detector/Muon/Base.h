/*****************************************************************************\
* (c) Copyright 2000-2021 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once

/**
 * Namespace for code/decode rules of MuonID class
 *
 * @author  Andrei Tsaregorodtsev
 * @date    9/01/2002
 */

namespace LHCb::Detector::Muon::Base {

  using ContentType = unsigned int;
  // SV updated to TELL40 -- inspired to LHCbKernel/include/Kernel/MuonBase.h

  inline const unsigned int BitsX       = 7;
  inline const unsigned int BitsY       = 5;
  inline const unsigned int BitsQuarter = 2;
  inline const unsigned int BitsRegion  = 2;
  inline const unsigned int BitsReadout = 0;
  inline const unsigned int BitsLayer   = 0;
  inline const unsigned int BitsStation = 2;
  inline const unsigned int BitsLayoutX = 6;
  inline const unsigned int BitsLayoutY = 5;

  // add new bits to allow the compactification of muonTileID in 28 as requested by LHCbID class
  // the trick is to evaluate BitsLayoutX+BitsLayouty*50 and store it in 10 bits instead of 11. Possible if max number
  // of granularity in X is 48 (as for Run1-2 and Run3)
  inline const unsigned int BitsCompactedLayout = 10;

  //
  inline const unsigned int BitsIndex = BitsX + BitsY + BitsRegion + BitsQuarter;
  inline const unsigned int BitsKey   = BitsIndex + BitsReadout + BitsLayer + BitsStation;
  //
  inline const unsigned int ShiftY       = 0;
  inline const unsigned int ShiftX       = ShiftY + BitsY;
  inline const unsigned int ShiftQuarter = ShiftX + BitsX;
  inline const unsigned int ShiftRegion  = ShiftQuarter + BitsQuarter;
  inline const unsigned int ShiftReadout = ShiftRegion + BitsRegion;
  inline const unsigned int ShiftLayer   = ShiftReadout + BitsReadout;
  inline const unsigned int ShiftStation = ShiftLayer + BitsLayer;
  inline const unsigned int ShiftLayoutX = ShiftStation + BitsStation;
  inline const unsigned int ShiftLayoutY = ShiftLayoutX + BitsLayoutX;
  //

  // start at standard layoutX
  inline const unsigned int ShiftCompactedLayout = ShiftStation + BitsStation;

  inline const unsigned int ShiftIndex = 0;
  inline const unsigned int ShiftKey   = 0;
  //
  inline const ContentType MaskX       = ( ( ( (ContentType)1 ) << BitsX ) - 1 ) << ShiftX;
  inline const ContentType MaskY       = ( ( ( (ContentType)1 ) << BitsY ) - 1 ) << ShiftY;
  inline const ContentType MaskRegion  = ( ( ( (ContentType)1 ) << BitsRegion ) - 1 ) << ShiftRegion;
  inline const ContentType MaskQuarter = ( ( ( (ContentType)1 ) << BitsQuarter ) - 1 ) << ShiftQuarter;
  inline const ContentType MaskLayoutX = ( ( ( (ContentType)1 ) << BitsLayoutX ) - 1 ) << ShiftLayoutX;
  inline const ContentType MaskLayoutY = ( ( ( (ContentType)1 ) << BitsLayoutY ) - 1 ) << ShiftLayoutY;
  inline const ContentType MaskReadout = ( ( ( (ContentType)1 ) << BitsReadout ) - 1 ) << ShiftReadout;
  inline const ContentType MaskLayer   = ( ( ( (ContentType)1 ) << BitsLayer ) - 1 ) << ShiftLayer;
  inline const ContentType MaskStation = ( ( ( (ContentType)1 ) << BitsStation ) - 1 ) << ShiftStation;
  //

  inline const unsigned int MaskCompactedLayout = ( ( ( (ContentType)1 ) << BitsCompactedLayout ) - 1 )
                                                  << ShiftCompactedLayout;
  //

  inline const ContentType MaskIndex = ( ( ( (ContentType)1 ) << BitsIndex ) - 1 ) << ShiftIndex;
  inline const ContentType MaskKey   = ( ( ( (ContentType)1 ) << BitsKey ) - 1 ) << ShiftKey;

  inline const int CENTER = 0;
  inline const int UP     = 1;
  inline const int DOWN   = -1;
  inline const int RIGHT  = 1;
  inline const int LEFT   = -1;

  inline const unsigned int max_compacted_xGrid = 50UL;
} // namespace LHCb::Detector::Muon::Base
