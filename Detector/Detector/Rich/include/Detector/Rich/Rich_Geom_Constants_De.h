/*****************************************************************************\
* (c) Copyright 2020 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/

#pragma once

// Rich Geometry constants used for fixing the array sizes in detector
// elements. These are also available from xml files. However for optimization
// some of the array sizes for detector elements are fixed at compile time.
// SE 2021-10-04

namespace Rich1 {
  inline static const unsigned int NumRich1Mirror1Segments = 4;
  //  (same as "Rh1NumberofMirror1Segments" from xml )

  inline static const unsigned int NumRich1Mirror2Sets = 2;
  // (same as "Rh1NumberOfMirror2Systems"  from xml )

  inline static const unsigned int NumRich1Mirror2SegSide = 8;
  //(same as "Rh1NumberOfMirror2SegmentsInaHalf" from xml )

  inline static const unsigned int NumRich1Sides = 2;
  // same as "Rh1NumSides"  from xml )

  inline static const unsigned int NumRich1MapmtModulesInAPanel = 66;
  // same as "Rh1TotalNumPMTModulesInPanel"  from xml

  inline static const unsigned int NumRich1MapmtStdModulesInAPanel = 44;
  // same as "Rh1TotalNumPMTStdModulesInPanel"  from xml

  inline static const unsigned int NumRich1MapmtEdgeModulesInAPanel = 20;
  // same as "Rh1NumPMTEdgeNonOuterCornerModulesInPanel" from xml

  inline static const unsigned int NumRich1MapmtOuterCornerModulesInAPanel = 2;
  // same as  "Rh1NumPMTOuterCornerModulesInPanel"  from xml

  // Maximum possible number of PMTs in a given module type
  inline static const unsigned int MaxNumRich1MapmtsInStdModule = 16;
  // actual value set by "Rh1NumPMTInStdModule"  from xml
  inline static const unsigned int MaxNumRich1MapmtsInEdgeModule = 12;
  // actual value set by "Rh1NumPMTInEdgeModule"  from xml
  inline static const unsigned int MaxNumRich1MapmtsInOuterCornerModule = 8;
  // actual value set by "Rh1NumPMTInOuterCornerModule"  from xml

} // end namespace Rich1

namespace Rich2 {
  inline static const unsigned int NumRich2Sides = 2;
  // same as "Rh2NumSides"  from xml )

  inline static const unsigned int NumRich2SphMirrorSegTotal = 56;
  // same as "Rh2NumSphMirrorSegments"  from xml
  inline static const unsigned int NumRich2SphMirrorSegSide = NumRich2SphMirrorSegTotal / 2;

  inline static const unsigned int NumRich2SecMirrorSegTotal = 40;
  // same as "Rh2NumSecMirrorSegments" from xml

  inline static const unsigned int NumRich2SecMirrorSegSide = NumRich2SecMirrorSegTotal / 2;

  inline static const unsigned int NumRich2MapmtModulesInAPanel = 72;
  // same as "Rh2TotalNumPMTInPanel"  from xml

  inline static const unsigned int NumRich2MapmtStdModulesInAPanel = 24;
  // same as  "Rh2TotalNumStdPMTModulesInPanel"  from xml

  inline static const unsigned int NumRich2MapmtGrandModulesInAPanel = 48;
  // same as "Rh2TotalNumGrandPMTModulesInPanel" from xml

  // Maximum possible number of PMTs in a given module type
  inline static const unsigned int MaxNumRich2MapmtsInGrandModule = 4;
  // actual value set by  "RhNumGrandPMTInModule" from xml

} // end namespace Rich2
