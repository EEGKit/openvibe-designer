# ---------------------------------
# Finds Common sources
# Sets common_plugins_source_files
# Adds appropriate include dir
# ---------------------------------

FILE(GLOB_RECURSE common_plugins_source_files ${MENSIA_BASE_DIR}/common/plugins/*.cpp ${MENSIA_BASE_DIR}/common/plugins/*.h ${MENSIA_BASE_DIR}/common/plugins/*.hpp)
INCLUDE_DIRECTORIES("${MENSIA_BASE_DIR}/common/plugins")
SET(source_files "${source_files};${common_plugins_source_files}")

