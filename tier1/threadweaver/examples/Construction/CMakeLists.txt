include_directories(${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/Weaver/)

include_directories( ../../src/Weaver/ ${CMAKE_CURRENT_BINARY_DIR}/../../src/Weaver )

set(Construction_SRCS
    Construction.cpp
)


add_executable(ThreadWeaverConstruction ${Construction_SRCS})

target_link_libraries(ThreadWeaverConstruction ThreadWeaver)

install(TARGETS ThreadWeaverConstruction DESTINATION ${BIN_INSTALL_DIR} )
