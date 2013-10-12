project(KIOFileWidgets)

configure_file(config-kiofilewidgets.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kiofilewidgets.h)

set(kiofilewidgets_SRCS
  kfilemetapreview.cpp
  kimagefilepreview.cpp
  kpreviewwidgetbase.cpp
  krecentdirs.cpp
  defaultviewadapter.cpp

  kdiroperator.cpp
  kdiroperatordetailview.cpp
  kdirsortfilterproxymodel.cpp #used in combination with kdirmodel.cpp
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

add_library(KIOFileWidgets ${kiofilewidgets_SRCS})
generate_export_header(KIOFileWidgets)
add_library(KF5::KIOFileWidgets ALIAS KIOFileWidgets)

target_include_directories(KIOFileWidgets PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>" # kio_version.h
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../include>" # kio/global.h
)

target_link_libraries(KIOFileWidgets
  LINK_PUBLIC
    KIOWidgets
    KBookmarks    # in KFilePlacesModel's API
    XmlGui        # for KActionCollection, used by KFileWidget/KDirOperator
    ${Solid_LIBRARIES} # KFilePlacesModel/KFilePlacesView
  LINK_PRIVATE
    KIconThemes   # KIconLoader
    KI18n
    ItemViews     # kdirsortfilterproxymodel
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
  krecentdirs.h

  kdiroperator.h
  kdirsortfilterproxymodel.h
  kfilefiltercombo.h
  kfileplacesmodel.h
  kfileplacesview.h
  kfilepreviewgenerator.h
  kfilewidget.h
  kurlnavigator.h
  knewfilemenu.h
  knameandurlinputdialog.h

   DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

