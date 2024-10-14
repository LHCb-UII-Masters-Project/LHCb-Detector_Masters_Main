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

#include "Detector/Magnet/DeMagnet.h"
#include "Core/PrintHelpers.h"

#include "DD4hep/Printout.h"

#include <stdexcept>

LHCb::Detector::detail::DeMagnetObject::DeMagnetObject(
    const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt,
    std::shared_ptr<LHCb::Magnet::MagneticFieldGrid> magneticFieldGrid, bool useRealMap, bool isDown,
    double signedRelativeCurrent )
    : DeIOVObject( de, ctxt, 9000000 )
    , m_magneticFieldGrid{std::move( magneticFieldGrid )}
    , m_useRealMap{useRealMap}
    , m_isDown{isDown}
    , m_signedRelativeCurrent{signedRelativeCurrent} {
  if ( !m_magneticFieldGrid ) {
    throw std::invalid_argument{"Magnetic Field Grid passed to DeMagnetObject should not be nullptr"};
  }
}
