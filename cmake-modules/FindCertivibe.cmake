# ---------------------------------
# Finds Certivibe binary distribution
# Adds library to target
# Adds include path
# ---------------------------------
option(DYNAMIC_LINK_CERTIVIBE "Dynamically link Certivibe" ON)

if(DYNAMIC_LINK_CERTIVIBE)
	set(CERTIVIBE_LINKING "")
	add_definitions(-DOV_Shared)
	add_definitions(-DEBML_Shared)
	add_definitions(-DFS_Shared)
	add_definitions(-DSocket_Shared)
	add_definitions(-DSystem_Shared)
	add_definitions(-DXML_Shared)
	add_definitions(-DOVTK_Shared)
else()
	set(CERTIVIBE_LINKING "-static")
	add_definitions(-DEBML_Static)
	add_definitions(-DFS_Static)
	add_definitions(-DSocket_Static)
	add_definitions(-DSystem_Static)
	add_definitions(-DXML_Static)
	add_definitions(-DOVTK_Static)
endif()

set(CERTIVIBE_DIRECTORY ${OPENVIBE_SDK_PATH})

set(PATH_CERTIVIBE "PATH_CERTIVIBE-NOTFOUND")
find_path(PATH_CERTIVIBE include/openvibe/ov_all.h PATHS ${CERTIVIBE_DIRECTORY} NO_DEFAULT_PATH)
if(${PATH_CERTIVIBE} STREQUAL "PATH_CERTIVIBE-NOTFOUND")
	message(FATAL_ERROR "  FAILED to find Certivibe [${PATH_CERTIVIBE}]")
endif()

debug_message( "  Found Certivibe... [${PATH_CERTIVIBE}]")

include_directories(${PATH_CERTIVIBE}/include/)

target_link_libraries(${PROJECT_NAME} "${PATH_CERTIVIBE}/lib/openvibe${CERTIVIBE_LINKING}.lib")
target_link_libraries(${PROJECT_NAME} "${PATH_CERTIVIBE}/lib/openvibe-module-ebml${CERTIVIBE_LINKING}.lib")
target_link_libraries(${PROJECT_NAME} "${PATH_CERTIVIBE}/lib/openvibe-module-system${CERTIVIBE_LINKING}.lib")
target_link_libraries(${PROJECT_NAME} "${PATH_CERTIVIBE}/lib/openvibe-module-fs${CERTIVIBE_LINKING}.lib")
target_link_libraries(${PROJECT_NAME} "${PATH_CERTIVIBE}/lib/openvibe-module-socket${CERTIVIBE_LINKING}.lib")
target_link_libraries(${PROJECT_NAME} "${PATH_CERTIVIBE}/lib/openvibe-module-xml${CERTIVIBE_LINKING}.lib")
target_link_libraries(${PROJECT_NAME} "${PATH_CERTIVIBE}/lib/openvibe-toolkit${CERTIVIBE_LINKING}.lib")

add_definitions(-DTARGET_HAS_OpenViBE)
add_definitions(-DTARGET_HAS_EBML)
add_definitions(-DTARGET_HAS_System)
add_definitions(-DTARGET_HAS_FS)
add_definitions(-DTARGET_HAS_Socket)
add_definitions(-DTARGET_HAS_XML)
add_definitions(-DTARGET_HAS_OpenViBEToolkit)

# if we link with the module socket in Static, we must link the project with the dependency on win32
if(WIN32 AND NOT DYNAMIC_LINK_CERTIVIBE)
	include("FindThirdPartyWinsock2")
	include("FindThirdPartyFTDI")
endif()

install(DIRECTORY ${PATH_CERTIVIBE}/bin/ DESTINATION ${CMAKE_INSTALL_FULL_BINDIR} FILES_MATCHING PATTERN "*dll")
install(DIRECTORY ${PATH_CERTIVIBE}/share/ DESTINATION ${CMAKE_INSTALL_FULL_DATADIR})


