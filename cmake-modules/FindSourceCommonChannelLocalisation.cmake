# ---------------------------------
# Finds Common channels
# Sets common_channel_files
# Adds appropriate include dir
# ---------------------------------

FILE(GLOB_RECURSE common_channel_files ${MENSIA_BASE_DIR}/common/channel-localisation/*.cpp ${MENSIA_BASE_DIR}/common/channel-localisation/*.c ${MENSIA_BASE_DIR}/common/channel-localisation/*.h ${MENSIA_BASE_DIR}/common/channel-localisation/*.hpp ${MENSIA_BASE_DIR}/common/channel-localisation/*.inl)
INCLUDE_DIRECTORIES("${MENSIA_BASE_DIR}/common/channel-localisation")
SET(channel_files "${channel_files};${common_channel_files}")

