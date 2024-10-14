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
#    sed 's/ volume=/ logvol=/g' |
#
sed 's/ = /=/g' ${1} | \
    sed 's/ = /=/g; s/ = /=/g; s/ = /=/g'  | \
    sed 's/ =/=/g; s/ =/=/g; s/ =/=/g; s/ =/=/g; s/ =/=/g; s/ =/=/g; s/ =/=/g; s/ =/=/g; s/ =/=/g'  | \
    sed 's/= /=/g; s/= /=/g; s/= /=/g; s/= /=/g; s/= /=/g'  | \
    sed 's/	/        /g' |\
    sed 's/<composite fractionmass=\"/<fraction n=\"/g' |\
    sed 's/<component fractionmass=\"/<fraction n=\"/g' |\
    sed 's/<component/<composite/g' |\
    sed 's/<composite name=/<composite ref=/g' |\
    sed 's/ natoms=\"/ n=\"/g' |\
    sed 's/ density=\"/>\n    <D type=\"density\" value=\"/g' |\
    sed 's/\*g\/cm3\"/\" unit=\"g\/cm3\"\//g'  |\
    sed 's/ I=\"/>\n    <ION type=\"ionization\" unit=\"ev\" value=\"/g' |\
    sed 's/\*ev\"/\" unit=\"ev\"\//g'  |\
    sed 's/\*ev\">/\"\/>/g'  |\
    sed 's/<parameter/<constant/g' |\
    sed 's/<logvol/<volume/g' |\
    sed 's/<\/logvol/<\/volume/g' |\
    sed 's/ logvol=/ volume=/g' |
    sed 's/\/dd\/Geometry\/BeforeMagnetRegion\///g' |\
    sed 's/UT\/Sensors\///g' |\
    sed 's/<box /<shape type=\"Box\" /g' |\
    sed 's/<trd /<shape type=\"Trap\" /g' |\
    sed 's/<tubs /<shape type=\"Tube\" /g' |\
    sed 's/<trap /<shape type=\"Trap\" /g' |\
    sed 's/<cons /<shape type=\"ConeSegment\" /g' |\
    sed 's/<polycone /<shape type=\"Polycone\" /g' |\
    sed 's/<union/\<shape type=\"BooleanShape\" operation="union"/g' |\
    sed 's/<subtraction/<shape type=\"BooleanShape\" operation="subtraction"/g' |\
    sed 's/<intersection/<shape type=\"BooleanShape\" operation="intersection"/g' |\
    sed 's/<\/box>/<\/shape>/g' |\
    sed 's/<\/tubs>/<\/shape>/g' |\
    sed 's/<\/cons>/<\/shape>/g' |\
    sed 's/<\/polycone>/<\/shape>/g' |\
    sed 's/<\/union>/<\/shape>/g' |\
    sed 's/<\/subtraction>/<\/shape>/g' |\
    sed 's/<\/intersection>/<\/shape>/g' |\
    sed 's/outerRadius/rmax/g; s/innerRadius/rmin/g' |\
    sed 's/startPhiAngle/startphi/g; s/deltaPhiAngle/deltaphi/g' |\
    sed 's/posXYZ/position/g; s/rotXYZ/rotation/g; s/rotX/x/g; s/rotY/y/g; s/rotZ/z/g' |\
    sed 's/posRPhiZ/positionRPhiZ/g' |\
    sed 's/sizeX=\"/dx=\"0.5*/g; s/sizeY=\"/dy=\"0.5*/g; s/sizeZ=\"/dz=\"0.5*/g' |\
    sed 's/sizeX1=\"/x1=\"0.5*/g; s/sizeY1=\"/y1=\"0.5*/g; s/sizeZ1=\"/z1=\"0.5*/g' |\
    sed 's/sizeX2=\"/x2=\"0.5*/g; s/sizeY2=\"/y2=\"0.5*/g; s/sizeZ2=\"/z2=\"0.5*/g' |\
    sed 's/sizeX3=\"/x3=\"0.5*/g; s/sizeX4=\"/x4=\"0.5*/g; s/sizeX5=\"/x5=\"0.5*/g' |\
    sed 's/rminPZ=/rmin2=/g; s/rminMZ=/rmin1=/g; s/rmaxPZ=/rmax2=/g; s/rmaxMZ=/rmax1=/g' |\
    sed 's/Pipe\/Pipe/Pipe:/g' |\
    sed 's/BPMSWUpStr/UpStr:BPMSW/g' |\
    sed 's/MBXWSUpStr/UpStr:MBXWS/g' |\
    sed 's/UX84Up/UpStr:UX84/g' |\
    sed 's/UX86Ada/UpStr:UX86Ada/g' |\
    sed 's/ \/>/\/>/g' > ${1}.tmp;
cat ${1}.tmp;
mv ${1}.tmp ${1};

