# Detector development how to

## Setting up the project

To download the code and setup the project (only the first time):

```sh
git clone ssh://git@gitlab.cern.ch:7999/lhcb/Detector.git
cd Detector
lb-project-init
```

## Choosing a platform and building

To be repeated in every new session:

```sh
lb-set-platform x86_64_v2-el9-gcc13-dbg
export LCG_VERSION=105
make install
```

## Viewing the geometry

At this stage, Detector is built and the the "run" script in the InstallArea:

```sh
./InstallArea/${BINARY_TAG}/bin/run geoDisplay compact/trunk/LHCb.xml
```

N.B. Due to interferences between ROOT and the llvmpipe graphics driver on *centos7*, segfaults can occur when using the ROOT visualization tools, in libLLVM-6.0-rhel.so e.g.

```log
#6  0x00007f4723b94e20 in __memcpy_ssse3 () from /lib64/libc.so.6
#7  0x00007f4706057bdb in llvm::WritableMemoryBuffer::getNewUninitMemBuffer(unsigned long, llvm::Twine const&) () from /usr/lib64/libLLVM-6.0-rhel.so
#8  0x00007f4706057caf in getMemBufferCopyImpl(llvm::StringRef, llvm::Twine const&) () from /usr/lib64/libLLVM-6.0-rhel.so
```

In that case, try using the drivers provided by EP-SFT on CVMFS:

```sh
export LIBGL_DRIVERS_PATH=/cvmfs/sft.cern.ch/lcg/contrib/mesa/18.0.5/x86_64-centos7/lib64/dri
```

It is also possible to visualize using the ROOT experimental web interface by doing:

```sh
./InstallArea/${BINARY_TAG}/bin/run geoWebDisplay compact/trunk/LHCb.xml
```

## Checking the sub-detector description

### Listing the DetElements and volumes

DD4hep features standard plugins that allow checking the description, e.g. it can dump the DetElement tree with:

```sh
./InstallArea/${BINARY_TAG}/bin/run geoPluginRun -input compact/trunk/LHCb.xml -destroy -plugin DD4hep_DetectorDump --level 10
```

or dumping the whole volume tree with commands such as:

```sh
./InstallArea/${BINARY_TAG}/bin/run geoPluginRun -input Detector/compact/trunk/LHCb.xml -plugin DD4hep_VolumeDump --shapes --positions --level 10 --pointers
```

The standard DD4hep plugins can be found in https://github.com/AIDASoft/DD4hep/blob/master/DDCore/src/plugins/StandardPlugins.cpp

#### Debugging individual subdetectors
The invocation for testing eg the `VP` is as follows:
```sh
geoPluginRun -input file:compact/components/debug/VP.xml \
                -volmgr -ui -inter \
                -plugin DD4hep_DetectorDump -positions  \
                -plugin DD4hep_VolumeDump -volume_ids -positions \
                -plugin DD4hep_GeometryDisplay -level 8
```
For more debugging options, see the associated detector description, eg `VP/detector.xml`

### Tracing leaks

In DD4hep, the DetElementObjects created by the constructors should be attached to the DetElement tree.
They are destroyed when the description itself is destroyed. Unattached DetElementObjects are memory leaks that should be avoided.

To check for them, the *DD4HEP_TRACE* environment variable can be used when running the plugins mentioned in the previous section, e.g. doing:

```sh
./InstallArea/${BINARY_TAG}/bin/run geoPluginRun -input compact/trunk/LHCb.xml -destroy -plugin DD4hep_DetectorDump --level 10
```

