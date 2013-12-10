
#test5.cpp test5.h: $(srcdir)/test5.kcfg ../kconfig_compiler $(srcdir)/test5.kcfgc
#	../kconfig_compiler $(srcdir)/test5.kcfg $(srcdir)/test5.kcfgc

macro(GEN_KCFG_TEST_SOURCE _testName _srcs)
   add_custom_command(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_testName}.cpp ${CMAKE_CURRENT_BINARY_DIR}/${_testName}.h
      COMMAND ${KConfig_KCFGC_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${_testName}.kcfg ${CMAKE_CURRENT_SOURCE_DIR}/${_testName}.kcfgc
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_testName}.kcfg ${CMAKE_CURRENT_SOURCE_DIR}/${_testName}.kcfgc kconfig_compiler)

#   set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${_testName}.h PROPERTIES GENERATED TRUE)
   qt5_generate_moc(${CMAKE_CURRENT_BINARY_DIR}/${_testName}.h ${CMAKE_CURRENT_BINARY_DIR}/${_testName}.moc )
# do not run automoc on the generated file
   set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${_testName}.cpp PROPERTIES SKIP_AUTOMOC TRUE)
   set( ${_srcs} ${${_srcs}} ${CMAKE_CURRENT_BINARY_DIR}/${_testName}.cpp ${CMAKE_CURRENT_BINARY_DIR}/${_testName}.h )

   set_property(SOURCE ${CMAKE_CURRENT_BINARY_DIR}/${_testName}.cpp APPEND PROPERTY OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${_testName}.moc )
endmacro(GEN_KCFG_TEST_SOURCE)

include(ECMMarkAsTest)

########### next target ###############

set(test1_SRCS test1main.cpp )


gen_kcfg_test_source(test1 test1_SRCS)

add_executable(test1 ${test1_SRCS})
ecm_mark_as_test(test1)
target_link_libraries(test1 KF5::ConfigGui)


########### next target ###############

set(test2_SRCS test2main.cpp )


gen_kcfg_test_source(test2 test2_SRCS)

add_executable(test2 ${test2_SRCS})
ecm_mark_as_test(test2)
target_link_libraries(test2 KF5::ConfigGui)


########### next target ###############

set(test3_SRCS test3main.cpp )


gen_kcfg_test_source(test3 test3_SRCS)

add_executable(test3 ${test3_SRCS})
ecm_mark_as_test(test3)
target_link_libraries(test3 KF5::ConfigGui)


########### next target ###############

set(test3a_SRCS test3amain.cpp )


gen_kcfg_test_source(test3a test3a_SRCS)

add_executable(test3a ${test3a_SRCS})
ecm_mark_as_test(test3a)
target_link_libraries(test3a KF5::ConfigGui)


########### next target ###############

set(test4_SRCS test4main.cpp )


gen_kcfg_test_source(test4 test4_SRCS)

add_executable(test4 ${test4_SRCS})
ecm_mark_as_test(test4)
target_link_libraries(test4 KF5::ConfigGui)


########### next target ###############

set(test5_SRCS test5main.cpp )


gen_kcfg_test_source(test5 test5_SRCS)

add_executable(test5 ${test5_SRCS})
ecm_mark_as_test(test5)
target_link_libraries(test5 KF5::ConfigGui)


########### next target ###############

set(test6_SRCS test6main.cpp )


gen_kcfg_test_source(test6 test6_SRCS)

add_executable(test6 ${test6_SRCS})
ecm_mark_as_test(test6)
target_link_libraries(test6 KF5::ConfigGui)


########### next target ###############

set(test7_SRCS test7main.cpp )


gen_kcfg_test_source(test7 test7_SRCS)

add_executable(test7 ${test7_SRCS})
ecm_mark_as_test(test7)
target_link_libraries(test7 KF5::ConfigGui)


########### next target ###############

set(test8_SRCS test8main.cpp )


gen_kcfg_test_source(test8a test8_SRCS)
gen_kcfg_test_source(test8b test8_SRCS)

add_executable(test8 ${test8_SRCS})
ecm_mark_as_test(test8)
target_link_libraries(test8 KF5::ConfigGui)


########### next target ###############

set(test9_SRCS test9main.cpp )


gen_kcfg_test_source(test9 test9_SRCS)

add_executable(test9 ${test9_SRCS})
ecm_mark_as_test(test9)
target_link_libraries(test9 KF5::ConfigGui)


########### next target ###############

set(test10_SRCS test10main.cpp )


gen_kcfg_test_source(test10 test10_SRCS)

add_executable(test10 ${test10_SRCS})
ecm_mark_as_test(test10)
target_link_libraries(test10 KF5::ConfigGui)


########### next target ###############

set(test11_SRCS test11main.cpp )


gen_kcfg_test_source(test11 test11_SRCS)
gen_kcfg_test_source(test11a test11_SRCS)

add_executable(test11 ${test11_SRCS})
ecm_mark_as_test(test11)
target_link_libraries(test11 KF5::ConfigGui)


########### next target ###############

set(test12_SRCS test12main.cpp )

gen_kcfg_test_source(test12 test12_SRCS)

add_executable(test12 ${test12_SRCS})
ecm_mark_as_test(test12)
target_link_libraries(test12 KF5::ConfigGui)


########### next target ###############

set(test_dpointer_SRCS test_dpointer_main.cpp )

gen_kcfg_test_source(test_dpointer test_dpointer_SRCS)

add_executable(test_dpointer ${test_dpointer_SRCS})
ecm_mark_as_test(test_dpointer)
target_link_libraries(test_dpointer KF5::ConfigGui)


########### next target ###############

set(test_signal_SRCS test_signal_main.cpp )
gen_kcfg_test_source(test_signal test_signal_SRCS)
add_executable(test_signal ${test_signal_SRCS})
ecm_mark_as_test(test_signal)
target_link_libraries(test_signal KF5::ConfigGui)

########### next target ###############

set(kconfigcompiler_test_SRCS kconfigcompiler_test.cpp )
add_executable(kconfigcompiler_test ${kconfigcompiler_test_SRCS})
ecm_mark_as_test(kconfigcompiler_test)
add_test(kconfig-kconfigcompiler kconfigcompiler_test)

target_link_libraries(kconfigcompiler_test Qt5::Test )

########### install files ###############





