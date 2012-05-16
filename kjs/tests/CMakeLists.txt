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

set(KHTMLDIR "${CMAKE_CURRENT_SOURCE_DIR}/../../khtml")

include_directories(${KHTMLDIR}/ecma/debugger ${QT_QTCORE_INCLUDE_DIR})

kde4_add_executable(ecmatest TEST
  ecmatest.cpp
  ecmatest.h
  ${KHTMLDIR}/ecma/debugger/value2string.cpp
  ${KHTMLDIR}/ecma/debugger/value2string.h
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
