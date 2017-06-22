CMAKE_MINIMUM_REQUIRED(VERSION 3.2)

cmake_policy(SET CMP0010 OLD)
cmake_policy(SET CMP0026 OLD)
cmake_policy(SET CMP0039 NEW)
cmake_policy(SET CMP0042 OLD)
cmake_policy(SET CMP0043 OLD)
cmake_policy(SET CMP0045 OLD) 
cmake_policy(SET CMP0048 OLD)
cmake_policy(SET CMP0057 NEW)

PROJECT(Designer)

# Certivibes modules list
SET(INCLUDED_CERTIVIBE_COMPONENTS BASE ALLPLUGINS EBML SYSTEM FS XML TOOLKIT)

# These versions are used by the subprojects by default.
# If you wish to maintain specific version numbers for a subproject, please do so in the projects CMakeLists.txt
SET(OV_GLOBAL_VERSION_MAJOR 0)
SET(OV_GLOBAL_VERSION_MINOR 1)
SET(OV_GLOBAL_VERSION_PATCH 99)
SET(OV_GLOBAL_VERSION_STRING "${OV_GLOBAL_VERSION_MAJOR}.${OV_GLOBAL_VERSION_MINOR}.${OV_GLOBAL_VERSION_PATCH}")
SET(OV_SDK_VERSION_STRING "0.1.0")

# ----------------------
# Build CMake flags
# ----------------------
OPTION(Flag_VerboseOutput "Verbose CMake output" OFF)
SET(OEM_DISTRIBUTION "openvibe" CACHE STRING "The OEM distribution for this build, can be openvibe or mensia")
OPTION(OV_DISPLAY_ERROR_LOCATION ON)

SET(OV_SOURCE_DEPENDENCIES_PATH "${CMAKE_SOURCE_DIR}/dependencies-source")
SET(OV_ICON_PATH "${CMAKE_SOURCE_DIR}/scripts/icons")

LIST(APPEND LIST_DEPENDENCIES_PATH "${CMAKE_SOURCE_DIR}/dependencies")

IF(CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT(OPENVIBE_SDK_PATH))
	FIND_PATH(OPENVIBE_SDK_PATH bin PATHS ${LIST_DEPENDENCIES_PATH} PATH_SUFFIXES openvibe-sdk-debug NO_DEFAULT_PATH)
ELSEIF(NOT(OPENVIBE_SDK_PATH))
	FIND_PATH(OPENVIBE_SDK_PATH bin PATHS ${LIST_DEPENDENCIES_PATH} PATH_SUFFIXES openvibe-sdk-release NO_DEFAULT_PATH)
ENDIF()

IF(OV_PACKAGE)
	SET(CMAKE_INSTALL_PREFIX .)
