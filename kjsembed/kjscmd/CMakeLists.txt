
project(kjsembed-kjscmd)


########### next target ###############

set(kjscmd_SRCS kjscmd.cpp )


qt5_add_resources(kjscmd_SRCS kjscmd.qrc )

if (NOT DEFINED QT_ONLY)
   set(KJSLIBNAME KJS)
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


add_executable(${KJSCMDNAME} ${kjscmd_SRCS})

target_link_libraries(${KJSCMDNAME} Qt5::Widgets ${KCoreAddons_LIBRARIES} ${KJSEMBEDLIBNAME} KJS KI18n KDE4Support)

install(TARGETS ${KJSCMDNAME} ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############





