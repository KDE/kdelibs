project(proxymodeltestsuite)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )


include_directories( ${KDE4_INCLUDES}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

set(proxymodeltestsuite_SRCS
    dynamictreemodel.cpp
    dynamictreewidget.cpp
    modelcommander.cpp
    modelspy.cpp
    modelselector.cpp
    modeltest.cpp
    proxymodeltest.cpp
    modeldumper.cpp
    modeleventlogger.cpp
)

set(eventlogger_RCSS
  eventlogger.qrc
)

qt4_add_resources(
  eventlogger_RCS_SRCS
  ${eventlogger_RCSS}
)

# Grantlee is used for generating compilable code by the ModelEventLogger.
# If Grantlee is not found, the logger does nothing.
find_package(Grantlee QUIET)
macro_log_feature(Grantlee_FOUND
  "Grantlee"
  "Grantlee template system"
  "http://www.grantlee.org" FALSE
  "0.1.0"
  "Required for the model logging feature"
)

if (Grantlee_FOUND)

  include_directories(${Grantlee_INCLUDE_DIRS})

  configure_file(grantlee_paths.h.cmake ${PROJECT_BINARY_DIR}/grantlee_paths.h)

  add_definitions("-DGrantlee_FOUND")

endif()

kde4_add_library(proxymodeltestsuite SHARED
  ${proxymodeltestsuite_SRCS}
  ${eventlogger_RCS_SRCS}
)

target_link_libraries(proxymodeltestsuite
   ${KDE4_KDEUI_LIBS}
   ${QT_QTTEST_LIBRARY}
   ${Grantlee_CORE_LIBRARIES}
)


