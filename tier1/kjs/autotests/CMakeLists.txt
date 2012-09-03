include(ECMMarkAsTest)

########### ecmatest ##############

include_directories(${QT_QTCORE_INCLUDE_DIR})

add_executable(ecmatest
  ecmatest.cpp
)
target_link_libraries(ecmatest ${KDE4_KDECORE_LIBS} kjs ${KDEWIN_LIBRARIES} ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY})

if (KDE4_BUILD_TESTS)
   set(ECMATEST_BASEDIR "" CACHE PATH "path to checkout of ECMAscript Tests262 checkout")
   if (ECMATEST_BASEDIR)
      file(GLOB _test262_chapters RELATIVE "${ECMATEST_BASEDIR}/test/suite/" "${ECMATEST_BASEDIR}/test/suite/*")
      list(SORT _test262_chapters)
      # The "Intl" object is not implemented (see bug 299962)
      list(REMOVE_ITEM _test262_chapters "intl402")
      foreach (_test262_chapter ${_test262_chapters})
         if (IS_DIRECTORY "${ECMATEST_BASEDIR}/test/suite/${_test262_chapter}")
            add_test(ECMAscript262_${_test262_chapter} ecmatest)
            if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ecmatest_broken_${_test262_chapter}")
               set(_test262_broken_env ";ECMATEST_BROKEN=${CMAKE_CURRENT_SOURCE_DIR}/ecmatest_broken_${_test262_chapter}")
            else ()
               set(_test262_broken_env)
            endif ()
            set_tests_properties(ECMAscript262_${_test262_chapter} PROPERTIES ENVIRONMENT "ECMATEST_BASEDIR=${ECMATEST_BASEDIR};ECMATEST_CHAPTER=${_test262_chapter}${_test262_broken_env}")
         endif ()
      endforeach ()
   endif ()
endif ()

##################### kjsapitest ################

include_directories(
   ${kjs_SOURCE_DIR}/src/kjs/api
   ${kjs_BINARY_DIR}/src/kjs/api
)

add_executable(kjsapitest kjsapitest.cpp)
add_test(kjsapitest kjsapitest)
target_link_libraries(kjsapitest kjsapi ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})
