
include_directories(${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/Weaver/)

########### next target ###############

set(ThreadWeaverExperimental_LIB_SRCS
    FileLoaderJob.cpp
)

QT4_AUTOMOC(${ThreadWeaverExperimental_LIB_SRCS})

add_library(ThreadWeaverExperimental SHARED ${ThreadWeaverExperimental_LIB_SRCS})
target_link_libraries(ThreadWeaverExperimental ThreadWeaver)
set_target_properties(ThreadWeaverExperimental PROPERTIES VERSION 0.6.0 SOVERSION 0)
install(TARGETS ThreadWeaverExperimental DESTINATION ${LIB_INSTALL_DIR})

install(FILES
    FileLoaderJob.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/threadweaver/
)
