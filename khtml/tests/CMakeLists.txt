set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include(ECMMarkAsTest)

include_directories(${KDE4_KPARTS_INCLUDES}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_BINARY_DIR}/..
)
include_directories(
  ${KAuth_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/khtml/misc
  ${CMAKE_SOURCE_DIR}/khtml/dom
  ${CMAKE_SOURCE_DIR}/khtml/xml
  ${CMAKE_SOURCE_DIR}/khtml/html
  ${CMAKE_SOURCE_DIR}/khtml/rendering
  ${CMAKE_SOURCE_DIR}/khtml/ecma
  ${CMAKE_SOURCE_DIR}/khtml/imload
  ${CMAKE_SOURCE_DIR}/khtml/imload/decoders
  ${CMAKE_SOURCE_DIR}/khtml/java
  ${CMAKE_SOURCE_DIR}/khtml/svg
  ${CMAKE_SOURCE_DIR}/khtml/svg/graphics
  ${CMAKE_SOURCE_DIR}/khtml/platform/graphics
  ${CMAKE_SOURCE_DIR}/khtml/platform/text
  ${CMAKE_SOURCE_DIR}/khtml/platform
  ${CMAKE_SOURCE_DIR}/khtml/compat
  ${CMAKE_SOURCE_DIR}/khtml/css
  ${ki18n_SOURCE_DIR}/src
  ${ki18n_BINARY_DIR}/src
)
include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}/../java
  ${CMAKE_CURRENT_BINARY_DIR}/../css
)
kde4_add_unit_test( khtmlparttest khtmlparttest.cpp )
target_link_libraries( khtmlparttest Qt5::Test ${KDE4_KDECORE_LIBS} khtml Qt5::Widgets)

# msvc linker doesn't like "#define protected public"
if(NOT MSVC)

    ########### next target ###############
    set(khtmltest_SRCS
        testkhtmlnew.cpp
    )

    qt5_add_resources( khtmltest_SRCS testkhtml.qrc )
    add_executable(khtmltest ${khtmltest_SRCS} )
    ecm_mark_as_test(khtmltest)
    target_link_libraries( khtmltest ${KDE4_KDECORE_LIBS} khtml )

    ########### next target ###############
endif(NOT MSVC)

set(kencodingdetector_SRCS
    ${CMAKE_SOURCE_DIR}/khtml/misc/kencodingdetector.cpp
    ${CMAKE_SOURCE_DIR}/khtml/misc/guess_ja.cpp
)
kde4_add_unit_test(kencodingdetectortest kencodingdetectortest.cpp ${kencodingdetector_SRCS})
target_link_libraries(kencodingdetectortest Qt5::Test ${KDE4_KDECORE_LIBS})
