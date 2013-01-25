
include(ECMMarkAsTest)

########### testkjs_static ###############

set(testkjs_static_SRCS testkjs.cpp )

add_executable(testkjs_static ${testkjs_static_SRCS})
ecm_mark_as_test(testkjs_static)

target_link_libraries(testkjs_static kjs ${KDEWIN_LIBRARIES} ${QT_QTCORE_LIBRARY})

########### testkjs ###############

set(testkjs_SRCS testkjs.cpp )

add_executable(testkjs ${testkjs_SRCS})
ecm_mark_as_test(testkjs)

target_link_libraries(testkjs kjs ${KDEWIN_LIBRARIES} ${QT_QTCORE_LIBRARY})

