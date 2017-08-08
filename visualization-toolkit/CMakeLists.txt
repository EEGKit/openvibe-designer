project(openvibe-visualization-toolkit)
MESSAGE(STATUS "Now building ${PROJECT_NAME} ${PROJECT_VERSION} (${OV_PROJECT_BRANCH}~${OV_PROJECT_COMMITHASH})" )

OV_ADD_THIS_TO_PROJECT_LIST()

file(GLOB_RECURSE source_files src/*.cpp src/*.h  src/*.hpp include/*.h)
include("FindSourceRCProperties")

include_directories(${openvibe-visualization-toolkit_SOURCE_DIR}/include/)
include_directories(${openvibe-visualization-toolkit_SOURCE_DIR}/include/visualization-toolkit)

add_library(${PROJECT_NAME} SHARED ${source_files})
set_target_properties(${PROJECT_NAME} PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION ${PROJECT_VERSION_MAJOR}
	COMPILE_FLAGS "-DOVVIZ_Exports -DOVVIZ_Shared")

include("AddOpenViBESDKComponents")
include("FindThirdPartyGTK")

# ---------------------------------
# Target macros
# Defines target operating system
# Defines target architecture
# Defines target compiler
# ---------------------------------
set_build_platform()

# -----------------------------
# Install files
# -----------------------------
install(TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION ${DIST_BINDIR}
	LIBRARY DESTINATION ${DIST_LIBDIR}
	ARCHIVE DESTINATION ${DIST_LIBDIR})

install(DIRECTORY include/   DESTINATION ${DIST_INCLUDEDIR} FILES_MATCHING PATTERN "*.h")
