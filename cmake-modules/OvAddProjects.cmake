# Adds the current project to the global properties
FUNCTION(OV_ADD_THIS_TO_PROJECT_LIST)
	debug_message( "ADDING: ${CMAKE_CURRENT_SOURCE_DIR}")

	# Add the dir to be parsed for documentation later. We need to do this before adding subdir, in case the subdir is the actual docs dir
	#GET_PROPERTY(OV_TMP GLOBAL PROPERTY OV_PROP_CURRENT_PROJECTS)
	#SET(OV_TMP "${OV_TMP};${CMAKE_CURRENT_SOURCE_DIR}")
	#SET_PROPERTY(GLOBAL PROPERTY OV_PROP_CURRENT_PROJECTS ${OV_TMP})
	IF(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/doc AND NOT(${SKIP_DOC_OPENVIBE}))
		FILE(COPY ${CMAKE_CURRENT_SOURCE_DIR}/doc/ DESTINATION ${DOCUMENTATION_TEMP_DIRECTORY}/openvibe FILES_MATCHING PATTERN "*.dox")
		FILE(COPY ${CMAKE_CURRENT_SOURCE_DIR}/doc/ DESTINATION ${DOCUMENTATION_TEMP_DIRECTORY}/openvibe PATTERN "*.dox" EXCLUDE)
	ENDIF()

	IF(IS_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/include) AND NOT(${SKIP_DOCUMENTATION}) AND NOT(${SKIP_DOC_OPENVIBE}))
		FILE(APPEND "${DOCUMENTATION_TEMP_DIRECTORY}/openvibe/source-list.txt" "${CMAKE_CURRENT_SOURCE_DIR}/include\n")
	ENDIF()
ENDFUNCTION()

FUNCTION(OV_SET_CUSTOM_DOCUMENTATION doc_project_name doc_product_name)
	# skip_documentation is project-wide
	IF(NOT(${SKIP_DOCUMENTATION}))
		
		debug_message( "CUSTOM DOCUMENTATION ENABLED FOR ${doc_project_name}")
		SET(HAS_CUSTOM_DOCUMENTATION_${doc_project_name} "1" PARENT_SCOPE)
		SET(CUSTOM_DOCUMENTATION_PRODUCT_NAME_${doc_project_name} "${doc_product_name}" PARENT_SCOPE)

		# if we need also the sources (e.g. the API is documented with doxygen)
		SET(ADDITIONAL_FOLDERS_STRING_LIST "")
		FOREACH(additional_cpp_source ${ARGN})
			SET(ADDITIONAL_FOLDERS_STRING_LIST "${ADDITIONAL_FOLDERS_STRING_LIST} ${CMAKE_CURRENT_SOURCE_DIR}/${doc_project_name}/${additional_cpp_source}")
		ENDFOREACH()
		SET(CUSTOM_DOCUMENTATION_CPP_SOURCES_${doc_project_name} "${ADDITIONAL_FOLDERS_STRING_LIST}" PARENT_SCOPE)


	ENDIF()
ENDFUNCTION()

#
# Adds all directories as subdirectories to the CMake build, using the branch specified (if any) in the root CMakeList.txt or
# trunk otherwise.
#
# The branch variable name that is checked is made up from ${CURRENT_BRANCH_PREFIX}_${DIRNAMEUPPER}.
#
# The script also adds the directory to the global list of projects.
#

