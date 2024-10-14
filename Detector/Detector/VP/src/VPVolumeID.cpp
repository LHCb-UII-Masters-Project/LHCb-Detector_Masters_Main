/*****************************************************************************\
* (c) Copyright 2000-2023 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/

#include "Detector/VP/VPVolumeID.h"

#include <ostream>

namespace LHCb::Detector {

  std::ostream& operator<<( std::ostream& s, const VPVolumeID& obj ) {
    return s << "{ "
             << " VPVolumeID : " << obj.volumeID() << " :"
             << " system = " << to_unsigned( obj.system() ) << " side = " << to_unsigned( obj.side() )
             << " motionside = " << to_unsigned( obj.motionside() ) << " module = " << to_unsigned( obj.module() )
             << " ladder = " << to_unsigned( obj.ladder() ) << " sensor number = " << to_unsigned( obj.sensor() )
             << " }";
  }
} // namespace LHCb::Detector