At the end of the program, a table should be printed, such as
```sh
+--------------------------------------------------------------------------+
|   I n s t a n c e   c o u n t e r s   b y    T Y P E I N F O             |
+----------+---------+---------+-------------------------------------------+
|   Total  |  Max    | Leaking |      Type identifier                      |
+----------+---------+---------+-------------------------------------------+
|         8|        8|        0|dd4hep::cond::ConditionsContent
|      1803|     1803|        0|dd4hep::cond::ConditionsLoadInfo
|         9|        9|        0|dd4hep::cond::ConditionDependency
|         9|        9|        0|dd4hep::cond::ConditionUpdateCall
|         1|        1|        0|dd4hep::DetectorData
|         1|        1|        0|dd4hep::DetectorImp
|     32375|    29196|     2448|dd4hep::DetElementObject
|        47|       47|        0|dd4hep::SensitiveDetectorObject
|         1|        1|        0|dd4hep::OverlayedField::Object
|     32423|    29244|     2448|dd4hep::ObjectExtensions
|        10|       10|        0|dd4hep::IDDescriptorObject
|        10|       10|        0|dd4hep::ReadoutObject
|         1|        1|        0|dd4hep::LimitSetObject
|       222|      222|        9|dd4hep::VisAttrObject
|      5498|     5498|        0|dd4hep::ConstantObject
|         1|        1|        0|dd4hep::HeaderObject
|     18674|    18674|        0|dd4hep::VolumeExtension
|     35873|    35873|        0|dd4hep::PlacedVolumeExtension
+----------+---------+---------+-------------------------------------------+
|    126966|   120608|     4905|Grand total (Sum of all counters)
+----------+-------+-------------------------------------------+
```


## Developing a subdetector

See [DETECTOR.md](doc/DETECTOR.md) for a description of the project, and  <http://dd4hep.web.cern.ch/dd4hep/> for DD4hep itself.

### Running DD4hep plugins

To run a specific plugin, the DD4hep commands are available in the environment setup by the "run" script.
e.g. to view the VP subdetector as described in compact representation:

```sh
./InstallArea/${BINARY_TAG}/bin/run geoDisplay compact/components/debug/VP.xml
```

The *geoDisplay* command keeps a ROOT prompt, with the geometry loaded. From there it is possible to:

  * export the geometry to a ROOT file for further use
  * run any ROOT tool or script, e.g. the overlap checker

### Check the dependence

The XML compact files must follow some rules in order to be able to modularize them follow the structure specified previously: subdetectors should not include XML from other components. Common parameters should be placed in speific directories. The only place where a version number can appear is in LHCb.xml, which role is to assemble the correct versions of the subdetectors.

`scripts/check_dependency.py` is to check the dependencies between compact files, should run it from the project top directory

### Updating and verifying the checksum files

In each geometry version, i.e. a subdirectory in the *compact* directory, a *checksums* file should be generated that contains
the checksums of sub-trees of the top levels of the geometry (4).

As the geometry counts millions of placements, it is unpractical to dumps the checksums of all the nodes in the checksum files: dumping the first 3 or 4 levels is enough to indicate which branch of the tree has changed. To view the differences for deeper levels, the *checksumGeo* command can be used directly.

#### Verifying the changes in all versioned geometry

```sh
./build.${BINARY_TAG}/bin/run scripts/verify_checksums.py --check version
```
Except in very rare cases, all released versions should not be changed!

#### Verifying the changes in trunk and a special version `run3/before-rich1-geom-update-26052022`

```sh
./build.${BINARY_TAG}/bin/run scripts/verify_checksums.py --check trunk
```
If you are modifying the trunk geometry, its checksums are expected to change, but please check that the changes are as expected given the modifications that you are making. Do not update the checksums by yourself, instead please ask a maintainer to do it. Maintainer can update the checksums by adding the `update-trunk-checksum` label and clicking on the `update-trunk-checksum` CI test.

#### Understanding changes in the geometry tree

*checksumGeo* can also help understanding the changes introduced and how they affect the tree.

### Running simulations

Simlations with the geometry should be run using the Gaussino tool. Please contact the simulation group for more information.

### Exporting the Geometry to GDML

It is possible to export the LHCb geometry to GDML, using the TGeo export functionality
(https://root.cern/doc/master/classTGeoManager.html#a4bf097b4cedad5f03ecce942e2c077a9)


One can load the geometry with DD4hep using geoPluginRun e.g. doing in a build of DD4hep:

```
./Detector/build.${BINARY_TAG}/bin/run geoPluginRun -interpreter -input Detector/compact/trunk/LHCb.xml  -plugin DD4hep_DetectorDump
```

One ends up in a ROOT prompt where it is possible to export with:

```
 gGeoManager->Export("LHCb.gdml")
 ```
