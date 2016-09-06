# ---------------------------------
# Finds Certivibe binary distribution
# Adds library to target
# Adds include path
# ---------------------------------
option(DYNAMIC_LINK_CERTIVIBE "Dynamically link Certivibe" ON)

if(DYNAMIC_LINK_CERTIVIBE)
	add_definitions(-DOV_Shared)
endif()

if(DYNAMIC_LINK_CERTIVIBE)
	set(CERTIVIBE_LINKING "")
else()
	set(CERTIVIBE_LINKING "-static")
endif()

set(CERTIVIBE_DIRECTORY ${OV_CUSTOM_DEPENDENCIES_PATH}/certivibe)

set(PATH_CERTIVIBE "PATH_CERTIVIBE-NOTFOUND")
find_path(PATH_CERTIVIBE include/openvibe/ov_all.h PATHS ${CERTIVIBE_DIRECTORY} NO_DEFAULT_PATH)
if(PATH_CERTIVIBE)
	debug_message( "  Found Certivibe... [${PATH_CERTIVIBE}]")
	include_directories(${PATH_CERTIVIBE}/include/)

	target_link_libraries(${PROJECT_NAME} openvibe${OPENVIBE_LINKING})
	
	add_definitions(-DTARGET_HAS_OpenViBE)
else()
	message(WARNING "  FAILED to find Certivibe...")
endif()


