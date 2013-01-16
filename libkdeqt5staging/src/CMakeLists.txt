set(libkdeqt5staging_SRCS
    qurlpathinfo.cpp
    qsavefile.cpp
)
if (QT5_BUILD AND NOT WIN32)
set(libkdeqt5staging_SRCS ${libkdeqt5staging_SRCS} qt5only/qx11info.cpp)
include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})
endif()

add_library(kdeqt5staging ${LIBRARY_TYPE} ${libkdeqt5staging_SRCS})

generate_export_header(kdeqt5staging)

target_link_libraries(kdeqt5staging ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

set_target_properties(kdeqt5staging PROPERTIES VERSION   ${ECM_VERSION_STRING}
                                               SOVERSION ${ECM_SOVERSION}
)


# Install into a kdeqt5staging subdir to prevent clashes with real Qt header files
install(FILES
    qurlpathinfo.h
    qsavefile.h
    qtest_staging.h
    ${CMAKE_CURRENT_BINARY_DIR}/kdeqt5staging_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdeqt5staging COMPONENT Devel
)

###################
# where to put the exports, in case we are not build as part as the whole kdelibs
if (NOT kdelibs_SOURCE_DIR)
  set(kdeqt5staging_EXPORT_NAME kdeqt5stagingLibraryTargets)
else ()
  set(kdeqt5staging_EXPORT_NAME kdelibsLibraryTargets)
endif ()

install(TARGETS kdeqt5staging EXPORT ${kdeqt5staging_EXPORT_NAME} ${ECM_TARGET_DEFAULT_ARGS})

# append our subdir so we don't clutter the default install path
# these files could collide with a real Qt5 installation.
file(APPEND "${CMAKE_BINARY_DIR}/kdeqt5stagingConfig.cmake" "set(kdeqt5staging_INCLUDE_DIR    \"\${kdeqt5staging_INCLUDE_DIR}/kdeqt5staging\")\n")

if (NOT kdelibs_SOURCE_DIR)
  file(APPEND "${CMAKE_BINARY_DIR}/kdeqt5stagingConfig.cmake" "include(\${CMAKE_CURRENT_LIST_DIR}/kdeqt5stagingTargets.cmake)\n")

  install(EXPORT ${kdeqt5staging_EXPORT_NAME}
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    FILE kdeqt5stagingTargets.cmake
    COMPONENT Devel
  )
endif ()
