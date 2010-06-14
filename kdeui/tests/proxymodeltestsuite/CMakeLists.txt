project(proxymodeltestsuite)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )


include_directories( ${KDE4_INCLUDES}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

set(proxymodeltestsuite_SRCS
    dynamictreemodel.cpp
    modelcommander.cpp
    modelspy.cpp
    modelselector.cpp
    modeltest.cpp
    proxymodeltest.cpp
)

kde4_add_library(proxymodeltestsuite ${proxymodeltestsuite_SRCS})

target_link_libraries(proxymodeltestsuite
   ${KDE4_KDEUI_LIBS}
   ${QT_QTTEST_LIBRARY}   
)


