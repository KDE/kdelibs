
project(kdefx)

configure_file(config-processor.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-processor.h )

include(kdefx.cmake)

include_directories( ${QT_INCLUDES} )
if(WIN32)
  include_directories( ${KDEWIN32_INCLUDES} )
endif(WIN32)

########### next target ###############

set(kdefx_LIB_SRCS
kpixmapsplitter.cpp
kpixmapeffect.cpp
kimageeffect.cpp
kdrawutil.cpp
kstyle.cpp
kcpuinfo.cpp
)

remove_definitions(-DQT3_SUPPORT_WARNINGS -DQT3_SUPPORT)

kde4_automoc(${kdefx_LIB_SRCS})

kde4_add_library(kdefx SHARED ${kdefx_LIB_SRCS})

target_link_libraries(kdefx  ${QT_QTCORE_LIBRARY}  ${QT_QTGUI_LIBRARY} )

set_target_properties(kdefx PROPERTIES VERSION 5.0.0 SOVERSION 5 )
install_targets(${LIB_INSTALL_DIR} kdefx )


########### install files ###############
install_files( /include FILES kpixmapsplitter.h  kpixmapeffect.h kimageeffect.h kdrawutil.h kstyle.h kcpuinfo.h )





