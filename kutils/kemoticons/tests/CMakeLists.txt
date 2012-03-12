set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include_directories(${CMAKE_SOURCE_DIR} ../ ${CMAKE_CURRENT_BINARY_DIR})

kde4_add_executable(kemoticonstest main.cpp)
target_link_libraries(kemoticonstest ${KDE4_KDEUI_LIBS} kcoreaddons kemoticons)

# TODO: QT5 Port to QFINDTESTDATA
set(kemoticontest2_SRCS kemoticontest.cpp)
kde4_add_unit_test(kemoticontest2 TESTNAME kutils-kemoticontest2 ${kemoticontest2_SRCS})
set_target_properties(kemoticontest2 PROPERTIES COMPILE_FLAGS -DSRCDIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/\\"")
target_link_libraries(kemoticontest2  ${QT_QTTEST_LIBRARY} ${KDE4_KIO_LIBS} kcoreaddons kemoticons)
