project(proxymodeltestsuite)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")

  # Building the test suite standalone
  set(proxymodeltestsuite_standalone TRUE)

  find_package(Qt4 REQUIRED)
  find_package(Automoc4 REQUIRED)
  cmake_minimum_required(VERSION 2.6.3)
endif()

include_directories(
  ${QT_INCLUDES}
  ${proxymodeltestsuite_BINARY_DIR}
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
find_package(Grantlee 0.1.0 ${GRANTLEE_FIND_ARG})

if(NOT proxymodeltestsuite_standalone)
  set_package_properties(Grantlee PROPERTIES DESCRIPTION "ModelEventLogger code generation (part of the ProxyModel test suite)"
                         URL "http://www.grantlee.org"
                         TYPE OPTIONAL
                         PURPOSE "Grantlee is used for generating compilable code by the ModelEventLogger.
                         Without Grantlee, the logger will do nothing."
                        )
endif()

if (Grantlee_FOUND)

  include_directories(${Grantlee_INCLUDE_DIRS})

  configure_file(grantlee_paths.h.cmake ${PROJECT_BINARY_DIR}/grantlee_paths.h)

  add_definitions("-DGrantlee_FOUND")

endif()

if (proxymodeltestsuite_standalone)
  set(LIBRARY_TYPE SHARED)
endif()


automoc4_add_library(proxymodeltestsuite ${LIBRARY_TYPE}
  ${proxymodeltestsuite_SRCS}
  ${eventlogger_RCS_SRCS}
)

target_link_libraries(proxymodeltestsuite
   ${QT_QTCORE_LIBRARY}
   ${QT_QTGUI_LIBRARY}
   ${QT_QTTEST_LIBRARY}
   ${Grantlee_CORE_LIBRARIES}
)
set_target_properties(proxymodeltestsuite PROPERTIES
    LINK_INTERFACE_LIBRARIES "" 
    DEFINE_SYMBOL MAKE_PROXYMODELTESTSUITE_LIB)

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
    proxymodeltestsuite_export.h
    modeldumper.h
    modeleventlogger.h
    eventloggerregister.h
    indexfinder.h
    DESTINATION ${INCLUDE_INSTALL_DIR}
  )
endif()

