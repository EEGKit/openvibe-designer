# Author Yann Renard / INRIA; Laurent Bonnet / Mensia Technologies
# Date 2008-10-15
# Modified 2013-06-26
#
# this CMake script iterates over several source documentation directories in
# order to compile it with doxygen. It has the ability to configure the
# doxyfile depending on some variables and to build documentation sources from
# computer generated templates (.dox-skeleton) and hand written documentation
# parts (.dox-part)

IF(MENSIA_BUILD_DOC)

	# look for doxygen, if not present, no need to generate documentation
	FIND_PROGRAM(doxygen_bin "doxygen" PATHS $ENV{OpenViBE_dependencies}/bin NO_DEFAULT_PATH)
	FIND_PROGRAM(doxygen_bin "doxygen" PATHS $ENV{OpenViBE_dependencies}/bin "C:/Program Files/doxygen/bin" "C:/Program Files (x86)/doxygen/bin")

	IF(doxygen_bin)
		debug_message( "  Found doxygen...")

		#Set the project on which the documentation commands will be attached
		SET(PROJECT_NAME_DOC ${PROJECT_NAME}-doc)
		ADD_CUSTOM_TARGET(${PROJECT_NAME_DOC} ALL)
		ADD_DEPENDENCIES(${PROJECT_NAME_DOC} ${PROJECT_NAME})

		# intializes the variable that will be used in the doxyfile for input
		# directories
		# STRING(REPLACE "\\" "/" ov_doxy_input "\"$ENV{OpenViBE_base}/cmake-modules\"")
		STRING(REPLACE "\\" "/" ov_doxy_input "\"${PROJECT_SOURCE_DIR}/src\"")

		# intializes the variable that will contain the list of resource files to
		# copy to the target directory
		SET(resource_files "")
		SET(scenario_files "")

		# for each project, we look at its resources and store them in a list
		# for each project, we look at partial documentation files (.dox-part) and
		# parse them to get |ov[a-zA-Z0-9_]*_begin| or |ov[a-zA-Z0-9_]*_end|
		# tokens. This tokens will later be included in the skeleton doxumentation
		# files (.dox-skeleton)
		SET(current_project_src "${PROJECT_SOURCE_DIR}/src")

		# updates the doxyfile variable for input directories
		SET(ov_doxy_input "${ov_doxy_input} \"${current_project_src}\"")
		debug_message( "    [  OK  ] Candidate directory found ${current_project_src}")

		# looks for resources and stores thm in a list
		FILE(GLOB_RECURSE resource_files_tmp "${current_project_src}/*.png" "${current_project_src}/*.svg" "${current_project_src}/*.css" "${current_project_src}/*.php")
		SET(resource_files ${resource_files} ${resource_files_tmp})

		# looks for scenario files
		FILE(GLOB_RECURSE scenario_files_tmp "${current_project_src}/*.xml")
		SET(scenario_files ${scenario_files} ${scenario_files_tmp})

		# looks for partial hand written documentation
		FILE(GLOB_RECURSE doxs "${current_project_src}/*.dox-part")
		FOREACH(dox ${doxs})
			GET_FILENAME_COMPONENT(dox_filename ${dox} NAME_WE)
			debug_message( "             Documentation part found ${dox}")

			SET(dox_tag_name NOTFOUND)

			# iterates on each line of the file to look after begin/end tags
			# "dox_tag_name" stores the name of the variable
			# to use to configure the skeleton file. It is computed from the
			# begin tag.
			FILE(READ ${dox} dox_lines)
			# replaces empty cariage returns with semi colons to be compliant
			# with CMake lists. note the space before and after the semi
			# colon, this is for CMake not to skip empty lines
			STRING(REPLACE "\n" " ; " dox_lines " ${dox_lines} ")
			FOREACH(dox_line ${dox_lines})
				# this regex removes the spaces we added before the loop
				STRING(REGEX REPLACE "^ (.*) $" "\\1" dox_line ${dox_line})

				# we initialize several variables that will be used in
				# this loop
				SET(dox_line_processed   FALSE)
				SET(dox_tag_begin NOTFOUND)
				SET(dox_tag_end   NOTFOUND)
				SET(dox_tag       NOTFOUND)

				# and look for a new tag in this line
				STRING(REGEX MATCH "\\|[a-zA-Z0-9_]*\\|" dox_tag "${dox_line}")
				IF(dox_tag)
					# a tag is found, so we want to know if it is a
					# OVP_DocBegin* or OVP_DocEnd* tag
					STRING(REGEX MATCH "\\|OVP_DocBegin_[a-zA-Z0-9_]*\\|" dox_tag_begin "${dox_line}")
					STRING(REGEX MATCH "\\|OVP_DocEnd_[a-zA-Z0-9_]*\\|"   dox_tag_end   "${dox_line}")

					# in case we already have something in
					# dox_tag_name, it means that begin tag has
					# already been processed, so either we terminate with end
					# tag, either we continue with come content to add in the
					# variable
					IF(dox_tag_name AND dox_tag_end)
						# in case we find end tag, we just terminate cleaning
						# the tag and what follows. We then terminate and
						# create a new CMake variable with the content of this
						# begin/end tagged things.
						STRING(REGEX REPLACE ".*\\|OVP_DocEnd_([a-zA-Z0-9_]*)\\|.*" "\\1" dox_tag_name_check ${dox_line})
						STRING(REGEX REPLACE   "\\|OVP_DocEnd_([a-zA-Z0-9_]*)\\|.*" "" dox_line "${dox_line}")

						debug_message( "             - Completed tag pair |${dox_tag_name}|")

						SET(dox_tag_name_value "${dox_tag_name_value}\n${dox_line}")
						SET("Doc_${dox_tag_name}_Content" ${dox_tag_name_value})
						SET(dox_tag_name NOTFOUND)
						SET(dox_line_processed TRUE)
					ENDIF(dox_tag_name AND dox_tag_end)

					# in case dox_tag_name is empty, it means
					# that begin tag has not yet been found, so we just look at it
					# or skip to next line
					IF(NOT dox_tag_name AND dox_tag_begin)
						# in case we find begin tag, we just start saving the
						# CMake variable name, and clean the tag and what
						# comes before. We then intialize the content of the
						# begin/end tagged thing with what comes after begin
						# tag.
						STRING(REGEX REPLACE ".*\\|OVP_DocBegin_([a-zA-Z0-9_]*)\\|.*" "\\1" dox_tag_name ${dox_line})
						STRING(REGEX REPLACE ".*\\|OVP_DocBegin_([a-zA-Z0-9_]*)\\|" "" dox_line "${dox_line}")
						SET(dox_tag_name_value "${dox_line}")
						SET(dox_line_processed TRUE)
					ENDIF(NOT dox_tag_name AND dox_tag_begin)

					# in case dox tag is not OVP_DocBegin* or OVP_DocEnd*
					# just print a warning and continue
					IF(NOT dox_line_processed)
						MESSAGE(STATUS "             - Unexpected tag ${dox_tag} will be ignored")
					ENDIF(NOT dox_line_processed)
				ENDIF(dox_tag)

				# in case this line was not processed, either because it does
				# not have any tag, either because the tag was unexpected, we
				# just append the whole line to the content of the current
				# variable
				IF(dox_tag_name AND NOT dox_line_processed)
					# in case we don't find the end tag, just append this
					# new line to the current content
					SET(dox_tag_name_value "${dox_tag_name_value}\n${dox_line}")
				ENDIF(dox_tag_name AND NOT dox_line_processed)
			ENDFOREACH(dox_line)

		ENDFOREACH(dox)

		# now we have stored all the begin/end tagged things in variable, we just
		# have to configure the skeleton configuration files with those variables.
		# note that the skeleon files should be prepared to receive the CMake
		# variables with @CMakeVariableName@ anywhere it is needed.
		#
		# in order to do so, we look after all the (.dox-skeleton) files and call
		# the configure command to build the final documentation (.dox) file.
		FILE(GLOB_RECURSE dox_skeletons "${PROJECT_SOURCE_DIR}/src/*.dox-skeleton")
		FOREACH(dox_skeleton ${dox_skeletons})
			GET_FILENAME_COMPONENT(dox_skeleton_filename ${dox_skeleton} NAME_WE)
			GET_FILENAME_COMPONENT(dox_skeleton_path     ${dox_skeleton} PATH)
			CONFIGURE_FILE(
				"${dox_skeleton}"
				"${dox_skeleton_path}/${dox_skeleton_filename}.dox"
				@ONLY)
			debug_message( "    [  OK  ] Configured skeleton ${dox_skeleton}")
		ENDFOREACH(dox_skeleton)

		# now add post-build commands to copy resources in the target directory
		IF(resource_files)
			debug_message( "  Found resources...")
			FOREACH(current_resource ${resource_files})
				GET_FILENAME_COMPONENT(current_resource_stripped ${current_resource} NAME)
				debug_message( "    [  OK  ] Resource file ${current_resource}")
				ADD_CUSTOM_COMMAND(
					TARGET ${PROJECT_NAME_DOC}
					POST_BUILD
					COMMAND ${CMAKE_COMMAND}
					ARGS -E copy_if_different "${current_resource}" "${PROJECT_SOURCE_DIR}/doc/${PROJECT_NAME}/html/${current_resource_stripped}"
					COMMAND ${CMAKE_COMMAND}
					ARGS -E copy_if_different "${current_resource}" "${PROJECT_SOURCE_DIR}/doc/${PROJECT_NAME}/latex/${current_resource_stripped}"
					COMMENT "      --->  [html/latex] Copying resource file ${current_resource_stripped}..."
				VERBATIM)
			ENDFOREACH(current_resource)
		ENDIF(resource_files)
		# now add post-build commands to copy scenario files in the target directory
		IF(scenario_files)
			debug_message( "  Found scenario files...")
			FOREACH(current_scenario ${scenario_files})
				GET_FILENAME_COMPONENT(current_scenario_stripped ${current_scenario} NAME)
				debug_message( "    [  OK  ] scenario file ${current_scenario}")
				ADD_CUSTOM_COMMAND(
					TARGET ${PROJECT_NAME_DOC}
					POST_BUILD
					COMMAND ${CMAKE_COMMAND}
					ARGS -E copy_if_different "${current_scenario}" "${PROJECT_SOURCE_DIR}/doc/${PROJECT_NAME}/html/scenarios/${current_scenario_stripped}"
					COMMAND ${CMAKE_COMMAND}
					ARGS -E copy_if_different "${current_scenario}" "${PROJECT_SOURCE_DIR}/doc/${PROJECT_NAME}/latex/scenarios/${current_scenario_stripped}"
					COMMENT "      --->  [html/latex] Copying scenario file ${current_scenario_stripped}..."
				VERBATIM)
			ENDFOREACH(current_scenario)
			# Saving all scenarios in a dedicated package
			ADD_CUSTOM_COMMAND(
				TARGET ${PROJECT_NAME_DOC}
				POST_BUILD
				COMMAND ${CMAKE_COMMAND} ARGS -E tar cfz ${PROJECT_NAME_DOC}-scenarios.tar.gz scenarios
				COMMAND ${CMAKE_COMMAND} ARGS -E copy ${PROJECT_NAME_DOC}-scenarios.tar.gz ${PROJECT_SOURCE_DIR}/doc/${PROJECT_NAME}/
				COMMAND ${CMAKE_COMMAND} ARGS -E remove ${PROJECT_NAME_DOC}-scenarios.tar.gz
				WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/doc/${PROJECT_NAME}/html"
				COMMENT "      --->  Packaging scenario files..."
			VERBATIM)
		ENDIF(scenario_files)

		# the final doxyfile filename is generated, platform compliantly
		SET(ov_doxy_final "${PROJECT_SOURCE_DIR}/src/doc/doxyfile")
		IF(WIN32)
			STRING(REPLACE "/" "\\" ov_doxy_final ${ov_doxy_final})
		ENDIF(WIN32)

		# these lines configure the variables used to configure the doxyfile
		SET(ov_doxy_strip_from_path ${ov_doxy_input})
		SET(ov_doxy_version ${PROJECT_VERSION})
		SET(ov_doxy_output_directory ../doc/${PROJECT_NAME})
		SET(ov_doxy_project_name ${PROJECT_NAME}-documentation)

		SET(ov_doxy_header_project_name ${PROJECT_NAME}-documentation)
		SET(ov_doxy_header_version ${PROJECT_VERSION})
		SET(ov_doxy_header_branch ${PROJECT_BRANCH})
		SET(ov_doxy_header_hash ${PROJECT_COMMITHASH})
		IF(PROJECT_PRODUCT_NAME)
			SET(ov_doxy_header_product_name ${PROJECT_PRODUCT_NAME})
		ELSE(PROJECT_PRODUCT_NAME)
			SET(ov_doxy_header_product_name ${PROJECT_NAME})
		ENDIF(PROJECT_PRODUCT_NAME)

		debug_message( "  Looking for additional documentation supports...")
		# configure doxyfile to generate a chm if available
		FIND_PROGRAM(HHC_BIN "hhc" PATHS "C:\Program Files\HTML Help Workshop" "C:\Program Files (x86)\HTML Help Workshop" NO_DEFAULT_PATH)
		IF(HHC_BIN)
			debug_message( "    [  OK  ] Found HHC, Windows help (.chm) file will be produced.")
			SET(ov_doxy_generate_chm YES)
			SET(ov_doxy_hhc_location "\"${HHC_BIN}\"")
			SET(ov_doxy_chm_file "\"..\\${PROJECT_NAME}-doc.chm\"")
		ELSE(HHC_BIN)
			MESSAGE(WARNING "    [FAILED] HHC not found, Windows help (.chm) file will not be produced.")
			SET(ov_doxy_generate_chm NO)
		ENDIF(HHC_BIN)

		# configure doxyfile to generate a latex->PDF file if available
		FIND_PROGRAM(LATEX_BIN "latex")
		IF(LATEX_BIN)
			debug_message( "    [  OK  ] Found latex, a PDF file will be produced.")
			SET(ov_doxy_generate_latex YES)
		ELSE(LATEX_BIN)
			MESSAGE(WARNING "    [FAILED] Latex not found, PDF file will not be produced.")
			SET(ov_doxy_generate_latex NO)
		ENDIF(LATEX_BIN)

		# then the doxyfile is configured
		CONFIGURE_FILE(
			src/doc/doxyfile-skeleton
			${ov_doxy_final}
			@ONLY)
		# the latex header
		CONFIGURE_FILE(
			src/doc/header.tex-skeleton
			${PROJECT_SOURCE_DIR}/src/doc/header.tex
			@ONLY)
		# and the html header
		CONFIGURE_FILE(
			src/doc/header.html-skeleton
			${PROJECT_SOURCE_DIR}/src/doc/header.html
			@ONLY)

		# and a post-build command is added in order to run doxygen
		ADD_CUSTOM_COMMAND(
			TARGET ${PROJECT_NAME_DOC}
			POST_BUILD
			# COMMAND ${CMAKE_COMMAND} ARGS -E make_directory ${PROJECT_NAME}/chm
			COMMAND "${doxygen_bin}" -u "${ov_doxy_final}" && "${doxygen_bin}" "${ov_doxy_final}"
			COMMAND ${CMAKE_COMMAND} ARGS -E tar cfz ${PROJECT_NAME}/${PROJECT_NAME_DOC}.tar.gz ${PROJECT_NAME}/html
			WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/doc"
			COMMENT "      --->   Running doxygen in [${PROJECT_SOURCE_DIR}/doc]..."
			VERBATIM)

		# make the pdf from latex output
		# latex2pdf.log will be actually removed only if make.bat returns successfully
		# therefore any error will be readable in the log file.
		IF(LATEX_BIN)
			IF(WIN32)
				ADD_CUSTOM_COMMAND(
					TARGET ${PROJECT_NAME_DOC}
					POST_BUILD
					COMMAND "${PROJECT_SOURCE_DIR}/doc/${PROJECT_NAME}/latex/make.bat > ../latex2pdf.log"
					COMMAND ${CMAKE_COMMAND} ARGS -E copy refman.pdf ../${PROJECT_NAME_DOC}.pdf
					COMMAND ${CMAKE_COMMAND} ARGS -E remove ../latex2pdf.log
					WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/doc/${PROJECT_NAME}/latex"
					COMMENT "      --->   Latex 2 PDF..."
					VERBATIM)
			ELSE(WIN32)
				ADD_CUSTOM_COMMAND(
					TARGET ${PROJECT_NAME_DOC}
					POST_BUILD
					COMMAND make -C "${PROJECT_SOURCE_DIR}/doc/${PROJECT_NAME}/latex" > ../latex2pdf.log
					COMMAND ${CMAKE_COMMAND} ARGS -E copy refman.pdf ../${PROJECT_NAME_DOC}.pdf
					COMMAND ${CMAKE_COMMAND} ARGS -E remove ../latex2pdf.log
					WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/doc/${PROJECT_NAME}/latex"
					COMMENT "      --->   Latex 2 PDF..."
					VERBATIM)
			ENDIF(WIN32)
		ENDIF(LATEX_BIN)

		# CLEANUP
		ADD_CUSTOM_COMMAND(
			TARGET ${PROJECT_NAME_DOC}
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} ARGS -E remove_directory latex
			COMMAND ${CMAKE_COMMAND} ARGS -E remove_directory html
			COMMAND ${CMAKE_COMMAND} ARGS -E remove ../doxygen.log
			WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/doc/${PROJECT_NAME}"
			COMMENT "      --->   Cleaning up..."
			VERBATIM)

	ELSE(doxygen_bin)

		MESSAGE(WARNING "  FAILED to find doxygen...")

	ENDIF(doxygen_bin)

ENDIF(MENSIA_BUILD_DOC)
