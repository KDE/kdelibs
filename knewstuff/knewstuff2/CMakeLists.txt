project(knewstuff2)

include_directories(${KDE4_KIO_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(knewstuff2_core_SRCS
   core/ktranslatable.cpp
   core/author.cpp
   core/category.cpp
   core/entry.cpp
   core/entryhandler.cpp
   core/entryloader.cpp
   core/feed.cpp
   core/provider.cpp
   core/providerhandler.cpp
   core/providerloader.cpp
   core/coreengine.cpp
   core/installation.cpp
   core/security.cpp
)

set(knewstuff2_dxs_SRCS
  dxs/soap.cpp
  dxs/dxs.cpp
  dxs/dxsengine.cpp
)

set(knewstuff2_ui_SRCS
  ui/kdxs.cpp
  ui/kdxsview.cpp
  ui/kdxsbutton.cpp
  ui/kdxsrating.cpp
  ui/kdxscomment.cpp
  ui/kdxscomments.cpp
  ui/kdxschanges.cpp
  ui/kdxstranslation.cpp
  ui/downloaddialog.cpp
  ui/qasyncpixmap.cpp
  ui/qasyncframe.cpp
  ui/qstarframe.cpp
  ui/qnowtooltip.cpp
  ui/uploaddialog.cpp
  ui/providerdialog.cpp
  ui/knewstuffbutton.cpp
  ui/knewstuffaction.cpp
)

set(knewstuff2_SRCS
  engine.cpp
)

kde4_automoc(${knewstuff2_core_SRCS})
kde4_automoc(${knewstuff2_dxs_SRCS})
kde4_automoc(${knewstuff2_ui_SRCS})
kde4_automoc(${knewstuff2_SRCS})

kde4_add_library(knewstuff2 SHARED ${knewstuff2_core_SRCS} ${knewstuff2_dxs_SRCS} ${knewstuff2_ui_SRCS} ${knewstuff2_SRCS})

target_link_libraries(knewstuff2 ${KDE4_KIO_LIBS} ${KDE4_KHTML_LIBS} ${QT_QTNETWORK_LIBRARY})
set_target_properties(knewstuff2 PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS knewstuff2 DESTINATION ${LIB_INSTALL_DIR})

install(FILES
  core/author.h
  core/category.h
  core/coreengine.h
  core/entry.h
  core/entryhandler.h
  core/entryloader.h
  core/feed.h
  core/ktranslatable.h
  core/provider.h
  core/providerhandler.h
  core/providerloader.h
  core/installation.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/knewstuff2/core
)

install(FILES
  dxs/dxsengine.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/knewstuff2/dxs
)

install(FILES
  ui/knewstuffbutton.h
  ui/knewstuffaction.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/knewstuff2/ui
)

install(FILES
  engine.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/knewstuff2
)

