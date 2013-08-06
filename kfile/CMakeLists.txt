project(kfile)

configure_file(config-kfile.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kfile.h )
macro_bool_to_01(X11_Xrender_FOUND HAVE_XRENDER)

if(Soprano_FOUND)
include_directories(${SOPRANO_INCLUDE_DIR})
endif(Soprano_FOUND)

add_subdirectory(tests)

set(kfile_LIB_SRCS
  kdiroperator.cpp
  kdiroperatordetailview.cpp
  kdirselectdialog.cpp
  kdirsortfilterproxymodel.cpp #used in combination with kio/kio/kdirmodel.cpp
  kfilebookmarkhandler.cpp
  kfilefiltercombo.cpp
  kfiletreeview.cpp
  kfilewidget.cpp
  kfileplacesitem.cpp
  kfileplacesmodel.cpp
  kfileplacessharedbookmarks.cpp
  kfileplacesview.cpp
  kfileplaceeditdialog.cpp
  kfilepreviewgenerator.cpp
  knameandurlinputdialog.cpp
  knewfilemenu.cpp
  kurlnavigatordropdownbutton.cpp
  kurlnavigatorbuttonbase.cpp
  kurlnavigatorbutton.cpp
  kurlnavigatorplacesselector.cpp
  kurlnavigatorprotocolcombo.cpp
  kurlnavigatortogglebutton.cpp
  kurlnavigator.cpp
  kurlnavigatormenu.cpp
)

kde4_add_library(kfile ${LIBRARY_TYPE} ${kfile_LIB_SRCS})
generate_export_header(kfile)
target_link_libraries(kfile
   ${KDE4_KIO_LIBS}
   ${ZLIB_LIBRARY}
   ${Solid_LIBRARIES}
   ${X11_LIBRARIES}
   ${KCoreAddons_LIBRARIES}
   ${KWidgetsAddons_LIBRARIES}
   KI18n
)

if(X11_Xrender_FOUND)
  target_link_libraries(kfile ${X11_Xrender_LIB} )
endif(X11_Xrender_FOUND)

target_link_libraries(kfile LINK_INTERFACE_LIBRARIES kio kdecore Qt5::Widgets ${KCoreAddons_LIBRARIES} KCompletion)

set_target_properties(kfile PROPERTIES VERSION ${GENERIC_LIB_VERSION}
                                       SOVERSION ${GENERIC_LIB_SOVERSION}
                     )

install(TARGETS kfile EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
  kdiroperator.h
  kdirselectdialog.h
  kdirsortfilterproxymodel.h
  kfilefiltercombo.h
  kfileplacesmodel.h
  kfileplacesview.h
  kfilepreviewgenerator.h
  kfiletreeview.h
  kfilewidget.h
  kurlnavigator.h
  knewfilemenu.h
  knameandurlinputdialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/kfile_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )

############ module used by KFileDialog ##########
set(kfilemodule_SRCS
  kfilemodule.cpp
)
kde4_add_plugin(kfilemodule ${kfilemodule_SRCS})
target_link_libraries(kfilemodule ${KDE4_KFILE_LIBS} ${KDE4_KIO_LIBS} ${KCoreAddons_LIBRARIES} )
install(TARGETS kfilemodule DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kfilemodule.desktop DESTINATION ${SERVICES_INSTALL_DIR})
