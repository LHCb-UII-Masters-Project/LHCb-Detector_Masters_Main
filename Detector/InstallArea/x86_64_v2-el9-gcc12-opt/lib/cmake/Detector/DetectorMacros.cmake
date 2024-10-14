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
#[========================================================================[.rst:
DetectorMacros
==============

Useful macros for the configuration of the Detector or downwnstream projects.

add_detector_plugin
-------------------

::

 add_detector_plugin(name sources...)

Create a plugin library for Detector/DD4hep with the given name from the
listed sources.

#]========================================================================]

function(add_detector_plugin name)
    add_library(${name} MODULE ${ARGN})
    target_link_libraries(${name} Detector::DetectorLib)

    if(NOT _LISTCOMPONENTS_ENV_GENERATED)
        file(GENERATE OUTPUT ${PROJECT_BINARY_DIR}/bin/listcomponentsenv.sh
                      CONTENT "#!/bin/sh
${RUN_SCRIPT_EXTRA_COMMANDS}
export LD_PRELOAD=$PRELOAD_SANITIZER_LIB  # workaround for issue #32
export LD_LIBRARY_PATH=.:$<TARGET_FILE_DIR:DD4hep::DD4hepGaudiPluginMgr>:$ENV{LD_LIBRARY_PATH}
exec \"$@\"
")
    endif()
    set(_LISTCOMPONENTS_ENV_GENERATED TRUE PARENT_SCOPE)

    # we need the .components in the same directory as the target library, but
    # also in a place we can install it from so it has to be listed in BYPRODUCTS,
    # but BYPRODUCTS cannot use generator expressions
    # The idea of the helper variable copy_cmd comes from
    # https://cmake.org/cmake/help/latest/manual/cmake-generator-expressions.7.html#whitespace-and-quoting
    set(copy_cmd ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:${name}>/${name}.components ${CMAKE_CURRENT_BINARY_DIR}/${name}.components)
    add_custom_command(TARGET ${name} POST_BUILD
        # COMMAND env LD_LIBRARY_PATH=.:$<TARGET_FILE_DIR:DD4hep::DD4hepGaudiPluginMgr>:$ENV{LD_LIBRARY_PATH}
        # FIXME listcomponents.sh is a workaround for #32
        # Once fixed at the source, revert to the commented line above.
        COMMAND bash ${PROJECT_BINARY_DIR}/bin/listcomponentsenv.sh
            $<TARGET_FILE:DD4hep::listcomponents>
                -o $<TARGET_FILE_DIR:${name}>/${name}.components
                $<TARGET_FILE_NAME:${name}>
        COMMAND "$<$<NOT:$<STREQUAL:$<TARGET_FILE_DIR:${name}>,${CMAKE_CURRENT_BINARY_DIR}>>:${copy_cmd}>"
        WORKING_DIRECTORY $<TARGET_FILE_DIR:${name}>
        BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/${name}.components
        COMMAND_EXPAND_LISTS
        VERBATIM)
    # components file destination should match module destination
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${name}.components DESTINATION lib)
    install(TARGETS ${name} DESTINATION lib)
endfunction()
