/*****************************************************************************\
* (c) Copyright 2020 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#include "Core/PrintHelpers.h"

std::string LHCb::Detector::detail::getIndentation( int level ) {
  char fmt[128], text[1024];
  ::snprintf( fmt, sizeof( fmt ), "%03d %%-%ds", level + 1, 2 * level + 1 );
  ::snprintf( text, sizeof( text ), fmt, "" );
  return text;
}