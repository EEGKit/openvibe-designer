# ---------------------------------
# Finds Metabox Loader sources
# Appends metabox_loader_source_files to source_files
# Adds appropriate include dir
# ---------------------------------


debug_message( "Including Mensia Scenario-IO")

FILE(GLOB_RECURSE metabox_loader_source_files ${OV_SOURCE_DEPENDENCIES_PATH}/metabox-loader/*.cpp)
INCLUDE_DIRECTORIES("${OV_SOURCE_DEPENDENCIES_PATH}")
SET(source_files "${source_files};${metabox_loader_source_files}")
