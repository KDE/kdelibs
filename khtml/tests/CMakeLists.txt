set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include(ECMMarkAsTest)

include_directories(${KDE4_KPARTS_INCLUDES}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_BINARY_DIR}/..
)
include_directories(
  ${KAuth_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/khtml/src/
  ${CMAKE_SOURCE_DIR}/khtml/src/misc
  ${CMAKE_SOURCE_DIR}/khtml/src/dom
  ${CMAKE_SOURCE_DIR}/khtml/src/xml
  ${CMAKE_SOURCE_DIR}/khtml/src/html
  ${CMAKE_SOURCE_DIR}/khtml/src/rendering
  ${CMAKE_SOURCE_DIR}/khtml/src/ecma
  ${CMAKE_SOURCE_DIR}/khtml/src/imload
  ${CMAKE_SOURCE_DIR}/khtml/src/imload/decoders
  ${CMAKE_SOURCE_DIR}/khtml/src/java
  ${CMAKE_SOURCE_DIR}/khtml/src/svg
  ${CMAKE_SOURCE_DIR}/khtml/src/svg/graphics
  ${CMAKE_SOURCE_DIR}/khtml/src/platform/graphics
  ${CMAKE_SOURCE_DIR}/khtml/src/platform/text
  ${CMAKE_SOURCE_DIR}/khtml/src/platform
  ${CMAKE_SOURCE_DIR}/khtml/src/compat
  ${CMAKE_SOURCE_DIR}/khtml/src/css


)
include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}/../java
  ${CMAKE_CURRENT_BINARY_DIR}/../css
)
kde4_add_unit_test( khtmlparttest khtmlparttest.cpp )
target_link_libraries( khtmlparttest Qt5::Test KHTML Qt5::Widgets XmlGui KTextWidgets KParts)

# msvc linker doesn't like "#define protected public"
if(NOT MSVC)

    ########### next target ###############
    set(khtmltest_SRCS
        testkhtmlnew.cpp
    )

    qt5_add_resources( khtmltest_SRCS testkhtml.qrc )
    add_executable(khtmltest ${khtmltest_SRCS} )
    ecm_mark_as_test(khtmltest)
    target_link_libraries( khtmltest KHTML XmlGui KTextWidgets KParts )
    #KJS KI18n

    ########### next target ###############
endif(NOT MSVC)
#
# set(kencodingdetector_SRCS
#     ${CMAKE_SOURCE_DIR}/khtml/src/misc/kencodingdetector.cpp
#     ${CMAKE_SOURCE_DIR}/khtml/src/misc/guess_ja.cpp
# )
# kde4_add_unit_test(kencodingdetectortest kencodingdetectortest.cpp ${kencodingdetector_SRCS})
# target_link_libraries(kencodingdetectortest KCodecs KI18n KHtml Qt5::Test)

# TODO: ksslkeygentest
