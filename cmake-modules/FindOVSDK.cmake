# ---------------------------------
# Finds OpenViBE SDK binary distribution
# ---------------------------------


if(NOT CMAKE_BUILD_TYPE AND CMAKE_GENERATOR MATCHES "Visual Studio*")
	set(MULTI_BUILD TRUE)
elseif(CMAKE_BUILD_TYPE AND OV_PACKAGE)
	set(SOLO_PACKAGE TRUE)
elseif(CMAKE_BUILD_TYPE)
	set(SOLO_BUILD TRUE)
else()
	message(FATAL_ERROR "Build should specify a type or use a multi-type generator (like Visual Studio)")
endif()

if(NOT DEFINED TRIED_FIND_OVSDK)
	if(MULTI_BUILD)
		set(SEEK_PATHS ${OPENVIBE_SDK_PATH};${LIST_DEPENDENCIES_PATH})
		unset(OPENVIBE_SDK_PATH CACHE)
		foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
			# set(OPENVIBE_SDK_PATH ${OPENVIBE_SDK_PATH}/$<UPPER_CASE:$<CONFIG>>)
			string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
			unset(OPENVIBE_SDK_PATH_TMP CACHE)
			find_path(OPENVIBE_SDK_PATH_TMP include/openvibe/ov_all.h PATHS ${SEEK_PATHS} PATH_SUFFIXES openvibe-sdk-${OUTPUTCONFIG} ${OUTPUTCONFIG} NO_DEFAULT_PATH)
			set(OPENVIBE_SDK_PATH_${OUTPUTCONFIG} ${OPENVIBE_SDK_PATH_TMP})
			if(OPENVIBE_SDK_PATH_TMP)
				message("Found ${OUTPUTCONFIG} of sdk at ${OPENVIBE_SDK_PATH_TMP}")
				string(CONCAT OPENVIBE_SDK_PATH ${OPENVIBE_SDK_PATH} $<$<CONFIG:${OUTPUTCONFIG}>:${OPENVIBE_SDK_PATH_TMP}>)
				set(AT_LEAST_ONE_OV_BUILD TRUE)
			endif()
		endforeach()
		if(NOT DEFINED AT_LEAST_ONE_OV_BUILD)
			message(FATAL_ERROR "Did not find any valid build of OpenViBE SDK")
		endif()
	else() # Regular build
		string(TOLOWER CMAKE_BUILD_TYPE OV_SDK_BUILD_TYPE)
		find_path(${OPENVIBE_SDK_PATH} include/openvibe/ov_all.h PATHS ${LIST_DEPENDENCIES_PATH} PATH_SUFFIXES openvibe-sdk-${OV_SDK_BUILD_TYPE} NO_DEFAULT_PATH)
		if(${OPENVIBE_SDK_PATH} STREQUAL "OPENVIBE_SDK_PATH-NOTFOUND")
			message(FATAL_ERROR "  FAILED to find OpenViBE SDK [${OPENVIBE_SDK_PATH}]")
		endif()
		string(REGEX REPLACE "\\\\+" "/" OPENVIBE_SDK_PATH ${OPENVIBE_SDK_PATH})
		message(STATUS "  Found OpenViBE SDK... [${OPENVIBE_SDK_PATH}]")
	endif()
	set(TRIED_FIND_OVSDK TRUE)
endif()

