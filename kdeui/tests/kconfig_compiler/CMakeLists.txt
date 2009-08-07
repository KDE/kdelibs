set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(${KDE4_KDEUI_INCLUDES})

#test5.cpp test5.h: $(srcdir)/test5.kcfg ../kconfig_compiler $(srcdir)/test5.kcfgc
#	../kconfig_compiler $(srcdir)/test5.kcfg $(srcdir)/test5.kcfgc

macro(GEN_KCFG_TEST_SOURCE _testName _srcs)
   add_custom_command(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_testName}.cpp ${CMAKE_CURRENT_BINARY_DIR}/${_testName}.h
      COMMAND ${KDE4_KCFGC_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${_testName}.kcfg ${CMAKE_CURRENT_SOURCE_DIR}/${_testName}.kcfgc
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_testName}.kcfg ${CMAKE_CURRENT_SOURCE_DIR}/${_testName}.kcfgc kconfig_compiler)

#   set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${_testName}.h PROPERTIES GENERATED TRUE)
   qt4_generate_moc(${CMAKE_CURRENT_BINARY_DIR}/${_testName}.h ${CMAKE_CURRENT_BINARY_DIR}/${_testName}.moc )
# do not run automoc on the generated file
   set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${_testName}.cpp PROPERTIES SKIP_AUTOMOC TRUE)
   set( ${_srcs} ${${_srcs}} ${CMAKE_CURRENT_BINARY_DIR}/${_testName}.cpp ${CMAKE_CURRENT_BINARY_DIR}/${_testName}.h )
   
#   macro_add_file_dependencies(${CMAKE_CURRENT_BINARY_DIR}/${_testName}.cpp ${CMAKE_CURRENT_BINARY_DIR}/${_testName}.moc )
endmacro(GEN_KCFG_TEST_SOURCE)

########### next target ###############

set(test1_SRCS test1main.cpp )


gen_kcfg_test_source(test1 test1_SRCS)

kde4_add_executable(test1 TEST ${test1_SRCS})

target_link_libraries(test1  ${KDE4_KDEUI_LIBS} )


########### next target ###############

set(test2_SRCS test2main.cpp )


gen_kcfg_test_source(test2 test2_SRCS)

kde4_add_executable(test2 TEST ${test2_SRCS})

target_link_libraries(test2  ${KDE4_KDEUI_LIBS} )


########### next target ###############

set(test3_SRCS test3main.cpp )


gen_kcfg_test_source(test3 test3_SRCS)

kde4_add_executable(test3 TEST ${test3_SRCS})

target_link_libraries(test3  ${KDE4_KDEUI_LIBS} )


########### next target ###############

set(test3a_SRCS test3amain.cpp )


gen_kcfg_test_source(test3a test3a_SRCS)

kde4_add_executable(test3a TEST ${test3a_SRCS})

target_link_libraries(test3a  ${KDE4_KDEUI_LIBS} )


########### next target ###############

set(test4_SRCS test4main.cpp )


gen_kcfg_test_source(test4 test4_SRCS)

kde4_add_executable(test4 TEST ${test4_SRCS})

target_link_libraries(test4  ${KDE4_KDEUI_LIBS} )


########### next target ###############

set(test5_SRCS test5main.cpp )


gen_kcfg_test_source(test5 test5_SRCS)

kde4_add_executable(test5 TEST ${test5_SRCS})

target_link_libraries(test5  ${KDE4_KDEUI_LIBS} )


########### next target ###############

set(test6_SRCS test6main.cpp )


gen_kcfg_test_source(test6 test6_SRCS)

kde4_add_executable(test6 TEST ${test6_SRCS})

target_link_libraries(test6  ${KDE4_KDEUI_LIBS} )


########### next target ###############

set(test7_SRCS test7main.cpp )


gen_kcfg_test_source(test7 test7_SRCS)

kde4_add_executable(test7 TEST ${test7_SRCS})

target_link_libraries(test7  ${KDE4_KDEUI_LIBS} )


########### next target ###############

set(test8_SRCS test8main.cpp )


gen_kcfg_test_source(test8a test8_SRCS)
gen_kcfg_test_source(test8b test8_SRCS)

kde4_add_executable(test8 TEST ${test8_SRCS})

target_link_libraries(test8  ${KDE4_KDEUI_LIBS} )


########### next target ###############

set(test9_SRCS test9main.cpp )


gen_kcfg_test_source(test9 test9_SRCS)

kde4_add_executable(test9 TEST ${test9_SRCS})

target_link_libraries(test9  ${KDE4_KDEUI_LIBS} )


########### next target ###############

set(test10_SRCS test10main.cpp )


gen_kcfg_test_source(test10 test10_SRCS)

kde4_add_executable(test10 TEST ${test10_SRCS})

target_link_libraries(test10  ${KDE4_KDEUI_LIBS} )


########### next target ###############

set(test11_SRCS test11main.cpp )


gen_kcfg_test_source(test11 test11_SRCS)
gen_kcfg_test_source(test11a test11_SRCS)

kde4_add_executable(test11 TEST ${test11_SRCS})

target_link_libraries(test11  ${KDE4_KDEUI_LIBS} )


########### next target ###############

set(test_dpointer_SRCS test_dpointer_main.cpp )


gen_kcfg_test_source(test_dpointer test_dpointer_SRCS)

kde4_add_executable(test_dpointer TEST ${test_dpointer_SRCS})

target_link_libraries(test_dpointer  ${KDE4_KDEUI_LIBS} )


########### next target ###############

set(test_signal_SRCS test_signal_main.cpp )


gen_kcfg_test_source(test_signal test_signal_SRCS)

kde4_add_executable(test_signal TEST ${test_signal_SRCS})

target_link_libraries(test_signal  ${KDE4_KDEUI_LIBS} )


########### next target ###############

set(kconfigcompiler_test_SRCS kconfigcompiler_test.cpp )



kde4_add_unit_test(kconfigcompiler_test ${kconfigcompiler_test_SRCS})

target_link_libraries(kconfigcompiler_test ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

########### install files ###############





