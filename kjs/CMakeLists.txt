
project(kjs)

# Configuration checks
check_library_exists(pthread pthread_attr_get_np "" HAVE_PTHREAD_ATTR_GET_NP)
check_library_exists(pthread pthread_getattr_np "" HAVE_PTHREAD_GETATTR_NP)

macro_optional_find_package(PCRE)
macro_bool_to_01(PCRE_FOUND HAVE_PCREPOSIX)

# Generate global.h
configure_file(global.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/global.h )

#macro_additional_clean_files( ${CMAKE_CURRENT_BINARY_DIR}/global.h )

include_directories( ${CMAKE_SOURCE_DIR}/kxmlcore ${QT_INCLUDES} )

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
   property_map.cpp
   property_slot.cpp
   nodes2string.cpp
   identifier.cpp
   reference.cpp
   reference_list.cpp
   scope_chain.cpp
   dtoa.cpp
   fpconst.cpp
   JSLock.cpp
   JSImmediate.cpp
   )

kde4_automoc(${kjs_LIB_SRCS})

kde4_add_library(kjs SHARED ${kjs_LIB_SRCS})

if(UNIX)
   target_link_libraries(kjs  ${KDE4_KDECORE_LIBS} m )
else(UNIX)
   target_link_libraries(kjs  ${KDE4_KDECORE_LIBS})
endif(UNIX)

if(PCRE_FOUND)
   target_link_libraries(kjs ${PCRE_LIBRARIES})
endif(PCRE_FOUND)

set_target_properties(kjs PROPERTIES VERSION 2.0.0 SOVERSION 2 )
install(TARGETS kjs  DESTINATION ${LIB_INSTALL_DIR} )


if(KDE4_BUILD_TESTS)

   ########### testkjs_static ###############

   set(testkjs_static_SRCS testkjs.cpp )

   kde4_automoc(${testkjs_static_SRCS})

   kde4_add_executable(testkjs_static NOGUI ${testkjs_static_SRCS})

   target_link_libraries(testkjs_static  ${KDE4_KDECORE_LIBS} kjs)

   ########### testkjs ###############

   set(testkjs_SRCS testkjs.cpp )

   kde4_automoc(${testkjs_SRCS})

   kde4_add_executable(testkjs NOGUI ${testkjs_SRCS})

   target_link_libraries(testkjs  ${KDE4_KDECORE_LIBS} kjs )

endif(KDE4_BUILD_TESTS)

########### install files ###############
install( FILES value.h types.h object.h interpreter.h operations.h             ustring.h lookup.h ${CMAKE_CURRENT_BINARY_DIR}/global.h identifier.h property_map.h           property_slot.h reference.h reference_list.h completion.h            scope_chain.h list.h function.h JSLock.h JSImmediate.h JSType.h  DESTINATION  ${INCLUDE_INSTALL_DIR}/kjs )


