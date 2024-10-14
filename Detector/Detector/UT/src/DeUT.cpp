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

#include "Detector/UT/DeUT.h"
#include "Core/PrintHelpers.h"

#include "DD4hep/Printout.h"
#include <fmt/format.h>

LHCb::Detector::UT::Status LHCb::Detector::UT::toStatus( std::string_view str ) {
  auto i = std::find_if( LHCb::Detector::UT::s_map.begin(), LHCb::Detector::UT::s_map.end(),
                         [&]( const auto& i ) { return i.second == str; } );
  return i == LHCb::Detector::UT::s_map.end() ? Status::UnknownStatus : i->first;
}

std::string LHCb::Detector::UT::toString( LHCb::Detector::UT::Status status ) {
  auto i = LHCb::Detector::UT::s_map.find( status );
  return i == LHCb::Detector::UT::s_map.end() ? "UnknownStatus" : i->second;
}

std::ostream& operator<<( std::ostream& os, LHCb::Detector::UT::Status const& s ) {
  return os << LHCb::Detector::UT::s_map.at( s );
}

std::ostream& LHCb::Detector::UT::toStream( LHCb::Detector::UT::Status status, std::ostream& os ) {
  return os << std::quoted( LHCb::Detector::UT::toString( status ), '\'' );
}

std::map<unsigned int, LHCb::Detector::UT::Status> LHCb::Detector::UT::toEnumMap( const std::map<int, int>& input ) {
  std::map<unsigned int, Status> output;
  std::transform( input.begin(), input.end(), std::inserter( output, output.end() ), []( auto in ) {
    return std::pair{in.first, Status( in.second )};
  } );
  return output;
}
std::map<unsigned int, LHCb::Detector::UT::Status> LHCb::Detector::UT::toEnumMap( const std::vector<double>& input ) {
  std::map<unsigned int, Status> output;
  for ( int i = 0; i < static_cast<int>( input.size() ); i++ ) {
    output.insert( std::make_pair( i, Status( static_cast<unsigned int>( input[i] ) ) ) );
  }
  return output;
}

