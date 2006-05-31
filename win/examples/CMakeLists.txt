
include_directories( ${CMAKE_SOURCE_DIR}/include/directx )

remove_definitions(-DUNICODE -DWIN32_LEAN_AND_MEAN)

add_executable(dmusic dmusic.c )
target_link_libraries(dmusic ole32 gdi32)
