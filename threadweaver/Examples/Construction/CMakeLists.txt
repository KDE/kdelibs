include_directories(${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/Weaver/)

include_directories( ../../Weaver/ )

set(Construction_SRCS
    Construction.cpp
)


add_executable(Construction ${Construction_SRCS})

target_link_libraries(Construction threadweaver)

install(TARGETS Construction DESTINATION ${BIN_INSTALL_DIR} )
