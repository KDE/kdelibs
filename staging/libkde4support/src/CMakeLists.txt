set(libkde4support_SRCS
    ktemporaryfile.cpp
)

include_directories(
    ${CMAKE_SOURCE_DIR}/libinqt5/src/
    ${CMAKE_BINARY_DIR}/libinqt5/src/
    ${CMAKE_SOURCE_DIR}/kdecore/kernel/ # for KComponentData
    ${CMAKE_BINARY_DIR}/kdecore/kernel/
    ${CMAKE_BINARY_DIR}/kdecore/
    ${CMAKE_SOURCE_DIR}/kdecore/config/
    ${CMAKE_SOURCE_DIR}/kdecore/util/
)

add_library(kde4support ${LIBRARY_TYPE} ${libkde4support_SRCS})
generate_export_header(kde4support)

target_link_libraries(kde4support ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY} kdecore inqt5)
set_target_properties(kde4support PROPERTIES VERSION ${ECM_VERSION_STRING} SOVERSION ${ECM_SOVERSION})

install(TARGETS kde4support ${ECM_TARGET_DEFAULT_ARGS})

install(FILES
    ktemporaryfile.h
    ${CMAKE_CURRENT_BINARY_DIR}/kde4support_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
