set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_SOURCE_DIR}/kjs ${kjs_SOURCE_DIR}/wtf ${KDEWIN_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR}/.. )

if (NOT DEFINED QT_ONLY)
   set(KJSLIBNAME kjs)
else (NOT DEFINED QT_ONLY)
   set(KJSLIBNAME qkjs)
endif (NOT DEFINED QT_ONLY)

########### testkjs_static ###############

set(testkjs_static_SRCS testkjs.cpp )

kde4_add_executable(testkjs_static TEST ${testkjs_static_SRCS})

target_link_libraries(testkjs_static ${KJSLIBNAME} ${KDEWIN_LIBRARIES} ${QT_QTCORE_LIBRARY})

########### testkjs ###############

set(testkjs_SRCS testkjs.cpp )

kde4_add_executable(testkjs TEST ${testkjs_SRCS})

target_link_libraries(testkjs ${KJSLIBNAME} ${KDEWIN_LIBRARIES} ${QT_QTCORE_LIBRARY})

########### ecmatest ##############

include_directories(${QT_QTCORE_INCLUDE_DIR} ${KDE4_KIO_INCLUDES})

kde4_add_executable(ecmatest TEST
  ecmatest.cpp
  ecmatest.h
)
target_link_libraries(ecmatest ${KDE4_KDECORE_LIBS} kjs ${KDEWIN_LIBRARIES} ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY})

function(ADD_ECMATEST ECMA262_CHAPTER)
   if (IS_DIRECTORY "${ECMATEST_BASEDIR}/test/suite/${ECMA262_CHAPTER}")
      string(REPLACE "/15" "" ECMA262_TESTNAME "${ECMA262_CHAPTER}")
      add_test(ECMAscript262_${ECMA262_TESTNAME} ecmatest)
      if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ecmatest_broken_${ECMA262_TESTNAME}")
         set(_test262_broken_env ";ECMATEST_BROKEN=${CMAKE_CURRENT_SOURCE_DIR}/ecmatest_broken_${ECMA262_TESTNAME}")
      else ()
         set(_test262_broken_env)
      endif ()
      set_tests_properties(ECMAscript262_${ECMA262_TESTNAME} PROPERTIES ENVIRONMENT "ECMATEST_BASEDIR=${ECMATEST_BASEDIR};ECMATEST_CHAPTER=${ECMA262_CHAPTER}${_test262_broken_env}")
   endif ()
endfunction(ADD_ECMATEST)

if (KDE4_BUILD_TESTS)
   set(ECMATEST_BASEDIR "" CACHE PATH "path to checkout of ECMAscript Tests262 checkout")
   if (ECMATEST_BASEDIR)
      file(GLOB _test262_chapters RELATIVE "${ECMATEST_BASEDIR}/test/suite/" "${ECMATEST_BASEDIR}/test/suite/*")
      list(SORT _test262_chapters)
      # The "Intl" object is not implemented (see bug 299962)
      list(REMOVE_ITEM _test262_chapters "intl402")
      # ch15 is way too big, split it into several subchapters
      list(REMOVE_ITEM _test262_chapters "ch15")
      foreach (_test262_chapter ${_test262_chapters})
         ADD_ECMATEST(${_test262_chapter})
      endforeach ()

      # now ch15
      file(GLOB _test262_15_chapters RELATIVE "${ECMATEST_BASEDIR}/test/suite/ch15" "${ECMATEST_BASEDIR}/test/suite/ch15/*")
      list(SORT _test262_15_chapters)
      foreach (_test262_15_chapter ${_test262_15_chapters})
         ADD_ECMATEST(ch15/${_test262_15_chapter})
      endforeach ()
   endif ()
endif ()