ELSEIF(NOT CMAKE_INSTALL_PREFIX)
	# Default is to build to dist/. If you wish a custom install, set your own MAKE_INSTALL_PREFIX when you call CMake. Safest to do under a fakeroot.
	SET(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/dist")
ENDIF()

LIST(APPEND CMAKE_MODULE_PATH "${OPENVIBE_SDK_PATH}/share")
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake-modules")

# We deduce the branding from the OEM_DISTRIBUTION
IF(OEM_DISTRIBUTION STREQUAL "openvibe")
	SET(OV_CONFIG_SUBDIR "OpenVIBE")
	SET(BRAND_NAME "OpenViBE")
	SET(STUDIO_NAME "Designer")
ELSEIF(OEM_DISTRIBUTION STREQUAL "mensia")
	SET(OV_CONFIG_SUBDIR "mensia")
	SET(BRAND_NAME "NeuroRT")
	SET(STUDIO_NAME "Studio")
ELSE()
	MESSAGE(FATAL_ERROR "OEM_DISTRIBUTION unrecognized (${OEM_DISTRIBUTION}) ! CMake cannot deduce the brand and product names. ")
ENDIF()

IF(OV_DISPLAY_ERROR_LOCATION)
	ADD_DEFINITIONS(-DOV_DISPLAY_ERROR_LOCATION)
ENDIF()

IF(NOT DEFINED OV_PACKAGE)
	SET(OV_PACKAGE FALSE)
ENDIF(NOT DEFINED OV_PACKAGE)

# OpenViBE-specific helper functions that are used by the CMakeLists of the subprojects.
INCLUDE("OvAddProjects")
INCLUDE("OvInstallLaunchScript")
INCLUDE("OvLinkBoostLib")
INCLUDE("OvDelayLoad")
INCLUDE("SetProjectVersion")
INCLUDE("Utilities") # Include functions debug_message, set_build_platform

# The user files will be stored in .../OV_CONFIG_SUBDIR folder
ADD_DEFINITIONS("-DOV_PROJECT_NAME=\"openvibe\"")
ADD_DEFINITIONS("-DOV_CONFIG_SUBDIR=\"${OV_CONFIG_SUBDIR}\"")

IF(NOT DEFINED OV_PACKAGE)
	SET(OV_PACKAGE FALSE)
ENDIF(NOT DEFINED OV_PACKAGE)

SET(DOCUMENTATION_TEMP_DIRECTORY "${CMAKE_INSTALL_PREFIX}/doc-tmp")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
IF(WIN32)
	ADD_DEFINITIONS("-DNOMINMAX -DBOOST_ALL_NO_LIB")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4355")
	SET(OV_WIN32_BOOST_VERSION "1_55")
ELSEIF(APPLE)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
ELSEIF(UNIX)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
	FIND_PATH(PKGCONFIG_PATH lib/pkgconfig/atk.pc PATHS ${LIST_DEPENDENCIES_PATH})
	SET(ENV{PKG_CONFIG_PATH} "${PKGCONFIG_PATH}/lib/pkgconfig")
ELSE(WIN32)
	MESSAGE(WARNING "Warning: unknown platform")
ENDIF(WIN32)

IF(NOT CMAKE_BUILD_TYPE)
	debug_message("Setting default build type to Release")
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF(NOT CMAKE_BUILD_TYPE)

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
string(TOLOWER ${STUDIO_NAME} LOWER_STUDIO_NAME)
ADD_DEFINITIONS(-DBRAND_NAME="${BRAND_NAME}")
ADD_DEFINITIONS(-DSTUDIO_NAME="${STUDIO_NAME}")
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

# Setup knowledge of GNU-style install path variables
INCLUDE("GNUInstallDirs")
SET(CMAKE_INSTALL_FULL_DOCDIR "${CMAKE_INSTALL_FULL_BINDIR}/../doc/html")

# Add directories that OpenViBE will use to look for its components runtime, unless overridden by environment variables in the launch scripts.
# These defines should only be used in "openvibe/ov_directories.h".
ADD_DEFINITIONS(-DOV_CMAKE_PATH_ROOT="${CMAKE_INSTALL_PREFIX}")
ADD_DEFINITIONS(-DOV_CMAKE_PATH_BIN="${CMAKE_INSTALL_FULL_BINDIR}")
ADD_DEFINITIONS(-DOV_CMAKE_PATH_LIB="${CMAKE_INSTALL_FULL_LIBDIR}")
ADD_DEFINITIONS(-DOV_CMAKE_PATH_DATA="${CMAKE_INSTALL_FULL_DATADIR}/openvibe")

# Sets the PROJECT_VERSION variable to something, depending on overriding OvSetProjectVersion.cmake files
OV_SET_PROJECT_VERSION()

SET(DIST_ROOT ${CMAKE_INSTALL_PREFIX})
IF(OV_PACKAGE)
	SET(DIST_BINDIR ${CMAKE_INSTALL_BINDIR})
	SET(DIST_LIBDIR ${CMAKE_INSTALL_LIBDIR})
	SET(DIST_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR})
	SET(DIST_DATADIR ${CMAKE_INSTALL_DATADIR})
	SET(DIST_DOCDIR ${CMAKE_INSTALL_BINDIR}/../doc)
	SET(CPACK_GENERATOR "ZIP")
	SET(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
	SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OpenViBE Designer")
	SET(CPACK_PACKAGE_VENDOR "Inria & Mensia Technologies")
	SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
	SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING.md")
	SET(CPACK_PACKAGE_VERSION_MAJOR ${OV_GLOBAL_VERSION_MAJOR})
	SET(CPACK_PACKAGE_VERSION_MINOR ${OV_GLOBAL_VERSION_MINOR})
	SET(CPACK_PACKAGE_VERSION_PATCH ${OV_GLOBAL_VERSION_PATCH})
	if(UNIX)
		execute_process(COMMAND bash -c "echo -n $(lsb_release -i -s)$(lsb_release -r -s)"
			OUTPUT_VARIABLE CPACK_SYSTEM_NAME)
	endif()
	SET(CPACK_PACKAGE_NAME "OpenViBE${CMAKE_PROJECT_NAME}-${CMAKE_BUILD_TYPE}")
	INCLUDE(CPack)
ELSE()
	SET(DIST_BINDIR ${CMAKE_INSTALL_FULL_BINDIR})
	SET(DIST_LIBDIR ${CMAKE_INSTALL_FULL_LIBDIR})
	SET(DIST_INCLUDEDIR ${CMAKE_INSTALL_FULL_INCLUDEDIR})
	SET(DIST_DATADIR ${CMAKE_INSTALL_FULL_DATADIR})
	SET(DIST_DOCDIR ${CMAKE_INSTALL_FULL_BINDIR}/../doc)
ENDIF()

MESSAGE(STATUS "Building ${BRAND_NAME} ${STUDIO_NAME} [${OEM_DISTRIBUTION}] Version : ${PROJECT_VERSION} ${OV_PROJECT_BRANCH}~${OV_PROJECT_COMMITHASH}")

# Add vendor specific installation instructions
# Traverse these directories and build their components

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
FILE(GLOB_RECURSE script_files scripts/*.cmd scripts/*.sh scripts/*.nsi)
ADD_CUSTOM_TARGET(openvibe-scripts SOURCES ${script_files})
