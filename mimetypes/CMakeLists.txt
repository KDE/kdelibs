# for KDE 4.3 we require at least version 0.30
set(SHARED_MIME_INFO_MINIMUM_VERSION "0.30")
find_package(SharedMimeInfo)
macro_log_feature(SHARED_MIME_INFO_FOUND "shared-mime-info" "The core MIME database used to represent types of files" "http://freedesktop.org/wiki/Software/shared-mime-info" TRUE "${SHARED_MIME_INFO_MINIMUM_VERSION}" "Required by KDE applications to help determine file types")

########### install files ###############
install(FILES kde.xml DESTINATION ${XDG_MIME_INSTALL_DIR})
update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})
