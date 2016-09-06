# ---------------------------------
# Finds JSON sources
# Sets json_source_files
# Adds appropriate include dir
# ---------------------------------

FILE(GLOB_RECURSE optionparser_source_files ${MENSIA_DEPENDENCIES}/optionparser/*.h )
ADD_DEFINITIONS(-DTARGET_HAS_OPTIONPARSER)
INCLUDE_DIRECTORIES("${MENSIA_DEPENDENCIES}")
SET(source_files "${source_files};${optionparser_source_files}")