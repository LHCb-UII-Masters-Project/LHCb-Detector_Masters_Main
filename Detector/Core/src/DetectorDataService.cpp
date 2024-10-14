/*****************************************************************************\
* (c) Copyright 2020-2024 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#include <TTimeStamp.h>

#include <Core/CondDBSchema.h>
#include <Core/DetectorDataService.h>
#include <Core/DetectorOptions.h>
#include <DD4hep/ConditionDerived.h>
#include <DD4hep/DetFactoryHelper.h>
#include <DD4hep/InstanceCount.h>
#include <DD4hep/Printout.h>

#include <DD4hep/ConditionDerived.h>
#include <DD4hep/Conditions.h>
#include <DD4hep/ConditionsMap.h>

#include <DDCond/ConditionsCleanup.h>
#include <DDCond/ConditionsContent.h>
#include <DDCond/ConditionsDataLoader.h>
#include <DDCond/ConditionsManager.h>
#include <DDCond/ConditionsManagerObject.h>
#include <DDCond/ConditionsSlice.h>

#include <Core/ConditionsLoader.h>
#include <Core/ConditionsRepository.h>
#include <Core/DeAlignmentCall.h>
#include <Core/DeConditionCall.h>
#include <Core/Keys.h>
#include <Core/Utils.h>

#include <Detector/LHCb/DeLHCb.h>

#include <algorithm>
#include <chrono>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <stdexcept>
#include <thread>
#include <utility>

#if FMT_VERSION >= 90000
// make nlohmann::json formattable in fmt via operator<<
template <>
struct fmt::formatter<nlohmann::json> : ostream_formatter {};
#endif

using ConditionsSliceCache = LHCb::Detector::DetectorDataService::ConditionsSliceCache;
using ConditionsSlicePtr   = LHCb::Detector::DetectorDataService::ConditionsSlicePtr;

namespace {
  struct ConditionsCleaner : dd4hep::cond::ConditionsFullCleanup {
    ConditionsCleaner( const ConditionsSliceCache& cache ) : m_cache( cache ) {}
    /// Request cleanup operation of regular conditions pool
    bool operator()( const dd4hep::cond::ConditionsPool& pool ) const override {
      // check that no slice in the cache uses the pool
      return m_cache.empty() || std::all_of( begin( m_cache ), end( m_cache ), [&pool]( auto& slice ) {
               // true if the pool IOV is outside the slice iov
               // (I'd love to use IOV methods, but they are so weird)
               return slice->iov().keyData.second < pool.iov->keyData.first ||
                      slice->iov().keyData.first > pool.iov->keyData.second;
             } );
    }
    const ConditionsSliceCache& m_cache;
  };
} // namespace

std::tuple<std::string_view, std::string_view>
LHCb::Detector::parseConditionIdentifier( std::string_view condIdentifier ) {
  auto colonPos = condIdentifier.find_first_of( ':' );
  if ( colonPos == std::string_view::npos ) {
    return {"/world", condIdentifier};
  } else {
    return {condIdentifier.substr( 0, colonPos ), condIdentifier.substr( colonPos + 1 )};
  }
}

LHCb::Detector::DetectorDataService::DetectorDataService( dd4hep::Detector&        description,
                                                          std::vector<std::string> detectorNames )
    : m_description( description )
    , m_manager( description, "DD4hep_ConditionsManager_Type1" )
    , m_detectorNames( detectorNames ) {
  m_description.addExtension<Options>( new Options{} );
  auto& opts             = *m_description.extension<Options>();
  opts["detector_names"] = detectorNames;
}

void LHCb::Detector::DetectorDataService::initialize( const nlohmann::json& config ) {
  m_manager["PoolType"]                 = "DD4hep_ConditionsLinearPool";
  m_manager["UserPoolType"]             = "DD4hep_ConditionsMapUserPool";
  m_manager["UpdatePoolType"]           = "DD4hep_ConditionsLinearUpdatePool";
  m_manager["LoaderType"]               = "LHCb_ConditionsLoader";
  m_manager["OutputUnloadedConditions"] = true;
  m_manager["LoadConditions"]           = true;
  m_manager.initialize();

  m_iov_typ = m_manager.registerIOVType( 0, "run" ).second;
  if ( 0 == m_iov_typ ) { dd4hep::except( "ConditionsPrepare", "++ Unknown IOV type supplied." ); }

  // Parsing the repository URL to extract the tag
  // it overrides specified as an option...
  std::string repository = config.at( "repository" ).get<std::string>();
  std::string tag;
  if ( std::string_view repo_schema = std::string_view( repository ).substr( 0, 5 );
       repo_schema == "file:" || repo_schema == "json:" ) {
    // "file:" and "json:" repositories do not need a tag
  } else if ( auto found = repository.find( "@" ); found != std::string::npos ) {
    tag        = repository.substr( found + 1 );
    repository = repository.substr( 0, found );
  } else {
    tag = config.at( "tag" ).get<std::string>();
  }
  dd4hep::printout( dd4hep::INFO, "DetectorDataService", "Using repository %s, tag:%s", repository.c_str(),
                    tag.c_str() );

  dd4hep::cond::ConditionsDataLoader& loader = m_manager.loader();
  loader["DBTag"]                            = tag;
  loader["ReaderType"]                       = "LHCb_ConditionsGitReader";
  loader["Directory"]                        = repository;
  loader["Match"]                            = "file:";
  loader["IOVType"]                          = "run";
  if ( config.contains( "overlay" ) ) {
    auto overlay = config["overlay"];
    if ( overlay.is_boolean() ) {
      loader["UseOverlay"] = overlay.get<bool>();
    } else if ( overlay.is_object() && overlay.contains( "path" ) ) {
      loader["UseOverlay"]      = true;
      loader["OverlayInitPath"] = overlay.at( "path" ).get<std::string>();
    } else {
      throw std::invalid_argument( fmt::format( "invalid configuration for conditions overlay '{}'", overlay ) );
    }
  }
  loader.initialize();
  CondDBSchema* condDBSchema = m_description.extension<CondDBSchema>( false );

  // make sure the detector_names option is in sync
  auto& opts             = *m_description.extension<Options>();
  opts["detector_names"] = m_detectorNames;

  // Now setup DeLHCb
  // BEWARE: HAS TO BE DONE BEFORE THE NEXT STEP THAT GATHERS ALL THE CONDITIONS
  // IN ONE CONTAINER
  LHCb::Detector::setup_DeLHCb_callback( m_description );

  // Iterate over all sub-detectors to be configured
  for ( const auto& det : m_detectorNames ) {
    dd4hep::DetElement de = m_description.detector( det );
    if ( !de.isValid() ) {
      dd4hep::printout( dd4hep::ERROR, "DetectorDataService", " Invalid detector element: %s", det.c_str() );
      throw std::runtime_error( "Invalid detector element: " + det );
    }
    try {
      const auto& de_conds = *de.extension<std::shared_ptr<ConditionsRepository>>();
      if ( condDBSchema ) {
        for ( const auto& c : de_conds->locations.get<by_filename>() ) {
          if ( !c->is_optional && !condDBSchema->has( c->sys_id, c->object ) ) {
            std::string msg = fmt::format( "detector {} requested a condition not in the database: {}[{}]", det,
                                           c->sys_id, c->object );
            dd4hep::printout( dd4hep::ERROR, "DetectorDataService", msg.c_str() );
            throw std::runtime_error( msg );
          }
        }
      }
      m_all_conditions->merge( *de_conds );
    } catch ( const std::runtime_error& e ) {
      dd4hep::printout( dd4hep::ERROR, "DetectorDataService", "Error loading the conditions for detector: %s",
                        det.c_str() );
      throw e;
    }
  }

  if ( loader["UseOverlay"].value<bool>() ) {
    // I do not like it, but I do not see other ways of jumping through the plugin barrier
    // imposed by the ConditionsManager.
    // Note that we can safely static_cast as the loader type is LHCb_ConditionsLoader and
    // the property manager ensures (at least) that the property "UseOverlay" exists.
    auto loader_impl = static_cast<ConditionsLoader*>( &loader );
    m_condOverlay    = &loader_impl->overlay();
  }

  if ( config.contains( "overrides" ) && config["overrides"].is_object() && !config["overrides"].empty() ) {
    auto loader_impl = static_cast<ConditionsLoader*>( &loader );
    // Now preparing the conditions overrides as understood by the ConditionsLoader:
    // We convert the pairs Detector element/conditions into condition file path/condition name in file
    // and keep them in the appropriate structure
    LHCb::Detector::ConditionsOverrides overrides;
    for ( auto const& [key, value] : config["overrides"].items() ) {
      // entry.first is Detector:ConditionName
      // entry.second is the YAML condition itself
      std::pair<std::string, std::string> o{key, value.get<std::string>()};
      dd4hep::printout( dd4hep::INFO, "DetectorDataService", "Overriding %s with %s", o.first.c_str(),
                        o.second.c_str() );
      auto [deName, condKey] = parseConditionIdentifier( o.first );
      auto cond_key =
          LHCb::Detector::ConditionKey( m_description.detector( std::string{deName} ), std::string{condKey} );
      // Check in the conditions mappings which file this corresponds to
      const auto& conditions_by_key = m_all_conditions->locations.get<by_condition_key>();
      if ( auto el = conditions_by_key.find( cond_key ); el != end( conditions_by_key ) ) {
        overrides.emplace_back(
            LHCb::Detector::ConditionOverrideEntry{( *el )->sys_id, ( *el )->object, YAML::Load( o.second )} );
      } else {
        throw std::invalid_argument{"cannot find " + std::string{condKey} + " in " + std::string{deName} +
                                    "'s ConditionsRepository"};
      }
    }
    loader_impl->set_conditions_overrides( overrides );
  }

  m_max_cache_entries = config.value( "max_cache_entries", m_max_cache_entries );

  if ( config.contains( "online_conditions" ) ) {
    auto loader_impl = static_cast<ConditionsLoader*>( &loader );
    loader_impl->set_limited_iov_paths( config["online_conditions"].get<std::vector<std::string>>() );
  }
}

void LHCb::Detector::DetectorDataService::finalize() {
  // beyond this point we cannot guarantee that the loader still exists, thus we cannot use
  // the overlay
  m_condOverlay = nullptr;
  /// Clear it
  clear_slice_cache();
  // Let's do the cleanup
  m_all_conditions.reset();
  m_manager.destroy();
}

ConditionsSlicePtr LHCb::Detector::DetectorDataService::get_slice( size_t iov ) {
  std::lock_guard<std::mutex> hold( m_cache_mutex );
  auto entry = std::find_if( m_cache.begin(), m_cache.end(), [dd4iov = make_iov( iov )]( const auto& slice ) {
    return slice->iov().contains( dd4iov );
  } );
  if ( entry != m_cache.end() ) { return *entry; }
  m_cache.emplace_front( load_slice( iov ) );
  if ( m_cache.size() > m_max_cache_entries ) {
    // identify the entries above max size that can be pruned
    // (i.e. those not used by other threads)
    auto to_erase = std::remove_if( m_cache.begin() + m_max_cache_entries, m_cache.end(),
                                    []( const auto& ptr ) { return ptr.use_count() <= 1; } );
    // if we have stale entries, we can clean up
    if ( to_erase != m_cache.end() ) {
      // delete unused entries
      m_cache.erase( to_erase, m_cache.end() );
      // drop stale pools from the underlying manager
      m_manager.clean( ConditionsCleaner( m_cache ) );
    }
  }
  return m_cache.front();
}

void LHCb::Detector::DetectorDataService::drop_slice( size_t iov ) {
  std::lock_guard<std::mutex> hold( m_cache_mutex );
  auto entry = std::find_if( m_cache.begin(), m_cache.end(), [dd4iov = make_iov( iov )]( const auto& slice ) {
    return slice->iov().contains( dd4iov );
  } );
  if ( entry != m_cache.end() ) {
    // wait that nobody else is using the slice
    auto iterations = 1000u;
    while ( --iterations && entry->use_count() > 1 ) {
      using namespace std::chrono_literals;
      std::this_thread::sleep_for( 1ms );
    }
    if ( entry->use_count() > 1 ) throw std::runtime_error( "failed to drop busy slice" );
    // drop our own shortcut to slice
    m_cache.erase( entry );
    // drop slice from the underlying manager
    m_manager.clean( ConditionsCleaner( m_cache ) );
  }
}

void LHCb::Detector::DetectorDataService::clear_slice_cache() {
  std::lock_guard<std::mutex> hold( m_cache_mutex );
  m_cache.clear();
  m_manager.clear();
}

ConditionsSlicePtr LHCb::Detector::DetectorDataService::load_slice( size_t iov ) {
  dd4hep::IOV req_iov( m_iov_typ, iov );
  auto        slice = std::make_shared<dd4hep::cond::ConditionsSlice>( m_manager, m_all_conditions );
  TTimeStamp  start;
  /// Load the conditions
  dd4hep::cond::ConditionsManager::Result total = m_manager.load( req_iov, *slice );
  TTimeStamp                              comp;
  total += m_manager.compute( req_iov, *slice );
  TTimeStamp                                   stop;
  dd4hep::cond::ConditionsContent::Conditions& missing = slice->missingConditions();
  if ( !missing.empty() ) {
    for ( const auto& m : missing ) {
      dd4hep::printout( dd4hep::ERROR, "DetectorDataService::load_slice", "Failed to load condition [%016llX]: %s",
                        m.first, m.second->toString().c_str() );
    }
    throw std::runtime_error( fmt::format( "Failed to load {} condition(s)", missing.size() ) );
  }

  dd4hep::printout( dd4hep::DEBUG, "Statistics",
                    "+  Created/Accessed a total of %ld conditions "
                    "(S:%6ld,L:%6ld,C:%6ld,M:%ld)  Load:%7.5f sec Compute:%7.5f sec",
                    total.total(), total.selected, total.loaded, total.computed, total.missing,
                    comp.AsDouble() - start.AsDouble(), stop.AsDouble() - comp.AsDouble() );
  return slice;
}

void LHCb::Detector::DetectorDataService::update_condition( const std::string& path, const std::string& name,
                                                            const YAML::Node& data ) {
  if ( !m_condOverlay )
    throw std::logic_error(
        "DetectorDataService::update_condition cannot be invoked if the conditions overlay is not enabled" );
  m_condOverlay->update( path, name, data );
}

void LHCb::Detector::DetectorDataService::update_condition( const std::string& path, const std::string& name,
                                                            const nlohmann::json& data ) {
  update_condition( path, name, utils::json2yaml( data ) );
}

void LHCb::Detector::DetectorDataService::dump_conditions( std::filesystem::path root_dir, bool only_dirty ) const {
  if ( !m_condOverlay )
    throw std::logic_error(
        "DetectorDataService::dump_conditions cannot be invoked if the conditions overlay is not enabled" );
  const bool all = !only_dirty;
  m_condOverlay->dump( std::move( root_dir ), all );
}

void LHCb::Detector::DetectorDataService::load_conditions( std::filesystem::path root_dir ) {
  if ( !m_condOverlay )
    throw std::logic_error(
        "DetectorDataService::load_conditions cannot be invoked if the conditions overlay is not enabled" );
  m_condOverlay->load( std::move( root_dir ) );
}

void LHCb::Detector::DetectorDataService::update_condition( const dd4hep::DetElement& de, const std::string& cond,
                                                            const nlohmann::json& data ) {
  update_condition( de, cond, utils::json2yaml( data ) );
}

void LHCb::Detector::DetectorDataService::update_condition( const dd4hep::DetElement& de, const std::string& cond,
                                                            const YAML::Node& data ) {
  auto        key               = LHCb::Detector::ConditionKey( de, cond );
  const auto& conditions_by_key = m_all_conditions->locations.get<by_condition_key>();
  if ( auto el = conditions_by_key.find( key ); el != end( conditions_by_key ) ) {
    update_condition( ( *el )->sys_id, ( *el )->object, data );
  } else {
    throw std::invalid_argument{"cannot find " + cond + " in " + de.name() + "'s ConditionsRepository"};
  }
}

void LHCb::Detector::DetectorDataService::update_alignment( const dd4hep::DetElement& de, const dd4hep::Delta& delta ) {
  YAML::Node data;
  data.SetTag( "!alignment" );
  auto add_units = []( std::initializer_list<double> values, double unit, std::string_view uname ) {
    std::vector<std::string> output( values.size() );
    std::transform( values.begin(), values.end(), output.begin(),
                    [unit, uname]( double v ) { return fmt::format( "{} * {}", v / unit, uname ); } );
    return output;
  };
  if ( delta.hasTranslation() ) {
    data["position"] =
        add_units( {delta.translation.X(), delta.translation.Y(), delta.translation.Z()}, dd4hep::mm, "mm" );
    data["position"].SetStyle( YAML::EmitterStyle::Flow );
  }
  if ( delta.hasRotation() ) {
    data["rotation"] =
        add_units( {delta.rotation.Psi(), delta.rotation.Theta(), delta.rotation.Phi()}, dd4hep::rad, "rad" );
    data["rotation"].SetStyle( YAML::EmitterStyle::Flow );
  }
  if ( delta.hasPivot() ) {
    data["pivot"] =
        add_units( {delta.pivot.Vect().X(), delta.pivot.Vect().Y(), delta.pivot.Vect().Z()}, dd4hep::mm, "mm" );
    data["pivot"].SetStyle( YAML::EmitterStyle::Flow );
  }
  // FIXME: I would prefer to use dd4hep::align::Keys instead of LHCb::Detector::Keys
  update_condition( de, LHCb::Detector::Keys::deltaName, data );
}

dd4hep::IOV LHCb::Detector::DetectorDataService::make_iov( size_t point ) const { return make_iov( point, point ); }
dd4hep::IOV LHCb::Detector::DetectorDataService::make_iov( size_t since, size_t until ) const {
  return dd4hep::IOV{m_iov_typ, {since, until}};
}
