# ---------------------------------
# Finds libmensia-base
# Adds library to target
# Adds include path
# ---------------------------------
set(MENSIA_SRC_DIR "${CMAKE_SOURCE_DIR}/libraries/lib-base/include")

UNSET(PATH_LIBMENSIABASE)
FIND_PATH(PATH_LIBMENSIABASE mensia/base.h PATHS ${MENSIA_SRC_DIR} NO_DEFAULT_PATH)
IF(PATH_LIBMENSIABASE)
	debug_message( "  Found libmensia-base... ${PATH_LIBMENSIABASE}")
	INCLUDE_DIRECTORIES(${PATH_LIBMENSIABASE}/)

	ADD_DEFINITIONS(-DTARGET_HAS_LibMensiabase)

	GET_TARGET_PROPERTY(TARGET_TYPE ${PROJECT_NAME} "TYPE")
	IF (NOT TARGET_TYPE STREQUAL "UTILITY")
		IF(WIN32)
			TARGET_LINK_LIBRARIES(${PROJECT_NAME} DbgHelp.lib)
		ELSEIF(APPLE)
		ELSEIF(UNIX)
			TARGET_LINK_LIBRARIES(${PROJECT_NAME} dl)
		ENDIF()
	ENDIF()

ELSE()
	MESSAGE(WARNING "  FAILED to find libmensia-base...")
ENDIF()
