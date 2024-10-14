#!/bin/bash
###############################################################################
# (c) Copyright 2020-2022 CERN for the benefit of the LHCb Collaboration      #
#                                                                             #
# This software is distributed under the terms of the GNU General Public      #
# Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   #
#                                                                             #
# In applying this licence, CERN does not waive the privileges and immunities #
# granted to it by virtue of its status as an Intergovernmental Organization  #
# or submit itself to any jurisdiction.                                       #
###############################################################################

# Test name
#export TEST_NAME=LHCb_TEST_load_derich1
export TEST_NAME=LHCb_TEST_dump_Rich1

# Location of the Conditions DB (if not defined externally)
if [[ -z "${LHCB_CONDITION_DB}" ]]; then
    export LHCB_CONDITION_DB=git:/cvmfs/lhcb.cern.ch/lib/lhcb/git-conddb/lhcb-conditions-database.git@master
fi

# test arguments
export ARGS="geoPluginRun -input ../compact/run3/trunk/LHCb.xml -destroy -plugin ${TEST_NAME} -conditions ${LHCB_CONDITION_DB}"
if [ "$TEST_NAME" == "LHCb_TEST_load_derich1" ]; then
    export ARGS=${ARGS}" -fieldmappath /cvmfs/lhcb.cern.ch/lib/lhcb/DBASE/FieldMap/v5r7/cdf"
fi

# run the test ...
echo ${ARGS}
../build.${BINARY_TAG}/bin/run ${ARGS}
