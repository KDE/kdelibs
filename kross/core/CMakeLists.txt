include_directories(
    ${CMAKE_SOURCE_DIR}
    ${KDE4_KDECORE_INCLUDES}
    ${KDE4_KDEUI_INCLUDES}
    ${KDE4_KIO_INCLUDES}
)

########### next target ###############

set(krosscore_LIB_SRCS
   krossconfig.cpp
   interpreter.cpp 
   script.cpp
   action.cpp
   actioncollection.cpp
   manager.cpp
   model.cpp
   guiclient.cpp
   )

kde4_automoc(${krosscore_LIB_SRCS})

kde4_add_library(krosscore SHARED ${krosscore_LIB_SRCS})

target_link_libraries(krosscore ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})

set_target_properties(krosscore PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS krosscore  DESTINATION ${LIB_INSTALL_DIR})

########### install files ###############

install( FILES krossconfig.h errorinterface.h childreninterface.h metatype.h interpreter.h script.h action.h actioncollection.h manager.h guiclient.h model.h DESTINATION ${INCLUDE_INSTALL_DIR}/kross/core)
