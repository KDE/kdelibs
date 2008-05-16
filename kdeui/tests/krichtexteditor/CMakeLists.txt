PROJECT( krichtexteditor )
FIND_PACKAGE(KDE4 REQUIRED)
INCLUDE_DIRECTORIES( ${KDE4_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} )

SET(krichtexteditorSources
  main.cpp
  krichtexteditor.cpp
  )

KDE4_ADD_EXECUTABLE(krichtexteditor ${krichtexteditorSources} )
TARGET_LINK_LIBRARIES(krichtexteditor ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} )

install(TARGETS krichtexteditor DESTINATION ${BIN_INSTALL_DIR})
install(FILES
  krichtexteditorui.rc
  DESTINATION  ${DATA_INSTALL_DIR}/krichtexteditor)



