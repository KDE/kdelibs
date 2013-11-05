set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include(ECMMarkAsTest)

include_directories(
  ${KHTML_SOURCE_DIR}
  ${KHTML_SOURCE_DIR}/
  ${KHTML_SOURCE_DIR}/misc
  ${KHTML_SOURCE_DIR}/dom
  ${KHTML_SOURCE_DIR}/xml
  ${KHTML_SOURCE_DIR}/html
  ${KHTML_SOURCE_DIR}/rendering
  ${KHTML_SOURCE_DIR}/ecma
  ${KHTML_SOURCE_DIR}/imload
  ${KHTML_SOURCE_DIR}/imload/decoders
  ${KHTML_SOURCE_DIR}/java
  ${KHTML_SOURCE_DIR}/svg
  ${KHTML_SOURCE_DIR}/svg/graphics
  ${KHTML_SOURCE_DIR}/platform/graphics
  ${KHTML_SOURCE_DIR}/platform/text
  ${KHTML_SOURCE_DIR}/platform
  ${KHTML_SOURCE_DIR}/compat
  ${KHTML_SOURCE_DIR}/css
)

include_directories(
  ${KHTML_BINARY_DIR}/..
  ${KHTML_BINARY_DIR}/java
  ${KHTML_BINARY_DIR}/css
)

add_subdirectory(java)

# msvc linker doesn't like "#define protected public"
if(NOT MSVC)

    ########### next target ###############
    set(khtmltest_SRCS
        testkhtmlnew.cpp
    )

    qt5_add_resources( khtmltest_SRCS testkhtml.qrc )
    add_executable(khtmltest ${khtmltest_SRCS} )
    ecm_mark_as_test(khtmltest)
    target_link_libraries( khtmltest KHtml XmlGui KTextWidgets KParts )
    #KJS KF5::KI18n

    ########### next target ###############
endif(NOT MSVC)

#######################################

# msvc linker doesn't like "#define protected public"
if(NOT MSVC)

    ########### next target ###############

    set(testkhtml_SRCS
        testkhtml.cpp
    )


    add_executable(testkhtml ${testkhtml_SRCS})
    ecm_mark_as_test(testkhtml)
    target_link_libraries(testkhtml KHtml XmlGui KTextWidgets KParts)

    ########### next target ###############

    set(testkhtml_static_SRCS
        testkhtml.cpp
    )


    add_executable(testkhtml_static ${testkhtml_static_SRCS} )
    ecm_mark_as_test(testkhtml_static)
    target_link_libraries(testkhtml_static KHtml XmlGui KTextWidgets KParts)

    ########### next target ###############
endif(NOT MSVC)

# Note that testregression can't be compiled by default, it needs private Qt headers.
# See also BUILDING-TESTREGRESSION
if (KHTML_BUILD_TESTREGRESSION)

    set(testregression_SRCS
        test_regression.cpp
        test_regression_fontoverload.cpp
    )

    include_directories( ${QT_INCLUDE_DIR}/../src/3rdparty/harfbuzz/src )

    add_executable(testregression ${testregression_SRCS})
    ecm_mark_as_test(testregression)
    target_link_libraries(testregression  ${X11_X11_LIB} KHtml)

    ########### next target ###############

    set(testregressiongui_SRCS
        test_regression_gui_window.cpp
        test_regression_gui_main.cpp
    )

    qt5_add_resources(testregressiongui_SRCS test_regression_gui.qrc)

    qt5_wrap_ui(testregressiongui_SRCS test_regression_gui.ui)


    add_executable(testregressiongui ${testregressiongui_SRCS})
    ecm_mark_as_test(testregressiongui)
    target_link_libraries(testregressiongui KHtml)
endif (KHTML_BUILD_TESTREGRESSION)

