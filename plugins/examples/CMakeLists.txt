PROJECT(openvibe-plugins-designer-examples)

FILE(GLOB_RECURSE SRC_FILES src/*.cpp src/*.h src/*.hpp src/*.inl)
ADD_LIBRARY(${PROJECT_NAME} SHARED ${SRC_FILES})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION ${PROJECT_VERSION_MAJOR}
	FOLDER ${PLUGINS_FOLDER}
	COMPILE_FLAGS "-DOVP_Exports -DOVP_Shared")
target_link_libraries(${PROJECT_NAME} 
                      openvibe
					  openvibe-common
                      openvibe-toolkit
                      openvibe-module-ebml
                      openvibe-module-system
                      openvibe-module-xml
)

add_definitions(-DTARGET_HAS_ThirdPartyOpenViBEPluginsGlobalDefines)

INCLUDE("FindOpenViBEVisualizationToolkit")
INCLUDE("FindThirdPartyGTK")

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

INSTALL(DIRECTORY box-tutorials  DESTINATION ${DIST_DATADIR}/openvibe/scenarios/)
