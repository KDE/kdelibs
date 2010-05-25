
project(kjs)

add_subdirectory(tests)

# Conflict between KJS::HashTable and WTF::HashTable, due to "using namespace" of both namespaces.
kde4_no_enable_final(kjs)

# Configuration checks
include(FindThreads)
check_library_exists(pthread pthread_attr_get_np "" HAVE_PTHREAD_ATTR_GET_NP)
check_library_exists(pthread pthread_getattr_np "" HAVE_PTHREAD_GETATTR_NP)
check_include_files(float.h       HAVE_FLOAT_H)
check_include_files(sys/timeb.h   HAVE_SYS_TIMEB_H)
check_include_files(ieeefp.h      HAVE_IEEEFP_H)
check_include_files("pthread.h;pthread_np.h" HAVE_PTHREAD_NP_H)
check_include_files(valgrind/memcheck.h   HAVE_MEMCHECK_H)

macro_push_required_vars()
if(NOT WIN32)
    set(CMAKE_REQUIRED_LIBRARIES "-lm")
endif(NOT WIN32)
check_function_exists(_finite    HAVE_FUNC__FINITE)
check_function_exists(finite     HAVE_FUNC_FINITE)
check_function_exists(posix_memalign     HAVE_FUNC_POSIX_MEMALIGN)
check_symbol_exists(isnan   "math.h" HAVE_FUNC_ISNAN)
check_symbol_exists(isinf   "math.h" HAVE_FUNC_ISINF)
macro_pop_required_vars()

#Do not make PCRE optional here. PCRE is a hard requirement for modern systems
#but we give old systems some slack... that's why we don't specify "REQUIRED".
find_package(PCRE)
macro_bool_to_01(PCRE_FOUND HAVE_PCREPOSIX)

option(KJS_FORCE_DISABLE_PCRE "Force building of KJS without PCRE. Doing this will result in many webpage working incorrectly, due to extremely poor regular expression support ")

# Generate global.h
configure_file(global.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/global.h )
configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )

include_directories(${KDE4_KDECORE_INCLUDES} ${CMAKE_SOURCE_DIR}/wtf ${KDEWIN_INCLUDES} )

macro_log_feature(PCRE_FOUND "PCRE" "Perl Compatible Regular Expression" "http://www.pcre.org" FALSE "" "Lack of PCRE will result in extremely poor regular expression support in KJS. ")

# the check for pcre is in kdelibs/CMakeLists.txt
if(PCRE_FOUND  AND NOT  KJS_FORCE_DISABLE_PCRE)
   include_directories(${PCRE_INCLUDE_DIR})

   # tell check_symbol_exists to -I pcre dirs.
   macro_push_required_vars()
   set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${PCRE_INCLUDE_DIR})

   check_symbol_exists(PCRE_CONFIG_UTF8 "pcre.h" HAVE_PCRE_UTF8)
   check_symbol_exists(PCRE_CONFIG_STACKRECURSE "pcre.h" HAVE_PCRE_STACK)

   macro_pop_required_vars()

   # Even though we "support" non-PCRE builds, if we build PCRE, we want a version 
   # recent enough, and we don't want to fallback to a completely crippled 
   # POSIX code just like that. 
   if (NOT HAVE_PCRE_UTF8  OR NOT  HAVE_PCRE_STACK)
      message(FATAL_ERROR "Your libPCRE is too old. KJS requires at least PCRE4.5")
   endif (NOT HAVE_PCRE_UTF8  OR NOT  HAVE_PCRE_STACK)

else (PCRE_FOUND  AND NOT  KJS_FORCE_DISABLE_PCRE)
   # if we're here, either PCRE support is disabled, or it's not found...
   # it better be disabled.
   if (NOT KJS_FORCE_DISABLE_PCRE)
        message(FATAL_ERROR "The PCRE regular expression library has not been found. KJS requires PCRE >= 4.5 to function properly. If you for some reason can not install it, you can force a build with POSIX regex.h by passing -DKJS_FORCE_DISABLE_PCRE=true to cmake. However, be advised that it'll result in many websites breaking")
   endif (NOT KJS_FORCE_DISABLE_PCRE)
   # if pcre is not installed or disabled, at least the posix regex.h has to be available
   if(APPLE)
      check_include_files("sys/types.h;regex.h" HAVE_REGEX_H)
   else(APPLE)
      check_include_files(regex.h HAVE_REGEX_H)
   endif(APPLE)
   if (NOT HAVE_REGEX_H)
      message(FATAL_ERROR "Neither the PCRE regular expression library nor the POSIX regex.h header have been found. Consider installing PCRE.")
   endif (NOT HAVE_REGEX_H)
endif(PCRE_FOUND  AND NOT  KJS_FORCE_DISABLE_PCRE)

