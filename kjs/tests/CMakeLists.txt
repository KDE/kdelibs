set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_SOURCE_DIR}/kjs ${kjs_SOURCE_DIR}/wtf ${KDEWIN_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR}/.. )

include(ECMMarkAsTest)

if (NOT DEFINED QT_ONLY)
   set(KJSLIBNAME kjs)
else (NOT DEFINED QT_ONLY)
   set(KJSLIBNAME qkjs)
endif (NOT DEFINED QT_ONLY)

########### testkjs_static ###############

set(testkjs_static_SRCS testkjs.cpp )

add_executable(testkjs_static ${testkjs_static_SRCS})
ecm_mark_as_test(testkjs_static)

target_link_libraries(testkjs_static ${KJSLIBNAME} ${KDEWIN_LIBRARIES} ${QT_QTCORE_LIBRARY})

########### testkjs ###############

set(testkjs_SRCS testkjs.cpp )

add_executable(testkjs ${testkjs_SRCS})
ecm_mark_as_test(testkjs)

target_link_libraries(testkjs ${KJSLIBNAME} ${KDEWIN_LIBRARIES} ${QT_QTCORE_LIBRARY})


