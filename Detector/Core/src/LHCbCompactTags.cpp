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
// Framework include files
#include "XML/XMLElements.h"
#include "XML/XMLTags.h"

// Define unicode tags
#define UNICODE( x ) extern const ::dd4hep::xml::Tag_t Unicode_##x( #x )
#include "Core/LHCbCompactTags.h"
