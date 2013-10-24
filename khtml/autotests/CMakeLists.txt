set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include(ECMMarkAsTest)

include_directories(${KDE4_KPARTS_INCLUDES}
  ${KHTML_SOURCE_DIR}
  ${KHTML_BINARY_DIR}/..
)
include_directories(
  ${KAuth_INCLUDE_DIRS}
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

add_executable(khtmlparttest khtmlparttest.cpp)
add_test(khtml-parttest khtmlparttest)
target_link_libraries(khtmlparttest Qt5::Test KHtml Qt5::Widgets XmlGui KTextWidgets KParts)
ecm_mark_as_test(khtmlparttest)

set(kencodingdetector_SRCS
    ${KHTML_SOURCE_DIR}/misc/kencodingdetector.cpp
    ${KHTML_SOURCE_DIR}/misc/guess_ja.cpp
)

add_executable(kencodingdetectortest kencodingdetectortest.cpp ${kencodingdetector_SRCS})
add_test(khtml-kencodingdetectortest kencodingdetectortest)
target_link_libraries(kencodingdetectortest KCodecs KF5::KI18n KHtml Qt5::Test)
ecm_mark_as_test(kencodingdetectortest)
