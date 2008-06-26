project(kfile)

include_directories(
  ${KDE4_KIO_INCLUDES}
  ${CMAKE_SOURCE_DIR}/solid
  ${CMAKE_BINARY_DIR}/solid
)

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
  kfileplacesselector.cpp
  kfileplacesview.cpp
  kfileplaceeditdialog.cpp
  kprotocolcombo.cpp
  kurlbutton.cpp
  kurldropdownbutton.cpp
  kurlnavigatorbutton.cpp
  kurlnavigator.cpp
  kurltogglebutton.cpp
)

kde4_add_library(kfile SHARED ${kfile_LIB_SRCS})
target_link_libraries(kfile
   ${KDE4_KIO_LIBS}
   ${ZLIB_LIBRARY}
   ${STRIGI_LIBRARIES}
   ${KDE4_SOLID_LIBS}
)

set_target_properties(kfile PROPERTIES VERSION ${GENERIC_LIB_VERSION} 
                                       SOVERSION ${GENERIC_LIB_SOVERSION} )

install(TARGETS kfile ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
  kdiroperator.h
  kdirselectdialog.h
  kdirsortfilterproxymodel.h
  kfilefiltercombo.h
  kfileplacesmodel.h
  kfileplacesview.h
  kfiletreeview.h
  kfilewidget.h
  kurlnavigator.h
  kfile_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )

############ module used by KFileDialog ##########
set(kfilemodule_SRCS
  kfilemodule.cpp
)
kde4_add_plugin(kfilemodule WITH_PREFIX ${kfilemodule_SRCS})
target_link_libraries(kfilemodule ${KDE4_KFILE_LIBS})
install(TARGETS kfilemodule DESTINATION ${PLUGIN_INSTALL_DIR})
