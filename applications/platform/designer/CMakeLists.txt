project(openvibe-designer)

set(PROJECT_PRODUCT_NAME "${BRAND_NAME} Studio")

if(WIN32)
	# Should be uncommented once we stop using cmd as launcher
	#SET(ENTRYPOINT WIN32)
	set(ENTRYPOINT )
else()
	set(ENTRYPOINT )
endif()

file(GLOB_RECURSE source_files src/*.cpp src/visualization/*.cpp src/dynamic_settings/*.cpp)

include("FindSourceDependencyJSON")

# Generate launch script
OV_INSTALL_LAUNCH_SCRIPT(SCRIPT_PREFIX "${LOWER_BRAND_NAME}-${LOWER_STUDIO_NAME}" EXECUTABLE_NAME "${PROJECT_NAME}" ICON_PATH "${OV_ICON_PATH}/designer.ico")

set(source_files "${source_files};${CMAKE_INSTALL_FULL_DATADIR}/resource-files/${LOWER_BRAND_NAME}-${LOWER_STUDIO_NAME}.rc")

add_executable(${PROJECT_NAME} ${ENTRYPOINT} ${source_files})

SET(INCLUDED_CERTIVIBE_COMPONENTS BASE ALLPLUGINS ALLMODULES DEPENDENCIES)
include("AddCertivibeComponents")
include("FindOpenViBEVisualizationToolkit")
include("FindThirdPartyGTK")
include("FindThirdPartyBoost")
include("FindThirdPartyBoost_System")
include("FindThirdPartyBoost_Thread")
include("FindThirdPartyBoost_Filesystem")

add_definitions(-DProjectVersion=\"${MENSIA_GLOBAL_VERSION_STRING}\")
add_definitions(-DM_VERSION_MAJOR=${MENSIA_GLOBAL_VERSION_MAJOR})
add_definitions(-DM_VERSION_MINOR=${MENSIA_GLOBAL_VERSION_MINOR})

set_build_platform()

# Install files
install(TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})

configure_file(share/about-dialog.ui-base "${CMAKE_CURRENT_BINARY_DIR}/about-dialog.ui")
configure_file(share/interface.ui-base "${CMAKE_CURRENT_BINARY_DIR}/interface.ui")
configure_file(share/interface-settings.ui-base "${CMAKE_CURRENT_BINARY_DIR}/interface-settings.ui")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/about-dialog.ui" DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/openvibe/applications/designer)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/interface.ui" DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/openvibe/applications/designer)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/interface-settings.ui" DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/openvibe/applications/designer)

install(DIRECTORY share/ DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/openvibe/applications/designer PATTERN PATTERN "*-base*" EXCLUDE)

