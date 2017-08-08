PROJECT(mensia-advanced-visualization)
MESSAGE(STATUS "Now building ${PROJECT_NAME} ${PROJECT_VERSION} (${OV_PROJECT_BRANCH}~${OV_PROJECT_COMMITHASH})" )

INCLUDE_DIRECTORIES("include/mensia/")
INCLUDE_DIRECTORIES("include/mensia/advanced-visualization")

FILE(GLOB_RECURSE source_files src/*.cpp src/*.h src/*.hpp include/*.h)
INCLUDE("FindSourceRCProperties")
ADD_LIBRARY(${PROJECT_NAME}-static STATIC ${source_files})
ADD_LIBRARY(${PROJECT_NAME} SHARED EXCLUDE_FROM_ALL ${source_files})

IF(UNIX)
	SET_TARGET_PROPERTIES(${PROJECT_NAME}-static PROPERTIES COMPILE_FLAGS "-fPIC")
ENDIF(UNIX)

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION ${PROJECT_VERSION_MAJOR}
	COMPILE_FLAGS "-DLMAV_Exports -DLMAV_Shared")

INCLUDE("FindThirdPartyOpenGL")
INCLUDE("FindThirdPartyEigen")

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
#INSTALL(TARGETS ${PROJECT_NAME}
#	RUNTIME DESTINATION ${DIST_BINDIR}
#	LIBRARY DESTINATION ${DIST_LIBDIR}
#	ARCHIVE DESTINATION ${DIST_LIBDIR})
INSTALL(TARGETS ${PROJECT_NAME}-static
	RUNTIME DESTINATION ${DIST_BINDIR}
	LIBRARY DESTINATION ${DIST_LIBDIR}
	ARCHIVE DESTINATION ${DIST_LIBDIR})


INSTALL(DIRECTORY include/ DESTINATION ${DIST_INCLUDEDIR} FILES_MATCHING PATTERN "*.h")
