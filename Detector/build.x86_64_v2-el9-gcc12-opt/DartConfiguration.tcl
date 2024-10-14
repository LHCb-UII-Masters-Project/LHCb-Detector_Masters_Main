# This file is configured by CMake automatically as DartConfiguration.tcl
# If you choose not to use CMake, this file may be hand configured, by
# filling in the required variables.


# Configuration directories and files
SourceDirectory: /disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector
BuildDirectory: /disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt

# Where to place the cost data store
CostDataFile: 

# Site is something like machine.domain, i.e. pragmatic.crd
Site: epldt003.ph.bham.ac.uk

# Build name is osname-revision-compiler, i.e. Linux-2.4.2-2smp-c++
BuildName: Linux-/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/toolchain/g++-Release

# Subprojects
LabelsForSubprojects: 

# Submission information
SubmitURL: http://
SubmitInactivityTimeout: 

# Dashboard start time
NightlyStartTime: 00:00:00 EDT

# Commands for the build/test/submit cycle
ConfigureCommand: "/cvmfs/lhcb.cern.ch/lib/var/lib/LbEnv/3297/stable/linux-64/bin/cmake" "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector"
MakeCommand: /cvmfs/lhcb.cern.ch/lib/var/lib/LbEnv/3297/stable/linux-64/bin/cmake --build . --config "${CTEST_CONFIGURATION_TYPE}"
DefaultCTestConfigurationType: Release

# version control
UpdateVersionOnly: 

# CVS options
# Default is "-d -P -A"
CVSCommand: 
CVSUpdateOptions: 

# Subversion options
SVNCommand: 
SVNOptions: 
SVNUpdateOptions: 

# Git options
GITCommand: /cvmfs/lhcb.cern.ch/lib/var/lib/LbEnv/3297/stable/linux-64/bin/git
GITInitSubmodules: 
GITUpdateOptions: 
GITUpdateCustom: 

# Perforce options
P4Command: 
P4Client: 
P4Options: 
P4UpdateOptions: 
P4UpdateCustom: 

# Generic update command
UpdateCommand: /cvmfs/lhcb.cern.ch/lib/var/lib/LbEnv/3297/stable/linux-64/bin/git
UpdateOptions: 
UpdateType: git

# Compiler info
Compiler: /disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/toolchain/g++
CompilerVersion: 12.1.0

# Dynamic analysis (MemCheck)
PurifyCommand: 
ValgrindCommand: 
ValgrindCommandOptions: 
DrMemoryCommand: 
DrMemoryCommandOptions: 
CudaSanitizerCommand: 
CudaSanitizerCommandOptions: 
MemoryCheckType: 
MemoryCheckSanitizerOptions: 
MemoryCheckCommand: /cvmfs/lhcb.cern.ch/lib/lcg/releases/valgrind/3.22.0-113bc/x86_64-el9-gcc12-opt/bin/valgrind
MemoryCheckCommandOptions: 
MemoryCheckSuppressionFile: 

# Coverage
CoverageCommand: /cvmfs/lhcb.cern.ch/lib/lcg/releases/gcc/12.1.0-2435c/x86_64-el9/bin/gcov
CoverageExtraFlags: -l

# Testing options
# TimeOut is the amount of time in seconds to wait for processes
# to complete during testing.  After TimeOut seconds, the
# process will be summarily terminated.
# Currently set to 25 minutes
TimeOut: 1500

# During parallel testing CTest will not start a new test if doing
# so would cause the system load to exceed this value.
TestLoad: 

TLSVerify: 
TLSVersion: 

UseLaunchers: 
CurlOptions: 
# warning, if you add new options here that have to do with submit,
# you have to update cmCTestSubmitCommand.cxx

# For CTest submissions that timeout, these options
# specify behavior for retrying the submission
CTestSubmitRetryDelay: 5
CTestSubmitRetryCount: 3
