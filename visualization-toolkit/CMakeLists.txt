project(openvibe-visualization-toolkit)
OV_ADD_THIS_TO_PROJECT_LIST()

set(PROJECT_VERSION_MAJOR ${OV_GLOBAL_VERSION_MAJOR})
set(PROJECT_VERSION_MINOR ${OV_GLOBAL_VERSION_MINOR})
set(PROJECT_VERSION_PATCH ${OV_GLOBAL_VERSION_PATCH})
set(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

file(GLOB_RECURSE source_files src/*.cpp src/*.h  src/*.hpp include/*.h)
include("FindSourceRCProperties")

include_directories(${openvibe-visualization-toolkit_SOURCE_DIR}/include/)
include_directories(${openvibe-visualization-toolkit_SOURCE_DIR}/include/visualization-toolkit)

add_library(${PROJECT_NAME} SHARED ${source_files})
set_target_properties(${PROJECT_NAME} PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION ${PROJECT_VERSION_MAJOR}
	COMPILE_FLAGS "-DOVVTK_Exports -DOVVTK_Shared")

include("FindCertivibe")
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
	RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})

install(DIRECTORY include/   DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR} FILES_MATCHING PATTERN "*.h")
