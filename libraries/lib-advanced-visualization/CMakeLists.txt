project(mensia-advanced-visualization)
message(STATUS "Now building ${PROJECT_NAME} ${PROJECT_VERSION} (${OV_PROJECT_BRANCH}~${OV_PROJECT_COMMITHASH})" )

file(GLOB_RECURSE SRC_FILES src/*.cpp src/*.hpp include/*.hpp)
include("FindSourceRCProperties")
add_library(${PROJECT_NAME}-static STATIC ${SRC_FILES})

target_include_directories(${PROJECT_NAME}-static
                           PRIVATE include/mensia include/mensia/advanced-visualization
                           PUBLIC include
)
add_library(${PROJECT_NAME} SHARED EXCLUDE_FROM_ALL ${SRC_FILES})
target_include_directories(${PROJECT_NAME}-static
                           PRIVATE include/mensia include/mensia/advanced-visualization
                           PUBLIC include
)

if(UNIX)
	set_target_properties(${PROJECT_NAME}-static PROPERTIES COMPILE_FLAGS "-fPIC")
endif(UNIX)

set_target_properties(${PROJECT_NAME} PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_compile_options(${PROJECT_NAME}
                       PRIVATE -DLMAV_Exports
                       PUBLIC -DLMAV_Shared
)

add_definitions(-D_USE_MATH_DEFINES)	# Definition for constant math as M_PI

include("FindThirdPartyOpenGL")
include("FindThirdPartyEigen")

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
