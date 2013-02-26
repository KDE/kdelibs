remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMMarkAsTest)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/core
  ${CMAKE_CURRENT_BINARY_DIR}/../src/core
  ${KDE4_KDEUI_INCLUDES}  # for sonnet/dialog.h (TODO)
)
########### test_suggest ###############

# TODO automate and merge with test_filter?
set(test_suggest_SRCS test.cpp)
add_executable(test_suggest ${test_suggest_SRCS})
target_link_libraries(test_suggest ${KDE4_KDECORE_LIBS} ${QT_QTMAIN_LIBRARY} sonnetcore)
ecm_mark_as_test(test_suggest)

########### test_dialog ###############

set(test_dialog_SRCS test_dialog.cpp)
add_executable(test_dialog ${test_dialog_SRCS})
target_link_libraries(test_dialog ${KDE4_KDEUI_LIBS} ${QT_QTMAIN_LIBRARY} ${KCoreAddons_LIBRARIES} )
ecm_mark_as_test(test_dialog)
