/*****************************************************************************\
* (c) Copyright 2000-2022 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once

#include <string>

/** @namespace PackMCHit
 *
 * bit pattern for hit address (gap,chamber,region,station,quadrant)
 *
 * @author unknown
 * created Fri Jan 13 10:32:31 2006
 * modified Tuesday Jan 4 2022 : migration to dd4hep
 *
 */

namespace PackMCHit {

  using ContentType = unsigned int;
  //
  inline const unsigned int bitGapID      = 2; ///< number of bit to define the gap
  inline const unsigned int bitChamberID  = 8; ///< number of bit to define the chamber
  inline const unsigned int bitRegionID   = 2; ///< number of bit to define the region
  inline const unsigned int bitStationID  = 3; ///< number of bit to define the station
  inline const unsigned int bitQuadrantID = 2; ///< number of bit to define the quadrant
  //
  inline const unsigned int shiftGapID      = 0;                             ///<
  inline const unsigned int shiftChamberID  = shiftGapID + bitGapID;         ///<
  inline const unsigned int shiftRegionID   = shiftChamberID + bitChamberID; ///<
  inline const unsigned int shiftStationID  = shiftRegionID + bitRegionID;   ///<
  inline const unsigned int shiftQuadrantID = shiftStationID + bitStationID; ///<
  //
  inline const ContentType maskGapID      = ( ( ( (ContentType)1 ) << bitGapID ) - 1 ) << shiftGapID;           ///<
  inline const ContentType maskChamberID  = ( ( ( (ContentType)1 ) << bitChamberID ) - 1 ) << shiftChamberID;   ///<
  inline const ContentType maskRegionID   = ( ( ( (ContentType)1 ) << bitRegionID ) - 1 ) << shiftRegionID;     ///<
  inline const ContentType maskStationID  = ( ( ( (ContentType)1 ) << bitStationID ) - 1 ) << shiftStationID;   ///<
  inline const ContentType maskQuadrantID = ( ( ( (ContentType)1 ) << bitQuadrantID ) - 1 ) << shiftQuadrantID; ///<

} // end of namespace  PackMCHit
