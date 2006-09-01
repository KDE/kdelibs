
include_directories(${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/Weaver/)

set(Construction_SRCS
    Construction.cpp
)

QT4_AUTOMOC(${Construction_SRCS})

add_executable(Construction ${Construction_SRCS})

target_link_libraries(Construction ThreadWeaver)

install(TARGETS Construction DESTINATION bin )
