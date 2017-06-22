# ---------------------------------
# Finds third party boost
# Adds library to target
# Adds include path
# ---------------------------------


IF(UNIX)
	FIND_LIBRARY(LIB_Boost_Regex NAMES "boost_regex-mt" PATHS ${LIST_DEPENDENCIES_PATH} PATH_SUFFIXES lib NO_DEFAULT_PATH)
	FIND_LIBRARY(LIB_Boost_Regex NAMES "boost_regex-mt" PATHS ${LIST_DEPENDENCIES_PATH} PATH_SUFFIXES lib)
	FIND_LIBRARY(LIB_Boost_Regex NAMES "boost_regex" PATHS ${LIST_DEPENDENCIES_PATH} PATH_SUFFIXES lib NO_DEFAULT_PATH)
	FIND_LIBRARY(LIB_Boost_Regex NAMES "boost_regex" PATHS ${LIST_DEPENDENCIES_PATH} PATH_SUFFIXES lib)
	IF(LIB_Boost_Regex)
		debug_message( "    [  OK  ] lib ${LIB_Boost_Regex}")
		TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIB_Boost_Regex} )
	ELSE(LIB_Boost_Regex)
		MESSAGE(WARNING "    [FAILED] lib boost_regex-mt")
	ENDIF(LIB_Boost_Regex)
ENDIF(UNIX)

IF(WIN32)
	OV_LINK_BOOST_LIB("regex" ${OV_WIN32_BOOST_VERSION})
ENDIF(WIN32)
