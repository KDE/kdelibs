set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(${KDE4_KPARTS_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/..)
kde4_add_unit_test( khtmlparttest khtmlparttest.cpp )
target_link_libraries( khtmlparttest ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} khtml )

# msvc linker doesn't like "#define protected public"
if(NOT MSVC)

    ########### next target ###############
    set(khtmltest_SRCS
        testkhtmlnew.cpp
    )

    qt4_add_resources( khtmltest_SRCS testkhtml.qrc )
    kde4_add_executable(khtmltest TEST ${khtmltest_SRCS} )
    target_link_libraries( khtmltest ${KDE4_KDECORE_LIBS} khtml )

    ########### next target ###############
endif(NOT MSVC)
