CMAKE_MINIMUM_REQUIRED(VERSION 3.4)

cmake_policy(SET CMP0026 OLD)
cmake_policy(SET CMP0039 NEW)
cmake_policy(SET CMP0042 OLD)
cmake_policy(SET CMP0043 OLD)
cmake_policy(SET CMP0045 OLD)
cmake_policy(SET CMP0048 OLD)
cmake_policy(SET CMP0057 NEW)

PROJECT(Designer)
SET(CMAKE_INSTALL_LIBDIR lib) # Fix for non debian-based distrib, as it seeems to be a common occurence to name this folder lib32/lib64
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake-modules")
# OpenViBE-specific helper functions that are used by the CMakeLists of the subprojects.
INCLUDE("OvAddProjects")
INCLUDE("OvInstallLaunchScript")
INCLUDE("OvLinkBoostLib")
INCLUDE("OvDelayLoad")
INCLUDE("SetProjectVersion")
INCLUDE("Utilities") # Include functions debug_message, set_build_platform

# ----------------------
# Build CMake flags
# ----------------------
OPTION(Flag_VerboseOutput "Verbose CMake output" OFF)
SET(OEM_DISTRIBUTION "openvibe" CACHE STRING "The OEM distribution for this build, can be openvibe or mensia")
OPTION(OV_DISPLAY_ERROR_LOCATION ON)

SET(OV_SOURCE_DEPENDENCIES_PATH "${CMAKE_SOURCE_DIR}/dependencies-source")
SET(OV_ICON_PATH "${CMAKE_SOURCE_DIR}/scripts/icons")

if(NOT(${Flag_VerboseOutput}))
	set(CMAKE_INSTALL_MESSAGE LAZY)
endif()

IF(WIN32)
	IF("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
		SET(OV_DEPENDENCIES_PLATFORM_FOLDER_NAME "dependencies_x64")
	ELSEIF("${CMAKE_SIZEOF_VOID_P}" EQUAL "4")
		SET(OV_DEPENDENCIES_PLATFORM_FOLDER_NAME "dependencies")
	ELSE()
		SET(OV_DEPENDENCIES_PLATFORM_FOLDER_NAME "dependencies")
	ENDIF()
ELSE()
	SET(OV_DEPENDENCIES_PLATFORM_FOLDER_NAME "dependencies")
ENDIF()

LIST(APPEND LIST_DEPENDENCIES_PATH "${CMAKE_SOURCE_DIR}/${OV_DEPENDENCIES_PLATFORM_FOLDER_NAME}")

# ----------------------
# Set build version
# ----------------------

# These versions are used by the subprojects by default.
set_version()
# Sets the PROJECT_VERSION variable to something, depending on overriding OvSetProjectVersion.cmake files
OV_SET_PROJECT_VERSION()
MESSAGE(STATUS "BUILDING VERSION Designer: ${OV_GLOBAL_VERSION_STRING} Branch : ${OV_PROJECT_BRANCH}~${OV_PROJECT_COMMITHASH}. Based on OpenViBE SDK: ${OV_SDK_VERSION_STRING}")

IF(NOT CMAKE_INSTALL_PREFIX)
	# Default is to build to dist/. If you wish a custom install, set your own MAKE_INSTALL_PREFIX when you call CMake. Safest to do under a fakeroot.
	SET(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/dist")
ENDIF()

IF(NOT DEFINED OV_PACKAGE)
	SET(OV_PACKAGE FALSE)
ENDIF()

# Setup knowledge of GNU-style install path variables
INCLUDE("GNUInstallDirs")

if(NOT CMAKE_BUILD_TYPE AND CMAKE_GENERATOR MATCHES "Visual Studio*")
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_INSTALL_PREFIX}/bin" )
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_INSTALL_PREFIX}/lib" )
	foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
		string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIGU )
		string(CONCAT DIST_ROOT ${DIST_ROOT} $<$<CONFIG:${OUTPUTCONFIGU}>:${CMAKE_INSTALL_PREFIX}/${OUTPUTCONFIG}>)
		set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIGU} "${CMAKE_INSTALL_PREFIX}/${OUTPUTCONFIG}/bin" )
		set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIGU} "${CMAKE_INSTALL_PREFIX}/${OUTPUTCONFIG}/lib" )
	endforeach()
	set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS "OV_CMAKE_PATH_ROOT=\"${DIST_ROOT}\"")
	set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS "OV_CMAKE_PATH_BIN=\"${DIST_ROOT}/bin\"")
	set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS "OV_CMAKE_PATH_LIB=\"${DIST_ROOT}/lib\"")
	set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS "OV_CMAKE_PATH_DATA=\"${DIST_ROOT}/share/openvibe\"")
	set(DIST_BINDIR ${DIST_ROOT}/bin)
	set(DIST_LIBDIR ${DIST_ROOT}/lib)
	set(DIST_INCLUDEDIR ${DIST_ROOT}/include)
	set(DIST_DATADIR ${DIST_ROOT}/share)
	set(DIST_DOCDIR ${DIST_ROOT}/doc)
	set(DIST_DOCTMP ${DIST_ROOT}/doc-tmp)
	set(DIST_SYSCONFDIR ${DIST_ROOT}/etc)
	# Install the .install file inside the dists folders
	install(FILES ${CMAKE_BINARY_DIR}/.install DESTINATION ${DIST_ROOT})
