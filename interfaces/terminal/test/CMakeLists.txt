


########### next target ###############



   set(test_SRCS main.cc )
   kde4_add_test_executable(test  ${test_SRCS})
   target_link_libraries(test  ${KDE4_KDECORE_LIBS} kdeui kio )

########### install files ###############





