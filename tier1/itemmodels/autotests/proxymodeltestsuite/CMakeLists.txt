project(proxymodeltestsuite)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")

  # Building the test suite standalone
  set(proxymodeltestsuite_standalone TRUE)

  find_package(Qt4 REQUIRED)
  find_package(Automoc4 REQUIRED)
  cmake_minimum_required(VERSION 2.8.10.20130309)
endif()

include_directories(
  ${proxymodeltestsuite_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src
  ${CMAKE_CURRENT_BINARY_DIR}/../../src
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
    eventloggerregister.cpp
)

set(eventlogger_RCSS
  eventlogger.qrc
)

qt4_add_resources(
  eventlogger_RCS_SRCS
  ${eventlogger_RCSS}
)

if(proxymodeltestsuite_standalone)
  set(GRANTLEE_FIND_ARG REQUIRED)
else ()
  set(GRANTLEE_FIND_ARG QUIET)
endif()

# Grantlee is used for generating compilable code by the ModelEventLogger.
# If Grantlee is not found, the logger does nothing.
find_package(Grantlee ${GRANTLEE_FIND_ARG} NO_MODULE)

if(NOT proxymodeltestsuite_standalone)
  # TODO: Put this part in Grantlee
  set_package_properties(Grantlee PROPERTIES
    URL "http://www.grantlee.org"
    DESCRIPTION "A String Template system for Qt"
  )

  set_package_properties(Grantlee PROPERTIES
    TYPE OPTIONAL
    PURPOSE "Grantlee is used for generating compilable code by the ModelEventLogger. Without Grantlee, the logger will do nothing."
  )
endif()

if (Grantlee_FOUND)

  include_directories(${Grantlee_INCLUDE_DIRS})

  configure_file(grantlee_paths.h.cmake ${PROJECT_BINARY_DIR}/grantlee_paths.h)

  add_definitions("-DGrantlee_FOUND")

endif()


add_library(proxymodeltestsuite SHARED
  ${proxymodeltestsuite_SRCS}
  ${eventlogger_RCS_SRCS}
)
generate_export_header(proxymodeltestsuite)

target_link_libraries(proxymodeltestsuite
   Qt5::Widgets
   ${QT_QTTEST_LIBRARY}
   ${Grantlee_CORE_LIBRARIES}
)

if(proxymodeltestsuite_standalone)

  set( LIB_SUFFIX "" CACHE STRING "Define suffix of library directory name (eg. '64')")
  set( INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include )

  install(TARGETS proxymodeltestsuite
         RUNTIME DESTINATION bin
         LIBRARY DESTINATION lib${LIB_SUFFIX}
         ARCHIVE DESTINATION lib${LIB_SUFFIX}
         COMPONENT Devel
  )

  install(FILES
    dynamictreemodel.h
    dynamictreewidget.h
    modelcommander.h
    modelspy.h
    modelselector.h
    modeltest.h
    proxymodeltest.h
    ${CMAKE_CURRENT_BINARY_DIR}/proxymodeltestsuite_export.h
    modeldumper.h
    modeleventlogger.h
    eventloggerregister.h
    indexfinder.h
    DESTINATION ${INCLUDE_INSTALL_DIR}
  )
endif()

