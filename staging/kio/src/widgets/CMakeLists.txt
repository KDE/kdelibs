project(KIOWidgets)

#include (ConfigureChecks.cmake)

#configure_file(config-kiowidgets.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kiowidgets.h )

set(kiowidgets_SRCS
  fileundomanager.cpp
  kurlpixmapprovider.cpp
  pixmaploader.cpp
  thumbsequencecreator.cpp
  thumbcreator.cpp
  kshellcompletion.cpp
  kurlcompletion.cpp
  kurifilter.cpp
  previewjob.cpp
  renamedialog.cpp
  ksslcertificatebox.cpp
  kdynamicjobtracker.cpp
  ksslinfodialog.cpp
  joburlcache.cpp
  skipdialog.cpp
  jobuidelegate.cpp
  kdirlister.cpp
  kdirmodel.cpp
)

qt5_add_dbus_adaptor(kiowidgets_SRCS org.kde.kio.FileUndoManager.xml fileundomanager_p.h KIO::FileUndoManagerPrivate fileundomanager_adaptor KIOFileUndoManagerAdaptor)

qt5_add_dbus_interface(kiowidgets_SRCS org.kde.kuiserver.xml kuiserver_interface)

qt5_wrap_ui(kiowidgets_SRCS
  certificateparty.ui
  sslinfo.ui
)

add_library(KIOWidgets ${kiowidgets_SRCS})
generate_export_header(KIOWidgets)

target_include_directories(KIOWidgets PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>" # kio_version.h
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../include>" # kio/global.h
)

target_link_libraries(KIOWidgets
 KIOCore
 KJobWidgets
 KIconThemes   # KIconLoader
 KCompletion   # KUrlCompletion uses KCompletion
 Qt5::Widgets
 Qt5::Network  # SSL
 #${KConfig_CORE_LIBRARY}
 #${KCoreAddons_LIBRARIES}
 #ki18n
 #kservice
 #Qt5::DBus
 #Qt5::Concurrent
)

if(ACL_FOUND)
   target_link_libraries(KIOWidgets ${ACL_LIBS})
endif(ACL_FOUND)

set_target_properties(KIOWidgets PROPERTIES VERSION ${ECM_VERSION_STRING}
                                          SOVERSION ${ECM_SOVERSION}
)

install(TARGETS KIOWidgets EXPORT kdelibsLibraryTargets ${ECM_TARGET_DEFAULT_ARGS})

install(FILES
   org.kde.kio.FileUndoManager.xml
   DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

install( FILES
  kurlpixmapprovider.h
  pixmaploader.h
  thumbsequencecreator.h
  thumbcreator.h
  kshellcompletion.h
  kurlcompletion.h
  kurifilter.h
  previewjob.h
  renamedialog.h
  ksslcertificatebox.h
  ksslinfodialog.h
  skipdialog.h
  jobuidelegate.h
  fileundomanager.h
  kdirlister.h
  kdirmodel.h

   ${CMAKE_CURRENT_BINARY_DIR}/kiowidgets_export.h
   DESTINATION ${INCLUDE_INSTALL_DIR}/kio COMPONENT Devel)

