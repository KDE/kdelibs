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
  soap.cpp
  dxs.cpp
  newstuff.cpp
  qasyncpixmap.cpp
  qasyncframe.cpp
  qstarframe.cpp
  qnowtooltip.cpp
)

#add_definitions(-DKNSSRCDIR=\\"${CMAKE_CURRENT_SOURCE_DIR}\\")
set(CMAKE_SKIP_RPATH TRUE)

kde4_automoc(${kdxspreview_SRCS})
kde4_add_executable(kdxspreview ${kdxspreview_SRCS})

target_link_libraries(kdxspreview ${KDE4_KDECORE_LIBS} ${KDE4_KHTML_LIBS} kio knewstuff2)
install(FILES kdxspreviewrc DESTINATION ${CONFIG_INSTALL_DIR})

