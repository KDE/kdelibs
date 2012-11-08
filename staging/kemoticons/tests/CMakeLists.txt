set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/../src
    ${CMAKE_SOURCE_DIR}/kdecore/util
    ${CMAKE_SOURCE_DIR}/kdeui/widgets
    ${KDE4_KDEUI_INCLUDES}
    ${CMAKE_BINARY_DIR}/staging/kwidgets/src
    ${CMAKE_SOURCE_DIR}/staging/kwidgets/src/utils
)

kde4_add_executable(kemoticonstest main.cpp)
target_link_libraries(kemoticonstest ${KDE4_KDEUI_LIBS} kcoreaddons kemoticons)

# TODO: QT5 Port to QFINDTESTDATA
set(kemoticontest2_SRCS kemoticontest.cpp)
kde4_add_unit_test(kemoticontest2 TESTNAME kutils-kemoticontest2 ${kemoticontest2_SRCS})
set_target_properties(kemoticontest2 PROPERTIES COMPILE_FLAGS -DSRCDIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/\\"")
target_link_libraries(kemoticontest2  ${QT_QTTEST_LIBRARY} ${KDE4_KIO_LIBS} kcoreaddons kemoticons kdecore kdeui)
