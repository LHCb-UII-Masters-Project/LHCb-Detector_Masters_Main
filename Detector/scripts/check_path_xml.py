###############################################################################
# (c) Copyright 2000-2018 CERN for the benefit of the LHCb Collaboration      #
#                                                                             #
# This software is distributed under the terms of the GNU General Public      #
# Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   #
#                                                                             #
# In applying this licence, CERN does not waive the privileges and immunities #
# granted to it by virtue of its status as an Intergovernmental Organization  #
# or submit itself to any jurisdiction.                                       #
###############################################################################

import os
import re
import xml.etree.ElementTree as ET

# Checks if the `path.xml` in each released version of the subdetector contains only paths to released versions of the detector


def read_xml(filename):
    log = ""
    success = True

    tree = ET.parse(filename)
    root = tree.getroot()

    define_branch = root[0]
    if define_branch.tag != "define":
        log += "First tag is not a define, does not follow path.xml schema \n"
        success = False

    for child in define_branch:
        det_path = child.attrib["value"]
        det_name = child.attrib["name"].split(":")[1]

        non_detectors = ["VMA", "Pipe", "Materials", "Rich",
                         "Regions"]  # Needs cleaning
        if det_name not in non_detectors:
            det_full_path = filename.replace(
                'path.xml', '') + det_path + "/" + det_name + ".xml"
            if not os.path.isfile(det_full_path):
                log += det_full_path + " does not exist\n"
                success = False

        if child.tag != "constant":
            log += "Expecting a constant tag, this does not follow path.xml schema \n"
            success = False

        if "trunk" in det_path:
            log += "Trunk found in " + child.attrib['name'] + "\n"
            success = False

    if not success:
        log += "There is an error, check error messages\n"

    return success, log


def main():
    full_log = ""
    Success = True
    list_of_allow_trunk = {
        "components", "common", "trunk", "debug",
        "before-rich1-geom-update-26052022", "branch-baseline"
    }
    pattern = re.compile(r'branch-\S+')
    for subdir, dirs, files in os.walk("compact/"):
        subdir_allowed = any(i in subdir for i in list_of_allow_trunk)
        if subdir_allowed or (pattern.match(subdir) and not subdir_allowed):
            continue

        pathfile = "path.xml"
        if pathfile not in files:
            continue

        fullpathfile = os.path.join(subdir, pathfile)

        print("Testing " + fullpathfile)
        temp_success, log = read_xml(fullpathfile)
        full_log += log
        if not temp_success:
            Success = False

    if not Success:
        raise ValueError(full_log)


if __name__ == "__main__":
    main()
