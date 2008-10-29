
project(kjsembed-kjscmd)

include_directories(
    ${KDE4_KJS_INCLUDES}
    ${CMAKE_SOURCE_DIR}/kjsembed
    ${KDE4_KDECORE_INCLUDES}
    ${KDE4_KDEUI_INCLUDES}
)


########### next target ###############

set(kjscmd_SRCS kjscmd.cpp )


qt4_add_resources(kjscmd_SRCS kjscmd.qrc )

if (NOT DEFINED QT_ONLY)
   set(KJSLIBNAME kjs)
   set(KJSEMBEDLIBNAME kjsembed)
	 set(KJSCMDNAME kjscmd)
else (NOT DEFINED QT_ONLY)
	 set(KJSCMDNAME qjscmd)
   if (NOT QTONLY_WEBKIT)
      set(KJSLIBNAME qkjs)
      set(KJSEMBEDLIBNAME qkjsembed)
   else (NOT QTONLY_WEBKIT)
	    set(KJSLIBNAME ${WEBKIT_KJS_LIBRARY})
      set(KJSEMBEDLIBNAME qwkjsembed)
   endif (NOT QTONLY_WEBKIT)
endif (NOT DEFINED QT_ONLY)


kde4_add_executable(${KJSCMDNAME} ${kjscmd_SRCS})

target_link_libraries(${KJSCMDNAME} ${KDE4_KDEUI_LIBS} ${KDE4_KDECORE_LIBS} ${KJSEMBEDLIBNAME} kjs )

install(TARGETS ${KJSCMDNAME} ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############





