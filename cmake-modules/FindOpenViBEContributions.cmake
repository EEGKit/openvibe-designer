# ---------------------------------
# Finds OpenViBE contributions
# Only serves to set up a define for the preprocessor
# ---------------------------------

SET(PATH_OPENVIBE_CONTRIBUTIONS "PATH_OPENVIBE_CONTRIBUTIONS-NOTFOUND")
FIND_PATH(PATH_OPENVIBE_CONTRIBUTIONS common/contribAcquisitionServer.cmake PATHS ${OV_BASE_DIR}/contrib  NO_DEFAULT_PATH)
IF(PATH_OPENVIBE_CONTRIBUTIONS)
	debug_message( "  Found openvibe-contributions...")

	ADD_DEFINITIONS(-DTARGET_HAS_OpenViBEContributions)
ELSE(PATH_OPENVIBE_CONTRIBUTIONS)
	MESSAGE(WARNING "  FAILED to find openvibe-contributions...")
ENDIF(PATH_OPENVIBE_CONTRIBUTIONS)

