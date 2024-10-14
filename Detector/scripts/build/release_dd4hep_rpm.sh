#!/bin/bash
###############################################################################
# (c) Copyright 2020 CERN for the benefit of the LHCb Collaboration           #
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


latest_tag=$(${SCRIPT_LOC}/find_latest_dd4hep_tag.sh)
echo "Latest DD4hep is: ${latest_tag}"

TORELEASE=$(ls /tmp/rpmbuild/RPMS/noarch/*DD4hep*${latest_tag//[-]/_}*)
echo "Going to release:"
for f in ${TORELEASE}; do 
echo "- ${f}"
done

TARGET=/eos/project/l/lhcbwebsites/www/lhcb-rpm/incubator
for f in ${TORELEASE}; do
echo "copying ${f} to ${TARGET}"
cp  ${f} ${TARGET}
done

echo "Updating RPM metadata"
createrepo --update ${TARGET}
