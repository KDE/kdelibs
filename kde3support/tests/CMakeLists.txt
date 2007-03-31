
if(KDE4_BUILD_TESTS)

  # Tests don't need to go into toplevel/bin, they are fine in the current dir.
  set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

   include_directories( ${CMAKE_SOURCE_DIR}/kde3support ${CMAKE_SOURCE_DIR}/kde3support/kdeui )
   
   
   ########### next target ###############

   set(kactivelabeltest_SRCS
   kactivelabeltest.cpp
   )

   kde4_automoc(${kactivelabeltest_SRCS})

   kde4_add_executable(kactivelabeltest NOGUI RUN_UNINSTALLED ${kactivelabeltest_SRCS})

   target_link_libraries(kactivelabeltest ${KDE4_KDEUI_LIBS} kde3support)

   ########### next target ###############
   
   set(kdockwidgetdemo_SRCS kdockwidgetdemo.cpp )
   
   kde4_automoc(${kdockwidgetdemo_SRCS})
   
   kde4_add_executable(kdockwidgetdemo NOGUI ${kdockwidgetdemo_SRCS})
   
   target_link_libraries(kdockwidgetdemo  ${KDE4_KDECORE_LIBS} kde3support)
   
   
   ########### next target ###############
   
   set(kdockwidgettest_SRCS kdockwidgettest.cpp )
   
   kde4_automoc(${kdockwidgettest_SRCS})
   
   kde4_add_executable(kdockwidgettest NOGUI ${kdockwidgettest_SRCS})
   
   target_link_libraries(kdockwidgettest  ${KDE4_KDECORE_LIBS} kde3support)
   
   
   ########### next target ###############
   
   set(kdocktest_SRCS kdocktest.cpp )
   
   kde4_automoc(${kdocktest_SRCS})
   
   kde4_add_executable(kdocktest NOGUI ${kdocktest_SRCS})
   
   target_link_libraries(kdocktest  ${KDE4_KDECORE_LIBS} kde3support)


   ########### next target ###############

   set(kaboutdialogtest_SRCS kaboutdialogtest.cpp)

   kde4_automoc(${kaboutdialogtest_SRCS})

   kde4_add_executable(kaboutdialogtest NOGUI RUN_UNINSTALLED ${kaboutdialogtest_SRCS})

   target_link_libraries(kaboutdialogtest ${KDE4_KDEUI_LIBS} kde3support)

########### next target ###############

set(krfcdatetest_SRCS krfcdatetest.cpp )

kde4_automoc(${krfcdatetest_SRCS})

kde4_add_executable(krfcdatetest NOGUI RUN_UNINSTALLED ${krfcdatetest_SRCS})

target_link_libraries(krfcdatetest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} kde3support)

add_test(kdecore-krfcdatetest ${EXECUTABLE_OUTPUT_PATH}/krfcdatetest)

endif(KDE4_BUILD_TESTS)
