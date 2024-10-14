/*****************************************************************************\
* (c) Copyright 2023 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once

#include <map>
#include <set>
#include <string>

namespace LHCb::Detector {
  /// @brief List of paths and conditions available in the database.
  ///
  /// If the conditions data base has the special file `.schema.json` describing the
  /// content of the database, an extension CondDBSchema is added to the top Detector
  /// so that user code can check which conditions are to be expected in the
  /// database and take proper actions if a condition is not present.
  class CondDBSchema {
  public:
    using schema_t = std::map<std::string, std::set<std::string>>;

    CondDBSchema( schema_t&& schema ) : m_schema{std::move( schema )} {}

    /// Check if a given condition path/key is available in the DB according to
    /// the schema (note that path/key is called sys_id/object in other functions).
    bool has( const std::string& path, const std::string& key ) const {
      if ( auto entry = m_schema.find( path ); entry != m_schema.end() ) {
        return entry->second.find( key ) != entry->second.end();
      }
      return false;
    }

  private:
    schema_t m_schema;
  };
} // namespace LHCb::Detector