LHCb::Detector::UT::detail::DeUTObject::DeUTObject( const dd4hep::DetElement&             de,
                                                    dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeIOVObject( de, ctxt, 9301 ), m_sides{{{de.child( "Cside" ), ctxt}, {de.child( "Aside" ), ctxt}}} {
  m_sectors.reserve( 2000 );
  // flatten sectors
  applyToAllSectors( [this]( DeUTSector s ) { m_sectors.emplace_back( &*s ); } );
}

LHCb::Detector::UT::detail::DeUTSideObject::DeUTSideObject( const dd4hep::DetElement&             de,
                                                            dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeIOVObject( de, ctxt, 9302 )
    , m_id( de.id() )
    , m_channelID{ChannelID::detType::typeUT, (unsigned int)de.placement().volIDs()[0].second, 0, 0, 0, 0, 0, 0}
    , m_layers{{{de.child( "UTaX" ), ctxt},
                {de.child( "UTaU" ), ctxt},
                {de.child( "UTbV" ), ctxt},
                {de.child( "UTbX" ), ctxt}}} {}

LHCb::Detector::UT::detail::DeUTLayerObject::DeUTLayerObject( const dd4hep::DetElement&             de,
                                                              dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeIOVObject( de, ctxt, 9303 )
    , m_id( de.id() )
    , m_nStaves{de.id() > 1 ? 9u : 8u}
    , m_staves{{{de.child( "Stave0" ), ctxt},
                {de.child( "Stave1" ), ctxt},
                {de.child( "Stave2" ), ctxt},
                {de.child( "Stave3" ), ctxt},
                {de.child( "Stave4" ), ctxt},
                {de.child( "Stave5" ), ctxt},
                {de.child( "Stave6" ), ctxt},
                {de.child( "Stave7" ), ctxt},
                // replicate stave 8 and do not use replica when we have only 8
                {de.id() > 1 ? de.child( "Stave8" ) : de.child( "Stave7" ), ctxt}}}
    , m_channelID{ChannelID::detType::typeUT,
                  (unsigned int)de.parent().placement().volIDs()[0].second,
                  (unsigned int)de.placement().volIDs()[0].second,
                  0,
                  0,
                  0,
                  0,
                  0} {
  { // using first stave stereo angle as layer stereo angle
    const DeUTStave       thisStave = DeUTStave{&m_staves[0]};
    ROOT::Math::XYZVector v1        = ROOT::Math::XYZVector( thisStave->toGlobal( ROOT::Math::XYZPoint( 0, 10, 0 ) ) -
                                                      thisStave->toGlobal( ROOT::Math::XYZPoint( 0, 0, 0 ) ) );
    ROOT::Math::XYZVector v2 =
        ROOT::Math::XYZVector( ROOT::Math::XYZPoint( 0, 10, 0 ) - ROOT::Math::XYZPoint( 0, 0, 0 ) );
    m_angle    = ROOT::Math::VectorUtil::Angle( v1, v2 );
    m_sinAngle = sin( m_angle );
    m_cosAngle = cos( m_angle );
  }
  { // initialize m_plane
    ROOT::Math::XYZPoint p1 = toGlobal( ROOT::Math::XYZPoint( 0, 0, 0 ) );
    ROOT::Math::XYZPoint p2 = toGlobal( ROOT::Math::XYZPoint( 3 * dd4hep::meter, 0, 0 ) );
    ROOT::Math::XYZPoint p3 = toGlobal( ROOT::Math::XYZPoint( 0, 3 * dd4hep::meter, 0 ) );
    m_plane                 = ROOT::Math::Plane3D( p1, p2, p3 );
  }
}

LHCb::Detector::UT::detail::DeUTStaveObject::DeUTStaveObject( const dd4hep::DetElement&             de,
                                                              dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeIOVObject( de, ctxt, 9310 )
    , m_id( de.id() )
    , m_type( de.volume().name(), std::strlen( de.volume().name() ) - 1 )
    , m_faces{{{de.child( "Face0" ), ctxt}, {de.child( "Face1" ), ctxt}}}
    , m_channelID{ChannelID::detType::typeUT,
                  (unsigned int)de.parent().parent().placement().volIDs()[0].second,
                  (unsigned int)de.parent().placement().volIDs()[0].second,
                  (unsigned int)de.placement().volIDs()[0].second,
                  0,
                  0,
                  0,
                  0} {
  // Backward compatibility: column and detRegion from old UTChannelID
  const unsigned int sideID  = m_channelID.side();
  const unsigned int layerID = m_channelID.layer();
  const unsigned int staveID = m_channelID.stave();
  if ( layerID < 2 ) {      // aStation, aX, aU
    if ( sideID == 0 ) {    // C-side
      if ( staveID >= 2 ) { // Region 1 (-x, C-side)
        m_detRegion = 1;
        m_column    = 9 - staveID;
      } else { // Region 2 (C-side)
        m_detRegion = 2;
        m_column    = 2 - staveID;
      }
    } else if ( sideID == 1 ) { // A-side
      if ( staveID < 2 ) {      // Region 2 (A-side)
        m_detRegion = 2;
        m_column    = staveID + 3;
      } else { // Region 3 (+x, A-side)
        m_detRegion = 3;
        m_column    = staveID - 1;
      }
    }
  } else if ( layerID >= 2 ) { // bStation, bV, bX
    if ( sideID == 0 ) {       // C-side
      if ( staveID >= 2 ) {    // Region 1 (-x, C-side)
        m_detRegion = 1;
        m_column    = 9 - staveID;
      } else { // Region 2 (C-side)
        m_detRegion = 2;
        m_column    = 2 - staveID;
      }
    } else if ( sideID == 1 ) { // A-side
      if ( staveID < 2 ) {      // Region 2 (A-side)
        m_detRegion = 2;
        m_column    = staveID + 3;
      } else { // Region 3 (+x, A-side)
        m_detRegion = 3;
        m_column    = staveID - 1;
      }
    }
  }
  m_nickname    = getStaveName( m_channelID );
  m_nicknameOld = getStaveNameOld( m_channelID );
}

LHCb::Detector::UT::detail::DeUTFaceObject::DeUTFaceObject( const dd4hep::DetElement&             de,
                                                            dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeIOVObject( de, ctxt, 9311 )
    , m_id( de.id() )
    , m_modules{}
    , m_channelID{ChannelID::detType::typeUT,
                  (unsigned int)de.parent().parent().parent().placement().volIDs()[0].second,
                  (unsigned int)de.parent().parent().placement().volIDs()[0].second,
                  (unsigned int)de.parent().placement().volIDs()[0].second,
                  (unsigned int)de.placement().volIDs()[0].second,
                  0,
                  0,
                  0} {
  for ( auto& module : de.children() ) { m_modules.emplace_back( module.second, ctxt ); }
}

LHCb::Detector::UT::detail::DeUTModuleObject::DeUTModuleObject( const dd4hep::DetElement&             de,
                                                                dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeIOVObject( de, ctxt, 9312 )
    , m_id( de.id() )
    , m_sectors{}
    , m_channelID{ChannelID::detType::typeUT,
                  (unsigned int)de.parent().parent().parent().parent().placement().volIDs()[0].second,
                  (unsigned int)de.parent().parent().parent().placement().volIDs()[0].second,
                  (unsigned int)de.parent().parent().placement().volIDs()[0].second,
                  (unsigned int)de.parent().placement().volIDs()[0].second,
                  (unsigned int)de.placement().volIDs()[0].second,
                  0,
                  0} {
  // type of stave
  auto staveVolName = de.parent().parent().volume().name(); // lvStaveA/B/C
  m_type            = std::string( staveVolName + 8, staveVolName + std::strlen( staveVolName ) );
  // flatten
  for ( auto& group : de.children() )
    for ( auto& sector : group.second.children() ) m_sectors.emplace_back( sector.second, ctxt );
}

LHCb::Detector::UT::detail::DeUTSectorObject::DeUTSectorObject( const dd4hep::DetElement&             de,
                                                                dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeIOVObject( de, ctxt, 9320 )
    , m_id( de.id() )
    , m_sensor{de.child( "Sensor0" ), ctxt}
    , m_channelID{ChannelID::detType::typeUT,
                  (unsigned int)de.parent().parent().parent().parent().parent().parent().placement().volIDs()[0].second,
                  (unsigned int)de.parent().parent().parent().parent().parent().placement().volIDs()[0].second,
                  (unsigned int)de.parent().parent().parent().parent().placement().volIDs()[0].second,
                  (unsigned int)de.parent().parent().parent().placement().volIDs()[0].second,
                  (unsigned int)de.parent().parent().placement().volIDs()[0].second,
                  (unsigned int)de.placement().volIDs()[0].second,
                  0} {
  // m_pitch
  const auto& boxSector = de.volume().boundingBox();
  m_halfLengths         = {boxSector.x(), boxSector.y()};
  m_thickness           = m_sensor.detector().volume().boundingBox().z() * 2;
  m_nickname            = getSectorName();

  auto        nameString = de.placement().volume().name();
  std::string m_type( nameString + 8, nameString + std::strlen( nameString ) );
  if ( m_type == "Norm" ) {
    m_pitch = dd4hep::_toDouble( "UTSensorAPitch" );
  } else {
    m_pitch = dd4hep::_toDouble( "UTSensorBCDPitch" );
  }

  { // cacheInfo
    auto thisSector = DeUTSector{this};
    auto firstTraj  = thisSector.createTraj( thisSector.firstStrip(), 0 );
    // if ( m_stripflip && xInverted() ) firstTraj = createTraj( m_nStrip, 0 );
    if ( thisSector.stripflip() && thisSector.xInverted() )
      firstTraj = thisSector.createTraj( thisSector.nStrip() + thisSector.firstStrip(), 0 );

    // get the start and end point. for piecewise trajectories, we
    // effectively make an approximation by a straight line.
    const ROOT::Math::XYZPoint g1 = firstTraj.beginPoint();
    const ROOT::Math::XYZPoint g2 = firstTraj.endPoint();

    const double activeWidth = DeUTSensor{&m_sensor}.activeWidth();

    // direction
    ROOT::Math::XYZVector direction = g2 - g1;
    m_stripLength                   = std::sqrt( direction.Mag2() );
    direction                       = direction.Unit();

    // cross with normal along z
    ROOT::Math::XYZVector zVec( 0, 0, 1 );
    ROOT::Math::XYZVector norm = direction.Cross( zVec );

    // trajectory of middle
    const ROOT::Math::XYZPoint g3 = g1 + 0.5 * ( g2 - g1 );
    const ROOT::Math::XYZPoint g4 = g3 + activeWidth * norm;

    // creating the 'fast' trajectories
    const ROOT::Math::XYZVector vectorlayer = ( g4 - g3 ).unit() * m_pitch;
    const ROOT::Math::XYZPoint  p0          = g3 - 0.5 * m_stripLength * direction;
    m_dxdy                                  = direction.x() / direction.y();
    m_dzdy                                  = direction.z() / direction.y();
    m_dy                                    = m_stripLength * direction.y();
    m_dp0di.SetX( vectorlayer.x() - vectorlayer.y() * m_dxdy );
    m_dp0di.SetY( vectorlayer.y() );
    m_dp0di.SetZ( vectorlayer.z() - vectorlayer.y() * m_dzdy );
    m_p0.SetX( p0.x() - p0.y() * m_dxdy );
    m_p0.SetY( p0.y() );
    m_p0.SetZ( p0.z() - p0.y() * m_dzdy );

    // Update the stereo angle. We correct by 'pi' if necessary.
    ROOT::Math::XYZVector dir = direction;
    if ( dir.y() < 0 ) dir *= -1;
    m_angle    = atan2( -dir.x(), dir.y() );
    m_cosAngle = cos( m_angle );
    m_sinAngle = sin( m_angle );
  }

  // get status & noise condition
  auto deUT             = de.parent().parent().parent().parent().parent().parent().parent();
  m_statusCondition     = ctxt.condition( dd4hep::ConditionKey::KeyMaker( deUT, "ReadoutSectors" ).hash, false );
  m_noiseCondition      = ctxt.condition( dd4hep::ConditionKey::KeyMaker( deUT, "NoiseValues" ).hash, false );
  m_readoutMapCondition = ctxt.condition( dd4hep::ConditionKey::KeyMaker( deUT, "ReadoutMap" ).hash );
  if ( m_statusCondition.isValid() ) {
    const auto& node = m_statusCondition.get<nlohmann::json>();
    if ( !node.is_null() && node.find( m_nickname ) != node.end() ) {
      m_status       = Status( node[m_nickname]["SectorStatus"].get<int>() );
      m_beetleStatus = toEnumMap( node[m_nickname]["SALTStatus"].get<std::vector<double>>() );
      m_stripStatus  = toEnumMap( node[m_nickname]["StripStatus"].get<std::vector<double>>() );
      m_measEff      = node[m_nickname]["measuredEff"].get<double>();
    } else {
      // NOTE: Don't want to annoy people with warnings, thus change to DEBUG
      // These conditions are needed in simulation but not for data-taking
      dd4hep::printout( dd4hep::DEBUG, "DeUTSector", "Status condition not provided for " + m_nickname );
    }
  }
  if ( m_noiseCondition.isValid() ) {
    const auto& node = m_noiseCondition.get<nlohmann::json>();
    if ( node.find( m_nickname ) != node.end() ) {
      if ( node[m_nickname]["electronsPerADC"].is_number() ) {
        m_electronsPerADC = node[m_nickname]["electronsPerADC"].get<double>();
      } else {
        // old electronsPerADC conditions uses std::vector<double>
        // but all elements are identical, so only take the first one
        m_electronsPerADC = node[m_nickname]["electronsPerADC"].get<std::vector<double>>()[0];
      }
      m_noiseValues  = node[m_nickname]["SectorNoise"].get<std::vector<double>>();
      m_cmModeValues = node[m_nickname]["cmNoise"].get<std::vector<double>>();
    } else {
      // NOTE: Don't want to annoy people with warnings, thus change to DEBUG
      // These conditions are needed in simulation but not for data-taking
      dd4hep::printout( dd4hep::DEBUG, "DeUTSector", "Noise condition not provided for " + m_nickname );
    }
  }

  // Loading hit error scaling factors
  m_hitErrorCondition = ctxt.condition( dd4hep::ConditionKey::KeyMaker( deUT, "HitError" ).hash, false );
  if ( m_hitErrorCondition.isValid() ) {
    auto node = m_hitErrorCondition.get<nlohmann::json>();
    if ( !node.is_null() ) { m_hitErrorFactors = node["HitErrorFactors"].get<std::array<double, 4>>(); }
  } else {
    dd4hep::printout( dd4hep::DEBUG, "DeUTSector", "Unable to find UT HitError conditions." );
  }
  std::ostringstream oss;
  for ( auto const error : m_hitErrorFactors ) { oss << error << " "; }
  dd4hep::printout( dd4hep::DEBUG, "DeUTSector", m_nickname + " using hit errors: " + oss.str() );
}

LHCb::Detector::UT::detail::DeUTSensorObject::DeUTSensorObject( const dd4hep::DetElement&             de,
                                                                dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeIOVObject( de, ctxt, 9330 )
    , m_id( de.id() )
    , m_channelID{ChannelID::detType::typeUT,
                  (unsigned int)de.parent()
                      .parent()
                      .parent()
                      .parent()
                      .parent()
                      .parent()
                      .parent()
                      .placement()
                      .volIDs()[0]
                      .second,
                  (unsigned int)de.parent().parent().parent().parent().parent().parent().placement().volIDs()[0].second,
                  (unsigned int)de.parent().parent().parent().parent().parent().placement().volIDs()[0].second,
                  (unsigned int)de.parent().parent().parent().parent().placement().volIDs()[0].second,
                  (unsigned int)de.parent().parent().parent().placement().volIDs()[0].second,
                  (unsigned int)de.parent().placement().volIDs()[0].second,
                  (unsigned int)de.placement().volIDs()[0].second} {
  // UTSensorObject actually for half sensors for TypeBCD

  // get pitch size
  auto        logVolName       = de.placement().volume().name();
  auto        moduleLogVolName = de.parent().parent().parent().placement().volume().name();
  std::string m_type( logVolName + 8, logVolName + std::strlen( logVolName ) ); // lvSensor(Norm|Dual|Quad|Hole)
  m_sensorType = moduleLogVolName[std::strlen( moduleLogVolName ) - 1];         // lvModule(A|B|C|D)
  m_pitch      = m_type == "Norm" ? dd4hep::_toDouble( "UTSensorAPitch" ) : dd4hep::_toDouble( "UTSensorBCDPitch" );

  m_uMaxLocal   = 0.5f * ( m_pitch * m_nStrip );
  m_uMinLocal   = -m_uMaxLocal;
  m_deadWidth   = 0.0; // LHCb/Det/UTDet: m_deadWidth = param<double>( "verticalGuardRing" );
  m_vMaxLocal   = de.volume().boundingBox().y() - m_deadWidth;
  m_vMinLocal   = -m_vMaxLocal;
  m_stripLength = fabs( m_vMaxLocal - m_vMinLocal );
  m_thickness   = de.volume().boundingBox().z() * 2;

  {
    auto const g1 = toGlobal( ROOT::Math::XYZPoint{m_uMinLocal, m_vMinLocal, 0.} );
    auto const g2 = toGlobal( ROOT::Math::XYZPoint{m_uMaxLocal, m_vMinLocal, 0.} );
    auto const g3 = toGlobal( ROOT::Math::XYZPoint{m_uMinLocal, m_vMaxLocal, 0.} );
    m_xInverted   = ( g1.x() > g2.x() );
    m_yInverted   = ( g1.y() > g3.y() );
  }

  {
    float yUpper = m_vMaxLocal;
    float yLower = m_vMinLocal;
    if ( m_yInverted ) std::swap( yUpper, yLower );

    float xUpper = m_uMaxLocal - 0.5f * m_pitch;
    float xLower = m_uMinLocal + 0.5f * m_pitch;
    if ( m_xInverted ) std::swap( xUpper, xLower );

    // direction
    auto const g1 = toGlobal( ROOT::Math::XYZPoint( {xLower, yLower, 0.} ) );
    auto const g2 = toGlobal( ROOT::Math::XYZPoint( {xLower, yUpper, 0.} ) );
    m_direction   = ( g2 - g1 ).Unit();

    // trajectory of middle
    auto const g3 = toGlobal( ROOT::Math::XYZPoint( {xLower, 0., 0.} ) );
    auto const g4 = toGlobal( ROOT::Math::XYZPoint( {xUpper, 0., 0.} ) );
    m_midTraj.emplace( g3, g4 );

    // range ---> strip Length
    m_range = std::make_pair( -0.5f * m_stripLength, 0.5f * m_stripLength );

    // plane
    m_plane = ROOT::Math::Plane3D( g1, g2, g4 );

    m_entryPlane =
        ROOT::Math::Plane3D( m_plane.Normal(), toGlobal( ROOT::Math::XYZPoint( {0., 0., -0.5f * m_thickness} ) ) );
    m_exitPlane =
        ROOT::Math::Plane3D( m_plane.Normal(), toGlobal( ROOT::Math::XYZPoint( {0., 0., 0.5f * m_thickness} ) ) );
  }
}
