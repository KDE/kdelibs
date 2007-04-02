project(kfile)
add_definitions(-DQT3_SUPPORT_WARNINGS -DQT3_SUPPORT)

include_directories(
  ${KDE4_KIO_INCLUDES}
  ${CMAKE_SOURCE_DIR}/kde3support/kdeui #for k3iconview, k3listview
)

add_subdirectory(tests)

set(kfile_LIB_SRCS
  k3filedetailview.cpp
  k3fileiconview.cpp
  k3filetreeview.cpp
  k3filetreeviewitem.cpp
  kcombiview.cpp
  kdiroperator.cpp
  kdirselectdialog.cpp
  kfilebookmarkhandler.cpp
  kfilefiltercombo.cpp
  kfilepreview.cpp
  kfilespeedbar.cpp
  kfiletreebranch.cpp
  kfileview.cpp
  kfilewidget.cpp
  kurlbar.cpp
)

kde4_automoc(${kfile_LIB_SRCS})
kde4_add_library(kfile SHARED ${kfile_LIB_SRCS})
target_link_libraries(kfile ${KDE4_KIO_LIBS} ${KDE4_KDE3SUPPORT_LIBRARY} ${ZLIB_LIBRARY} ${STRIGI_LIBRARIES} ${KDE4_SOLID_LIBS})
set_target_properties(kfile PROPERTIES VERSION 5.0.0 SOVERSION 5 )


install( FILES
  k3filedetailview.h
  k3fileiconview.h
  k3filetreeview.h
  k3filetreeviewitem.h
  kcombiview.h
  kdiroperator.h
  kdirselectdialog.h
  kfilefiltercombo.h
  kfilepreview.h
  kfiletreebranch.h
  kfileview.h
  kfilewidget.h
  # kurlbar.h
  kfile_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR})

############ module used by KFileDialog ##########
set(kfilemodule_SRCS
  kfilemodule.cpp
)
kde4_automoc(${kfilemodule_SRCS})
kde4_add_plugin(kfilemodule WITH_PREFIX ${kfilemodule_SRCS})
target_link_libraries(kfilemodule ${KDE4_KFILE_LIBS})
install(TARGETS kfilemodule DESTINATION ${PLUGIN_INSTALL_DIR})