elseif(CMAKE_BUILD_TYPE AND OV_PACKAGE)
	# Add directories that OpenViBE will use to look for its components runtime, unless overridden by environment variables in the launch scripts.
	# These defines should only be used in "openvibe/ov_directories.h".

	add_definitions(-DOV_CMAKE_PATH_ROOT="${CMAKE_INSTALL_PREFIX}")
	add_definitions(-DOV_CMAKE_PATH_BIN="${CMAKE_INSTALL_FULL_BINDIR}")
	add_definitions(-DOV_CMAKE_PATH_LIB="${CMAKE_INSTALL_FULL_LIBDIR}")
	add_definitions(-DOV_CMAKE_PATH_DATA="${CMAKE_INSTALL_FULL_DATADIR}/openvibe")
	set(CMAKE_INSTALL_PREFIX .)
	set(DIST_ROOT ${CMAKE_INSTALL_PREFIX})
	set(DIST_BINDIR ${CMAKE_INSTALL_BINDIR})
	set(DIST_LIBDIR ${CMAKE_INSTALL_LIBDIR})
	set(DIST_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR})
	set(DIST_DATADIR ${CMAKE_INSTALL_DATADIR})
	set(DIST_DOCDIR ${DIST_ROOT}/doc)
	set(DIST_DOCTMP ${DIST_ROOT}/doc-tmp)
	set(DIST_SYSCONFDIR ${CMAKE_INSTALL_SYSCONFDIR})
	set(CPACK_GENERATOR "ZIP")
	set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
	set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OpenViBE Designer")
	set(CPACK_PACKAGE_VENDOR "Inria & Mensia Technologies")
	set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
	set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING.md")
	set(CPACK_PACKAGE_VERSION ${OV_GLOBAL_VERSION_MAJOR}.${OV_GLOBAL_VERSION_MINOR}.${OV_GLOBAL_VERSION_PATCH}.${OV_GLOBAL_VERSION_BUILD})
	set(CPACK_PACKAGE_VERSION_MAJOR ${OV_GLOBAL_VERSION_MAJOR})
	set(CPACK_PACKAGE_VERSION_MINOR ${OV_GLOBAL_VERSION_MINOR})
	set(CPACK_PACKAGE_VERSION_PATCH ${OV_GLOBAL_VERSION_PATCH})
	if(UNIX)
		execute_process(COMMAND bash -c "echo -n $(lsb_release -i -s)$(lsb_release -r -s)"
			OUTPUT_VARIABLE CPACK_SYSTEM_NAME)
	endif()

	IF("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
		SET(PLATFORM_TARGET "x64")
	ELSEIF("${CMAKE_SIZEOF_VOID_P}" EQUAL "4")
		SET(PLATFORM_TARGET "x86")
	ELSE()
		SET(PLATFORM_TARGET "Unknown")
	ENDIF()

	set(CPACK_PACKAGE_NAME "OpenViBE${CMAKE_PROJECT_NAME}-${CMAKE_BUILD_TYPE}-${PLATFORM_TARGET}")
	include(CPack)
elseif(CMAKE_BUILD_TYPE)
	# Add directories that OpenViBE will use to look for its components runtime, unless overridden by environment variables in the launch scripts.
	# These defines should only be used in "openvibe/ov_directories.h".
	add_definitions(-DOV_CMAKE_PATH_ROOT="${CMAKE_INSTALL_PREFIX}")
	add_definitions(-DOV_CMAKE_PATH_BIN="${CMAKE_INSTALL_FULL_BINDIR}")
	add_definitions(-DOV_CMAKE_PATH_LIB="${CMAKE_INSTALL_FULL_LIBDIR}")
	add_definitions(-DOV_CMAKE_PATH_DATA="${CMAKE_INSTALL_FULL_DATADIR}/openvibe")
	set(DIST_ROOT ${CMAKE_INSTALL_PREFIX})
	set(DIST_BINDIR ${CMAKE_INSTALL_FULL_BINDIR})
	set(DIST_LIBDIR ${CMAKE_INSTALL_FULL_LIBDIR})
	set(DIST_INCLUDEDIR ${CMAKE_INSTALL_FULL_INCLUDEDIR})
	set(DIST_DATADIR ${CMAKE_INSTALL_FULL_DATADIR})
	set(DIST_DOCDIR ${DIST_ROOT}/doc)
	set(DIST_DOCTMP ${DIST_ROOT}/doc-tmp)
	set(DIST_SYSCONFDIR ${CMAKE_INSTALL_FULL_SYSCONFDIR})
else()
	message(FATAL_ERROR "Build should specify a type or use a multi-type generator (like Visual Studio)")
endif()

find_path(SDK_MODULE_PATH NAMES "AddOpenViBESDKComponents.cmake" PATHS ${OPENVIBE_SDK_PATH} PATH_SUFFIXES share NO_DEFAULT_PATH) # Finds multi-type builds

if(CMAKE_BUILD_TYPE)
	string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_LOWER)
	find_path(SDK_MODULE_PATH NAMES AddOpenViBESDKComponents.cmake PATHS ${OPENVIBE_SDK_PATH};${LIST_DEPENDENCIES_PATH} PATH_SUFFIXES openvibe-sdk-${CMAKE_BUILD_TYPE_LOWER}/share NO_DEFAULT_PATH) # Find "regular builds
