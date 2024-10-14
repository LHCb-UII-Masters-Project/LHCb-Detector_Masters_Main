//==========================================================================
//  AIDA Detector description implementation
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
//  \author  Markus Frank
//  \date    2016-02-02
//  \version 1.0
//
//==========================================================================

// Framework include files
#include "Core/ConditionsLoader.h"
#include "Core/ConditionsReaderContext.h"
#include "Core/ConditionsRepository.h"
#include "Core/Keys.h"

// Other dd4hep includes
#include "Core/UpgradeTags.h"
#include "DD4hep/Operators.h"
#include "DD4hep/PluginCreators.h"
#include "DD4hep/Printout.h"
#include "DD4hep/detail/ConditionsInterna.h"
#include "DDCond/ConditionsManagerObject.h"
#include "DDCond/ConditionsTags.h"
#include "XML/XML.h"

#include "TTimeStamp.h"

#include <Core/CondDBSchema.h>
#include <Core/yaml_converters.h>
#include <boost/range/iterator_range_core.hpp>
#include <exception>
#include <set>
#include <sstream>
#include <string_view>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>
#include <range/v3/view/all.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/unique.hpp>

namespace {
  std::tuple<bool, YAML::Node> empty_condition( const LHCb::Detector::ConditionIdentifier* cond ) {
    const std::array<dd4hep::Condition::key_type, 2> alignment_keys = {LHCb::Detector::Keys::deltaKey,
                                                                       LHCb::Detector::Keys::alignmentKey};

    dd4hep::ConditionKey::KeyMaker key( cond->hash );

    bool is_alignment = std::any_of( alignment_keys.begin(), alignment_keys.end(),
                                     [&key]( auto hash ) { return key.values.item_key == hash; } );

    auto const* payload = is_alignment ? "NotFound: !alignment\n  position: [0.0 * mm, 0.0 * mm, 0.0 * mm]\n  "
                                         "rotation: [0.0 * rad, 0.0 * rad, 0.0 * rad]\n"
                                       : "";
    auto node = YAML::Load( payload );
    return {is_alignment, is_alignment ? node["NotFound"] : node};
  }

  namespace rv = ranges::views;
  namespace ra = ranges::actions;
} // namespace

/// Standard constructor, initializes variables
LHCb::Detector::ConditionsLoader::ConditionsLoader( dd4hep::Detector& description, dd4hep::cond::ConditionsManager mgr,
                                                    const std::string& nam )
    : ConditionsDataLoader( description, mgr, nam ) {
  declareProperty( "ReaderType", m_readerType );
  declareProperty( "Directory", m_directory );
  declareProperty( "DBTag", m_dbtag );
  declareProperty( "Match", m_match );
  declareProperty( "IOVType", m_iovTypeName );
  declareProperty( "UseOverlay", m_useOverlay );
  declareProperty( "OverlayInitPath", m_overlayInitPath );
}

/// Initialize loader according to user information
void LHCb::Detector::ConditionsLoader::initialize() {
  std::string typ    = m_readerType;
  const void* argv[] = {"ConditionsReader", this, 0};
  m_reader.reset( dd4hep::createPlugin<ConditionsReader>( typ, m_detector, 2, argv ) );
  ( *m_reader )["Directory"] = m_directory;
  ( *m_reader )["Match"]     = m_match;
  ( *m_reader )["DBTag"]     = m_dbtag;
  m_iovType                  = manager().iovType( m_iovTypeName );
  if ( !m_iovType ) {
    // Error
  }

  try {
    std::string             data;
    ConditionsReaderContext ctxt;
    ctxt.event_time = 0;
    if ( m_reader->getObject( ".schema.json", &ctxt, data ) ) {
      // attach CondDB schema to the main Detector
      m_detector.addExtension<CondDBSchema>(
          new CondDBSchema( nlohmann::json::parse( data ).get<CondDBSchema::schema_t>() ) );
    } else {
      dd4hep::printout( dd4hep::INFO, "ConditionsLoader",
                        "no schema description found in the DB, schema check not available" );
    }
  } catch ( const std::runtime_error& err ) {
    dd4hep::printout( dd4hep::INFO, "ConditionsLoader", "exception '%s' loading DB schema, schema check not available",
                      err.what() );
  }

  if ( m_useOverlay ) {
    dd4hep::printout( dd4hep::DEBUG, "ConditionsLoader", "overlay enabled" );
    if ( !m_overlayInitPath.empty() ) {
      dd4hep::printout( dd4hep::DEBUG, "ConditionsLoader", "overlay initialized from %s", m_overlayInitPath.c_str() );
      m_overlay.load( m_overlayInitPath );
    }
  }
}

