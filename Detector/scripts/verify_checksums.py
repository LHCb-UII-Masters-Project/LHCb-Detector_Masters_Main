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
import os
import subprocess
import argparse
import sys
import re
from pathlib import Path


def get_xml_paths(check_entity):
    working_versions = [
        'trunk', 'before-rich1-geom-update-26052022', 'branch-baseline'
    ]
    pattern = re.compile(r'branch-\S+')
    lhcb_xml = 'LHCb.xml'
    # get the path of the current script
    script_dir = Path(__file__).resolve().parent
    # get the path of the compact directory
    compact_path = script_dir.parent / 'compact'
    run_folders = [
        folder for folder in compact_path.iterdir()
        if folder.is_dir() and folder.name.startswith("run")
    ]
    xml_paths = []
    for folder in run_folders:
        for version in folder.iterdir():
            if check_entity == 'working':
                if any(working_version in version.name for working_version in
                       working_versions) or pattern.match(version.name):
                    xml_paths.append(version / lhcb_xml)
            elif check_entity == 'version':
                if all(working_version not in version.name
                       for working_version in
                       working_versions) and not pattern.match(version.name):
                    xml_paths.append(version / lhcb_xml)
    print(xml_paths)
    return xml_paths


def main():
    parser = argparse.ArgumentParser(description="")
    parser.add_argument(
        '--check', type=str, required=True, choices=['working', 'version'])
    args = parser.parse_args()

    error_messages = []
    xml_paths = get_xml_paths(args.check)
    for xml_path in xml_paths:
        new_checksum_file = xml_path.parent / "checksums.new"
        subprocess.run([
            "checksumGeo", "--maxlevel", "4", "-o", new_checksum_file, xml_path
        ],
                       check=True)
        old = (xml_path.parent / 'checksums').open().readlines()
        new = new_checksum_file.open().readlines()
        if old != new:
            from difflib import unified_diff
            sys.stdout.writelines(
                unified_diff(
                    old,
                    new,
                    fromfile=str(xml_path.parent / 'checksums'),
                    tofile=str(new_checksum_file)))
            error_messages.append(
                f"ERROR: Checksums in {xml_path.parent}: is changed")
        else:
            new_checksum_file.unlink()
    if error_messages:
        for error_message in error_messages:
            print(error_message, file=sys.stderr)
        sys.exit(1)
    else:
        print("All checksums are OK.")


if __name__ == '__main__':
    main()
