set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_SOURCE_DIR}/kjs ${CMAKE_SOURCE_DIR}/wtf ${KDEWIN_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR}/.. )

if (NOT DEFINED QT_ONLY)
   set(KJSLIBNAME kjs)
else (NOT DEFINED QT_ONLY)
   set(KJSLIBNAME qkjs)
endif (NOT DEFINED QT_ONLY)

########### testkjs_static ###############

set(testkjs_static_SRCS testkjs.cpp )

kde4_add_executable(testkjs_static TEST ${testkjs_static_SRCS})

target_link_libraries(testkjs_static ${KJSLIBNAME} ${KDEWIN_LIBRARIES} ${QT_QTCORE_LIBRARY})

########### testkjs ###############

set(testkjs_SRCS testkjs.cpp )

kde4_add_executable(testkjs TEST ${testkjs_SRCS})

target_link_libraries(testkjs ${KJSLIBNAME} ${KDEWIN_LIBRARIES} ${QT_QTCORE_LIBRARY})


