project(tetest)

find_package(KDE4 REQUIRED)
include_directories(${KDE4_INCLUDES})

include(ECMMarkAsTest)

########### next target ###############

add_executable(tetest main.cc)
target_link_libraries(tetest KParts)

set(test_SRCS main.cc )
add_executable(test ${test_SRCS})
ecm_mark_as_test(test)
target_link_libraries(test KF5::KIOCore)

########### install files ###############
install(TARGETS tetest DESTINATION ${BIN_INSTALL_DIR})
