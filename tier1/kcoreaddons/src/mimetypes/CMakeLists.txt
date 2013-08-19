# for KDE frameworks 5 we require at least version 0.70
find_package(SharedMimeInfo 0.70)
set_package_properties(SharedMimeInfo PROPERTIES DESCRIPTION "Shared Mime Info"
                       URL "http://freedesktop.org/wiki/Software/shared-mime-info"
                       TYPE REQUIRED
                       PURPOSE "Allows KDE applications to determine file types"
                      )

########### install files ###############
install(FILES kde.xml DESTINATION ${XDG_MIME_INSTALL_DIR})
update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})
