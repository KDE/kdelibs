
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

########### cupsdconftest ###############
kde4_add_test_executable( cupsdconftest main.cpp )

target_link_libraries( cupsdconftest cupsdconf )


