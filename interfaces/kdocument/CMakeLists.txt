include_directories( ${KDE4_KPARTS_INCLUDES} )

set(kdocument_LIB_SRCS kdocument.cpp )

kde4_automoc(${kdocument_LIB_SRCS})
kde4_add_library(kdocument SHARED ${kdocument_LIB_SRCS})
target_link_libraries(kdocument  ${KDE4_KPARTS_LIBS} )
set_target_properties(kdocument PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install(TARGETS kdocument  DESTINATION ${LIB_INSTALL_DIR} )
install( FILES factory.h document.h view.h  DESTINATION  ${INCLUDE_INSTALL_DIR}/kdocument )
