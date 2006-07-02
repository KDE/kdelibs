
if(KDE4_BUILD_TESTS)

   include_directories( ${CMAKE_SOURCE_DIR}/kde3support/kdeui )
   
   
   ########### next target ###############
   
   set(kdockwidgetdemo_SRCS kdockwidgetdemo.cpp )
   
   kde4_automoc(${kdockwidgetdemo_SRCS})
   
   kde4_add_executable(kdockwidgetdemo ${kdockwidgetdemo_SRCS})
   
   target_link_libraries(kdockwidgetdemo  ${KDE4_KDECORE_LIBS} kde3support)
   
   
   ########### next target ###############
   
   set(kdockwidgettest_SRCS kdockwidgettest.cpp )
   
   kde4_automoc(${kdockwidgettest_SRCS})
   
   kde4_add_executable(kdockwidgettest ${kdockwidgettest_SRCS})
   
   target_link_libraries(kdockwidgettest  ${KDE4_KDECORE_LIBS} kde3support)
   
   
   ########### next target ###############
   
   set(kdocktest_SRCS kdocktest.cpp )
   
   kde4_automoc(${kdocktest_SRCS})
   
   kde4_add_executable(kdocktest ${kdocktest_SRCS})
   
   target_link_libraries(kdocktest  ${KDE4_KDECORE_LIBS} kde3support)

endif(KDE4_BUILD_TESTS)

########### install files ###############





