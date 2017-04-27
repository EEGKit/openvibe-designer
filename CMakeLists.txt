CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

IF (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 3.0)
cmake_policy(SET CMP0026 OLD)
cmake_policy(SET CMP0039 NEW)
cmake_policy(SET CMP0042 OLD)
cmake_policy(SET CMP0043 OLD)
cmake_policy(SET CMP0045 OLD)
cmake_policy(SET CMP0048 OLD)
cmake_policy(SET CMP0057 NEW)
ENDIF()

PROJECT(Designer)

# Certivibes modules list
SET(INCLUDED_CERTIVIBE_COMPONENTS BASE ALLPLUGINS EBML SYSTEM FS XML TOOLKIT)

# These versions are used by the subprojects by default.
# If you wish to maintain specific version numbers for a subproject, please do so in the projects CMakeLists.txt
SET(MENSIA_GLOBAL_VERSION_MAJOR 2)
SET(MENSIA_GLOBAL_VERSION_MINOR 6)
SET(MENSIA_GLOBAL_VERSION_TWEAK 0)
SET(MENSIA_GLOBAL_VERSION_PATCH 0)
SET(MENSIA_GLOBAL_VERSION_STRING "${MENSIA_GLOBAL_VERSION_MAJOR}.${MENSIA_GLOBAL_VERSION_MINOR}.${MENSIA_GLOBAL_VERSION_TWEAK}.${MENSIA_GLOBAL_VERSION_PATCH}")

SET(OV_GLOBAL_VERSION_MAJOR 0)
SET(OV_GLOBAL_VERSION_MINOR 17)
SET(OV_GLOBAL_VERSION_PATCH 0)
SET(OV_GLOBAL_VERSION_STRING "${OV_GLOBAL_VERSION_MAJOR}.${OV_GLOBAL_VERSION_MINOR}.${OV_GLOBAL_VERSION_PATCH}")

OPTION(Flag_VerboseOutput "Verbose CMake output" OFF)
OPTION(Flag_SilentConsole "Disable logs to console in Archway" ON)
SET(OEM_DISTRIBUTION "openvibe" CACHE STRING "The OEM distribution for this build, can be openvibe or mensia")

SET(OV_CUSTOM_DEPENDENCIES_PATH "${CMAKE_SOURCE_DIR}/dependencies")
SET(OV_SOURCE_DEPENDENCIES_PATH "${CMAKE_SOURCE_DIR}/dependencies-source")
SET(OV_ICON_PATH "${CMAKE_SOURCE_DIR}/scripts/icons")

IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
  SET(OPENVIBE_SDK_PATH "${OV_CUSTOM_DEPENDENCIES_PATH}/openvibe-sdk-debug" CACHE STRING "")
ELSE()
  SET(OPENVIBE_SDK_PATH "${OV_CUSTOM_DEPENDENCIES_PATH}/openvibe-sdk-release" CACHE STRING "")
ENDIF()

SET(CV_DEPENDENCIES_PATH "${CMAKE_SOURCE_DIR}/dependencies" CACHE STRING "")

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
	MESSAGE(FATAL_ERROR "OEM_DISTRIBUTION unrocognized (${OEM_DISTRIBUTION}) ! CMake cannot deduce the brand and product names. ")
ENDIF()


INCLUDE("OvMessages")

# The user files will be stored in .../OV_CONFIG_SUBDIR folder
ADD_DEFINITIONS("-DOV_PROJECT_NAME=\"openvibe\"")
ADD_DEFINITIONS("-DOV_CONFIG_SUBDIR=\"${OV_CONFIG_SUBDIR}\"")

