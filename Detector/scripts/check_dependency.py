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

import os
import re
import xml.etree.ElementTree as ET


def extract_includes(xml_file):
    includes = []
    try:
        tree = ET.parse(xml_file)
    except ET.ParseError as e:
        e.msg += f" ({xml_file})"
        raise e
    root = tree.getroot()
    pattern = re.compile(r"(?<!\.\./)\.\./(\.\./)+")
    for include_elem in (root.findall(".//gdmlFile") +
                         root.findall(".//include") + root.findall(".//file")):
        ref_value = include_elem.get("ref")
        if ref_value and pattern.search(ref_value):
            includes.append(ref_value)
    return includes


def check_dependency(xml_file, include_paths):
    for path in include_paths:
        if "common" in path or "Pipe" in path:
            continue
        return xml_file
    return None


error_messages = []
for root, dirs, files in os.walk("compact/components/"):
    if root.startswith("compact/components/debug"):
        continue
    for file_name in files:
        if file_name.endswith(".xml") and "Doc" not in file_name:
            include_paths = extract_includes(root + "/" + file_name)
            if include_paths:
                wrong_xml = check_dependency(file_name, include_paths)
                if wrong_xml:
                    error_messages.append(f"{root}/{wrong_xml}")
if error_messages:
    error_messages.insert(
        0,
        "subdetectors should not include XML from other components, please check the following files:"
    )
    raise ValueError("\n".join(error_messages))
