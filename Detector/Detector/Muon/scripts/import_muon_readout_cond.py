#!/usr/bin/env python3
# -*- coding: utf-8 -*-
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
"""
Helper script used to convert Conditions/Muon/ReadoutConf/Modules.xml into YAML.
"""

import requests
from xml.etree import ElementTree as ET
from pathlib import Path


def main():
    """
    Main function
    """
    input_url = "https://gitlab.cern.ch/lhcb-conddb/SIMCOND/-/raw/upgrade/master/Conditions/Muon/ReadoutConf/Modules.xml"
    project_root = Path(__file__).resolve().parents[3]
    output_path = project_root / "tests/ConditionsIOV/Conditions/Muon/ReadoutConf/Modules.yml"

    xml = ET.fromstring(requests.get(input_url).text)

    output = ["---\n", "ReadoutModules:\n"]
    for cond in xml.findall("condition"):
        output.append("  {}:\n".format(cond.attrib["name"].replace(
            "Readout", "")))
        for readout in cond[0]:
            first_key = True
            for key, value in readout.attrib.items():
                if key.startswith("Cluster"):
                    # we need special handling of cluster data
                    continue
                if "," in value:
                    value = value.split(",")
                    if len(value) < 10:
                        output.append("      {}: [{}]\n".format(
                            key, ", ".join(value)))
                    else:
                        output.append("      {}: [\n".format(key))
                        while value:
                            chunk = value[:10]
                            output.append("        {},\n".format(
                                ", ".join(chunk)))
                            value[:10] = []
                        output.append("      ]\n")
                else:
                    output.append("      {}: {}\n".format(key, value))
                if first_key:  # add a "-" in front of the first attribute
                    first_key = False
                    output[-1] = "    - " + output[-1][6:]
                if key.startswith("PadEdgeSigma"):
                    # after PadEdgeSigma{X,Y} we handle cluster data
                    axis = key[-1]
                    sizes = readout.attrib[f"ClusterSize{axis}"].split(",")
                    probs = readout.attrib[f"ClusterProb{axis}"].split(",")
                    output.append("      Cluster{}: [{}]\n".format(
                        axis, ", ".join("{{size: {}, prob: {}}}".format(s, p)
                                        for (s, p) in zip(sizes, probs))))

    output_path.open("w").writelines(output)
    print("written", output_path)


if __name__ == "__main__":
    main()