# The crosscompiling parts are commented out on purpose. Alex
# if (CMAKE_CROSSCOMPILING)
#    set(IMPORT_ICEMAKER_EXECUTABLE "${KDE_HOST_TOOLS_PATH}/ImportIcemakerExecutable.cmake" CACHE FILEPATH "Point it to the export file of icemaker from a native build")
#    include(${IMPORT_ICEMAKER_EXECUTABLE})
#    set(ICEMAKER_EXECUTABLE icemaker)
# else (CMAKE_CROSSCOMPILING)

   ########### icemaker, generates some tables for kjs/frostbyte ###############
   set(icemaker_SRCS
       bytecode/generator/tablebuilder.cpp
       bytecode/generator/types.cpp
       bytecode/generator/codeprinter.cpp
       bytecode/generator/driver.cpp
       bytecode/generator/lexer.cpp
       bytecode/generator/parser.cpp
      )
   kde4_add_executable(icemaker NOGUI ${icemaker_SRCS})

   # get the name of the generated wrapper script (which sets up LD_LIBRARY_PATH)
   get_target_property(ICEMAKER_EXECUTABLE icemaker WRAPPER_SCRIPT)

#    export(TARGETS icemaker FILE ${CMAKE_BINARY_DIR}/ImportIcemakerExecutable.cmake)
# endif (CMAKE_CROSSCOMPILING)

# and the custom command
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/opcodes.h ${CMAKE_CURRENT_BINARY_DIR}/opcodes.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/machine.cpp
  COMMAND ${ICEMAKER_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/bytecode
  DEPENDS icemaker ${CMAKE_CURRENT_SOURCE_DIR}/bytecode/codes.def
          ${CMAKE_CURRENT_SOURCE_DIR}/bytecode/opcodes.cpp.in
          ${CMAKE_CURRENT_SOURCE_DIR}/bytecode/opcodes.h.in
          ${CMAKE_CURRENT_SOURCE_DIR}/bytecode/machine.cpp.in
)

########### next target ###############

# We don't want -pedantic/--pedantic for KJS since we want to use GCC extension when available
string(REPLACE "--pedantic" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REPLACE "-pedantic" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

add_definitions(-DBUILDING_KDE__)

add_subdirectory( wtf )

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
   ExecState.cpp
   Parser.cpp
   array_object.cpp
   array_instance.cpp
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
   CommonIdentifiers.cpp
   JSVariableObject.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/opcodes.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/machine.cpp
   nodes2bytecode.cpp
   CompileState.cpp
   )

if (NOT DEFINED QT_ONLY)
   set(KJSLIBNAME kjs)
else (NOT DEFINED QT_ONLY)
   set(KJSLIBNAME qkjs)
endif (NOT DEFINED QT_ONLY)


kde4_add_library(${KJSLIBNAME} SHARED ${kjs_LIB_SRCS})

if(WIN32)
   target_link_libraries(${KJSLIBNAME} ${KDEWIN_LIBRARIES})
endif(WIN32)

if(CMAKE_THREAD_LIBS_INIT)
   target_link_libraries(${KJSLIBNAME} ${CMAKE_THREAD_LIBS_INIT})
endif(CMAKE_THREAD_LIBS_INIT)

if(UNIX)
   target_link_libraries(${KJSLIBNAME} m)
endif(UNIX)

if(PCRE_FOUND)
   target_link_libraries(${KJSLIBNAME} ${PCRE_LIBRARIES})
endif(PCRE_FOUND)

set_target_properties(${KJSLIBNAME} PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS ${KJSLIBNAME} EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### kjs - basic shell ###############

set(kjs_SRCS kjs.cpp)

#required by win32 see  kdelibs/cmake/modules/kde4Macros.cmake kde4_add_manifest
set (kjs_bin_OUTPUT_NAME kjs)

# 'kjs_bin' because cmake doesn't like having a lib and app with the same name
kde4_add_executable(kjs_bin NOGUI ${kjs_SRCS})

set_target_properties(kjs_bin PROPERTIES OUTPUT_NAME ${kjs_bin_OUTPUT_NAME})

target_link_libraries(kjs_bin ${KJSLIBNAME})

install(TARGETS kjs_bin ${INSTALL_TARGETS_DEFAULT_ARGS})

########### KDE-specific API ##############

add_subdirectory(api)

########### install files ###############
# install( FILES
#     ExecState.h
#     JSImmediate.h
#     JSLock.h
#     JSType.h
#     PropertyNameArray.h
#     collector.h
#     completion.h
#     function.h
#     identifier.h
#     interpreter.h
#     list.h
#     lookup.h
#     object.h
#     operations.h
#     package.h
#     property_map.h
#     property_slot.h
#     protect.h
#     scope_chain.h
#     types.h
#     ustring.h
#     value.h
#     CommonIdentifiers.h
#
#     ${CMAKE_CURRENT_BINARY_DIR}/global.h
# 
#     DESTINATION  ${INCLUDE_INSTALL_DIR}/kjs COMPONENT Devel )



