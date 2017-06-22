# ---------------------------------
# Finds third party boost
# Adds library to target
# Adds include path
# ---------------------------------


IF(UNIX)
	FIND_LIBRARY(LIB_Boost_System NAMES "boost_system-mt" PATHS ${LIST_DEPENDENCIES_PATH} PATH_SUFFIXES lib NO_DEFAULT_PATH)
	FIND_LIBRARY(LIB_Boost_System NAMES "boost_system-mt" PATHS ${LIST_DEPENDENCIES_PATH} PATH_SUFFIXES lib)
	FIND_LIBRARY(LIB_Boost_System NAMES "boost_system" PATHS ${LIST_DEPENDENCIES_PATH} PATH_SUFFIXES lib NO_DEFAULT_PATH)
	FIND_LIBRARY(LIB_Boost_System NAMES "boost_system" PATHS ${LIST_DEPENDENCIES_PATH} PATH_SUFFIXES lib)
	IF(LIB_Boost_System)
		debug_message( "    [  OK  ] lib ${LIB_Boost_System}")
		TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIB_Boost_System} )
	ELSE(LIB_Boost_System)
		MESSAGE(WARNING "    [FAILED] lib boost_system-mt")
	ENDIF(LIB_Boost_System)
ENDIF(UNIX)

IF(WIN32)
	OV_LINK_BOOST_LIB("system" ${OV_WIN32_BOOST_VERSION})
ENDIF(WIN32)
