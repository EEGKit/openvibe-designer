PROJECT(openvibe-plugins-mensia-advanced-visualization)
MESSAGE(STATUS "Now building ${PROJECT_NAME} ${PROJECT_VERSION} (${PROJECT_BRANCH}/${PROJECT_COMMITHASH})" )
SET(PROJECT_PRODUCT_NAME "Mensia Advanced Visualization Toolset")

FILE(GLOB_RECURSE source_files src/*.cpp src/*.h src/*.hpp src/*.inl include/*.h)

INCLUDE("FindSourceCommonBase")
INCLUDE("FindSourceDependencyRepriseLicenseManager")
INCLUDE("FindSourceRCProperties")

ADD_LIBRARY(${PROJECT_NAME} SHARED ${source_files})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${RLM_LIB_FILES})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION ${PROJECT_VERSION_MAJOR}
	COMPILE_FLAGS "-DOVP_Exports -DOVP_Shared")

IF(MENSIA_ADVANCED_VISUALIZATION_EXPERIMENTAL)
	MESSAGE(STATUS "  MENSIA_ADVANCED_VISUALIZATION_EXPERIMENTAL is set, experimental visualization will be built.")
	ADD_DEFINITIONS(-DTARGET_Has_Experimental)
ENDIF()

ADD_DEFINITIONS(-D__MensiaLog_LogFilename__=\"mensia-plugins-advanced-visualization.log\")

INCLUDE("FindOpenViBE")
INCLUDE("FindOpenViBECommon")
INCLUDE("FindOpenViBEToolkit")
INCLUDE("FindOpenViBEModuleEBML")
INCLUDE("FindOpenViBEModuleSocket")
INCLUDE("FindOpenViBEModuleSystem")
INCLUDE("FindOpenViBEPluginsGlobalDefines")
INCLUDE("FindThirdPartyGTK")

INCLUDE("FindLibMensiaBase")
INCLUDE("FindLibMensiaAdvancedVisualisation")
INCLUDE("FindThirdPartyOpenGL")

INCLUDE("GenerateDocumentation")

# ---------------------------------
# Target macros
# Defines target operating system
# Defines target architecture
# Defines target compiler
# ---------------------------------
SET_BUILD_PLATFORM()


# ---------------------------------
# License information
# ---------------------------------
# add_definitions(-DOVP_MENSIA_LICENSE_INITIAL_TIMESTAMP=1369137600) # 21-05-2013 / 12:00
# add_definitions(-DOVP_MENSIA_LICENSE_INITIAL_TIMESTAMP=1069137600) # very old, for testing
# add_definitions(-DOVP_MENSIA_LICENSE_DURATION=31536000) # 1 year
# add_definitions(-DOVP_MENSIA_LICENSE_CLIENT_ID="\\"Mensia/developer\\"")

# -----------------------------
# Install files
# -----------------------------
INSTALL(TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})

INSTALL(DIRECTORY share/ DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/openvibe-plugins   PATTERN ".svn" EXCLUDE)

#INSTALL(DIRECTORY src/   DESTINATION include FILES_MATCHING PATTERN "*.h" PATTERN ".svn" EXCLUDE PATTERN "doc" EXCLUDE)
