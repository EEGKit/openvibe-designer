# ---------------------------------
# Finds Common sources
# Sets common_resampler_source_files
# Adds appropriate include dir
# ---------------------------------

FILE(GLOB_RECURSE common_resampler_source_files ${MENSIA_BASE_DIR}/common/resampler/*.cpp ${MENSIA_BASE_DIR}/common/resampler/*.h ${MENSIA_BASE_DIR}/common/resampler/*.hpp)
INCLUDE_DIRECTORIES("${MENSIA_BASE_DIR}/common/resampler")
INCLUDE("FindSourceDependencyR8Brain")
SET(source_files "${source_files};${common_resampler_source_files}")