if(INSTALL_SDK)
	if(MULTI_BUILD) # Replace with generator expression in CMake 3.5+
		foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
			string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIGU)
			file(GLOB EXE_SCRIPT_LIST "${OPENVIBE_SDK_PATH_${OUTPUTCONFIGU}}/*.cmd" "${OPENVIBE_SDK_PATH_${OUTPUTCONFIGU}}/*.sh")
			if(EXE_SCRIPT_LIST)
				foreach(SCRIPT IN LISTS EXE_SCRIPT_LIST)
					get_filename_component(base_name ${SCRIPT} NAME_WE)
					if(WIN32)
						set(exe_name "${base_name}.exe")
					else()
						set(exe_name ${base_name})
					endif()
					IF(WIN32)
						SET(SCRIPT_POSTFIX ".cmd")
					ELSEIF(APPLE)
						SET(SCRIPT_POSTFIX "-macos.sh")
					ELSEIF(UNIX)
						# Debian recommends that extensions such as .sh are not used; On Linux, scripts with such extensions shouldn't be packaged
						SET(SCRIPT_POSTFIX ".sh")
					ENDIF()
					IF(WIN32)
						SET(OV_CMD_EXECUTABLE "%OV_PATH_ROOT%/bin/${exe_name}")
					ENDIF()					
					SET(SCRIPT_NAME ${base_name}${SCRIPT_POSTFIX})
					SET(OV_CMD_ARGS "")
					SET(OV_PAUSE "")
					
					CONFIGURE_FILE(${OV_LAUNCHER_SOURCE_PATH}/openvibe-launcher${SCRIPT_POSTFIX}-base ${CMAKE_CURRENT_BINARY_DIR}/${SCRIPT_NAME} @ONLY)
					INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${SCRIPT_NAME} DESTINATION ${DIST_ROOT})
				endforeach()
			endif()
			install(DIRECTORY ${OPENVIBE_SDK_PATH_${OUTPUTCONFIGU}}/include DESTINATION ${DIST_INCLUDEDIR} CONFIGURATIONS ${OUTPUTCONFIG})
			install(DIRECTORY ${OPENVIBE_SDK_PATH_${OUTPUTCONFIGU}}/bin DESTINATION ${DIST_BINDIR} CONFIGURATIONS ${OUTPUTCONFIG}) # FILES_MATCHING PATTERN "openvibe-plugins*dll") or *so*
			install(DIRECTORY ${OPENVIBE_SDK_PATH_${OUTPUTCONFIGU}}/lib DESTINATION ${DIST_LIBDIR} CONFIGURATIONS ${OUTPUTCONFIG}) # FILES_MATCHING PATTERN "openvibe-plugins*dll")
			install(DIRECTORY ${OPENVIBE_SDK_PATH_${OUTPUTCONFIGU}}/etc DESTINATION ${DIST_SYSCONFDIR} CONFIGURATIONS ${OUTPUTCONFIG})
			install(DIRECTORY ${OPENVIBE_SDK_PATH_${OUTPUTCONFIGU}}/share DESTINATION ${DIST_DATADIR} CONFIGURATIONS ${OUTPUTCONFIG})
		endforeach()
	else()
		file(GLOB EXE_SCRIPT_LIST "${OPENVIBE_SDK_PATH}/*.cmd" "${OPENVIBE_SDK_PATH}/*.sh")
		foreach(SCRIPT IN LISTS EXE_SCRIPT_LIST)
			get_filename_component(base_name ${SCRIPT} NAME_WE)
			if(WIN32)
				set(exe_name "${base_name}.exe")
			else()
				set(exe_name ${base_name})
			endif()
			OV_INSTALL_LAUNCH_SCRIPT(SCRIPT_PREFIX ${base_name} EXECUTABLE_NAME ${exe_name} NOPROJECT)
		endforeach()
		install(DIRECTORY ${OPENVIBE_SDK_PATH}/include DESTINATION ${DIST_INCLUDEDIR})
		install(DIRECTORY ${OPENVIBE_SDK_PATH}/bin DESTINATION ${DIST_BINDIR}) # FILES_MATCHING PATTERN "openvibe-plugins*dll") or *so*
		install(DIRECTORY ${OPENVIBE_SDK_PATH}/lib DESTINATION ${DIST_LIBDIR}) # FILES_MATCHING PATTERN "openvibe-plugins*dll")
		install(DIRECTORY ${OPENVIBE_SDK_PATH}/etc DESTINATION ${DIST_SYSCONFDIR})
		install(DIRECTORY ${OPENVIBE_SDK_PATH}/share DESTINATION ${DIST_DATADIR})
	endif()
endif()