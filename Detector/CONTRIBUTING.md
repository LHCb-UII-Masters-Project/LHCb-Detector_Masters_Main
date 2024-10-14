# General information

This software component contains the description of the versions of the LHCb Detector
used in LHC Run3, Run4 and Run5, in the DD4hep CompactXML format.
It can be compiled independently of the LHCb stack and only
relies on ROOT and DD4hep (and their dependencies) to be available.

# Detector geometry structure

The geometry description consists of:
  - compact XML file with the parameters required to invoke DD4hep C++ plugins.
  - C++ plugin that are used to produce a ROOT TGeo object hierarchy describing the geometry.

As this component is used by the simulation software to produce samples with different versions of the geometry, it is crucial that versions used in past simulation productions
remain untouched. For this purpose a system of geometry version with associated checksums
has been put in place to verify this is the case.

## Compact XML file structure

The Compact XML description is organized in a way that separates the sub-detectors from common
infrastructure and from the overall LHCb definition, allowing to separate responsibility for the various parts of the code.
Concerning the sub-detector development:

 - **sub-detector description shall be defined in the `Detector/compact/components/<sub-detector name>/<version>` folder.**
 - **sub-detector description shall be self-contained and can only include external files from the  `Detector/compact/common` folder.**
 - **sub-detector versions shall follow the same naming conventions as for simulation tags, defined below. `trunk` is a reserved name for the latest developments.**
 - **sub-detector development and versioning is under the authority of the sub-detector teams that will create merge requests to add the new version to the code base.**
 - **the checksums for the new versions of the code have to be updated in the same merge request as the code (following the project documentation).**


 The versions of the overall LHCb Detector are managed by the `Detector` project administrators, and:

 - **The versions of the LHCb detector including shall be assembled by the LHCb admins, referring to versions of the components defined by the sub-detector teams**

## Naming conventions

The versions of the sub-detectors and of the whole of LHCb shall be called:
follow the following convention:

    <data-taking period>-<version>


- <data-taking period> is comprised of the data-taking year which is to be used, with the added convention of further granularity beyond year. For instance:
  - 2011 corresponds to a year of 2011, with validity over the entirety of the year.
  - 2022.Q1.3 corresponds to year 2022, with validity between Quarter 1 and 3 expected in Qq.q format with q∈ [1, 4], as a single character.
  - 2023.M11.12 corresponds to year 2023, with validity between Month 11 and 12, expected in Mmm.mm format with mm∈[01,12], as two characters.
  - 2024.W35.52 corresponds to year 2024, with validity between weeks 35 and 52 of the year, expected in in Www.ww format with ww∈[01,52], as two characters.
 -  Run{X} is used to accommodate nominal conditions for future data taking
- <version of tag> should follow the format vXX.YY starting with v00.00.


## C++ plugin versioning

There are several parts in the C++ code required to build the geometry and provide the interface required
by the LHCb stack:
 - the DD4hep plugins needed to constructing the geometry
 - the detector elements for the various sub-detectors, which are the interface between Gaudi algorithms
 and the detector geometry and conditions.

 Modifying them can change the description of already released version of the geometry, or break LHCb algorithms, it was therefore decided that:
 - **DD4hep geometry plugins should always be modified in a backwards compatible way**
 - **New DD4hep geometry plugins can be created to introduce backwards incompatible changes, a version number should be introduced in the plugin name to improve readability**. For example, the VP geometry plugin is called `LHCb_VP_v1_0`.
 - **Detector elements are characterized by their interface, which should therefore be backwards compatible. In case of detector upgrades, a new name should be used (e.g. the Velo DetElement name is called "VP" for Run3)**

 C++ namespaces should be used to scope classes linked to a specific run or to a specific sub-detector. This can be particularly useful for detector elements. Classes dedicated to future runs should therefore be placed in namespaces such as
   - `LHCb::Detector::Run4`
   - `LHCb::Detector::Run5`

## Documenting the versions

In order to add detailed information to the geometry versions, **a file called INFO.md shall be added to the folder containing the Compact XML, with a detailed description of the geometry modelled.**