else()
	find_path(SDK_MODULE_PATH NAMES "AddOpenViBESDKComponents.cmake" PATHS ${LIST_DEPENDENCIES_PATH} PATH_SUFFIXES openvibe-sdk-Debug/share openvibe-sdk-Release/share NO_DEFAULT_PATH) # Finds multi-type builds
endif()

if(SDK_MODULE_PATH)
	LIST(APPEND CMAKE_MODULE_PATH ${SDK_MODULE_PATH})
endif()
include(FindOVSDK)
# include(AddOpenViBESDKComponents)
# OpenViBE SDK modules list
SET(INCLUDED_OV_SDK_COMPONENTS BASE ALLPLUGINS EBML SYSTEM FS XML TOOLKIT)


string(TIMESTAMP COPYRIGHT_DATE "2012-%Y")

# We deduce the branding from the OEM_DISTRIBUTION
IF(OEM_DISTRIBUTION STREQUAL "openvibe")
	SET(OV_CONFIG_SUBDIR "OpenVIBE" CACHE STRING "Subdirectory under user directory when configuration and logs will be saved")
	SET(BRAND_NAME "OpenViBE")
	SET(COMPANY_NAME "Inria")
	SET(DESIGNER_NAME "Designer")
	SET(LICENCE_TEXT "\
This program is free software: you can redistribute it and/or modify\
it under the terms of the GNU Affero General Public License as\
published by the Free Software Foundation, either version 3 of the\
License, or (at your option) any later version.\
\
This program is distributed in the hope that it will be useful, but\
WITHOUT ANY WARRANTY; without even the implied warranty of\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU\
Affero General Public License for more details.\
\
You should have received a copy of the GNU Affero General Public\
License along with this program.  If not, see http://www.gnu.org/licenses/ .")
ELSEIF(OEM_DISTRIBUTION STREQUAL "mensia")
	SET(OV_CONFIG_SUBDIR "mensia" CACHE STRING "Subdirectory under user directory when configuration and logs will be saved")
	SET(BRAND_NAME "NeuroRT")
	SET(DESIGNER_NAME "Studio")
	SET(COMPANY_NAME "Mensia Technologies SA")
	ADD_DEFINITIONS(-DMENSIA_DISTRIBUTION)
ELSE()
	MESSAGE(FATAL_ERROR "OEM_DISTRIBUTION unrecognized (${OEM_DISTRIBUTION}) ! CMake cannot deduce the brand and product names. ")
ENDIF()

IF(OV_DISPLAY_ERROR_LOCATION)
	ADD_DEFINITIONS(-DOV_DISPLAY_ERROR_LOCATION)
ENDIF()

# The user files will be stored in .../OV_CONFIG_SUBDIR folder
ADD_DEFINITIONS("-DOV_PROJECT_NAME=\"openvibe\"")
ADD_DEFINITIONS("-DOV_CONFIG_SUBDIR=\"${OV_CONFIG_SUBDIR}\"")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
IF(WIN32)
	ADD_DEFINITIONS("-DNOMINMAX -DBOOST_ALL_NO_LIB")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4355")
	SET(OV_WIN32_BOOST_VERSION "1_58")
