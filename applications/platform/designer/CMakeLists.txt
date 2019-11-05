project(openvibe-designer)

set(PROJECT_PRODUCT_NAME "${BRAND_NAME} Designer")

if(WIN32)
	# Should be uncommented once we stop using cmd as launcher
	#SET(ENTRYPOINT WIN32)
	set(ENTRYPOINT )
else()
	set(ENTRYPOINT )
endif()

file(GLOB_RECURSE SRC_FILES src/*.cpp src/visualization/*.cpp src/dynamic_settings/*.cpp)

include("FindSourceDependencyJSON")

set(SRC_FILES "${SRC_FILES};${CMAKE_BINARY_DIR}/resource-files/${LOWER_BRAND_NAME}-${LOWER_DESIGNER_NAME}.rc")
add_executable(${PROJECT_NAME} ${ENTRYPOINT} ${SRC_FILES})

# Generate launch script
OV_INSTALL_LAUNCH_SCRIPT(SCRIPT_PREFIX "${LOWER_BRAND_NAME}-${LOWER_DESIGNER_NAME}" EXECUTABLE_NAME "${PROJECT_NAME}" ICON_PATH "${OV_ICON_PATH}/designer.ico")

SET(INCLUDED_OV_SDK_COMPONENTS BASE ALLPLUGINS ALLMODULES DEPENDENCIES)
include("AddOpenViBESDKComponents")
include("FindOpenViBEVisualizationToolkit")
include("FindThirdPartyGTK")
include("FindThirdPartyBoost")
include("FindThirdPartyBoost_System")
include("FindThirdPartyBoost_Thread")
include("FindThirdPartyBoost_Filesystem")

add_definitions(-DProjectVersion=\"${OV_GLOBAL_VERSION_STRING}\")
add_definitions(-DM_VERSION_MAJOR=${OV_GLOBAL_VERSION_MAJOR})
add_definitions(-DM_VERSION_MINOR=${OV_GLOBAL_VERSION_MINOR})

set_build_platform()

# Install files
install(TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION ${DIST_BINDIR}
	LIBRARY DESTINATION ${DIST_LIBDIR}
	ARCHIVE DESTINATION ${DIST_LIBDIR})


configure_file(share/designer.conf-base "${CMAKE_CURRENT_BINARY_DIR}/designer.conf" @ONLY)
configure_file(share/about-dialog.ui-base "${CMAKE_CURRENT_BINARY_DIR}/about-dialog.ui")
configure_file(share/interface.ui-base "${CMAKE_CURRENT_BINARY_DIR}/interface.ui")
configure_file(share/interface-settings.ui-base "${CMAKE_CURRENT_BINARY_DIR}/interface-settings.ui")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/designer.conf" DESTINATION ${DIST_DATADIR}/openvibe/applications/designer)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/about-dialog.ui" DESTINATION ${DIST_DATADIR}/openvibe/applications/designer)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/interface.ui" DESTINATION ${DIST_DATADIR}/openvibe/applications/designer)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/interface-settings.ui" DESTINATION ${DIST_DATADIR}/openvibe/applications/designer)

install(DIRECTORY share/ DESTINATION ${DIST_DATADIR}/openvibe/applications/designer PATTERN PATTERN "*-base*" EXCLUDE)

