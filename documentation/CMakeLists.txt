# Sphinx Documentation

FILE(MAKE_DIRECTORY "${DIST_DOCTMP}/sphinx")

# Collect all sphinx documentation files from all projects
set(INCLUDED_OV_SDK_COMPONENTS DEPENDENCIES DEVELOPER_TOOLS)
include("AddOpenViBESDKComponents")

install(DIRECTORY "${OPENVIBE_SDK_PATH}/doc-tmp/" DESTINATION ${DIST_DOCTMP})

file(GLOB SPHINX_DOC_DIRS LIST_DIRECTORIES true LIST_FILES false
	"${PROJECT_SOURCE_DIR}/plugins/visualization/*"
	"${PROJECT_SOURCE_DIR}/plugins/*"
	)
foreach(DOCDIR ${SPHINX_DOC_DIRS})
	if(IS_DIRECTORY "${DOCDIR}/doc/boxes")
		INSTALL(DIRECTORY "${DOCDIR}/doc/boxes" DESTINATION ${DIST_DOCTMP}/sphinx/source)
	endif()
endforeach()

install(DIRECTORY sphinx/source DESTINATION ${DIST_DOCTMP}/sphinx)
