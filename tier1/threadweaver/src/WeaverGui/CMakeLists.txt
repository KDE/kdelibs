

set(ThreadWeaverGui_LIB_SRCS  WeaverThreadGrid.cpp )

find_package(Qt5Transitional REQUIRED Gui)

include_directories(${QT_INCLUDES}
                    ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
                    ../Weaver ${CMAKE_CURRENT_BINARY_DIR}/../Weaver)

add_library(ThreadWeaverGui ${LIBRARY_TYPE} ${ThreadWeaverGui_LIB_SRCS})
target_link_libraries(ThreadWeaverGui threadweaver ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY})
set_target_properties(ThreadWeaverGui PROPERTIES VERSION   ${ECM_VERSION_STRING}
                                                 SOVERSION ${ECM_SOVERSION})

install(TARGETS ThreadWeaverGui EXPORT kdelibsLibraryTargets ${ECM_TARGET_DEFAULT_ARGS})

install(FILES WeaverThreadGrid.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/threadweaver/ COMPONENT Devel
)
