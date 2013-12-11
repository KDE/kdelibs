
set(krosscore_LIB_SRCS
   krossconfig.cpp
   interpreter.cpp
   script.cpp
   action.cpp
   actioncollection.cpp
   manager.cpp
   object.cpp
)

add_library(KF5KrossCore ${krosscore_LIB_SRCS})
target_link_libraries(KF5KrossCore PRIVATE Qt5::Script Qt5::Xml Qt5::Widgets KF5::ConfigCore KF5::I18n)
target_link_libraries(KF5KrossCore PUBLIC Qt5::Script Qt5::Xml)

target_include_directories(KF5KrossCore PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../include>")

set_target_properties(KF5KrossCore PROPERTIES VERSION ${KROSS_VERSION_STRING}
                                   SOVERSION ${KROSS_SOVERSION}
                                   EXPORT_NAME KrossCore
                     )


install(TARGETS KF5KrossCore EXPORT KF5KrossLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

generate_export_header(KF5KrossCore BASE_NAME KrossCore)
add_library(KF5::KrossCore ALIAS KF5KrossCore)

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

install(TARGETS KF5KrossCore EXPORT KF5KrossTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
