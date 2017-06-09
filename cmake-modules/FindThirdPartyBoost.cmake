# ---------------------------------
# Finds third party boost
# Adds library to target
# Adds include path
# ---------------------------------

SET(PATH_BOOST "PATH_BOOST-NOTFOUND")
FIND_PATH(PATH_BOOST "include/boost/config/auto_link.hpp" PATHS ${LIST_DEPENDENCIES_PATH} PATH_SUFFIXES boost NO_DEFAULT_PATH)

IF(PATH_BOOST)
	debug_message( "  Found boost includes...")
	INCLUDE_DIRECTORIES(${PATH_BOOST}/include)

	ADD_DEFINITIONS(-DTARGET_HAS_Boost)
ELSE(PATH_BOOST)
	MESSAGE(WARNING "  FAILED to find boost includes...")
ENDIF(PATH_BOOST)
