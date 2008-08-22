# for KDE 4.1 we require at least version 0.20
set(SHARED_MIME_INFO_MINIMUM_VERSION "0.19")
find_package(SharedMimeInfo REQUIRED)

########### install files ###############
install(FILES kde.xml DESTINATION ${XDG_MIME_INSTALL_DIR})
# without this workaround, update_xdg_mimetypes tries to install to share/mime, which
# results in unreadable messageboxes when installing kde.xml
if(WIN32)
  update_xdg_mimetypes(${CMAKE_INSTALL_PREFIX}/${XDG_MIME_INSTALL_DIR})
else(WIN32)
  update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})
endif(WIN32)
