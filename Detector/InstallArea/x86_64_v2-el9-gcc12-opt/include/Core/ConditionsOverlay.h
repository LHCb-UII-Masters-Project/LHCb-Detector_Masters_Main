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
#pragma once
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <yaml-cpp/yaml.h>

namespace LHCb::Detector {
  /// Writable in-memory no-IOV YAML-based conditions database
  ///
  /// The main purpose is to optionally override entries from the actual database, but can be used
  /// for testing.
  ///
  /// Until there's any write (via ConditionsOverlay::update) the overlay is transparent and calls
  /// to ConditionsOverlay::get will be just passthrough (keeping a copy of the docs that are processed).
  ///
  /// When part of a document is updated the call to ConditionsOverlay::get will return the copy in the
  /// overlay ignoring the content retrived from the database, while still being transparent for all
  /// other documents.
  ///
  /// \warning { ConditionsOverlay is only meant to work with YAML documents. }
  class ConditionsOverlay {

    struct OverlayEntry {
      std::string data;
      bool        dirty = false;
    };
    std::map<std::string, OverlayEntry> m_store;

  public:
    /// Add (or overwrite) a document to the overlay and return a copy of the added data.
    YAML::Node add( const std::string& path, YAML::Node doc = {} ) {
      std::stringstream s;
      s << doc;
      add( path, s.str() );
      return doc;
    }
    /// Add (or overwrite) a document to the overlay and return a copy of the added data.
    const std::string& add( const std::string& path, std::string doc = {} ) {
      if ( path.empty() ) throw std::domain_error( "invalid path" ); // FIXME use boost::contract
      return m_store.insert_or_assign( path, OverlayEntry{std::move( doc )} ).first->second.data;
    }
    /// Get the YAML::Node from the overlay if present and have been written to, otherwise use (parse)
    /// the content of the raw_data
    YAML::Node get( const std::string& path, const std::string& raw_data ) {
      return YAML::Load( get_or_default( path, raw_data ) );
    }
    /// Get the data from the overlay if present and have been written to, otherwise use
    /// the provided default value
    const std::string& get_or_default( const std::string& path, const std::string& default_value ) {
      if ( auto item = m_store.find( path ); item != m_store.end() && item->second.dirty ) {
        // we have and entry that has been touched once, so we give it back
        return item->second.data;
      }
      // otherwise we parse and give back the result (keeping a copy for later)
      return add( path, default_value );
    }

    /// Update a condition value in the overlay.
    /// If the containing document is not present, an empty one is used as starting point
    void update( const std::string& path, const std::string& name, YAML::Node value ) {
      if ( path.empty() ) throw std::domain_error( "invalid path" ); // FIXME use boost::contract
      auto& [data, dirty] = m_store[path];
      auto node           = YAML::Load( data );
      node[name]          = std::move( value );
      std::stringstream s;
      s << node;
      data  = s.str();
      dirty = true;
    }
    /// Update a condition value in the overlay.
    /// If the containing document is not present, an empty one is used as starting point
    void update( const std::string& path, const std::string& name, const std::string& value ) {
      if ( path.empty() ) throw std::domain_error( "invalid path" ); // FIXME use boost::contract
      auto& [data, dirty] = m_store[path];
      auto node           = YAML::Load( data );
      node[name]          = YAML::Load( value );
      std::stringstream s;
      s << node;
      data  = s.str();
      dirty = true;
    }
    /// Write the content of the overlay to disk, in the given directory
    void dump( std::filesystem::path root_dir, bool all = false ) const {
      namespace fs = std::filesystem;
      std::for_each( m_store.begin(), m_store.end(), [&root_dir, all]( const auto& element ) {
        if ( all || element.second.dirty ) { // dump an entry only if it's "dirty" or we want a full dump
          std::string_view path{element.first};
          if ( path[0] == '/' ) path.remove_prefix( 1 );
          auto target = root_dir / path;
          fs::create_directories( target.parent_path() );
          std::ofstream( target ) << element.second.data << '\n';
        }
      } );
    }
    /// Fill the overlay from the data found in a directory.
    ///
    /// This is the inverse of dump() in the sense that after the content of the
    /// overlay is dumped to disc a new identical instace can be created from the
    /// created filesystem tree.
    ///
    /// @note files containing invalid (unparsable) YAML are silently ignored
    void load( const std::filesystem::path root_dir ) {
      namespace fs = std::filesystem;
      // add each regular (YAML) file we find under root_dir to the store as "dirty"
      for ( const fs::directory_entry& dir_entry : fs::recursive_directory_iterator( root_dir ) ) {
        if ( dir_entry.is_regular_file() ) {
          try {
            std::stringstream s;
            s << YAML::LoadFile( dir_entry.path() );
            m_store.insert_or_assign( fs::relative( dir_entry, root_dir ), OverlayEntry{s.str(), true} );
          } catch ( const YAML::Exception& ) {
            // silently ignoring malformed entries
            // FIXME: it would be nice to have some warning about which path is invalid
          }
        }
      }
    }
    /// Get the number of documents in the overlay
    auto size() const { return m_store.size(); }
    /// Empty the overlay
    void clear() { m_store.clear(); }
  };
} // namespace LHCb::Detector
