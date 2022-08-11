PROJECT(openvibe-plugins-designer-advanced-vis)
SET(PROJECT_PRODUCT_NAME "Mensia Advanced Visualization Toolset")
MESSAGE(STATUS "Now building ${PROJECT_NAME} ${PROJECT_VERSION} (${OV_PROJECT_BRANCH}~${OV_PROJECT_COMMITHASH})" )

FILE(GLOB_RECURSE SRC_FILES src/*.cpp src/*.h src/*.hpp src/*.inl include/*.h include/*.hpp)

INCLUDE("FindSourceRCProperties")

ADD_LIBRARY(${PROJECT_NAME} SHARED ${SRC_FILES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} 
					  ${RLM_LIB_FILES}
					  openvibe
					  openvibe-common
					  openvibe-toolkit
					  openvibe-module-system
)
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION ${PROJECT_VERSION_MAJOR}
	FOLDER ${PLUGINS_FOLDER}
	COMPILE_FLAGS "-DOVP_Exports -DOVP_Shared")

add_definitions(-DTARGET_HAS_ThirdPartyOpenViBEPluginsGlobalDefines)

IF(MENSIA_ADVANCED_VISUALIZATION_EXPERIMENTAL)
	MESSAGE(STATUS "  MENSIA_ADVANCED_VISUALIZATION_EXPERIMENTAL is set, experimental visualization will be built.")
	ADD_DEFINITIONS(-DTARGET_Has_Experimental)
ENDIF()

INCLUDE("FindOpenViBEVisualizationToolkit")
INCLUDE("FindThirdPartyGTK")

INCLUDE("FindLibMensiaAdvancedVisualisation")
INCLUDE("FindThirdPartyOpenGL")

# ---------------------------------
# Target macros
# Defines target operating system, architecture and compiler
# ---------------------------------
SET_BUILD_PLATFORM()

# -----------------------------
# Install files
# -----------------------------
INSTALL(TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION ${DIST_BINDIR}
	LIBRARY DESTINATION ${DIST_LIBDIR}
	ARCHIVE DESTINATION ${DIST_LIBDIR})

INSTALL(DIRECTORY share/ DESTINATION ${DIST_DATADIR}/openvibe/plugins)
INSTALL(DIRECTORY box-tutorials/  DESTINATION ${DIST_DATADIR}/openvibe/scenarios/box-tutorials/advanced-visualization)
