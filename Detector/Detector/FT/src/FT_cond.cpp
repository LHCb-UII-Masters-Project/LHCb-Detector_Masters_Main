//==========================================================================
//  AIDA Detector description implementation
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
// Author     : M.Frank
// Modified by Zehua Xu for halflayer construction
//==========================================================================
//
// Specialized generic detector constructor
//
//==========================================================================
#include "Core/ConditionIdentifier.h"
#include "Core/ConditionsRepository.h"
#include "DD4hep/ConditionDerived.h"
#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/ExtensionEntry.h"
#include "DD4hep/Printout.h"
#include "DDCond/ConditionsContent.h"
#include "DDCond/ConditionsTags.h"
#include "Detector/FT/DeFTConditionCalls.h"

static long create_conditions_recipes( dd4hep::Detector& description, xml_h e ) {
  using Ext_t                                 = std::shared_ptr<LHCb::Detector::ConditionsRepository>;
  auto                               requests = Ext_t( new LHCb::Detector::ConditionsRepository() );
  std::map<std::string, std::string> alignment_locations;
  std::map<std::string, std::string> condition_locations;
  std::string                        location;
  for ( xml_coll_t i( e, _U( alignments ) ); i; ++i ) {
    xml_comp_t c                                           = i;
    alignment_locations[c.attr<std::string>( _U( type ) )] = c.attr<std::string>( _U( ref ) );
  }
  for ( xml_coll_t i( e, _UC( conditions ) ); i; ++i ) {
    xml_comp_t c                                           = i;
    condition_locations[c.attr<std::string>( _U( type ) )] = c.attr<std::string>( _U( ref ) );
  }
  // All information we need to create the new conditions we will receive during the callbacks.
  // It is not necessary to cache information such as detector elements etc.
  auto ft_iovUpdate = std::make_shared<LHCb::Detector::DeFTConditionCall>( requests );

  // --------------------------------------------------------------------------
  // 1) Setup the addresses for the raw conditions (call: addLocation())
  // 2) Setup the callbacks and their dependencies:
  // 2.1) Static detector elements depend typically on nothing.
  //      These parameters normally were read during
  //      the detector construction in the XML define section.
  //      The callback ensures the creation of the objects
  // 2.2) IOV dependent detector elements depend on
  //      - the raw conditions data (aka the time dependent conditions) (see 1)
  //      - and the static detector element
  // 2.3) Typically parents have references to the next level children
  //      - Add reference to the child detector elements
  // --------------------------------------------------------------------------
  auto deFT = description.detector( "FT" );
  auto ftAddr =
      requests->addLocation( deFT, LHCb::Detector::Keys::deltaKey, alignment_locations["system"], "FTSystem" );
  auto ftIOV = requests->addDependency( deFT, LHCb::Detector::Keys::deKey, ft_iovUpdate );
  ftIOV.second->dependencies.push_back( ftAddr.first );
  for ( auto& station : deFT.children() ) {
    auto        deStation = station.second;
    std::string st_nam    = deStation.name();
    auto        stAddr =
        requests->addLocation( deStation, LHCb::Detector::Keys::deltaKey, alignment_locations["system"], st_nam );
    ftIOV.second->dependencies.push_back( stAddr.first );

    for ( auto& layer : deStation.children() ) {
      auto        deLay   = layer.second;
      std::string lay_nam = st_nam + deLay.name();
      auto        layAddr =
          requests->addLocation( deLay, LHCb::Detector::Keys::deltaKey, alignment_locations["system"], lay_nam );
      ftIOV.second->dependencies.push_back( layAddr.first );

      for ( auto& halflayer : deLay.children() ) {
        auto        deHalflayer   = halflayer.second;
        std::string halflayer_nam = lay_nam + deHalflayer.name();
        auto        halflayerAddr = requests->addLocation( deHalflayer, LHCb::Detector::Keys::deltaKey,
                                                    alignment_locations["system"], halflayer_nam );
        ftIOV.second->dependencies.push_back( halflayerAddr.first );

        for ( auto& quad : deHalflayer.children() ) {
          auto        deQuad   = quad.second;
          std::string quad_nam = halflayer_nam + deQuad.name();
          auto        quadAddr =
              requests->addLocation( deQuad, LHCb::Detector::Keys::deltaKey, alignment_locations["system"], quad_nam );
          ftIOV.second->dependencies.push_back( quadAddr.first );

          for ( auto& module : deQuad.children() ) {
            auto        deMod   = module.second;
            std::string mod_nam = quad_nam + deMod.name();
            auto        modAddr =
                requests->addLocation( deMod, LHCb::Detector::Keys::deltaKey, alignment_locations["modules"], mod_nam );
            ftIOV.second->dependencies.push_back( modAddr.first );

            for ( auto& mat : deMod.children() ) {
              auto        deMat   = mat.second;
              std::string mat_nam = mod_nam + deMat.name();
              auto        matAddr =
                  requests->addLocation( deMat, LHCb::Detector::Keys::deltaKey, alignment_locations["mats"], mat_nam );
              ftIOV.second->dependencies.push_back( matAddr.first );
            }
          }
        }
      }
    }
  }

  // register classic conditions
  for ( const auto& [condition_name, condition_file] : condition_locations ) {
    requests->addLocation( deFT, LHCb::Detector::item_key( condition_name ), condition_file, condition_name );
  }

  deFT.addExtension( new dd4hep::detail::DeleteExtension<Ext_t, Ext_t>( new Ext_t( std::move( requests ) ) ) );
  return 1;
}
DECLARE_XML_DOC_READER( LHCb_FT_cond, create_conditions_recipes )
