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

#include "Detector/VP/VPChannelID.h"

namespace LHCb::Detector {

  std::ostream& operator<<( std::ostream& s, const VPChannelID& obj ) {
    return s << "{ "
             << " VPChannelID : " << obj.channelID() << " : row = " << to_unsigned( obj.row() )
             << " col = " << to_unsigned( obj.col() ) << " chip = " << to_unsigned( obj.chip() )
             << " sensor = " << to_unsigned( obj.sensor() ) << " orfx = " << to_unsigned( obj.orfx() )
             << " orfy = " << to_unsigned( obj.orfy() ) << " }";
  }
} // namespace LHCb::Detector
