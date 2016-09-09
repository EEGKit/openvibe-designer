PROJECT(openvibe-designer)
SET(PROJECT_PRODUCT_NAME "${BRAND_NAME} Studio")
IF(WIN32)
# Should be uncommented once we stop using cmd as launcher
#SET(ENTRYPOINT WIN32)
SET(ENTRYPOINT )
ELSE(WIN32)
SET(ENTRYPOINT )
ENDIF(WIN32)

FILE(GLOB_RECURSE source_files src/*.cpp src/*.inl)
INCLUDE("FindSourceCommonBase")
INCLUDE("FindSourceDependencyOptionParser")
INCLUDE("FindSourceCommonMetaboxLoader")

OV_INSTALL_LAUNCH_SCRIPT(SCRIPT_PREFIX "${LOWER_BRAND_NAME}-${LOWER_STUDIO_NAME}" EXECUTABLE_NAME "${PROJECT_NAME}" ICON_PATH "${OV_ICON_PATH}/designer.ico")
SET(source_files "${source_files};${CMAKE_INSTALL_FULL_DATADIR}/resource-files/${LOWER_BRAND_NAME}-${LOWER_STUDIO_NAME}.rc")
ADD_EXECUTABLE(${PROJECT_NAME} ${ENTRYPOINT} ${source_files}) 
# ----------------------
# Generate launch script
# ----------------------

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${RLM_LIB_FILES})

# Used by CActivationTool
ADD_DEFINITIONS("-DOV_PATH_ROOT=\"${CMAKE_INSTALL_PREFIX}\"")

INCLUDE("FindCertivibe")
INCLUDE("FindThirdPartyGTK")
INCLUDE("FindThirdPartyBoost")
INCLUDE("FindThirdPartyBoost_System")
INCLUDE("FindThirdPartyBoost_Thread")
INCLUDE("FindThirdPartyBoost_Filesystem")

IF(DISABLE_FEATURE_SCENARIO_CONFIGURATION)
	SET(DISABLE_FEATURE_METABOXES ON)
	ADD_DEFINITIONS(-DTARGET_HasScenarioConfigurationDisabled)
ENDIF()

IF(DISABLE_FEATURE_METABOXES)
	ADD_DEFINITIONS(-DTARGET_HasMetaboxesDisabled)
ENDIF()

ADD_DEFINITIONS(-DProjectVersion=\"${MENSIA_GLOBAL_VERSION_STRING}\")
ADD_DEFINITIONS(-DM_VERSION_MAJOR=${MENSIA_GLOBAL_VERSION_MAJOR})
ADD_DEFINITIONS(-DM_VERSION_MINOR=${MENSIA_GLOBAL_VERSION_MINOR})
## ---------------------------------
# Target macros
# Defines target operating system
# Defines target architecture
# Defines target compiler
# ---------------------------------
SET_BUILD_PLATFORM()

# -----------------------------
# Install files
# -----------------------------
INSTALL(TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})

CONFIGURE_FILE(share/about-dialog.ui-base "${CMAKE_CURRENT_BINARY_DIR}/about-dialog.ui")
CONFIGURE_FILE(share/interface.ui-base "${CMAKE_CURRENT_BINARY_DIR}/interface.ui")
CONFIGURE_FILE(share/interface-settings.ui-base "${CMAKE_CURRENT_BINARY_DIR}/interface-settings.ui")

INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/about-dialog.ui" DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/openvibe/applications/designer)
INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/interface.ui" DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/openvibe/applications/designer)
INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/interface-settings.ui" DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/openvibe/applications/designer)

INSTALL(DIRECTORY share/ DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/openvibe/applications/designer PATTERN PATTERN "*-base*" EXCLUDE)

