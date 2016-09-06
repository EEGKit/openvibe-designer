# ---------------------------------
# Finds Scenario IO sources
# Appendsscenario_io_source_files to source_files
# Adds appropriate include dir
# ---------------------------------


debug_message( "Including Mensia Scenario-IO from ${MENSIA_BASE_DIR}/common/scenario-io")

FILE(GLOB_RECURSE scenario_io_source_files ${MENSIA_BASE_DIR}/common/scenario-io/*.cpp ${MENSIA_BASE_DIR}/common/scenario-io/*.c ${MENSIA_BASE_DIR}/common/scenario-io/*.h ${MENSIA_BASE_DIR}/common/scenario-io/*.hpp ${MENSIA_BASE_DIR}/common/scenario-io/*.inl)
INCLUDE_DIRECTORIES(${OV_BASE_DIR}/modules/ebml/include)
INCLUDE_DIRECTORIES("${MENSIA_BASE_DIR}/common/scenario-io")
SET(source_files "${source_files};${scenario_io_source_files}")
