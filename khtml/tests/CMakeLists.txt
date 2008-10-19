set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(${KDE4_KPARTS_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(
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
)
include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}/../misc
  ${CMAKE_CURRENT_BINARY_DIR}/../dom
  ${CMAKE_CURRENT_BINARY_DIR}/../xml
  ${CMAKE_CURRENT_BINARY_DIR}/../html
  ${CMAKE_CURRENT_BINARY_DIR}/../rendering
  ${CMAKE_CURRENT_BINARY_DIR}/../ecma
  ${CMAKE_CURRENT_BINARY_DIR}/../imload
  ${CMAKE_CURRENT_BINARY_DIR}/../imload/decoders
  ${CMAKE_CURRENT_BINARY_DIR}/../java
  ${CMAKE_CURRENT_BINARY_DIR}/../svg
  ${CMAKE_CURRENT_BINARY_DIR}/../svg/graphics
  ${CMAKE_CURRENT_BINARY_DIR}/../platform/graphics
  ${CMAKE_CURRENT_BINARY_DIR}/../platform/text
  ${CMAKE_CURRENT_BINARY_DIR}/../compat
  ${CMAKE_CURRENT_BINARY_DIR}/../css
)
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
