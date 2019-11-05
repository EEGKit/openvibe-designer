PROJECT(openvibe-plugins-designer-examples)

FILE(GLOB_RECURSE SRC_FILES src/*.cpp src/*.h src/*.inl)
ADD_LIBRARY(${PROJECT_NAME} SHARED ${SRC_FILES})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION ${PROJECT_VERSION_MAJOR}
	COMPILE_FLAGS "-DOVP_Exports -DOVP_Shared")

SET(INCLUDED_OV_SDK_COMPONENTS MAIN COMMON TOOLKIT EBML SYSTEM XML)
INCLUDE("AddOpenViBESDKComponents")
INCLUDE("FindOpenViBEVisualizationToolkit")
INCLUDE("FindThirdPartyGTK")

# ---------------------------------
# Target macros
# Defines target operating system
# Defines target architecture
# Defines target compiler
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
