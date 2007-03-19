add_subdirectory(knewstuff2)

project(kdxspreview)

include_directories(${KDE4_KIO_INCLUDES})

set(kdxspreview_SRCS
  main.cpp
  kdxs.cpp
  kdxsview.cpp
  kdxsbutton.cpp
  kdxsrating.cpp
  kdxscomment.cpp
  kdxscomments.cpp
  kdxschanges.cpp
  kdxstranslation.cpp
  newstuff.cpp
  qasyncpixmap.cpp
  qasyncframe.cpp
  qstarframe.cpp
  qnowtooltip.cpp
)

set(CMAKE_SKIP_RPATH TRUE)

kde4_automoc(${kdxspreview_SRCS})
kde4_add_executable(kdxspreview ${kdxspreview_SRCS})

target_link_libraries(kdxspreview ${KDE4_KDECORE_LIBS} ${KDE4_KHTML_LIBS} kio knewstuff2 kdxs)
install(FILES kdxspreviewrc DESTINATION ${CONFIG_INSTALL_DIR})

##########################

set(kdxs_SRCS
  soap.cpp
  dxs.cpp
  dxsengine.cpp
)

#set(CMAKE_SKIP_RPATH TRUE)

kde4_automoc(${kdxs_SRCS})
kde4_add_library(kdxs SHARED ${kdxs_SRCS})

target_link_libraries(kdxs ${KDE4_KDECORE_LIBS} kio knewstuff2 ${QT_QTNETWORK_LIBRARY})

