PROJECT(Designer VERSION ${OPENVIBE_MAJOR_VERSION}.${OPENVIBE_MINOR_VERSION}.${OPENVIBE_PATCH_VERSION})

SET(CMAKE_MODULE_PATH
	${CMAKE_MODULE_PATH_BASE}
	${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules
	# ${CMAKE_MODULE_PATH}
	)

# OpenViBE-specific helper functions that are used by the CMakeLists of the subprojects.
INCLUDE("OvAddProjects")
INCLUDE("OvLinkBoostLib")
INCLUDE("OvDelayLoad")
INCLUDE("SetProjectVersion")

# ----------------------
# Build CMake flags
# ----------------------
SET(OEM_DISTRIBUTION "openvibe" CACHE STRING "The OEM distribution for this build, can be openvibe")
SET(OV_SOURCE_DEPENDENCIES_PATH "${CMAKE_CURRENT_SOURCE_DIR}/dependencies-source")
SET(OV_ICON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/scripts/icons")

SET(CMAKE_INSTALL_LIBDIR lib) # Fix for non debian-based distrib, as it seeems to be a common occurence to name this folder lib32/lib64

if(NOT(${Flag_VerboseOutput}))
	set(CMAKE_INSTALL_MESSAGE LAZY)
endif()

# ----------------------
# Set build version
# ----------------------
# These versions are used by the subprojects by default.
set_version()
# Sets the PROJECT_VERSION variable to something, depending on overriding OvSetProjectVersion.cmake files
SET_PROJECT_VERSION()
MESSAGE(STATUS "BUILDING VERSION Designer: ${OV_GLOBAL_VERSION_STRING} Branch : ${OV_PROJECT_BRANCH}~${OV_PROJECT_COMMITHASH}. Based on OpenViBE SDK: ${OV_SDK_VERSION_STRING}")

# Sort target into directories for better visualization in IDE
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)
SET(APP_FOLDER Applications)
SET(MODULES_FOLDER Modules)
SET(PLUGINS_FOLDER Plugins)
SET(TESTS_FOLDER Unit-Tests)
SET(VALIDATION_FOLDER Validation-Tests)


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
IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/CustomCMakeLists.txt")
	debug_message("Found custom build settings")
	INCLUDE("${CMAKE_CURRENT_SOURCE_DIR}/CustomCMakeLists.txt")
ENDIF()


#################################################################
string(TOLOWER ${BRAND_NAME} LOWER_BRAND_NAME)
string(TOLOWER ${DESIGNER_NAME} LOWER_DESIGNER_NAME)
ADD_DEFINITIONS(-DBRAND_NAME="${BRAND_NAME}")
ADD_DEFINITIONS(-DDESIGNER_NAME="${DESIGNER_NAME}")
#################################################################

SET(OV_LAUNCHER_SOURCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules/launchers")

# a list of all project which will be filled by the directory traversal. This is needed to generate the documentation.
SET_PROPERTY(GLOBAL PROPERTY OV_PROP_CURRENT_PROJECTS "")
SET_PROPERTY(GLOBAL PROPERTY OV_PROP_CURRENT_PROJECTS_BUILD_DIR "")

# Used by the various Find* scripts to locate OpenViBE modules
SET(OV_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

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
ADD_CUSTOM_TARGET(openvibe-designer-scripts SOURCES ${script_files})

INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/.install-designer" DESTINATION ${CMAKE_INSTALL_PREFIX})
