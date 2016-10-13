# ---------------------------------
# Finds JSON sources
# Sets json_source_files
# Adds appropriate include dir
# ---------------------------------

FILE(GLOB_RECURSE json_source_files ${OV_SOURCE_DEPENDENCIES_PATH}/json/*.cpp ${OV_SOURCE_DEPENDENCIES_PATH}/json/*.c ${OV_SOURCE_DEPENDENCIES_PATH}/json/*.h)
INCLUDE_DIRECTORIES("${OV_SOURCE_DEPENDENCIES_PATH}")
SET(source_files "${source_files};${json_source_files}")