/// Optimized update using conditions slice data
size_t LHCb::Detector::ConditionsLoader::load_many( const dd4hep::IOV& req_iov, RequiredItems& conditions_to_load,
                                                    LoadedItems& loaded, dd4hep::IOV& /* conditions_validity */ ) {

  // the context is used to pass the requested event time to the UriReader
  // and to carry back the matching IOV (since and until)...
  // it's both an input and an output variable to the reader calls... if only and interface could be more confusing
  ConditionsReaderContext ctxt;
  ctxt.event_time = req_iov.keyData.first;

  const size_t loaded_initial_size = loaded.size();
  size_t       loaded_len          = loaded_initial_size;
  loaded.clear();

  // this is what is used by XercesC to resolve URIs included by XML documents
  // via "SYSTEM" keyword (needed only for XML parsing but done here instead of inside the loop)
  dd4hep::xml::UriContextReader xml_entity_resolver( m_reader.get(), &ctxt );

  // FIXME using a stable buffer for storing the condition docs to be parsed might look like an optimization,
  // but in the case of GitCondDB (or the ConditionsFileReader) it does not really help as each request
  // creates a new string that then has to be moved into this buffer
  std::string buffer;

  bool print_results = true; // isActivePrintLevel( dd4hep::DEBUG );
  // dd4hep::setPrintLevel( dd4hep::DEBUG );

  // I have the impression ConditionsIdentifier is not a very good name, but I stick by it for the moment
  ConditionsRepository::Locations<true> all_conditions{};
  std::for_each( conditions_to_load.begin(), conditions_to_load.end(), [&all_conditions]( auto entry ) {
    auto* load_info = entry.second->ptr();
    if ( !load_info ) {
      dd4hep::printout( dd4hep::INFO, "ConditionsLoader", "++ CANNOT update condition: [%16llX] [No load info]",
                        entry.first );
    } else {
      all_conditions.insert( reinterpret_cast<const ConditionIdentifier*>( load_info ) );
    }
  } );

  const auto& conditions_by_location_hash = all_conditions.get<by_location_hash>();
  const auto& conditions_by_filename      = all_conditions.get<by_filename>();

  // Extract the range of unique filenames from the conditions
  auto unique_filenames = conditions_by_filename | rv::transform( &ConditionIdentifier::sys_id ) | rv::unique;

  auto block_register = [this]( dd4hep::IOV::Key iov_key, const std::vector<dd4hep::Condition>& entity_conditions ) {
    dd4hep::cond::ConditionsPool* pool = manager().registerIOV( *m_iovType, iov_key );
    return manager().blockRegister( *pool, entity_conditions );
  };

  try {
    for ( auto filename : unique_filenames ) {

      auto register_condition = [&loaded]( std::vector<dd4hep::Condition>& entity_conditions,
                                           const ConditionIdentifier* cond_ident, const std::string& cond_name,
                                           const YAML::Node& cond_data, const std::string& doc ) {
        dd4hep::Condition c = LHCb::YAMLConverters::make_condition( cond_name, cond_data );
        if ( !c.isValid() ) {
          // Will eventually have to raise an exception
          dd4hep::printout( dd4hep::ERROR, "ConditionsLoader", "++ Failed to load condition %s from %s",
                            cond_name.c_str(), doc.c_str() );
        } else {
          const auto cond_key = cond_ident->hash;

          c->hash = cond_key;
          // pass the empty condition to the caller
          loaded[cond_key] = c;

          // record the condition for bulk registration to the manager
          entity_conditions.push_back( c );
        }
      };

      // ready, set, chrono started!
      TTimeStamp start;
      // Load from the git cond db here XXX
      // If the file can't be loaded, create empty conditions for all
      // of the conditions expected in this file
      if ( m_reader->load( filename, &ctxt, buffer ) ) {
        auto ext = std::string_view{filename};
        ext.remove_prefix( ext.find_last_of( '.' ) );
        if ( ext == ".yml" || ext == ".yaml" ) {
          // Use the filename hash as the upper bits of the hash
          dd4hep::ConditionKey::KeyMaker kmaker( dd4hep::detail::hash32( filename ), 0 );

          // condition YAML files are expected to contain a mapping <name> -> <condition data>
          auto data = YAML::Load( ( m_useOverlay ) ? m_overlay.get_or_default( filename, buffer ) : buffer );

          // Checking whether we have conditions to override in that same file (i.e. o.path)
          // in that case we directly update the YAML node
          for ( auto const& o : m_conditionsOverride ) {
            if ( o.path == filename ) {
              dd4hep::printout( dd4hep::DEBUG, "ConditionsLoader", "++ Overriding %s:%s", o.path.c_str(),
                                o.name.c_str() );
              data[o.name] = o.value;
            }
          }

          // prepare the store for the conditions we will register to the conditions manager
          // ... it would be nice if we had an actual type here and not a handle
          std::vector<dd4hep::Condition> entity_conditions;
          entity_conditions.reserve( data.size() );

          // loop over all conditions in the YAML document
          for ( const auto& cond_data : data ) {
            const std::string cond_name = cond_data.first.as<std::string>();

            // Check if the condition has been requested.
            // Only the upper 32 bits of kmaker.hash have been set
            // above, set the lower bits here
            kmaker.values.item_key = dd4hep::detail::hash32( cond_name );

            auto cond_ident = conditions_by_location_hash.find( kmaker.hash ); // O[log(N)]
            if ( cond_ident != conditions_by_location_hash.end() ) {
              register_condition( entity_conditions, *cond_ident, cond_name, cond_data.second, filename );
            } else {
              dd4hep::printout( dd4hep::WARNING, "ConditionsLoader", "++ Got stray condition %s from %s",
                                cond_name.c_str(), filename.c_str() );
            }
          }

          // Check if there are conditions in this file that have been
          // requested, but have not yet been loaded. If so, create
          // empty conditions for those too.
          for ( const auto* cond_ident :
                boost::make_iterator_range( conditions_by_filename.equal_range( filename ) ) ) {
            auto loaded_it = loaded.find( cond_ident->hash );
            if ( loaded_it == loaded.end() ) {
              auto [is_alignment, cond] = empty_condition( cond_ident );
              register_condition( entity_conditions, cond_ident, "NotFound", cond, filename );
              dd4hep::printout( is_alignment ? dd4hep::WARNING : dd4hep::INFO, "ConditionsLoader",
                                "++ Created empty%s condition %s in %s", is_alignment ? " alignment" : "",
                                cond_ident->object.c_str(), cond_ident->sys_id.c_str() );
            }
          }

          // now that we converted the whole file and created empty conditions for those that are missing,
          // we make a pool for the IOV of the YAML file and we register the conditions; the IOV for missing
          // conditions should be the same as for those that were found, as they may only appear at the end of
          // the IOV
          // FIXME why one pool per YAML file? should it not be one pool for all conditions? (given an event time)
          dd4hep::IOV::Key iov_key( ctxt.valid_since, ctxt.valid_until );
          // FIXME where does the IOVType comes from? why it is bound the the ConditionsLoader instance?
          size_t ret = block_register( iov_key, entity_conditions );
          // FIXME it looks like blockRegister returns the number of conditions registered and we have to check
          //       that it's what we expect, but clearly we do not know what to do with that information
          if ( ret != entity_conditions.size() ) {
            // Error!
          }
        }

        // now that all is done (for this URL) we can stop the chrono
        TTimeStamp  stop;
        std::string valid_end = ctxt.valid_until == std::numeric_limits<std::int64_t>::max()
                                    ? std::string{"inf   "}
                                    : std::to_string( ctxt.valid_until );
        dd4hep::printout( dd4hep::INFO, "ConditionsLoader",
                          "++ Added %4ld conditions from %-48s to pool [%6s-%6s] [%7.3f sec]",
                          loaded.size() - loaded_len, filename.c_str(), std::to_string( ctxt.valid_since ).c_str(),
                          valid_end.c_str(), stop.AsDouble() - start.AsDouble() );
        loaded_len = loaded.size();
      } else {
        // File containing Conditions not found, create empty
        // conditions for all requested conditions that are in this
        // file.
        std::vector<dd4hep::Condition> entity_conditions;
        entity_conditions.reserve( conditions_by_filename.count( filename ) );

        for ( const auto* cond_ident : boost::make_iterator_range( conditions_by_filename.equal_range( filename ) ) ) {
          auto [is_alignment, cond] = empty_condition( cond_ident );
          register_condition( entity_conditions, cond_ident, "NotFound", cond, filename );
        }

        // If the file is not in the DB, it's never going to appear,
        // so register empty conditions with an infinite IOV
        dd4hep::IOV::Key iov_key( 0, std::numeric_limits<std::int64_t>::max() );
        block_register( iov_key, entity_conditions );

        dd4hep::printout( dd4hep::INFO, "ConditionsLoader", "++ Added %4ld null conds for  %-48s to pool [%6s-%6s]",
                          loaded.size() - loaded_len, filename.c_str(), std::to_string( 0 ).c_str(), "inf   " );
        loaded_len = loaded.size();
      }
      if ( print_results ) {
        // for each requested key we actually load (not skipped) we print all loaded conditions
        // ... including those already printed... (as we are still in the loop over todo_list)
        for ( const auto& e : loaded ) {
          const dd4hep::Condition& cond = e.second;
          dd4hep::printout( dd4hep::DEBUG, "ConditionsLoader", "++ %16llX: %s -> %s", cond.key(), cond->value.c_str(),
                            cond.name() );
        }
      }
    }
  } catch ( const std::exception& e ) {
    dd4hep::printout( dd4hep::ERROR, "ConditionsLoader", "+++ Load exception: %s", e.what() );
    throw;
  } catch ( ... ) {
    dd4hep::printout( dd4hep::ERROR, "ConditionsLoader", "+++ UNKNWON Load exception." );
    throw;
  }
  // FIXME somebody will have to explain how this is useful
  return loaded.size() - loaded_initial_size;
}

#include "DD4hep/Factories.h"

//==========================================================================
/// Plugin function
static void* create_loader( dd4hep::Detector& description, int argc, char** argv ) {
  const char*                            name = argc > 0 ? argv[0] : "ConditionsLoader";
  dd4hep::cond::ConditionsManagerObject* m    = (dd4hep::cond::ConditionsManagerObject*)( argc > 0 ? argv[1] : 0 );
  return new LHCb::Detector::ConditionsLoader( description, m, name );
}
DECLARE_DD4HEP_CONSTRUCTOR( LHCb_ConditionsLoader, create_loader )
