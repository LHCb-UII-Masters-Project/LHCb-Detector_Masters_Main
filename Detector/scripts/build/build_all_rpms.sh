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


source /cvmfs/sft.cern.ch/lcg/releases/gcc/7.3.0/x86_64-centos7/setup.sh
ARCH=$(uname -m)
export PATH=/cvmfs/sft.cern.ch/lcg/contrib/CMake/3.11.1/Linux-${ARCH}/bin:${PATH}


git clone -b LCG_94b  https://gitlab.cern.ch/bcouturi/lcgcmake.git
sed -i "s/LCG_AA_project(DD4hep .*)/LCG_AA_project(DD4hep ${latest_tag})/"  lcgcmake/cmake/toolchain/heptools-94.cmake

git clone https://gitlab.cern.ch/sft/lcgjenkins.git
mkdir lcgcmake-build
cd  lcgcmake-build
cmake -DLCG_INSTALL_PREFIX=/cvmfs/sft.cern.ch/lcg/releases -DLCG_VERSION=94 -DLCG_IGNORE='DD4hep;Geant4' -DCMAKE_CXX_STANDARD=17 -DCMAKE_INSTALL_PREFIX=../lcgcmake-install ../lcgcmake
make -j 32 DD4hep
cd ../lcgcmake-install/
../lcgjenkins/extract_LCG_summary.py . x86_64-centos7-gcc7-opt 94 RELEASE
 ../lcgjenkins/LCGRPM/package/createLCGRPMSpec.py -p x86_64-centos7-gcc7-opt --release=1 --match="(.*DD4hep.*|.*Geant4*)" -o all.spec ./LCG_externals_x86_64-centos7-gcc7-opt.txt
rpmbuild -bb all.spec


