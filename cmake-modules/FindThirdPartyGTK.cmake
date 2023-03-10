# ---------------------------------
# Finds GTK toolkit
#
# Sets GTK_FOUND
# Sets GTK_LIBRARIES
# Sets GTK_LIBRARY_DIRS
# Sets GTK_LDFLAGS
# Sets GTK_LDFLAGS_OTHERS
# Sets GTK_INCLUDE_DIRS
# Sets GTK_CFLAGS
# Sets GTK_CFLAGS_OTHERS
# ---------------------------------

INCLUDE("FindPkgConfig")

IF(WIN32)
	pkg_check_modules(GTK gtk+-win32-2.0 gthread-2.0)
ELSE(WIN32)
	pkg_check_modules(GTK "gtk+-2.0" "gthread-2.0")
ENDIF(WIN32)

IF(${BUILD_ARCH} STREQUAL "x64")
	SET(GTK_LIB_SUBFOLDER "i686-pc-vs10")
	SET(LIB_Z_NAME "zlib1")
ELSE()
	SET(GTK_LIB_SUBFOLDER "2.10.0")
	SET(LIB_Z_NAME "zdll")
ENDIF()
IF(GTK_FOUND)
	debug_message( "  Found GTK+...")
	# This is a bit convoluted way of finding zlib (because GTK_INCLUDE_DIRS
	# is actually a list of folders, not a single one)
	find_path(PATH_ZLIB gtk/include/zlib.h PATHS ${LIST_DEPENDENCIES_PATH} NO_DEFAULT_PATH)
	INCLUDE_DIRECTORIES(${GTK_INCLUDE_DIRS} ${GTHREAD_INCLUDE_DIRS} "${PATH_ZLIB}/gtk/include")
	#shouldn't add GTK_CFLAGS, this results in AdditionalIncludeDirectories becoming broken in visual studio
	#ADD_DEFINITIONS(${GTK_CFLAGS} ${GTK_CFLAGS_OTHERS} ${GTHREAD_CFLAGS}${GTHREAD_CFLAGS_OTHERS})
	#LINK_DIRECTORIES(${GTK_LIBRARY_DIRS} ${GTHREAD_LIBRARY_DIRS})
	IF(WIN32)
		SET( GTK_LIB_LIST ${GTK_LIBRARIES} ${GTHREAD_LIBRARIES} ${LIB_Z_NAME})
	ELSE(WIN32)
		SET( GTK_LIB_LIST ${GTK_LIBRARIES} ${GTHREAD_LIBRARIES} z)
	ENDIF(WIN32)

	IF(WIN32)
		# gdi32.lib could be under the MS Windows SDK
		INCLUDE("OvSetWindowsSDKPath")
		INSTALL(
			DIRECTORY ${GTK_LIBRARY_DIRS}/../bin/
			DESTINATION ${DIST_BINDIR}
			FILES_MATCHING PATTERN "*.dll")
		INSTALL(
			DIRECTORY ${GTK_LIBRARY_DIRS}/gtk-2.0/${GTK_LIB_SUBFOLDER}/engines/
			DESTINATION ${DIST_LIBDIR}/gtk-2.0/${GTK_LIB_SUBFOLDER}/engines/
			FILES_MATCHING PATTERN "*.dll")
		INSTALL(
			FILES ${GTK_LIBRARY_DIRS}/..//etc/gtk-2.0/gtkrc
			DESTINATION ${DIST_SYSCONFDIR}/gtk-2.0/)
	ENDIF()
	
	FOREACH(GTK_LIB ${GTK_LIB_LIST})
		SET(GTK_LIB1 "GTK_LIB1-NOTFOUND")
		FIND_LIBRARY(GTK_LIB1 NAMES ${GTK_LIB} PATHS ${GTK_LIBRARY_DIRS} ${GTK_LIBDIR} NO_DEFAULT_PATH)
		FIND_LIBRARY(GTK_LIB1 NAMES ${GTK_LIB} PATHS ${GTK_LIBRARY_DIRS} ${GTK_LIBDIR})
		IF(WIN32)
			FIND_LIBRARY(GTK_LIB1 NAMES ${GTK_LIB} PATHS ${OV_MS_SDK_PATH}/lib)
		ENDIF(WIN32)
		IF(GTK_LIB1)
			debug_message( "    [  OK  ] Third party lib ${GTK_LIB1}")
			TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${GTK_LIB1})
		ELSE(GTK_LIB1)
		    MESSAGE(WARNING "    [FAILED] Third party lib ${GTK_LIB}")
		ENDIF(GTK_LIB1)
	ENDFOREACH(GTK_LIB)
ELSE(GTK_FOUND)
	MESSAGE(WARNING "  FAILED to find GTK+...")
	IF(NOT PKG_CONFIG_FOUND) 
		MESSAGE(WARNING "  Did not even find pkg-config exe")
	ENDIF(NOT PKG_CONFIG_FOUND)
ENDIF(GTK_FOUND)
