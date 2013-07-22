set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

set(krichtexteditorSources
  main.cpp
  krichtexteditor.cpp
)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/..
   ${CMAKE_CURRENT_BINARY_DIR}/..
   )
add_executable(krichtexteditor ${krichtexteditorSources} )
target_link_libraries(krichtexteditor Qt5::Widgets Qt5::Test KWidgets SonnetUi KTextWidgets XmlGui)

#install(TARGETS krichtexteditor DESTINATION ${BIN_INSTALL_DIR})
#install(FILES krichtexteditorui.rc DESTINATION  ${DATA_INSTALL_DIR}/krichtexteditor)

