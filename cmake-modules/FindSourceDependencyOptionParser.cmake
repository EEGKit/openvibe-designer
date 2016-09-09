# ---------------------------------
# Finds JSON sources
# Sets json_source_files
# Adds appropriate include dir
# ---------------------------------

FILE(GLOB_RECURSE optionparser_source_files ${MENSIA_DEPENDENCIES}/optionparser/*.h )
ADD_DEFINITIONS(-DTARGET_HAS_OPTIONPARSER)
INCLUDE_DIRECTORIES("${OV_SOURCE_DEPENDENCIES_PATH}")
SET(source_files "${source_files};${optionparser_source_files}")
