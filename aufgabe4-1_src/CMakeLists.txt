cmake_minimum_required( VERSION 3.20 FATAL_ERROR )
# Disallow in-source builds as it may mess up the sources
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}" )
	message( FATAL_ERROR
		"FANTOM requires an out of source build. "
		"Please create a separate binary directory and run cmake there." )
endif()
# set the default build type if none was set
if( NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES )
	message( STATUS "Setting build type to 'Debug' as none was specified." )
	set( CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE )
	# set the possible values of build type for cmake-gui
	set_property( CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo" )
endif()


################################################################################
project( aufgabe4_1_toolbox )
set( FANTOM_TOOLBOX_NAME "aufgabe4-1" CACHE STRING "Name of the toolbox to generate" )
################################################################################


# set up compiler flags
if( CMAKE_COMPILER_IS_GNUCXX ) # gcc
  set( BUILD_CXX_DIALECT "-pedantic -Wall -Wextra -Wno-unknown-pragmas -Wnon-virtual-dtor -Wunused -Woverloaded-virtual" CACHE STRING "c++ dialect and warnings" )
  if( CMAKE_SYSTEM_NAME MATCHES "Windows" )
    set( BUILD_CXX_DIALECT "${BUILD_CXX_DIALECT} -U__STRICT_ANSI__" )
  endif()
elseif( CMAKE_CXX_COMPILER_ID MATCHES "Clang" ) # Clang or AppleClang
  set( BUILD_CXX_DIALECT "-pedantic -Wall -Wextra -Wno-unknown-pragmas -Wnon-virtual-dtor -Wunused -Woverloaded-virtual" CACHE STRING "c++ dialect and warnings" )
else() # unknown compiler
  message( "unknown compiler" )
  set( BUILD_CXX_DIALECT "" CACHE STRING "c++ dialect and warnings" )
endif()
mark_as_advanced( FORCE BUILD_CXX_DIALECT )
separate_arguments( BUILD_CXX_DIALECT )

if( CMAKE_COMPILER_IS_GNUCXX ) #gcc
  # FIXME: this doesn't seem to work on clang :-(
  set( CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined ${CMAKE_SHARED_LINKER_FLAGS}" )
endif()

# Activate compilation database
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

# Must use GNUInstallDirs to install libraries into correct
# locations on all platforms.
include(GNUInstallDirs)

# find FAnToM installation dir and setup FAnToMs cmake libraries
# First check if FANTOM_DIR is set as CMake variable or environment variable
if( NOT FANTOM_DIR )
	set( FANTOM_DIR $ENV{FANTOM_DIR} )
endif()

# If still not set, try to find it
if( NOT FANTOM_DIR )
	find_path( FANTOM_DIR NAMES share/FAnToM
		PATHS
		/usr
		/usr/local
		${CMAKE_SOURCE_DIR}/../FAnToM_MacOS26
		${CMAKE_SOURCE_DIR}/../../FAnToM_MacOS26
	)
endif()

if( NOT FANTOM_DIR OR ${FANTOM_DIR} STREQUAL "FANTOM_DIR-NOTFOUND" )
	message( FATAL_ERROR "FAnToM installation directory not set. Please specify FANTOM_DIR to point at /usr, /usr/local, or any other path where a local FAnToM has been installed to.\nExample: cmake -D FANTOM_DIR=/path/to/FAnToM ../" )
endif()

message( STATUS "Using FAnToM directory: ${FANTOM_DIR}" )
list( APPEND CMAKE_MODULE_PATH ${FANTOM_DIR}/share/FAnToM/cmake/ )

include( FAnToMMacros )


# running and debugging
add_custom_target(run
	COMMAND "${FANTOM_DIR}/bin/fantom" -P "${CMAKE_BINARY_DIR}/lib/fantom-plugins"
	WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
	COMMENT "Launching FAnToM with locally built Aufgabe 4.1 plugins" )
add_custom_target(debug
	COMMAND lldb "${FANTOM_DIR}/bin/fantom" -- -P "${CMAKE_BINARY_DIR}/lib/fantom-plugins"
	WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
	COMMENT "Debugging Aufgabe 4.1 plugins inside FAnToM" )

# build the plugin
file(GLOB DIR_CONTENTS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} * )
foreach( DIR ${DIR_CONTENTS} )
	if( IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${DIR} AND NOT ${DIR} STREQUAL "build" )
		FANTOM_ADD_PLUGIN_DIRECTORY( ${DIR} )
		add_dependencies( run ${FANTOM_TOOLBOX_NAME}_${DIR} )
		add_dependencies( debug ${FANTOM_TOOLBOX_NAME}_${DIR} )
	endif()
endforeach()
