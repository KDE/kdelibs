project(KIOFileWidgets)

# configure_file(config-kiofilewidgets.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kiofilewidgets.h)

set(kiofilewidgets_SRCS
  kfilemetapreview.cpp
  kimagefilepreview.cpp
  kpreviewwidgetbase.cpp
)

add_library(KIOFileWidgets ${kiofilewidgets_SRCS})
generate_export_header(KIOFileWidgets)

target_include_directories(KIOFileWidgets PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>" # kio_version.h
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../include>" # kio/global.h
)

target_link_libraries(KIOFileWidgets
  LINK_PUBLIC
    KIOWidgets
    KBookmarks    # in KFilePlacesModel's API
    XmlGui        # for KActionCollection, used by KFileWidget/KDirOperator
  LINK_PRIVATE
    KIconThemes   # KIconLoader
    KI18n
    ItemViews     # kdirsortfilterproxymodel
    ${Solid_LIBRARIES} # KFilePlacesModel/KFilePlacesView
)

set_target_properties(KIOFileWidgets PROPERTIES VERSION ${KIO_VERSION_STRING}
                                                SOVERSION ${KIO_SOVERSION}
)

install(TARGETS KIOFileWidgets EXPORT KIOTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES

   ${CMAKE_CURRENT_BINARY_DIR}/kiofilewidgets_export.h
   DESTINATION ${INCLUDE_INSTALL_DIR}/kio COMPONENT Devel)

install( FILES
  kimagefilepreview.h
  kpreviewwidgetbase.h

   DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

