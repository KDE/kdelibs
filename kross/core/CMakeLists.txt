
set(krosscore_LIB_SRCS
   krossconfig.cpp
   interpreter.cpp
   script.cpp
   action.cpp
   actioncollection.cpp
   manager.cpp
   object.cpp
)

add_library(krosscore ${LIBRARY_TYPE} ${krosscore_LIB_SRCS})
target_link_libraries(krosscore KI18n Qt5::Script Qt5::Xml Qt5::Widgets KConfigCore)
target_link_libraries(krosscore LINK_INTERFACE_LIBRARIES Qt5::Script Qt5::Xml)

target_include_directories(krosscore PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../..;${CMAKE_CURRENT_SOURCE_DIR}/../..>")

set_target_properties(krosscore PROPERTIES VERSION ${GENERIC_LIB_VERSION}
                                SOVERSION ${GENERIC_LIB_SOVERSION}
                     )


install(TARGETS krosscore EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

generate_export_header(krosscore)

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
