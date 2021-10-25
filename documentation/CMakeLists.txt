# Sphinx Documentation

install(DIRECTORY DESTINATION "${DIST_DOCTMP}/sphinx")

#add_custom_target(copy_sdk_documentation ALL COMMAND ${CMAKE_COMMAND} -E copy "${OPENVIBE_SDK_PATH}/doc-tmp/" "${CMAKE_CURRENT_BINARY_DIR}/doc-tmp")

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
