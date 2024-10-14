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

OPTS="${@}"

# Are conditions requested ?
if [[ $OPTS == *"-conditions"* ]]; then
    if [[ -z "${LHCB_CONDITION_DB}" ]]; then
        # default is primary git DB and master tag
        export LHCB_CONDITION_DB=git:/cvmfs/lhcb.cern.ch/lib/lhcb/git-conddb/lhcb-conditions-database.git@master
    fi
    OPTS=${OPTS/-conditions/"-conditions $LHCB_CONDITION_DB"}
fi

# Is the field map requested ?
if [[ $OPTS == *"-fieldmappath"* ]]; then
    # For now just hard code but perhaps need more control eventally ?
    OPTS=${OPTS/-fieldmappath/"-fieldmappath /cvmfs/lhcb.cern.ch/lib/lhcb/DBASE/FieldMap/v5r7/cdf"}
fi

# Finally spawn the test with the correct arguments
echo "Spawning test" geoPluginRun ${OPTS}
exec ../../bin/run geoPluginRun ${OPTS}
