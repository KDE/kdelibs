set(libkde4support_SRCS
    kdecore/ktemporaryfile.cpp
    kdecore/ktempdir.cpp
    kdecore/kmd5.cpp
)

include_directories(
    ${CMAKE_SOURCE_DIR}/kdecore/kernel/ # for KComponentData
    ${CMAKE_BINARY_DIR}/kdecore/
    ${CMAKE_SOURCE_DIR}/kdecore/config/
    ${CMAKE_SOURCE_DIR}/kdecore/util/
    ${kcoreaddons_SOURCE_DIR}/src/randomness # for KRandom
    ${kcoreaddons_BINARY_DIR}/src
)

add_library(kde4support ${LIBRARY_TYPE} ${libkde4support_SRCS})
generate_export_header(kde4support)

target_link_libraries(kde4support ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY} kdecore kcoreaddons)
set_target_properties(kde4support PROPERTIES VERSION ${ECM_VERSION_STRING} SOVERSION ${ECM_SOVERSION})

install(TARGETS kde4support EXPORT kdelibsLibraryTargets ${ECM_TARGET_DEFAULT_ARGS})

install(FILES
    kdecore/ktemporaryfile.h
    kdecore/ktempdir.h
    kdecore/kmd5.h
    ${CMAKE_CURRENT_BINARY_DIR}/kde4support_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

# kparts/*.h
install(FILES
    kparts/genericfactory.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kparts COMPONENT Devel
)
