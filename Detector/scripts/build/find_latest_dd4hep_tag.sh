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
cd $TMPDIR
if [ -d  DD4hep.git.temp  ]; then
    cd DD4hep.git.temp
    git fetch --quiet --all
else
    git clone --quiet --mirror https://github.com/AIDASoft/DD4hep.git DD4hep.git.temp    
    cd DD4hep.git.temp
fi
# should try ls-remote as well
git describe --tags master

