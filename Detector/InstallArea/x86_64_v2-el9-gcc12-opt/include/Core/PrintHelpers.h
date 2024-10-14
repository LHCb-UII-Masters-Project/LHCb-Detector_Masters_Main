/*****************************************************************************\
* (c) Copyright 2000-2019 CERN for the benefit of the LHCb Collaboration      *
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

namespace LHCb::Detector {

  namespace detail {

    std::string getIndentation( int level );

    enum PrintFlags {
      BASICS     = 1 << 1,
      PARAMS     = 1 << 2,
      DETAIL     = 1 << 4,
      SPECIFIC   = 1 << 5,
      ALIGNMENTS = 1 << 7,
      CHILDREN   = 1 << 8,
      ALL        = BASICS | PARAMS | DETAIL | SPECIFIC | CHILDREN
    };
  } // namespace detail
} // namespace LHCb::Detector
