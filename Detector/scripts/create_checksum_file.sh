#!/bin/bash
###############################################################################
# (c) Copyright 2022 CERN for the benefit of the LHCb Collaboration           #
#                                                                             #
# This software is distributed under the terms of the GNU General Public      #
# Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   #
#                                                                             #
# In applying this licence, CERN does not waive the privileges and immunities #
# granted to it by virtue of its status as an Intergovernmental Organization  #
# or submit itself to any jurisdiction.                                       #
###############################################################################
set -e

filename=$(readlink -f  $BASH_SOURCE)
dir=$(dirname $filename)
SCRIPT_LOC=$(cd  $dir;pwd)

if [ $# -eq 0 ]
  then
    echo "Please specify the compact file to checksum"
    exit 1
fi

COMPACT_FILE=$1
CHECKSUM_FILE="$(dirname $1)/checksums"

echo "Writing geometry checksum for ${COMPACT_FILE} in ${CHECKSUM_FILE}"
checksumGeo --maxlevel 4 -o $CHECKSUM_FILE $COMPACT_FILE

