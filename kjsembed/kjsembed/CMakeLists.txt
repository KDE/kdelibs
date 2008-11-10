project(kjsembed-kjsembed)

if (NOT QTONLY_WEBKIT)
   include_directories(
       ${CMAKE_SOURCE_DIR}
       ${CMAKE_SOURCE_DIR}/kjsembed 
       ${KDE4_KJS_INCLUDES}
       ${KDE4_KDECORE_INCLUDES}
   )
else (NOT QTONLY_WEBKIT)
   include_directories(
       $(QTONLY_WEBKIT_DIR)/JavaScriptCore/kjs
       $(QTONLY_WEBKIT_DIR)/JavaScriptCore
       ${CMAKE_SOURCE_DIR}
       ${CMAKE_SOURCE_DIR}/kjsembed 
       ${KDE4_KDECORE_INCLUDES}
   )
endif (NOT QTONLY_WEBKIT)


########### next target ###############

set(kjsembed_LIB_SRCS
   kjseglobal.cpp
   binding_support.cpp
   static_binding.cpp
   variant_binding.cpp
   object_binding.cpp
   builtins.cpp
   fileio.cpp
   jseventmapper.cpp
   eventproxy.cpp
   slotproxy.cpp
   jseventutils.cpp
   qobject_binding.cpp
   kjsembed.cpp
   value_binding.cpp
   iosupport.cpp
   qwidget_binding.cpp
   qaction_binding.cpp
   qlayout_binding.cpp
   qpainter_binding.cpp
   settings.cpp
   svg_binding.cpp
   filedialog_binding.cpp
   application.cpp
   color.cpp dom.cpp font.cpp image.cpp pen.cpp
   pixmap.cpp point.cpp rect.cpp size.cpp url.cpp
   brush.cpp
   QBrush_bind.cpp
   quiloader_binding.cpp
   )

if (NOT DEFINED QT_ONLY)
   set(KJSLIBNAME kjs)
   set(KJSEMBEDLIBNAME kjsembed)
else (NOT DEFINED QT_ONLY)
   if (NOT QTONLY_WEBKIT)
      set(KJSLIBNAME qkjs)
      set(KJSEMBEDLIBNAME qkjsembed)
   else (NOT QTONLY_WEBKIT)
      set(KJSLIBNAME "${WEBKIT_KJS_LIBRARY}")
      set(KJSEMBEDLIBNAME qwkjsembed)
   endif (NOT QTONLY_WEBKIT)
endif (NOT DEFINED QT_ONLY)

kde4_add_library(${KJSEMBEDLIBNAME} SHARED ${kjsembed_LIB_SRCS})

target_link_libraries(${KJSEMBEDLIBNAME} ${KDE4_KDECORE_LIBS} ${QT_QTUITOOLS_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTSVG_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTCORE_LIBRARY} ${KJSLIBNAME} )

set_target_properties(${KJSEMBEDLIBNAME} PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS ${KJSEMBEDLIBNAME} EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})







