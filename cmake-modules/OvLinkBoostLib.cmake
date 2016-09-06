#
# Win32 only
#
# This concoction mainly exists to locate the boost libraries on WIN32 in the case where our dependency/ folder has only a
# truncated version of boost and we cannot use find_package(). The background story is that we wish to link explicitly
# instead of using automatic linking and specifying a generic boost path to linker before each project definition.
#
# The downside of this is that it assumes the boost library names to have a particular format and only works for VC90/VC100
#
# Function: Links specified boost library defined by COMPONENT and BOOST_VERSION to PROJECT_NAME.
#

FUNCTION(OV_LINK_BOOST_LIB BOOST_COMPONENT BOOST_VERSION)

IF(WIN32)
	IF(MSVC)
		IF(MSVC_VERSION EQUAL 1500) # vc9
			SET(LIB_BOOST "libboost_${BOOST_COMPONENT}-vc90-mt-${BOOST_VERSION}.lib")
			SET(LIB_BOOST_DEBUG "libboost_${BOOST_COMPONENT}-vc90-mt-gd-${BOOST_VERSION}.lib")
		ELSEIF(MSVC_VERSION EQUAL 1600) # vc 10
			SET(LIB_BOOST "libboost_${BOOST_COMPONENT}-vc100-mt-${BOOST_VERSION}.lib")
			SET(LIB_BOOST_DEBUG "libboost_${BOOST_COMPONENT}-vc100-mt-gd-${BOOST_VERSION}.lib")
		ELSEIF(MSVC_VERSION EQUAL 1700) # vc 11
			SET(LIB_BOOST "libboost_${BOOST_COMPONENT}-vc110-mt-${BOOST_VERSION}.lib")
			SET(LIB_BOOST_DEBUG "libboost_${BOOST_COMPONENT}-vc110-mt-gd-${BOOST_VERSION}.lib")
		ELSEIF(MSVC_VERSION EQUAL 1800) # vc12
			SET(LIB_BOOST "libboost_${BOOST_COMPONENT}-vc120-mt-${BOOST_VERSION}.lib")
			SET(LIB_BOOST_DEBUG "libboost_${BOOST_COMPONENT}-vc120-mt-gd-${BOOST_VERSION}.lib")
		ENDIF(MSVC_VERSION EQUAL 1500)
	ENDIF(MSVC)

	SET(LIB_BOOST_PATH "-NOTFOUND")
	FIND_LIBRARY(LIB_BOOST_PATH NAMES ${LIB_BOOST} PATHS ${OV_CUSTOM_DEPENDENCIES_PATH}/boost/lib NO_DEFAULT_PATH)
	IF(LIB_BOOST_PATH)
		debug_message( "    [  OK  ] lib ${LIB_BOOST_PATH}")
		IF(TARGET ${PROJECT_NAME})
			TARGET_LINK_LIBRARIES(${PROJECT_NAME} optimized ${LIB_BOOST_PATH})
		ENDIF()
	ELSE(LIB_BOOST_PATH)
		MESSAGE(WARNING "    Unable to find boost ${LIB_BOOST}")
	ENDIF(LIB_BOOST_PATH)

	SET(LIB_BOOST_DEBUG_PATH "-NOTFOUND")
	FIND_LIBRARY(LIB_BOOST_DEBUG_PATH NAMES ${LIB_BOOST_DEBUG} PATHS ${OV_CUSTOM_DEPENDENCIES_PATH}/boost/lib NO_DEFAULT_PATH)
	IF(LIB_BOOST_DEBUG_PATH)
		debug_message( "    [  OK  ] lib ${LIB_BOOST_DEBUG_PATH}")
		IF(TARGET ${PROJECT_NAME})
			TARGET_LINK_LIBRARIES(${PROJECT_NAME} debug ${LIB_BOOST_DEBUG_PATH})
		ENDIF()
	ELSE(LIB_BOOST_DEBUG_PATH)
		MESSAGE(WARNING "    Unable to find boost ${LIB_BOOST_DEBUG}")
	ENDIF(LIB_BOOST_DEBUG_PATH)
ENDIF(WIN32)

ENDFUNCTION(OV_LINK_BOOST_LIB)
