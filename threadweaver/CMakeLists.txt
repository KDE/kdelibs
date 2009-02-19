project(threadweaver)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

add_subdirectory(Weaver)
# disabled for default kdelibs build:
# add_subdirectory(WeaverGui)
# add_subdirectory(Experimental)
 add_subdirectory(Tests)
# add_subdirectory(Examples)

install(FILES threadweaver_export.h DESTINATION ${INCLUDE_INSTALL_DIR}/threadweaver COMPONENT Devel)

