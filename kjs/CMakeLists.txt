
project(kjs)
KDE4_NO_ENABLE_FINAL(kjs)

# Configuration checks
check_library_exists(pthread pthread_attr_get_np "" HAVE_PTHREAD_ATTR_GET_NP)
check_library_exists(pthread pthread_getattr_np "" HAVE_PTHREAD_GETATTR_NP)
check_include_files(float.h       HAVE_FLOAT_H)
check_include_files(sys/timeb.h   HAVE_SYS_TIMEB_H)
check_include_files(ieeefp.h      HAVE_IEEEFP_H)
check_include_files("pthread.h;pthread_np.h" HAVE_PTHREAD_NP_H)
check_include_files(valgrind/memcheck.h   HAVE_MEMCHECK_H)
check_function_exists(_finite    HAVE_FUNC__FINITE)
check_function_exists(finite     HAVE_FUNC_FINITE)
check_function_exists(isinf      HAVE_FUNC_ISINF)
check_function_exists(isnan      HAVE_FUNC_ISNAN)



macro_optional_find_package(PCRE)
macro_bool_to_01(PCRE_FOUND HAVE_PCREPOSIX)

# Generate global.h
configure_file(global.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/global.h )
configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )

include_directories( ${CMAKE_SOURCE_DIR}/wtf ${QT_INCLUDES} ${KDEWIN32_INCLUDES} )

# the check for pcre is in kdelibs/CMakeLists.txt
if(PCRE_FOUND)
   include_directories(${PCRE_INCLUDE_DIR})
else(PCRE_FOUND)
# if pcre is not installed, at least the posix regex.h has to be available
   if(APPLE)
      check_include_files("sys/types.h;regex.h" HAVE_REGEX_H)
   else(APPLE)
      check_include_files(regex.h HAVE_REGEX_H)
   endif(APPLE)
   if (NOT HAVE_REGEX_H)
      message(FATAL_ERROR "Neither the PCRE regular expression library nor the POSIX regex.h header have been found. Consider installing PCRE.")
   endif (NOT HAVE_REGEX_H)
endif(PCRE_FOUND)

########### next target ###############
add_definitions(-DBUILDING_KDE__)

set(CREATE_HASH_TABLE ${CMAKE_CURRENT_SOURCE_DIR}/create_hash_table )

macro(CREATE_LUT _srcs_LIST _in_FILE _out_FILE _dep_FILE)

   add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_out_FILE}
      COMMAND ${PERL_EXECUTABLE} ${CREATE_HASH_TABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${_in_FILE} -i > ${CMAKE_CURRENT_BINARY_DIR}/${_out_FILE}
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_in_FILE} )
   set( ${_srcs_LIST}  ${${_srcs_LIST}} ${CMAKE_CURRENT_BINARY_DIR}/${_out_FILE})
endmacro(CREATE_LUT)

create_lut(kjs_LIB_SRCS date_object.cpp date_object.lut.h date_object.cpp)
create_lut(kjs_LIB_SRCS number_object.cpp number_object.lut.h number_object.cpp)
create_lut(kjs_LIB_SRCS string_object.cpp string_object.lut.h string_object.cpp)
create_lut(kjs_LIB_SRCS array_object.cpp array_object.lut.h array_object.cpp)
create_lut(kjs_LIB_SRCS math_object.cpp math_object.lut.h math_object.cpp)
create_lut(kjs_LIB_SRCS regexp_object.cpp regexp_object.lut.h regexp_object.cpp)
create_lut(kjs_LIB_SRCS keywords.table lexer.lut.h lexer.cpp)

set(kjs_LIB_SRCS
   ${kjs_LIB_SRCS}
   ustring.cpp
   date_object.cpp
   collector.cpp
   nodes.cpp
   grammar.cpp
   lexer.cpp
   lookup.cpp
   operations.cpp
   regexp.cpp
   function_object.cpp
   string_object.cpp
   bool_object.cpp
   number_object.cpp
   internal.cpp
   Context.cpp
   ExecState.cpp
   Parser.cpp
   array_object.cpp
   math_object.cpp
   object_object.cpp
   regexp_object.cpp
   error_object.cpp
   function.cpp
   debugger.cpp
   value.cpp
   list.cpp
   object.cpp
   interpreter.cpp
   package.cpp
   property_map.cpp
   property_slot.cpp
   nodes2string.cpp
   identifier.cpp
   scope_chain.cpp
   dtoa.cpp
   fpconst.cpp
   JSLock.cpp
   JSImmediate.cpp
   PropertyNameArray.cpp
   JSWrapperObject.cpp
   )

if (NOT DEFINED QT_ONLY)
   set(KJSLIBNAME kjs)
else (NOT DEFINED QT_ONLY)
   set(KJSLIBNAME qkjs)
endif (NOT DEFINED QT_ONLY)


kde4_add_library(${KJSLIBNAME} SHARED ${kjs_LIB_SRCS})

if(UNIX)
   target_link_libraries(${KJSLIBNAME}  ${QT_QTCORE_LIBRARY} m )
else(UNIX)
   target_link_libraries(${KJSLIBNAME}  ${QT_QTCORE_LIBRARY})
endif(UNIX)

if(PCRE_FOUND)
   target_link_libraries(${KJSLIBNAME} ${PCRE_LIBRARIES})
endif(PCRE_FOUND)

set_target_properties(${KJSLIBNAME} PROPERTIES VERSION 2.0.0 SOVERSION 2 )
install(TARGETS ${KJSLIBNAME}  DESTINATION ${LIB_INSTALL_DIR} )

########### js - basic shell ###############

set(js_SRCS kjs.cpp)

kde4_add_executable(js NOGUI ${js_SRCS})

target_link_libraries(js ${KJSLIBNAME})

install(TARGETS js DESTINATION ${BIN_INSTALL_DIR})

if(KDE4_BUILD_TESTS)

   ########### testkjs_static ###############

   set(testkjs_static_SRCS testkjs.cpp )

   kde4_add_executable(testkjs_static NOGUI ${testkjs_static_SRCS})

   target_link_libraries(testkjs_static  ${QT_QTCORE_LIBRARY} ${KJSLIBNAME} ${KDEWIN32_LIBRARIES})

   ########### testkjs ###############

   set(testkjs_SRCS testkjs.cpp )

   kde4_add_executable(testkjs NOGUI ${testkjs_SRCS})

   target_link_libraries(testkjs  ${QT_QTCORE_LIBRARY} ${KJSLIBNAME} ${KDEWIN32_LIBRARIES})

endif(KDE4_BUILD_TESTS)

########### install files ###############
install( FILES
    kjs_export.h
    ExecState.h
    JSImmediate.h
    JSLock.h
    JSType.h
    PropertyNameArray.h
    collector.h
    completion.h
    function.h
    identifier.h
    interpreter.h
    list.h
    lookup.h
    object.h
    operations.h
    package.h
    property_map.h
    property_slot.h
    protect.h
    scope_chain.h
    types.h
    ustring.h
    value.h
    nodes.h

    ${CMAKE_CURRENT_BINARY_DIR}/global.h

    DESTINATION  ${INCLUDE_INSTALL_DIR}/kjs )


