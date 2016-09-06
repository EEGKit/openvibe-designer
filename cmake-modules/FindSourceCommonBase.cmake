# ---------------------------------
# Finds Common sources
# Sets common_source_files
# Adds appropriate include dir
# ---------------------------------

FILE(GLOB_RECURSE common_source_files ${MENSIA_BASE_DIR}/common/base/*.cpp ${MENSIA_BASE_DIR}/common/base/*.c ${MENSIA_BASE_DIR}/common/base/*.h ${MENSIA_BASE_DIR}/common/base/*.hpp ${MENSIA_BASE_DIR}/common/base/*.inl)
INCLUDE_DIRECTORIES("${MENSIA_BASE_DIR}/common/base")
SET(source_files "${source_files};${common_source_files}")

