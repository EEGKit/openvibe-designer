function(debug_message)
	if(${Flag_VerboseOutput})
		message(STATUS "${ARGV}")
	endif()
endfunction()

# SET(OV_COMPILE_TESTS "true")
FUNCTION(SET_BUILD_PLATFORM)
	IF("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
		ADD_DEFINITIONS(-DTARGET_ARCHITECTURE_x64)
	ELSEIF("${CMAKE_SIZEOF_VOID_P}" EQUAL "4")
		ADD_DEFINITIONS(-DTARGET_ARCHITECTURE_i386)
	ELSE()
		ADD_DEFINITIONS(-DTARGET_ARCHITECTURE_Unknown)
	ENDIF()

	IF(WIN32)
		ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
		ADD_DEFINITIONS(-DTARGET_OS_Windows)
		ADD_DEFINITIONS(-DTARGET_COMPILER_VisualStudio)
	ELSEIF(APPLE)
		ADD_DEFINITIONS(-fnon-call-exceptions)
		ADD_DEFINITIONS(-DTARGET_OS_MacOS)
		# ADD_DEFINITIONS(-DTARGET_ARCHITECTURE_x64)
		ADD_DEFINITIONS(-DTARGET_COMPILER_LLVM)
	ELSEIF(UNIX)
		# ADD_DEFINITIONS(-fvisibility=hidden) # This flag should be present... man gcc
		ADD_DEFINITIONS(-fnon-call-exceptions)
		ADD_DEFINITIONS(-DTARGET_OS_Linux)
		ADD_DEFINITIONS(-DTARGET_COMPILER_GCC)
	ENDIF()

ENDFUNCTION()

# Set version based on git tag.
#  If current commit is tagged, use the tag as it is, and add build number based on content of .build file, written by Jenkins
#  Else use last tag major and minor number and set patch number to 99
#
# This function should remain generic to be usable in every projects.
function(set_version)  
	execute_process(COMMAND git describe
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		OUTPUT_VARIABLE  PROJECT_VERSION
		ERROR_VARIABLE  ERROR)
	
	if(ERROR)
		message(WARNING "No tags found, set version to 0.0.0")
		set(PROJECT_VERSION "0.0.0")
	endif()

	# if current commit is not tagged result is formed as: "major.minor.patch-number of commits since last tag-hash"
	string(STRIP ${PROJECT_VERSION} PROJECT_VERSION)
	string(REPLACE "-" ";" version_list ${PROJECT_VERSION})
	list(LENGTH version_list version_list_length)
	if(${version_list_length} EQUAL 3) # if result is formed as "major.minor.patch-number of commits since last tag-hash" set patch as 99
		list(GET version_list 0 PROJECT_VERSION)
		set(PROJECT_VERSION_PATCH 99)
	endif()

	string(REPLACE "." ";" version_list ${PROJECT_VERSION})
	list(GET version_list 0 PROJECT_VERSION_MAJOR)
	list(GET version_list 1 PROJECT_VERSION_MINOR)
	if(NOT PROJECT_VERSION_PATCH)
		list(GET version_list 2 PROJECT_VERSION_PATCH)
	endif()

	# These versions are used by the subprojects by default.
	# If you wish to maintain specific version numbers for a subproject, please do so in the projects CMakeLists.txt
	if(EXISTS ${CMAKE_SOURCE_DIR}/.build)
		file(READ ${CMAKE_SOURCE_DIR}/.build PROJECT_VERSION_BUILD)
		string(STRIP ${PROJECT_VERSION_BUILD} PROJECT_VERSION_BUILD)
	else()
		set(PROJECT_VERSION_BUILD 0)
	endif()

	set(PROJECT_VERSION_MAJOR ${PROJECT_VERSION_MAJOR} PARENT_SCOPE)
	set(PROJECT_VERSION_MINOR ${PROJECT_VERSION_MINOR} PARENT_SCOPE)
	set(PROJECT_VERSION_PATCH ${PROJECT_VERSION_PATCH} PARENT_SCOPE)
	set(PROJECT_VERSION_BUILD ${PROJECT_VERSION_BUILD} PARENT_SCOPE)
	set(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH} PARENT_SCOPE)
endfunction()
