
project(kdefx)

configure_file(config-processor.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-processor.h )

include(kdefx.cmake)

include_directories( ${QT_INCLUDES} ${_KDE4_PLATFORM_INCLUDE_DIRS} )

########### next target ###############

set(kdefx_LIB_SRCS
kpixmapsplitter.cpp
kpixmapeffect.cpp
kimageeffect.cpp
kcpuinfo.cpp
)


kde4_add_library(kdefx SHARED ${kdefx_LIB_SRCS})

target_link_libraries(kdefx ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} )

set_target_properties(kdefx PROPERTIES VERSION ${KDE_NON_GENERIC_LIB_VERSION} SOVERSION ${KDE_NON_GENERIC_LIB_SOVERSION} )
install(TARGETS kdefx
             RUNTIME DESTINATION ${BIN_INSTALL_DIR}
             LIBRARY DESTINATION ${LIB_INSTALL_DIR}
             ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
)

########### install files ###############
install( FILES kdefx_export.h kpixmapsplitter.h  kpixmapeffect.h kimageeffect.h  DESTINATION  ${INCLUDE_INSTALL_DIR} )






