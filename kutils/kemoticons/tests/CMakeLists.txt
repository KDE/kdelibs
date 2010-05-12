set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include_directories(${CMAKE_SOURCE_DIR} ../ ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDES})

kde4_add_executable(kemoticonstest main.cpp)

target_link_libraries(kemoticonstest ${KDE4_KDEUI_LIBS} kemoticons)

add_definitions( -DSRCDIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/\\"" )

set(kemoticontest2_SRCS kemoticontest.cpp)

kde4_add_unit_test(kemoticontest2 ${kemoticontest2_SRCS})

target_link_libraries(kemoticontest2  ${QT_QTTEST_LIBRARY} ${KDE4_KIO_LIBS} kemoticons)