ELSEIF(APPLE)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
ELSEIF(UNIX)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
	FIND_PATH(PKGCONFIG_PATH lib/pkgconfig/atk.pc PATHS ${LIST_DEPENDENCIES_PATH})
	SET(ENV{PKG_CONFIG_PATH} "${PKGCONFIG_PATH}/lib/pkgconfig")
ELSE(WIN32)
	MESSAGE(WARNING "Warning: unknown platform")
ENDIF(WIN32)

# Print the used compilation parameters (for transparency)
GET_DIRECTORY_PROPERTY(TMP_DEFINITIONS COMPILE_DEFINITIONS)
debug_message("Compilation flags used at source root: ")
debug_message("  COMPILE_DEFINITIONS = '${TMP_DEFINITIONS}'")
debug_message("  CMAKE_CXX_FLAGS = '${CMAKE_CXX_FLAGS}'")
debug_message("  CMAKE_CXX_FLAGS_RELEASE = '${CMAKE_CXX_FLAGS_RELEASE}'")
debug_message("  CMAKE_CXX_FLAGS_DEBUG = '${CMAKE_CXX_FLAGS_DEBUG}'")

# if no specific branch has been given for an openvibe component, default is to compile its trunk folder
## SET(OV_TRUNK "trunc")

# By setting SKIP[_FOLDER]* you can skip a subtree (example: SKIP_A_B_C skips folder a/b/c and all its subfolders if any)

# Custom cmakelist can be used to overwrite the default compilation & packaging parameters
IF(EXISTS "${CMAKE_SOURCE_DIR}/CustomCMakeLists.txt")
	debug_message("Found custom build settings")
	INCLUDE("${CMAKE_SOURCE_DIR}/CustomCMakeLists.txt")
ENDIF()


#################################################################
string(TOLOWER ${BRAND_NAME} LOWER_BRAND_NAME)
string(TOLOWER ${DESIGNER_NAME} LOWER_DESIGNER_NAME)
ADD_DEFINITIONS(-DBRAND_NAME="${BRAND_NAME}")
ADD_DEFINITIONS(-DDESIGNER_NAME="${DESIGNER_NAME}")
#################################################################

SET(OV_LAUNCHER_SOURCE_PATH "${CMAKE_SOURCE_DIR}/cmake-modules/launchers")

# a list of all project which will be filled by the directory traversal. This is needed to generate the documentation.
SET_PROPERTY(GLOBAL PROPERTY OV_PROP_CURRENT_PROJECTS "")
SET_PROPERTY(GLOBAL PROPERTY OV_PROP_CURRENT_PROJECTS_BUILD_DIR "")

# Used by the various Find* scripts to locate OpenViBE modules
SET(OV_BASE_DIR ${CMAKE_SOURCE_DIR})

# needed for making visual studio projects when this script is called without CMAKE_BUILD_TYPE
SET_PROPERTY(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS_DEBUG "TARGET_BUILDTYPE_Debug")
SET_PROPERTY(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS_RELEASE "TARGET_BUILDTYPE_Release")
SET_PROPERTY(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS_RELWITHDEBINFO "TARGET_BUILDTYPE_Release")

MESSAGE(STATUS "Building ${BRAND_NAME} ${DESIGNER_NAME} [${OEM_DISTRIBUTION}] Version : ${PROJECT_VERSION} ${OV_PROJECT_BRANCH}~${OV_PROJECT_COMMITHASH}")

# Add vendor specific installation instructions
# Traverse these directories and build their components

ADD_SUBDIRECTORY("documentation/")

IF(NOT(SKIP_PLUGINS))
	ADD_SUBDIRECTORY("plugins/")
ENDIF()

IF(NOT(SKIP_APPLICATIONS))
	ADD_SUBDIRECTORY("applications/")
ENDIF()

IF(NOT(SKIP_LIBRARIES))
	ADD_SUBDIRECTORY("libraries/")
ENDIF()

IF(NOT(SKIP_VISUALIZATION_TOOLKIT))
	ADD_SUBDIRECTORY("visualization-toolkit/")
ENDIF()

# add the scripts to the project so IDEs using the CMake file are aware of them
FILE(GLOB_RECURSE script_files scripts/*.cmd scripts/*.sh )
ADD_CUSTOM_TARGET(openvibe-scripts SOURCES ${script_files})

INSTALL(FILES "${CMAKE_BINARY_DIR}/.install" DESTINATION ${CMAKE_INSTALL_PREFIX})
