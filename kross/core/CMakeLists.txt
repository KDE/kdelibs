include_directories(${CMAKE_SOURCE_DIR} ${KDE4_KDECORE_INCLUDES} ${KDE4_KDEUI_INCLUDES})

set(krosscore_LIB_SRCS
   krossconfig.cpp
   interpreter.cpp
   script.cpp
   action.cpp
   actioncollection.cpp
   manager.cpp
   object.cpp
)

kde4_add_library(krosscore SHARED ${krosscore_LIB_SRCS})
target_link_libraries(krosscore ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${QT_QTSCRIPT_LIBRARY} ${QT_QTXML_LIBRARY})
target_link_libraries(krosscore LINK_INTERFACE_LIBRARIES kdecore ${QT_QTSCRIPT_LIBRARY} ${QT_QTXML_LIBRARY} )

set_target_properties(krosscore PROPERTIES VERSION ${GENERIC_LIB_VERSION} 
                                SOVERSION ${GENERIC_LIB_SOVERSION}
                     )


install(TARGETS krosscore EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    kross_export.h
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
