# ---------------------------------
# Finds Lepton sources
# Sets lepton_source_files
# Adds appropriate include dir
# ---------------------------------

FILE(GLOB_RECURSE lepton_source_files ${MENSIA_DEPENDENCIES}/lepton/*.cpp ${MENSIA_DEPENDENCIES}/lepton/*.h)
ADD_DEFINITIONS(-DTARGET_HAS_LEPTON)
INCLUDE_DIRECTORIES("${MENSIA_DEPENDENCIES}")
SET(source_files "${source_files};${lepton_source_files}")
