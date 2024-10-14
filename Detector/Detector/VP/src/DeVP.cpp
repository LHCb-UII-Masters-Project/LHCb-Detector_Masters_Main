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

#include "Detector/VP/DeVP.h"
#include "Core/PrintHelpers.h"

#include "DD4hep/Printout.h"

LHCb::Detector::detail::DeVPObject::DeVPObject( const dd4hep::DetElement&             de,
                                                dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeIOVObject( de, ctxt, 1008106 )
    , // matches DetDesc CLID_VeloAlignCond
    sides{{{de.child( "MotionVPLeft" ).child( "VPLeft" ), ctxt, true},
           {de.child( "MotionVPRight" ).child( "VPRight" ), ctxt, false}}} {
  // fill caches
  for ( auto& side : sides ) {
    for ( auto& module : side.modules ) {
      for ( auto& ladder : module.ladders ) {
        auto& sensor = ladder.sensor;
        // Take care of the different convention between ROOT and LHCb for lengths (cm vs mm)
        // get the local to global transformation matrix and
        // store it in a flat float array of sixe 12.
        ROOT::Math::Rotation3D ltg_rot;
        ROOT::Math::XYZVector  ltg_trans;
        DeVPSensor::getGlobalMatrixDecomposition( sensor, ltg_rot, ltg_trans );
        assert( to_unsigned( sensor.sensorNumber ) < m_ltg.size() );
        auto& ltg = m_ltg[to_unsigned( sensor.sensorNumber )];
        ltg_rot.GetComponents( std::begin( ltg ) );
        ltg_trans.GetCoordinates( ltg.data() + 9 );
        assert( to_unsigned( sensor.sensorNumber ) < m_c2.size() );
        assert( to_unsigned( sensor.sensorNumber ) < m_s2.size() );
        const auto vg =
            sensor.detectorAlignment().localToWorld( {1, 0, 0} ) - sensor.detectorAlignment().localToWorld( {0, 0, 0} );
        m_c2[to_unsigned( sensor.sensorNumber )] = vg.x() * vg.x(); // cos^2
        m_s2[to_unsigned( sensor.sensorNumber )] = vg.y() * vg.y(); // sin^2
      }
    }
  }
  // Compute the center of the moving velo and call it 'beamSpot'
  const auto leftpos  = this->toGlobal( sides[0].motionSystemTransform().value()( ROOT::Math::XYZPoint{0, 0, 0} ) );
  const auto rightpos = this->toGlobal( sides[1].motionSystemTransform().value()( ROOT::Math::XYZPoint{0, 0, 0} ) );
  m_beamSpot = ROOT::Math::XYZPoint{0.5 * ( ROOT::Math::XYZVector{leftpos} + ROOT::Math::XYZVector{rightpos} )};
  // Computes whether velo is closed
  m_isVeloClosed = ( std::abs( leftpos.x() - m_beamSpot.x() ) < 5 * dd4hep::mm &&
                     std::abs( rightpos.x() - m_beamSpot.x() ) < 5 * dd4hep::mm );
}

LHCb::Detector::detail::DeVPSideObject::DeVPSideObject( const dd4hep::DetElement&             de,
                                                        dd4hep::cond::ConditionUpdateContext& ctxt, bool isLeft )
    : DeIOVObject( de, ctxt, 1008106 ) // matches DetDesc CLID_VeloAlignCond
    , modules{{{de.child( isLeft ? "Module01" : "Module00" ), ctxt, isLeft},
               {de.child( isLeft ? "Module03" : "Module02" ), ctxt, isLeft},
               {de.child( isLeft ? "Module05" : "Module04" ), ctxt, isLeft},
               {de.child( isLeft ? "Module07" : "Module06" ), ctxt, isLeft},
               {de.child( isLeft ? "Module09" : "Module08" ), ctxt, isLeft},
               {de.child( isLeft ? "Module11" : "Module10" ), ctxt, isLeft},
               {de.child( isLeft ? "Module13" : "Module12" ), ctxt, isLeft},
               {de.child( isLeft ? "Module15" : "Module14" ), ctxt, isLeft},
               {de.child( isLeft ? "Module17" : "Module16" ), ctxt, isLeft},
               {de.child( isLeft ? "Module19" : "Module18" ), ctxt, isLeft},
               {de.child( isLeft ? "Module21" : "Module20" ), ctxt, isLeft},
               {de.child( isLeft ? "Module23" : "Module22" ), ctxt, isLeft},
               {de.child( isLeft ? "Module25" : "Module24" ), ctxt, isLeft},
               {de.child( isLeft ? "Module27" : "Module26" ), ctxt, isLeft},
               {de.child( isLeft ? "Module29" : "Module28" ), ctxt, isLeft},
               {de.child( isLeft ? "Module31" : "Module30" ), ctxt, isLeft},
               {de.child( isLeft ? "Module33" : "Module32" ), ctxt, isLeft},
               {de.child( isLeft ? "Module35" : "Module34" ), ctxt, isLeft},
               {de.child( isLeft ? "Module37" : "Module36" ), ctxt, isLeft},
               {de.child( isLeft ? "Module39" : "Module38" ), ctxt, isLeft},
               {de.child( isLeft ? "Module41" : "Module40" ), ctxt, isLeft},
               {de.child( isLeft ? "Module43" : "Module42" ), ctxt, isLeft},
               {de.child( isLeft ? "Module45" : "Module44" ), ctxt, isLeft},
               {de.child( isLeft ? "Module47" : "Module46" ), ctxt, isLeft},
               {de.child( isLeft ? "Module49" : "Module48" ), ctxt, isLeft},
               {de.child( isLeft ? "Module51" : "Module50" ), ctxt, isLeft}}} {

  // Finding the detector parent
  auto              parent    = detector().parent();
  auto              kalign    = LHCb::Detector::ConditionKey( parent, Keys::alignmentKey );
  dd4hep::Alignment alignment = ctxt.condition( kalign );
  TGeoHMatrix       geoDelta{};
  alignment.delta().computeMatrix( geoDelta );
  auto delta              = lhcb::geometrytools::toTransform3D( geoDelta );
  m_motionSystemTransform = detail::toLHCbLengthUnits( delta );
}

LHCb::Detector::detail::DeVPModuleObject::DeVPModuleObject( const dd4hep::DetElement&             de,
                                                            dd4hep::cond::ConditionUpdateContext& ctxt, bool isLeft )
    : DeIOVObject( de, ctxt, 1008106 ) // matches DetDesc CLID_VeloAlignCond
    , ladders{{{de.child( "ladder_0" ), ctxt, (unsigned int)de.id(), isLeft},
               {de.child( "ladder_1" ), ctxt, (unsigned int)de.id(), isLeft},
               {de.child( "ladder_2" ), ctxt, (unsigned int)de.id(), isLeft},
               {de.child( "ladder_3" ), ctxt, (unsigned int)de.id(), isLeft}}} {
  zpos = toGlobal( ROOT::Math::XYZPoint{0.0, 0.0, 0.0} ).z();
}

LHCb::Detector::detail::DeVPLadderObject::DeVPLadderObject( const dd4hep::DetElement&             de,
                                                            dd4hep::cond::ConditionUpdateContext& ctxt,
                                                            unsigned int moduleId, bool isLeft )
    : DeIOVObject( de, ctxt, 1008106 ) // matches DetDesc CLID_VeloAlignCond
    , sensor{de.child( "sensor" ), ctxt, moduleId, isLeft} {}

int LHCb::Detector::detail::DeVPObject::sensitiveVolumeID( const ROOT::Math::XYZPoint& point ) const {
  auto module = findModuleFromZ( point.z() );
  if ( !module ) { return -1; }

  for ( auto& ladder : module->ladders ) {
    if ( ladder.sensor.isInside( point ) ) { return to_unsigned( ladder.sensor.sensorNumber ); }
  }
  return -1;
}

const LHCb::Detector::detail::DeVPModuleObject*
LHCb::Detector::detail::DeVPObject::findModuleFromZ( const double z ) const {
  // z-extent of module (substrate plus sensor ladders) plus some headroom
  constexpr double moduleSize{2.0 * dd4hep::mm};
  for ( auto& side : sides ) {
    for ( auto& module : side.modules ) {
      if ( std::abs( z - module.zpos ) < moduleSize ) { return &module; }
    }
  }
  return nullptr;
}

void LHCb::Detector::detail::DeVPObject::print( int indent, int flg ) const {
  std::string prefix = getIndentation( indent );
  DeIOVObject::print( indent, flg );
  unsigned int i = 0;
  for ( auto& side : sides ) {
    for ( auto& module : side.modules ) {
      for ( auto& ladder : module.ladders ) {
        dd4hep::printout( dd4hep::INFO, "DeVeloGenStatic", "%s+   >> [%ld] %s", prefix.c_str(), i,
                          ladder.sensor.detector().path().c_str() );
        i++;
      }
    }
  }
  dd4hep::printout( dd4hep::INFO, "DeVP", "%s >> Sensors:%ld", prefix.c_str(), detector().path().c_str(),
                    VP::NSensors );
  for ( auto& side : sides ) {
    for ( auto& module : side.modules ) {
      for ( auto& ladder : module.ladders ) { ladder.sensor.print( indent + 1, flg ); }
    }
  }
}
