project(mensia-advanced-visualization VERSION ${OPENVIBE_MAJOR_VERSION}.${OPENVIBE_MINOR_VERSION}.${OPENVIBE_PATCH_VERSION})
message(STATUS "Now building ${PROJECT_NAME} ${PROJECT_VERSION} (${OV_PROJECT_BRANCH}~${OV_PROJECT_COMMITHASH})" )

file(GLOB_RECURSE SRC_FILES src/*.cpp src/*.hpp include/*.hpp)
include("FindSourceRCProperties")

add_library(${PROJECT_NAME}-static STATIC ${SRC_FILES})

target_include_directories(${PROJECT_NAME}-static
						   PRIVATE include/mensia include/mensia/advanced-visualization
						   PUBLIC include)

target_link_libraries(${PROJECT_NAME}-static
					  Eigen3::Eigen)

set_target_properties(${PROJECT_NAME}-static PROPERTIES
	VERSION ${PROJECT_VERSION}
	FOLDER ${MODULES_FOLDER})

add_library(${PROJECT_NAME} SHARED EXCLUDE_FROM_ALL ${SRC_FILES})
target_include_directories(${PROJECT_NAME}
						   PRIVATE include/mensia include/mensia/advanced-visualization
						   PUBLIC include)

target_link_libraries(${PROJECT_NAME}
					  Eigen3::Eigen)

if(UNIX)
	set_target_properties(${PROJECT_NAME}-static PROPERTIES COMPILE_FLAGS "-fPIC")
endif(UNIX)

set_target_properties(${PROJECT_NAME} PROPERTIES
					  VERSION ${PROJECT_VERSION}
					  SOVERSION ${PROJECT_VERSION_MAJOR}
					  FOLDER ${MODULES_FOLDER})

target_compile_options(${PROJECT_NAME}
					   PRIVATE -DLMAV_Exports
					   PUBLIC -DLMAV_Shared)

add_definitions(-D_USE_MATH_DEFINES)	# Definition for constant math as M_PI

include("FindThirdPartyOpenGL")

# ---------------------------------
# Target macros
# Defines target operating system, architecture and compiler
# ---------------------------------
SET_BUILD_PLATFORM()

# -----------------------------
# Install files
# -----------------------------
#INSTALL(TARGETS ${PROJECT_NAME}
#	RUNTIME DESTINATION ${DIST_BINDIR}
#	LIBRARY DESTINATION ${DIST_LIBDIR}
#	ARCHIVE DESTINATION ${DIST_LIBDIR})
install(TARGETS ${PROJECT_NAME}-static
		RUNTIME DESTINATION ${DIST_BINDIR}
		LIBRARY DESTINATION ${DIST_LIBDIR}
		ARCHIVE DESTINATION ${DIST_LIBDIR})

install(DIRECTORY include/ DESTINATION ${DIST_INCLUDEDIR} FILES_MATCHING PATTERN "*.hpp")

add_subdirectory(test)
