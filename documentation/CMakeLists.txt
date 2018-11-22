# Sphinx Documentation

FILE(MAKE_DIRECTORY "${DIST_DOCTMP}/sphinx")

# Collect all sphinx documentation files from all projects
set(INCLUDED_OV_SDK_COMPONENTS DEPENDENCIES DEVELOPER_TOOLS)
include("AddOpenViBESDKComponents")

file(COPY "${OPENVIBE_SDK_PATH}/doc-tmp/" DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/doc-tmp)

file(GLOB SPHINX_DOC_DIRS LIST_DIRECTORIES true LIST_FILES false
	"${PROJECT_SOURCE_DIR}/plugins/visualization/*"
	"${PROJECT_SOURCE_DIR}/plugins/*"
	)
foreach(DOCDIR ${SPHINX_DOC_DIRS})
	if(IS_DIRECTORY "${DOCDIR}/doc/boxes")
		file(COPY "${DOCDIR}/doc/boxes" DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/doc-tmp/sphinx/source)
	endif()
endforeach()

file(COPY sphinx/source DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/doc-tmp/sphinx)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc-tmp/ DESTINATION ${DIST_DOCTMP})
