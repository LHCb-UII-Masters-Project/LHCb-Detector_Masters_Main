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

#include "Detector/UT/ChannelID.h"

#include <sstream>

//-----------------------------------------------------------------------------
// Implementation file for class : UTChannelID
//
// 2018-09-04 : A Beiter (based on code by M Needham)
//-----------------------------------------------------------------------------
namespace LHCb::Detector::UT {

  std::string ChannelID::toString() const {
    std::ostringstream o;
    o << *this;
    return o.str();
  }

  std::ostream& operator<<( std::ostream& s, const ChannelID& obj ) {
    s << "{ ";
    if ( obj.isUT() ) s << "UT ";
    return s << "UTChannelID : " << obj.channelID() << " : type=" << obj.type() << " strip=" << obj.strip()
             << " sector=" << obj.sector() << " module=" << obj.module() << " face=" << obj.face()
             << " stave=" << obj.stave() << " layer=" << obj.layer() << " side=" << obj.side() << " }";
  }
} // namespace LHCb::Detector::UT
