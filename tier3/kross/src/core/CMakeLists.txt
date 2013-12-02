
set(krosscore_LIB_SRCS
   krossconfig.cpp
   interpreter.cpp
   script.cpp
   action.cpp
   actioncollection.cpp
   manager.cpp
   object.cpp
)

add_library(KrossCore ${krosscore_LIB_SRCS})
target_link_libraries(KrossCore PRIVATE KF5::KI18n Qt5::Script Qt5::Xml Qt5::Widgets KF5::ConfigCore)
target_link_libraries(KrossCore PUBLIC Qt5::Script Qt5::Xml)

target_include_directories(KrossCore PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../include>")

set_target_properties(KrossCore PROPERTIES VERSION ${KROSS_VERSION_STRING}
                                SOVERSION ${KROSS_SOVERSION}
                     )


install(TARGETS KrossCore EXPORT KrossLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

generate_export_header(KrossCore)
add_library(KF5::KrossCore ALIAS KrossCore)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/krosscore_export.h
    krossconfig.h
    errorinterface.h
    childreninterface.h
    wrapperinterface.h
    metatype.h
    metafunction.h
    interpreter.h
    script.h
    action.h
    actioncollection.h
    manager.h
    object.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kross/core COMPONENT Devel)

install(TARGETS KrossCore EXPORT KrossTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
