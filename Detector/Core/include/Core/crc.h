/*****************************************************************************\
* (c) Copyright 2022 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once

// Using the gcc instrinsic for crc32 computation
#if defined( __x86_64 )
namespace lhcb::Detector::crc {
  long checksum_long( long checksum, long value ) { return __builtin_ia32_crc32di( checksum, value ); }
} // namespace lhcb::Detector::crc
#elif defined( __aarch64__ )
// we need at least --march=armv8_a+crc or --march=armv8.1_a
#  include <arm_acle.h>
namespace lhcb::Detector::crc {
  long checksum_long( long checksum, long value ) { return __crc32cd( checksum, value ); }
} // namespace lhcb::Detector::crc
#else
#  error crc32 algorithm not available on the current platform, please file a bug report
#endif