FUNCTION(OV_ADD_PROJECTS CURRENT_FOLDER_PREFIX)

	FILE(GLOB FILENAMES "*")

	FOREACH(FULLPATH ${FILENAMES})
		IF(IS_DIRECTORY ${FULLPATH} AND NOT ${FULLPATH} MATCHES ".*\\.svn")
			IF(EXISTS "${FULLPATH}/CMakeLists.txt")
			GET_FILENAME_COMPONENT(DIRNAME ${FULLPATH} NAME)

			STRING(TOUPPER ${DIRNAME} DIRNAMEUPPER)
			SET(SKIP_THIS_FOLDER "SKIP_${CURRENT_FOLDER_PREFIX}_${DIRNAMEUPPER}")
			SET(SKIP_THIS_CUSTOM_DOCUMENTATION "SKIP_DOC_${CURRENT_FOLDER_PREFIX}_${DIRNAMEUPPER}")

			debug_message( "Checking SKIP_${CURRENT_FOLDER_PREFIX}_${DIRNAMEUPPER} as branch var ${SKIP_THIS_FOLDER}")

			IF(${SKIP_THIS_FOLDER})
				debug_message( "Note: ${FULLPATH} has been skipped by setting SKIP_${CURRENT_FOLDER_PREFIX}_${DIRNAMEUPPER}")
			ELSE(${SKIP_THIS_FOLDER})
				debug_message( "Inserting folder ${FULLPATH}")

				# Add the dir to be parsed for documentation later. We need to do this before adding subdir, in case the subdir is the actual docs dir
				GET_PROPERTY(OV_TMP GLOBAL PROPERTY OV_PROP_CURRENT_PROJECTS)
				SET(OV_TMP "${OV_TMP};${FULLPATH}")
				SET_PROPERTY(GLOBAL PROPERTY OV_PROP_CURRENT_PROJECTS ${OV_TMP})

				SET(OV_CURRENT_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/${DIRNAME}")
				GET_PROPERTY(OV_TMP GLOBAL PROPERTY OV_PROP_CURRENT_PROJECTS_BUILD_DIR)
				SET(OV_TMP "${OV_TMP};${OV_CURRENT_BINARY_DIR}")
				SET_PROPERTY(GLOBAL PROPERTY OV_PROP_CURRENT_PROJECTS_BUILD_DIR ${OV_TMP})


				SET(HAS_CUSTOM_DOCUMENTATION "HAS_CUSTOM_DOCUMENTATION_${DIRNAME}")
				SET(CUSTOM_DOCUMENTATION_PRODUCT_NAME "CUSTOM_DOCUMENTATION_PRODUCT_NAME_${DIRNAME}")
				SET(CUSTOM_DOCUMENTATION_CPP_SOURCES "${CUSTOM_DOCUMENTATION_CPP_SOURCES_${DIRNAME}}")
				IF(NOT(${SKIP_DOCUMENTATION}) AND NOT(${SKIP_THIS_CUSTOM_DOCUMENTATION}))
					IF(NOT(${HAS_CUSTOM_DOCUMENTATION}))

						IF(NOT(${SKIP_DOC_OPENVIBE}))
							IF(IS_DIRECTORY ${FULLPATH}/doc)
								debug_message( "Documentation files found in ${FULLPATH}/doc")
								FILE(COPY ${FULLPATH}/doc/ DESTINATION ${DOCUMENTATION_TEMP_DIRECTORY}/openvibe/box-algorithm-doc/dox-part/ FILES_MATCHING PATTERN "*.dox-part")
								FILE(COPY ${FULLPATH}/doc/ DESTINATION ${DOCUMENTATION_TEMP_DIRECTORY}/openvibe FILES_MATCHING PATTERN "*.dox")
								FILE(COPY ${FULLPATH}/doc/ DESTINATION ${DOCUMENTATION_TEMP_DIRECTORY}/openvibe PATTERN "*.dox-part" EXCLUDE PATTERN "*.dox" EXCLUDE)
							ENDIF()

							IF(IS_DIRECTORY ${FULLPATH}/include)
								FILE(APPEND "${DOCUMENTATION_TEMP_DIRECTORY}/openvibe/source-list.txt" "${FULLPATH}/include\n")
							ENDIF()
						ENDIF()
					ELSE()
						debug_message( "Project ${DIRNAME} has custom documentation")

						# we copy the doc files into the temporary folder where everything is built thanks to a cmd script
						IF(IS_DIRECTORY ${FULLPATH}/doc/)
							FILE(COPY ${FULLPATH}/doc/ DESTINATION "${DOCUMENTATION_TEMP_DIRECTORY}/${DIRNAME}")
						ENDIF()

						SET(DOC_PROJECT_NAME ${DIRNAME})
						SET(DOC_PROJECT_VERSION ${PROJECT_VERSION})
						SET(DOC_PROJECT_PRODUCT_NAME "${${CUSTOM_DOCUMENTATION_PRODUCT_NAME}}")
						SET(DOC_PROJECT_COMMITHASH ${PROJECT_COMMITHASH})
						SET(DOC_PROJECT_BRANCH ${PROJECT_BRANCH})
						SET(DOC_PROJECT_CPP_SOURCES "${CUSTOM_DOCUMENTATION_CPP_SOURCES}")

						get_directory_property( DirDefs DIRECTORY ${CMAKE_SOURCE_DIR} COMPILE_DEFINITIONS )
						SET(DOC_DEFINES "")
						foreach( d ${DirDefs} )
							#message( STATUS "Found Define: " ${d} )
							SET(DOC_DEFINES "${DOC_DEFINES} \\\\ \\n ${d}")
						endforeach()
						STRING(REPLACE "\"" "\\\"" DOC_DEFINES "${DOC_DEFINES}")

						#message( STATUS ${DOC_DEFINES} )

						CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/documentation/src/build-documentation.cmake-skeleton" "${DOCUMENTATION_TEMP_DIRECTORY}/build-documentation-${DIRNAME}.cmake" @ONLY)

						FILE(APPEND "${DOCUMENTATION_TEMP_DIRECTORY}/build-documentation.cmd"
							"cmake -P build-documentation-${DIRNAME}.cmake\n")
					ENDIF()
				ENDIF()

				add_subdirectory(${FULLPATH})

			ENDIF(${SKIP_THIS_FOLDER})
			ENDIF(EXISTS "${FULLPATH}/CMakeLists.txt")
		ENDIF(IS_DIRECTORY ${FULLPATH} AND NOT ${FULLPATH} MATCHES ".*\\.svn")
	ENDFOREACH(FULLPATH ${FILENAMES})

ENDFUNCTION(OV_ADD_PROJECTS)