# Default is to build to dist/. If you wish a custom install, set your own MAKE_INSTALL_PREFIX when you call CMake. Safest to do under a fakeroot.
IF(NOT CMAKE_INSTALL_PREFIX)
	SET(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/dist")
ENDIF(NOT CMAKE_INSTALL_PREFIX)

SET(DOCUMENTATION_TEMP_DIRECTORY "${CMAKE_INSTALL_PREFIX}/doc-tmp")

IF(WIN32)
	ADD_DEFINITIONS("-DNOMINMAX -DBOOST_ALL_NO_LIB")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4355")
	SET(OV_WIN32_BOOST_VERSION "1_55")
ELSEIF(APPLE)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
ELSEIF(UNIX)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall")
	# This ENV var is needed to locate our custom IT++ on Linux
	SET(ENV{PKG_CONFIG_PATH} "${OV_CUSTOM_DEPENDENCIES_PATH}/lib/pkgconfig")
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

# SET(OV_COMPILE_TESTS "true")
FUNCTION(SET_BUILD_PLATFORM)
	IF("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
		ADD_DEFINITIONS(-DTARGET_ARCHITECTURE_x64)
	ELSEIF("${CMAKE_SIZEOF_VOID_P}" EQUAL "4")
		ADD_DEFINITIONS(-DTARGET_ARCHITECTURE_i386)
	ELSE()
		ADD_DEFINITIONS(-DTARGET_ARCHITECTURE_Unknown)
	ENDIF()

	IF(WIN32)
		ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
		ADD_DEFINITIONS(-DTARGET_OS_Windows)
		ADD_DEFINITIONS(-DTARGET_COMPILER_VisualStudio)
	ELSEIF(APPLE)
		ADD_DEFINITIONS(-fnon-call-exceptions)
		ADD_DEFINITIONS(-DTARGET_OS_MacOS)
		# ADD_DEFINITIONS(-DTARGET_ARCHITECTURE_x64)
		ADD_DEFINITIONS(-DTARGET_COMPILER_LLVM)
	ELSEIF(UNIX)
		# ADD_DEFINITIONS(-fvisibility=hidden) # This flag should be present... man gcc
		ADD_DEFINITIONS(-fnon-call-exceptions)
		ADD_DEFINITIONS(-DTARGET_OS_Linux)
		ADD_DEFINITIONS(-DTARGET_COMPILER_GCC)
	ENDIF()

ENDFUNCTION()

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

# OpenViBE-specific helper functions that are used by the CMakeLists of the subprojects.
#INCLUDE("OvAddSingleProject")
INCLUDE("OvAddProjects")
INCLUDE("OvInstallLaunchScript")
INCLUDE("OvLinkBoostLib")
INCLUDE("OvDelayLoad")
INCLUDE("SetProjectVersion")
INCLUDE("OvMessages")

# Setup knowledge of GNU-style install path variables
INCLUDE("GNUInstallDirs")
SET(CMAKE_INSTALL_FULL_DOCDIR "${CMAKE_INSTALL_FULL_BINDIR}/../doc/html")

# Add directories that OpenViBE will use to look for its components runtime, unless overridden by environment variables in the launch scripts.
# These defines should only be used in "openvibe/ov_directories.h".
ADD_DEFINITIONS(-DOV_CMAKE_PATH_ROOT="${CMAKE_INSTALL_PREFIX}")
ADD_DEFINITIONS(-DOV_CMAKE_PATH_BIN="${CMAKE_INSTALL_FULL_BINDIR}")
ADD_DEFINITIONS(-DOV_CMAKE_PATH_LIB="${CMAKE_INSTALL_FULL_LIBDIR}")
ADD_DEFINITIONS(-DOV_CMAKE_PATH_DATA="${CMAKE_INSTALL_FULL_DATADIR}/openvibe")

# Add Mensia definitions
SET(MENSIA_BASE_DIR "${CMAKE_SOURCE_DIR}/externals/mensia")
ADD_DEFINITIONS(-DMENSIA_CMAKE_PATH_ROOT="${CMAKE_INSTALL_PREFIX}")
ADD_DEFINITIONS(-DMENSIA_CMAKE_PATH_BIN="${CMAKE_INSTALL_FULL_BINDIR}")
ADD_DEFINITIONS(-DMENSIA_CMAKE_PATH_DATA="${CMAKE_INSTALL_FULL_DATADIR}/mensia")
ADD_DEFINITIONS(-DMENSIA_CMAKE_PATH_LIB="${CMAKE_INSTALL_FULL_LIBDIR}")

# Sets the PROJECT_VERSION variable to something, depending on overriding OvSetProjectVersion.cmake files
OV_SET_PROJECT_VERSION()

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
