# ---------------------------------
# Configure RC file
# Sets source_files to add the path to the newly created rc file
# Adds appropriate include dir
# ---------------------------------
IF(WIN32)
	OV_CONFIGURE_RC(NAME ${PROJECT_NAME})
	SET(source_files "${source_files};${CMAKE_INSTALL_FULL_DATADIR}/resource-files/${PROJECT_NAME}.rc")
ENDIF()

