#################################################################################
#
#  Environment script for DD4hep examples - initializes DD4hep (and ROOT)
#  for package: Detector
# 
#  @author F.Gaede, DESY, 2013
#  @author M.Frank, CERN, 2015
#
#################################################################################
# Default of DD4hep is the primary installation directory
if [ ! ${DD4hep_DIR} ]; then
    export DD4hep_DIR=/cvmfs/lhcb.cern.ch/lib/lcg/releases/DD4hep/01.27.02-a9b45/x86_64-el9-gcc12-opt;
fi;
if [  ]; then
    export CLHEP_DIR=;
fi;
source ${DD4hep_DIR}/bin/thisdd4hep.sh;
#
SOURCE=${BASH_ARGV[0]}
if [ "x$SOURCE" = "x" ]; then
    SOURCE=${(%):-%N} # for zsh
fi

dd4hep_parse_this $SOURCE  Detector;
#
#----PATH---------------------------------------------------------------------
dd4hep_add_path    PATH ${THIS}/bin;
#----PYTHONPATH---------------------------------------------------------------
dd4hep_add_path    PYTHONPATH ${THIS}/lib/python3.9/site-packages;
#----ROOT_INCLUDE_PATH--------------------------------------------------------
dd4hep_add_path    ROOT_INCLUDE_PATH ${THIS}/include;
#----LIBRARY_PATH-------------------------------------------------------------
dd4hep_add_library_path ${THIS}/lib;
# -- need to extend dynamic search path for all external libraries:
if [  ]; then
    for lp in ; do
	dd4hep_add_library_path ${lp};
    done;
fi;
