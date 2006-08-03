
project(kjsembed-kjsembed)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/kjsembed
    ${KDE4_KDECORE_INCLUDES}
)


########### next target ###############

set(kjsembed_LIB_SRCS
   global.cpp
   binding_support.cpp
   static_binding.cpp
   value_binding.cpp
   object_binding.cpp
   builtins.cpp
   fileio.cpp
   jseventmapper.cpp
   eventproxy.cpp
   slotproxy.cpp
   jseventutils.cpp
   qobject_binding.cpp
   kjsembed.cpp
   scalar_binding.cpp
   iosupport.cpp
   qwidget_binding.cpp
   qformbuilder_binding.cpp
   qpainter_binding.cpp
   settings.cpp
   svg_binding.cpp
   filedialog_binding.cpp
   brush.cpp color.cpp dom.cpp font.cpp image.cpp pen.cpp
   pixmap.cpp point.cpp rect.cpp size.cpp url.cpp
   bind_qlcdnumber.cpp bind_qtimer.cpp
   )

kde4_automoc(${kjsembed_LIB_SRCS})

kde4_add_library(kjsembed SHARED ${kjsembed_LIB_SRCS})

target_link_libraries(kjsembed ${KDE4_KDECORE_LIBS} ${QT_QTUITOOLS_LIBRARY} ${QT_QTSVG_LIBRARY} ${QT_QTXML_LIBRARY} kjs )
if(UNIX OR MINGW)
  # this is maybe a work-around for a Qt/Windows bug
  target_link_libraries(kjsembed ${QT_QTDESIGNER_LIBRARY})
endif(UNIX OR MINGW)

set_target_properties(kjsembed PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kjsembed  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install( FILES global.h binding_support.h static_binding.h value_binding.h                            object_binding.h builtins.h fileio.h jseventmapper.h                            eventproxy.h slotproxy.h jseventutils.h qobject_binding.h                            kjsembed.h scalar_binding.h iosupport.h qwidget_binding.h                            qformbuilder_binding.h qpainter_binding.h                            settings.h svg_binding.h filedialog_binding.h                            brush.h color.h dom.h font.h image.h pen.h                            pixmap.h point.h rect.h size.h url.h pointer.h bind_qlcdnumber.h bind_qtimer.h  DESTINATION  ${INCLUDE_INSTALL_DIR}/kjsembed )




