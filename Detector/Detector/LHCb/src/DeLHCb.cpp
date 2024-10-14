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

#include <Core/CondDBSchema.h>
#include <Core/DeConditionCall.h>
#include <Core/DetectorOptions.h>
#include <Core/PrintHelpers.h>
#include <DD4hep/Detector.h>
#include <DD4hep/Printout.h>
#include <Detector/LHCb/DeLHCb.h>
#include <stdexcept>

LHCb::Detector::detail::DeLHCbObject::DeLHCbObject( const dd4hep::DetElement&             de,
                                                    dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeIOVObject( de, ctxt, 9000000, false ) {
  {
    auto cond = ctxt.condition( hash_key( de, "Tell40Links" ), false );
    if ( cond.isValid() ) {
      auto links = cond.get<nlohmann::json>();
      if ( !links.is_null() ) { m_tell40links = links; }
    }
  }
  {
    auto cond = ctxt.condition( hash_key( de, "InteractionRegion" ), false );
    if ( cond.isValid() ) {
      auto ir = cond.get<nlohmann::json>();
      if ( !ir.is_null() ) { m_interactionRegion = ir; }
    }
  }
  {
    auto cond = ctxt.condition( hash_key( de, "LHC" ), false );
    if ( cond.isValid() ) {
      auto lhcinfo = cond.get<nlohmann::json>();
      if ( !lhcinfo.is_null() ) { m_lhcinfo = lhcinfo; }
    }
  }
  {
    auto cond = ctxt.condition( hash_key( de, "SMOG" ), false );
    if ( cond.isValid() ) {
      auto SMOG = cond.get<nlohmann::json>();
      if ( !SMOG.is_null() ) { m_SMOG = SMOG; }
    }
  }
}

void LHCb::Detector::detail::DeLHCbObject::applyToAllChildren(
    const std::function<void( DeIOVElement<detail::DeIOVObject> )>& callback ) const {
  for ( const auto& c : children ) { callback( c ); }
}

// Utility method to lookup DeIOV object from the condition slice
LHCb::Detector::DeIOV LHCb::Detector::detail::get_DeIOV_from_context( dd4hep::cond::ConditionUpdateContext& context,
                                                                      std::string                           name ) {
  auto det  = dd4hep::Detector::getInstance().detector( name );
  auto cond = context.condition( hash_key( det, LHCb::Detector::Keys::deKey ) );
  return cond;
}

namespace {
  struct DeLHCbConditionCall : LHCb::Detector::DeConditionCall {
    using DeConditionCall::DeConditionCall;
    // List that should be set at configuration time with the names of the dd4hep::DetElements of the
    // DetectorElements to load. Defaults to empty, in that case a call to applyToAllChildren will throw an
    // exception
    std::vector<std::string> children_names;
    // Contains the DeIOVs for all the children

    virtual dd4hep::Condition operator()( const dd4hep::ConditionKey&           key,
                                          dd4hep::cond::ConditionUpdateContext& context ) override final {
      if ( key.item_key() == LHCb::Detector::Keys::deKey ) {
        // This is the pointer we need to return at the end of the callback
        auto lhcbdet = dd4hep::Detector::getInstance().world();
        // Creating the Detector element with the desired field
        auto delhcbobj = new LHCb::Detector::detail::DeLHCbObject( lhcbdet, context );
        // Now looking up the children DeIOVs
        for ( const auto& name : children_names ) {
          LHCb::Detector::DeIOV deiov = LHCb::Detector::detail::get_DeIOV_from_context( context, name );
          delhcbobj->children.push_back( deiov );
        }
        return LHCb::Detector::DeIOV( delhcbobj );
      }
      throw std::logic_error( "Key unknown to DeLHCbConditionCall" );
    }
  };
} // namespace

void LHCb::Detector::setup_DeLHCb_callback( dd4hep::Detector& description ) {

  // We are looking for the repository in case it's already been created
  using Ext_t   = std::shared_ptr<LHCb::Detector::ConditionsRepository>;
  auto de       = description.world();
  auto requests = de.extension<Ext_t>( false );
  if ( !requests ) {
    requests = new Ext_t( new LHCb::Detector::ConditionsRepository() );
    de.addExtension( new dd4hep::detail::DeleteExtension<Ext_t, Ext_t>( requests ) );
  }

  auto opts = description.extension<Options>();

  std::vector<std::string> children_names;
  if ( opts && opts->contains( "detector_names" ) ) {
    children_names = opts->at( "detector_names" ).get<std::vector<std::string>>();
  }

  // Creating the callback that creates the DeLHCb object
  auto iovUpdate = std::make_shared<DeLHCbConditionCall>( *requests );
  // We have to ignore the path of the top geometry as it is DeLHCb itself
  std::copy_if( children_names.begin(), children_names.end(), std::back_inserter( iovUpdate->children_names ),
                []( std::string n ) { return n != "/world"; } );

  // Adding the depedencies
  dd4hep::cond::DependencyBuilder depbuilder( de, LHCb::Detector::Keys::deKey, iovUpdate );
  for ( const auto& name : iovUpdate->children_names ) {
    depbuilder.add( hash_key( description.detector( name ), LHCb::Detector::Keys::deKey ) );
  }

  CondDBSchema* schema = description.extension<CondDBSchema>( false );
  if ( !schema || schema->has( "Conditions/LHCb/Online/Tell40Links.yml", "Tell40Links" ) ) {
    ( *requests )
        ->addLocation( de, LHCb::Detector::item_key( "Tell40Links" ), "Conditions/LHCb/Online/Tell40Links.yml",
                       "Tell40Links" );
    depbuilder.add( hash_key( de, "Tell40Links" ) );
  }

  if ( !schema || schema->has( "Conditions/LHCb/Online/InteractionRegion.yml", "InteractionRegion" ) ) {
    ( *requests )
        ->addLocation( de, LHCb::Detector::item_key( "InteractionRegion" ),
                       "Conditions/LHCb/Online/InteractionRegion.yml", "InteractionRegion" );
    depbuilder.add( hash_key( de, "InteractionRegion" ) );
  }

  if ( !schema || schema->has( "Conditions/LHCb/Online/LHC.yml", "LHC" ) ) {
    ( *requests )->addLocation( de, LHCb::Detector::item_key( "LHC" ), "Conditions/LHCb/Online/LHC.yml", "LHC" );
    depbuilder.add( hash_key( de, "LHC" ) );
  }
  if ( !schema || schema->has( "Conditions/LHCb/Online/SMOG.yml", "SMOG" ) ) {
    ( *requests )->addLocation( de, LHCb::Detector::item_key( "SMOG" ), "Conditions/LHCb/Online/SMOG.yml", "SMOG" );
    depbuilder.add( hash_key( de, "SMOG" ) );
  }
  ( *requests )->addDependency( depbuilder.release() );
}
