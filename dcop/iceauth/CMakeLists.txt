
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )

add_executable(iceauth iceauth.c process.c )
target_link_libraries(iceauth kdewin32)

