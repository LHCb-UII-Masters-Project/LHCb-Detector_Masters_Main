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

// Framework include files
#include "Detector/FT/DeFT.h"
#include "Detector/FT/FTChannelID.h"
#include "Detector/FT/FTSourceID.h"
#include <Core/Keys.h>
#include <yaml-cpp/yaml.h>

#include "DD4hep/Printout.h"

#include <cassert>
#include <cmath>

LHCb::Detector::detail::DeFTObject::DeFTObject( const dd4hep::DetElement&             de,
                                                dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeIOVObject( de, ctxt )
    , m_stations{{{de.child( "T1" ), ctxt, 1}, {de.child( "T2" ), ctxt, 2}, {de.child( "T3" ), ctxt, 3}}} {}

LHCb::Detector::detail::DeFTStationObject::DeFTStationObject( const dd4hep::DetElement&             de,
                                                              dd4hep::cond::ConditionUpdateContext& ctxt,
                                                              unsigned int                          iStation )
    : DeIOVObject( de, ctxt )
    , m_id{iStation}
    , m_layers{{{de.child( "X1" ), ctxt, 0, m_id},
                {de.child( "U" ), ctxt, 1, m_id},
                {de.child( "V" ), ctxt, 2, m_id},
                {de.child( "X2" ), ctxt, 3, m_id}}} {}

LHCb::Detector::detail::DeFTLayerObject::DeFTLayerObject( const dd4hep::DetElement&             de,
                                                          dd4hep::cond::ConditionUpdateContext& ctxt,
                                                          unsigned int iLayer, unsigned int stationID )
    : DeIOVObject( de, ctxt )
    , m_layerID{( stationID * FT::nLayers + iLayer )}
    , m_halflayers{{{de.child( "HL0" ), ctxt, 0, m_layerID}, {de.child( "HL1" ), ctxt, 1, m_layerID}}} {

  const auto g1    = this->toGlobal( ROOT::Math::XYZPoint{0., 0., 0.} );
  const auto g2    = this->toGlobal( ROOT::Math::XYZPoint{1., 0., 0.} );
  const auto g3    = this->toGlobal( ROOT::Math::XYZPoint{0., 1., 0.} );
  m_globalZ        = g1.z();
  m_plane          = ROOT::Math::Plane3D( g1, g2, g3 );
  const auto delta = g1 - g3;
  m_dzdy           = delta.z() / delta.y();
  if ( iLayer == 1 ) {
    m_stereoAngle = dd4hep::_toFloat( "FT:StereoAngleU" );
  } else if ( iLayer == 2 ) {
    m_stereoAngle = dd4hep::_toFloat( "FT:StereoAngleV" );
  } else {
    m_stereoAngle = dd4hep::_toFloat( "FT:StereoAngleX" );
  }
  m_dxdy = -std::tan( m_stereoAngle );
}

// Implement Halflayer object
LHCb::Detector::detail::DeFTHalfLayerObject::DeFTHalfLayerObject( const dd4hep::DetElement&             de,
                                                                  dd4hep::cond::ConditionUpdateContext& ctxt,
                                                                  unsigned int iHalfLayer, unsigned int layerID )
    : DeIOVObject( de, ctxt )
    , m_halflayerID{( layerID * FT::nHalfLayers + iHalfLayer )}
    , m_quarters{{{de.child( iHalfLayer == 0 ? "Q0" : "Q1" ), ctxt, iHalfLayer, layerID},
                  {de.child( iHalfLayer == 0 ? "Q2" : "Q3" ), ctxt, iHalfLayer + 2, layerID}}} {

  // Get the global z position of the layer
  ROOT::Math::XYZPoint globalPoint = this->toGlobal( ROOT::Math::XYZPoint( 0., 0., 0. ) );
  m_globalZ                        = globalPoint.z();

  // Make the plane for the half layer
  const ROOT::Math::XYZPoint g1 = this->toGlobal( ROOT::Math::XYZPoint( 0., 0., 0. ) );
  const ROOT::Math::XYZPoint g2 = this->toGlobal( ROOT::Math::XYZPoint( 1., 0., 0. ) );
  const ROOT::Math::XYZPoint g3 = this->toGlobal( ROOT::Math::XYZPoint( 0., 1., 0. ) );
  m_plane                       = ROOT::Math::Plane3D( g1, g2, g3 );
  m_dzdy                        = ( g3.z() - g1.z() ) / ( g3.y() - g1.y() );
  m_dxdy                        = ( g3.x() - g1.x() ) / ( g3.y() - g1.y() );
  m_stereoAngle                 = -atan( m_dxdy );
}

LHCb::Detector::detail::DeFTQuarterObject::DeFTQuarterObject( const dd4hep::DetElement&             de,
                                                              dd4hep::cond::ConditionUpdateContext& ctxt,
                                                              unsigned int iQuarter, unsigned int layerID )
    : DeIOVObject( de, ctxt ), m_id{layerID * FT::nQuarters + iQuarter}, m_modules{} {

  m_modules.emplace_back( de.child( "M0" ), ctxt, 0, m_id );
  m_modules.emplace_back( de.child( "M1" ), ctxt, 1, m_id );
  m_modules.emplace_back( de.child( "M2" ), ctxt, 2, m_id );
  m_modules.emplace_back( de.child( "M3" ), ctxt, 3, m_id );
  m_modules.emplace_back( de.child( "M4" ), ctxt, 4, m_id );
  // Only T3 has this module, we check whether we have it defined
  const auto& c = de.children();
  if ( c.find( "M5" ) != c.end() ) { m_modules.emplace_back( de.child( "M5" ), ctxt, 5, m_id ); }

  // Quarters are not aligned. To still get a better estimate of the global quarter
  // position, the aligned module quantities are averaged.
  this->applyToAllChildren( [&]( const auto& module ) {
    const auto m1     = module.toGlobal( ROOT::Math::XYZPoint{0., 0., 0.} );
    const auto m2     = module.toGlobal( ROOT::Math::XYZPoint{0., 1., 0.} );
    const auto deltaY = m1 - m2;
    m_meanModuleDxdy += deltaY.x() / deltaY.y();
    m_meanModuleDzdy += deltaY.z() / deltaY.y();
    // get the module shape as a box
    const dd4hep::Box& containing_box = module.detector().solid();
    // the y function gives half the y extent of the box, i.e. double it
    const auto sizeY = 2 * containing_box.y();
    // the central point is in the centre of the module, so go down half the size in y
    // to get the z position at the mirror
    m_meanModuleZ += module.toGlobal( ROOT::Math::XYZPoint( 0, -0.5f * sizeY, 0. ) ).z();
  } );
  m_meanModuleDxdy /= this->m_modules.size();
  m_meanModuleDzdy /= this->m_modules.size();
  m_meanModuleZ /= this->m_modules.size();
  const auto g1 = this->toGlobal( ROOT::Math::XYZPoint{0., 0., 0.} );
  const auto g2 = this->toGlobal( ROOT::Math::XYZPoint{1., 0., 0.} );
  const auto g3 = this->toGlobal( ROOT::Math::XYZPoint{0., 1., 0.} );
  m_plane       = ROOT::Math::Plane3D( g1, g2, g3 );
}

LHCb::Detector::detail::DeFTModuleObject::DeFTModuleObject( const dd4hep::DetElement&             de,
                                                            dd4hep::cond::ConditionUpdateContext& ctxt,
                                                            unsigned int iModule, unsigned int quarterID )
    : DeIOVObject( de, ctxt )
    , m_id{quarterID * FT::nModulesMax + iModule}
    , m_mats{{{de.child( "Mat0" ), ctxt, 0, m_id},
              {de.child( "Mat1" ), ctxt, 1, m_id},
              {de.child( "Mat2" ), ctxt, 2, m_id},
              {de.child( "Mat3" ), ctxt, 3, m_id}}} {
  const auto leftPoint         = this->toGlobal( ROOT::Math::XYZPoint{1, 0, 0} );  // point on the left
  const auto rightPoint        = this->toGlobal( ROOT::Math::XYZPoint{-1, 0, 0} ); // point on the right
  bool       m_centre_to_outer = std::abs( rightPoint.x() ) > std::abs( leftPoint.x() );
  m_reversed                   = m_centre_to_outer;
  // Make the plane for the module
  const ROOT::Math::XYZPoint g1 = this->toGlobal( ROOT::Math::XYZPoint( 0., 0., 0. ) );
  const ROOT::Math::XYZPoint g2 = this->toGlobal( ROOT::Math::XYZPoint( 1., 0., 0. ) );
  const ROOT::Math::XYZPoint g3 = this->toGlobal( ROOT::Math::XYZPoint( 0., 1., 0. ) );
  m_plane                       = ROOT::Math::Plane3D( g1, g2, g3 );

  // Derive the module id/quarter id/layer id/ station id
  auto tmp = m_id;
  auto m   = tmp % FT::nModulesMax;
  tmp /= FT::nModulesMax;
  auto q = tmp % FT::nQuarters;
  tmp /= FT::nQuarters;
  auto l = tmp % FT::nLayers;
  tmp /= FT::nLayers;
  auto s = tmp;
  // Validation debug

  auto localModuleID  = FTChannelID::ModuleID{static_cast<unsigned int>( m )};
  auto localQuarterID = FTChannelID::QuarterID{static_cast<unsigned int>( q )};
  auto localLayerID   = FTChannelID::LayerID{static_cast<unsigned int>( l )};
  auto localStationID = FTChannelID::StationID{static_cast<unsigned int>( s )};
  m_elementID         = FTChannelID( localStationID, localLayerID, localQuarterID, localModuleID, 0 );
}

LHCb::Detector::detail::DeFTMatObject::DeFTMatObject( const dd4hep::DetElement&             de,
                                                      dd4hep::cond::ConditionUpdateContext& ctxt, unsigned int iMat,
                                                      unsigned int moduleID )
    : DeIOVObject( de, ctxt ), m_matID{iMat} {
  // Element ID
  auto tmp = moduleID;
  auto m   = tmp % FT::nModulesMax;
  tmp /= FT::nModulesMax;
  auto q = tmp % FT::nQuarters;
  tmp /= FT::nQuarters;
  auto l = tmp % FT::nLayers;
  tmp /= FT::nLayers;
  auto s              = tmp;
  auto localModuleID  = FTChannelID::ModuleID{static_cast<unsigned int>( m )};
  auto localQuarterID = FTChannelID::QuarterID{static_cast<unsigned int>( q )};
  auto localLayerID   = FTChannelID::LayerID{static_cast<unsigned int>( l )};
  auto localStationID = FTChannelID::StationID{static_cast<unsigned int>( s )};
  m_elementID =
      FTChannelID( localStationID, localLayerID, localQuarterID, localModuleID, FTChannelID::MatID{iMat}, 0, 0 );
  // Get some useful geometric parameters from the database
  m_halfChannelPitch = 0.5f * m_channelPitch;

  m_sipmPitch      = m_nChannelsInSiPM * m_channelPitch + m_dieGap + 2 * m_airGap + 2 * m_deadRegion;
  m_nChannelsInDie = m_nChannelsInSiPM / m_nDiesInSiPM;
  m_diePitch       = m_nChannelsInDie * m_channelPitch + m_dieGap;
  m_uBegin         = m_airGap + m_deadRegion - 2.f * m_sipmPitch;

  const dd4hep::Box& containing_box = de.solid();
  m_sizeY                           = 2 * containing_box.y();
  // Get the central points of the fibres at the mirror and at the SiPM locations
  m_mirrorPoint = this->toGlobal( ROOT::Math::XYZPoint( 0, -0.5f * m_sizeY, 0. ) );
  m_sipmPoint   = this->toGlobal( ROOT::Math::XYZPoint( 0, +0.5f * m_sizeY, 0 ) );
  // Define the global z position to be at the point closest to the mirror
  m_globalZ = m_mirrorPoint.z();

  // Define the global length in y of the mat
  m_globaldy = m_sipmPoint.y() - m_mirrorPoint.y();

  // Make the plane for the mat
  const ROOT::Math::XYZPoint g1 = this->toGlobal( ROOT::Math::XYZPoint( 0., 0., 0. ) );
  const ROOT::Math::XYZPoint g2 = this->toGlobal( ROOT::Math::XYZPoint( 1., 0., 0. ) );
  const ROOT::Math::XYZPoint g3 = this->toGlobal( ROOT::Math::XYZPoint( 0., 1., 0. ) );
  m_plane                       = ROOT::Math::Plane3D( g1, g2, g3 );

  // Get the slopes in units of local delta x
  m_ddx = ROOT::Math::XYZVectorF( g2 - g1 );
  // Get the slopes in units of delta y (needed by PrFTHit, mind the sign)
  ROOT::Math::XYZVectorF deltaY( g1 - g3 );
  m_dxdy = deltaY.x() / deltaY.y();
  m_dzdy = deltaY.z() / deltaY.y();

  // get the mat contraction conditions
  m_matContractionParameterVector.reserve( m_nChannelsInSiPM * m_nSiPMsInMat );
  auto              deFT                     = dd4hep::Detector::getInstance().detector( "FT" );
  const std::string contractionConditionName = getContractionConditionName();
  m_matContractionCondition = ctxt.condition( dd4hep::ConditionKey::KeyMaker( deFT, "matContraction" ).hash );
  auto& params              = m_matContractionCondition.get<nlohmann::json>();
  if ( params.find( contractionConditionName ) != params.end() ) {
    m_matContractionParameterVector = params[contractionConditionName].get<std::vector<double>>();
  } else {
    dd4hep::printout( dd4hep::DEBUG,
                      "Unable to find FT condition %s. It will not be possible to correct for temperature distortions.",
                      getContractionConditionName() );
  }
  m_matContractionParameterVector.shrink_to_fit();
}
