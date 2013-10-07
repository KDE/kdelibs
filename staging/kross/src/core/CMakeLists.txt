
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
target_link_libraries(KrossCore KI18n Qt5::Script Qt5::Xml Qt5::Widgets KConfigCore)
target_link_libraries(KrossCore LINK_INTERFACE_LIBRARIES Qt5::Script Qt5::Xml)

target_include_directories(KrossCore PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../include>")

set_target_properties(KrossCore PROPERTIES VERSION ${GENERIC_LIB_VERSION}
                                SOVERSION ${GENERIC_LIB_SOVERSION}
                     )


install(TARGETS KrossCore EXPORT KrossLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

generate_export_header(KrossCore)

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
