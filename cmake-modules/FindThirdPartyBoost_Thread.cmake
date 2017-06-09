# ---------------------------------
# Finds third party boost
# Adds library to target
# Adds include path
# ---------------------------------

IF(UNIX)
	FIND_LIBRARY(LIB_Boost_Thread NAMES "boost_thread-mt" PATHS ${LIST_DEPENDENCIES_PATH} PATH_SUFFIXES lib NO_DEFAULT_PATH)
	FIND_LIBRARY(LIB_Boost_Thread NAMES "boost_thread-mt" PATHS ${LIST_DEPENDENCIES_PATH} PATH_SUFFIXES lib)
	FIND_LIBRARY(LIB_Boost_Thread NAMES "boost_thread" PATHS ${LIST_DEPENDENCIES_PATH} PATH_SUFFIXES lib NO_DEFAULT_PATH)
	FIND_LIBRARY(LIB_Boost_Thread NAMES "boost_thread" PATHS ${LIST_DEPENDENCIES_PATH} PATH_SUFFIXES lib)
	IF(LIB_Boost_Thread)
		debug_message( "    [  OK  ] lib ${LIB_Boost_Thread}")
		TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIB_Boost_Thread} )
	ELSE(LIB_Boost_Thread)
		MESSAGE(WARNING "    [FAILED] lib boost_thread-mt")
	ENDIF(LIB_Boost_Thread)

	# For Fedora
	FIND_LIBRARY(LIB_STANDARD_MODULE_PTHREAD pthread)
	IF(LIB_STANDARD_MODULE_PTHREAD)
		debug_message( "  Found pthread...")
		TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIB_STANDARD_MODULE_PTHREAD})
	ELSE(LIB_STANDARD_MODULE_PTHREAD)
		MESSAGE(WARNING "  FAILED to find pthread...")
	ENDIF(LIB_STANDARD_MODULE_PTHREAD)

	# For Ubuntu 13.04 (interprocess/ipc/message_queue.hpp in ovasCPluginExternalStimulations.cpp caused dep)
	IF(NOT APPLE)
		FIND_LIBRARY(LIB_STANDARD_MODULE_RT rt)
		IF(LIB_STANDARD_MODULE_RT)
			debug_message( "  Found rt...")
			TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIB_STANDARD_MODULE_RT})
		ELSE(LIB_STANDARD_MODULE_RT)
			MESSAGE(WARNING "  FAILED to find rt...")
		ENDIF(LIB_STANDARD_MODULE_RT)
	ENDIF()
	
ENDIF(UNIX)

IF(WIN32)
	OV_LINK_BOOST_LIB("thread" ${OV_WIN32_BOOST_VERSION})
ENDIF(WIN32)
	
