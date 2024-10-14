/*****************************************************************************\
* (c) Copyright 2022 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once

#include <cerrno>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <stdlib.h>

namespace Detector::Test {
  /// Helper to create a temporary directory that is automatically removed at exit.
  struct TmpDir {
    /// Create a temporary directory that will be removed when the object goes out of scope.
    /// If the argument do_not_remove is set to true, the temporary directory is not removed
    /// and its name is printed.
    TmpDir( bool remove = true ) : to_remove( remove ) {
      std::string tmp_dir_path = std::filesystem::temp_directory_path() / "detector_test_XXXXXX";
      if ( !mkdtemp( tmp_dir_path.data() ) ) { throw std::runtime_error( std::string( std::strerror( errno ) ) ); }
      path = tmp_dir_path;
    }
    ~TmpDir() {
      if ( to_remove )
        std::filesystem::remove_all( path );
      else
        std::cout << "temporary directory " << path << " not removed\n";
    }
    std::filesystem::path path;
    bool                  to_remove;
  };
} // namespace Detector::Test
