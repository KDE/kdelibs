project(KIOWidgets)

#include (ConfigureChecks.cmake)

#configure_file(config-kiowidgets.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kiowidgets.h )

set(kiowidgets_SRCS
  accessmanager.cpp
  accessmanagerreply_p.cpp
  fileundomanager.cpp
  pastedialog.cpp
  paste.cpp
  clipboardupdater.cpp
  kabstractfileitemactionplugin.cpp
  kbuildsycocaprogressdialog.cpp
  sslui.cpp
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
  LINK_PUBLIC
    KIOCore
    KJobWidgets
    Qt5::Widgets
    Qt5::Network  # SSL
    KCompletion   # KUrlCompletion uses KCompletion
  LINK_PRIVATE
    KIconThemes   # KIconLoader
)

if(ACL_FOUND)
   target_link_libraries(KIOWidgets ${ACL_LIBS})
endif(ACL_FOUND)

set_target_properties(KIOWidgets PROPERTIES VERSION ${KIO_VERSION_STRING}
                                          SOVERSION ${KIO_SOVERSION}
)

install(TARGETS KIOWidgets EXPORT KIOTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
   org.kde.kio.FileUndoManager.xml
   DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

install( FILES
  pastedialog.h
  paste.h
  kabstractfileitemactionplugin.h
  kbuildsycocaprogressdialog.h
 accessmanager.h
  sslui.h
  pixmaploader.h
  thumbsequencecreator.h
  thumbcreator.h
  previewjob.h
  renamedialog.h
  skipdialog.h
  jobuidelegate.h
  fileundomanager.h

   ${CMAKE_CURRENT_BINARY_DIR}/kiowidgets_export.h
   DESTINATION ${INCLUDE_INSTALL_DIR}/kio COMPONENT Devel)

install( FILES
  pastedialog.h
  paste.h
  kabstractfileitemactionplugin.h
  kbuildsycocaprogressdialog.h
  kurlpixmapprovider.h
  ksslcertificatebox.h
  ksslinfodialog.h
  kdirlister.h
  kdirmodel.h
  kshellcompletion.h
  kurlcompletion.h
  kurifilter.h
   DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

