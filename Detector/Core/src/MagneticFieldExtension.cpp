/*****************************************************************************\
* (c) Copyright 2021 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/

#include "Core/MagneticFieldExtension.h"
#include "Core/MagneticFieldGrid.h"
#include "Core/MagneticFieldGridReader.h"

#include "Detector/Magnet/DeMagnet.h"

#include "Core/Keys.h"
#include "DD4hep/DetElement.h"
#include "DD4hep/Printout.h"

#include <nlohmann/json.hpp>

#include <cassert>
#include <memory>
#include <utility>

using namespace LHCb::Magnet;
using LHCb::Detector::ConditionKey;
using nlohmann::json;

void LHCb::Magnet::setup_magnetic_field_extension( const dd4hep::Detector& description, std::string field_map_path,
                                                   const double nominal_current ) {
  auto magnetdet = description.detector( "Magnet" );
  using Ext_t    = MagneticFieldExtension;
  auto mfhelper  = std::make_unique<Ext_t>( description, std::move( field_map_path ), nominal_current );
  auto dExt      = std::make_unique<dd4hep::detail::DeleteExtension<Ext_t, Ext_t>>( mfhelper.release() );
  magnetdet.addExtension( dExt.release() );
}

/*
 * Method to load the magnetic field file based on the conditions
 */
MagneticFieldExtension::FieldData
MagneticFieldExtension::load_magnetic_field_grid( const dd4hep::cond::ConditionUpdateContext& context ) {

  MagneticFieldGridReader reader{m_magneticFieldFilesLocation};

  FieldData data;

  data.grid = std::make_shared<LHCb::Magnet::MagneticFieldGrid>();

  // Getting the Magnet DetectorElement
  auto magnetdet = m_description.detector( "Magnet" );

  // Loading the current value
  const auto& magnet_cond = context.condition( ConditionKey( magnetdet, "Magnet" ) ).get<json>();
  const auto  current     = magnet_cond["Current"].get<double>();
  const auto  polarity    = magnet_cond["Polarity"].get<int>();
  dd4hep::printout( dd4hep::DEBUG, "MagneticFieldExtension", "Current value: %f", current );
  dd4hep::printout( dd4hep::DEBUG, "MagneticFieldExtension", "Current polarity: %d", polarity );

  // Loading the appropriate map file list depending on the polarity
  const std::string mapCondName      = polarity > 0 ? "FieldMapFilesUp" : "FieldMapFilesDown";
  const auto&       magnet_filescond = context.condition( ConditionKey( magnetdet, mapCondName ) ).get<json>();
  const auto        filenames        = magnet_filescond["Files"].get<std::vector<std::string>>();
  assert( !filenames.empty() );

  // Loading the ScaleUp/ScaleDown attributes
  const std::string scaleCondName    = polarity > 0 ? "ScaleUp" : "ScaleDown";
  const auto&       magnet_scalecond = context.condition( ConditionKey( magnetdet, scaleCondName ) ).get<json>();
  const auto        coeffs           = magnet_scalecond["Coeffs"].get<std::vector<double>>();

  // Computing the scale factor for the magnetic field
  const auto scale_factor =
      abs( nominalCurrent() ) > 0. ? coeffs[0] + ( coeffs[1] * ( current / nominalCurrent() ) ) : 0.;
  dd4hep::printout( dd4hep::INFO, "MagneticFieldExtension", "Scale factor: %f", scale_factor );
  data.grid->setScaleFactor( scale_factor );

  // loading the files into the grid
  const auto sc = ( filenames.size() == 1 ? reader.readDC06File( filenames.front(), *data.grid )
                                          : reader.readFiles( filenames, *data.grid ) );
  if ( !sc ) {
    dd4hep::printout( dd4hep::ERROR, "MagneticFieldExtension", "Error loading magnetic field map" );
    dd4hep::except( "load_magnetic_field_map", "Error loading magnetic field map" );
  }

  // Imported from the MagneticFieldSvc
  // Setting constants
  data.useRealMap            = ( filenames.size() == 4 );
  data.isDown                = data.grid->fieldVectorClosestPoint( {0, 0, 5200} ).y() < 0;
  data.signedRelativeCurrent = std::abs( scale_factor ) * ( data.isDown ? -1 : +1 );

  return data;
}
