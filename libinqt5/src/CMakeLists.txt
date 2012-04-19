set(libinqt5_SRCS
    q5cryptographichash.cpp
    qstandardpaths.cpp
    qstandardpaths_unix.cpp
    qtemporarydir.cpp
)

set(libinqt5_HDRS
    q5cryptographichash.h
    qstandardpaths.h
    qtemporarydir.h
)

# TODO windows: qstandardpaths_win.cpp
# TODO mac: qstandardpaths_mac.cpp

add_library(inqt5 ${LIBRARY_TYPE} ${libinqt5_SRCS} ${libinqt5_HDRS})

generate_export_header(inqt5)

target_link_libraries(inqt5 ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

set_target_properties(inqt5 PROPERTIES VERSION   ${ECM_VERSION_STRING}
                                       SOVERSION ${ECM_SOVERSION}
)

install(TARGETS inqt5 EXPORT ${inqt5_EXPORT_NAME}LibraryTargets ${ECM_TARGET_DEFAULT_ARGS})

# append our subdir so we don't clutter the default install path
# these files could collide with a real Qt5 installation.
file(APPEND "${CMAKE_BINARY_DIR}/inqt5Config.cmake" "set(inqt5_INCLUDE_DIR    \"\${inqt5_INCLUDE_DIR}/inqt5\")\n")

install(FILES
   ${libinqt5_HDRS}
   ${CMAKE_CURRENT_BINARY_DIR}/inqt5_export.h
   DESTINATION ${INCLUDE_INSTALL_DIR}/inqt5 COMPONENT Devel
)

if (inqt5_EXPORT_NAME STREQUAL inqt5)
  file(APPEND "${CMAKE_BINARY_DIR}/inqt5Config.cmake" "include(\${myDir}/inqt5Targets.cmake)\n")
  install(EXPORT inqt5LibraryTargets
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    FILE inqt5Targets.cmake
    COMPONENT Devel
  )
endif ()

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(libqtmimetypes)
