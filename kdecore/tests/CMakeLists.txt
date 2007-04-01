# Tests don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

# This can be used for finding data files in the source dir, without installing them
add_definitions( -DKDESRCDIR=\\"${CMAKE_CURRENT_SOURCE_DIR}\\" )

include_directories( ${KDE4_KDECORE_INCLUDES} )

MACRO(KDECORE_BUILD_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_automoc(${_testname}.cpp)
               kde4_add_test(${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})
       ENDFOREACH(_testname)
ENDMACRO(KDECORE_BUILD_TESTS)
MACRO(KDECORE_EXECUTE_TESTS)
       FOREACH(_testname ${ARGN})
               add_test(kdecore-${_testname} ${EXECUTABLE_OUTPUT_PATH}/${_testname})
       ENDFOREACH(_testname)
ENDMACRO(KDECORE_EXECUTE_TESTS)


########### next target ###############

set(kdecore_unittests
 klocaletest
 klocalizedstringtest
 kstandarddirstest
 kurltest
 kstringhandlertest
 cplusplustest
 kdatetimetest
 ksortablelisttest
 kcharsetstest
 kcalendartest
 kmacroexpandertest
 kshelltest
 kasciitest
 ktimezonestest
 kconfigtest
 kurlmimetest
 klibloadertest
 klockfiletest
 ktempdirtest
 ksharedptrtest
 kfiltertest
 kdatetimetest
 ksavefiletest
 kautosavefiletest
 kdesktopfiletest
 ktemporaryfiletest
 kautostarttest
 kjobtest
 kservicetest
 kglobalstatictest
 kmountpointtest
)

if(UNIX)
 set(kdecore_unittests ${kdecore_unittests} klocalsockettest klocalsocketservertest )
endif(UNIX)

KDECORE_BUILD_TESTS(
 ${kdecore_unittests}
 krandomsequencetest
 kprocesstest
 kdebugtest
 kcmdlineargstest
 kmemtest
 dbuscalltest
 kmdcodectest
 kprociotest
 startserviceby
)

KDECORE_EXECUTE_TESTS( ${kdecore_unittests} )

########### kmimetypetest ###############

# compile kmimemagicrule.cpp into the test since it's not exported and we call match().
set(kmimetypetest_SRCS kmimetypetest.cpp ../services/kmimemagicrule.cpp)
kde4_automoc(${kmimetypetest_SRCS})
kde4_add_executable(kmimetypetest NOGUI RUN_UNINSTALLED ${kmimetypetest_SRCS})
target_link_libraries(kmimetypetest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} )
add_test(kdecore-kmimetypetest ${EXECUTABLE_OUTPUT_PATH}/kmimetypetest)

########### module for klibloadertest ###############

set(klibloadertestmodule_PART_SRCS klibloadertest_module.cpp )

kde4_automoc(${klibloadertestmodule_PART_SRCS})

kde4_add_plugin(klibloadertestmodule WITH_PREFIX ${klibloadertestmodule_PART_SRCS})

target_link_libraries(klibloadertestmodule  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# hack - we don't really want to install this module [it would break later runs when the module is modified locally]
# so we do the .la by hand
#install(TARGETS klibloadertestmodule  DESTINATION ${PLUGIN_INSTALL_DIR} )

#   set(_laname klibloadertestmodule)
#   GET_TARGET_PROPERTY(_target_location klibloadertestmodule LOCATION)
#   GET_FILENAME_COMPONENT(_laname ${_target_location} NAME_WE)
#   GET_FILENAME_COMPONENT(_soname ${_target_location} NAME)
#   set(_laname ${LIBRARY_OUTPUT_PATH}/${_laname}.la)
#   FILE(WRITE ${_laname} "dlname='${_soname}'\n")
#   FILE(APPEND ${_laname} "library_names='${_soname} ${_soname} ${_soname}'\n")

set_target_properties(klibloadertestmodule PROPERTIES SKIP_BUILD_RPATH FALSE BUILD_WITH_INSTALL_RPATH FALSE)

########### module for klibloadertest4 ###############

set(klibloadertestmodule4_PART_SRCS klibloadertest4_module.cpp )

kde4_automoc(${klibloadertestmodule4_PART_SRCS})

kde4_add_plugin(klibloadertestmodule4 WITH_PREFIX ${klibloadertestmodule4_PART_SRCS})

target_link_libraries(klibloadertestmodule4  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

# hack - we don't really want to install this module [it would break later runs when the module is modified locally]
# so we do the .la by hand
#install(TARGETS klibloadertestmodule  DESTINATION ${PLUGIN_INSTALL_DIR} )

#   set(_laname klibloadertestmodule4)
#   GET_TARGET_PROPERTY(_target_location klibloadertestmodule4 LOCATION)
#   GET_FILENAME_COMPONENT(_laname ${_target_location} NAME_WE)
#   GET_FILENAME_COMPONENT(_soname ${_target_location} NAME)
#   set(_laname ${LIBRARY_OUTPUT_PATH}/${_laname}.la)
#   FILE(WRITE ${_laname} "dlname='${_soname}'\n")
#   FILE(APPEND ${_laname} "library_names='${_soname} ${_soname} ${_soname}'\n")

set_target_properties(klibloadertestmodule4 PROPERTIES SKIP_BUILD_RPATH FALSE BUILD_WITH_INSTALL_RPATH FALSE)
