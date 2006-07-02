


########### next target ###############



if(KDE4_BUILD_TESTS)

   set(test_SRCS main.cc )
   kde4_automoc(${test_SRCS})
   kde4_add_executable(test RUN_UNINSTALLED ${test_SRCS})
   target_link_libraries(test  ${KDE4_KDECORE_LIBS} kdeui kio )

endif(KDE4_BUILD_TESTS)

########### install files ###############





