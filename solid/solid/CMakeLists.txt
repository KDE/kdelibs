set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
add_subdirectory( ifaces )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

#find_path( HUpnp_INCLUDE_DIRS HUpnp )
#find_library( HUpnp_LIBS HUpnp )

#include_directories( ${HUpnp_INCLUDE_DIRS} )

if(WIN32)
   include_directories( ${KDEWIN_INCLUDES} )
endif(WIN32)

configure_file(solid_export.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/solid_export.h)

include (CheckCXXSourceCompiles)

check_cxx_source_compiles("	#ifdef __SUNPRO_CC
					#define __asm__ asm
				#endif
				int main() { __asm__(\"pxor %mm0, %mm0\") ; }" HAVE_X86_MMX)
check_cxx_source_compiles("     #ifdef __SUNPRO_CC
                                        #define __asm__ asm
                                #endif
				int main() { __asm__(\"xorps %xmm0, %xmm0\"); }" HAVE_X86_SSE)
check_cxx_source_compiles("     #ifdef __SUNPRO_CC
                                        #define __asm__ asm
                                #endif
				int main() { __asm__(\"xorpd %xmm0, %xmm0\"); }" HAVE_X86_SSE2)
check_cxx_source_compiles("     #ifdef __SUNPRO_CC
                                        #define __asm__ asm
                                #endif
				int main() { __asm__(\"femms\"); }" HAVE_X86_3DNOW)
check_cxx_source_compiles("     #ifdef __SUNPRO_CC
                                        #define __asm__ asm
                                #endif
				int main() { __asm__(\"mtspr 256, %0; vand %%v0, %%v0, %%v0\" : : \"r\"(-1) ); }" HAVE_PPC_ALTIVEC)

configure_file(config-processor.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-processor.h )

########### next target ###############

file(MAKE_DIRECTORY
   ${CMAKE_CURRENT_BINARY_DIR}/backends/fakehw
   ${CMAKE_CURRENT_BINARY_DIR}/backends/hal
   ${CMAKE_CURRENT_BINARY_DIR}/backends/wmi
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${KDE4_C_FLAGS}") # enable -fvisibility=hidden for C sources

set(solid_LIB_SRCS
   solidnamespace.cpp
   managerbase.cpp
   device.cpp
   devicemanager.cpp
   deviceinterface.cpp
   genericinterface.cpp
   processor.cpp
   block.cpp
   storagedrive.cpp
   opticaldrive.cpp
   storagevolume.cpp
   opticaldisc.cpp
   storageaccess.cpp
   camera.cpp
   portablemediaplayer.cpp
   networkinterface.cpp
   serialinterface.cpp
   acadapter.cpp
   battery.cpp
   button.cpp
   audiointerface.cpp
   dvbinterface.cpp
   predicate.cpp
   predicateparse.cpp
   predicate_lexer.c
   predicate_parser.c
   powermanagement.cpp
   networking.cpp
   video.cpp
   smartcardreader.cpp
#   internetgateway.cpp

   ifaces/acadapter.cpp
   ifaces/audiointerface.cpp
   ifaces/battery.cpp
   ifaces/block.cpp
   ifaces/button.cpp
   ifaces/camera.cpp
   ifaces/opticaldrive.cpp
   ifaces/device.cpp
   ifaces/deviceinterface.cpp
   ifaces/devicemanager.cpp
   ifaces/dvbinterface.cpp
   ifaces/genericinterface.cpp
   ifaces/networkinterface.cpp
   ifaces/serialinterface.cpp
   ifaces/opticaldisc.cpp
   ifaces/portablemediaplayer.cpp
   ifaces/processor.cpp
   ifaces/storagedrive.cpp
   ifaces/storagevolume.cpp
   ifaces/storageaccess.cpp
   ifaces/video.cpp
   ifaces/smartcardreader.cpp
#   ifaces/internetgateway.cpp

   backends/fakehw/fakeacadapter.cpp
   backends/fakehw/fakeaudiointerface.cpp
   backends/fakehw/fakebattery.cpp
   backends/fakehw/fakeblock.cpp
   backends/fakehw/fakebutton.cpp
   backends/fakehw/fakecamera.cpp
   backends/fakehw/fakecdrom.cpp
   backends/fakehw/fakedevice.cpp
   backends/fakehw/fakedeviceinterface.cpp
   backends/fakehw/fakedvbinterface.cpp
   backends/fakehw/fakegenericinterface.cpp
   backends/fakehw/fakemanager.cpp
   backends/fakehw/fakenetworkinterface.cpp
   backends/fakehw/fakeopticaldisc.cpp
   backends/fakehw/fakeportablemediaplayer.cpp
   backends/fakehw/fakeprocessor.cpp
   backends/fakehw/fakestorage.cpp
   backends/fakehw/fakestorageaccess.cpp
   backends/fakehw/fakevideo.cpp
   backends/fakehw/fakevolume.cpp
   backends/fakehw/fakesmartcardreader.cpp
)

if(NOT WIN32 AND NOT APPLE)
   message(STATUS "-- Building Solid KUPnP backend." )
   set(solid_LIB_SRCS ${solid_LIB_SRCS}

   backends/kupnp/cagibidbuscodec.cpp
   backends/kupnp/cagibidevice.cpp
   backends/kupnp/kupnpstorageaccess.cpp
   backends/kupnp/kupnpdeviceinterface.cpp
   backends/kupnp/mediaserver1.cpp
   backends/kupnp/mediaserver2.cpp
   backends/kupnp/mediaserver3.cpp
   backends/kupnp/internetgatewaydevice1.cpp
   backends/kupnp/kupnpdevice.cpp
   backends/kupnp/kupnprootdevice.cpp
   backends/kupnp/kupnpmanager.cpp

   )

#   message(STATUS "-- Building Solid UPnP backend." )
#   set(solid_LIB_SRCS ${solid_LIB_SRCS}

#   backends/upnp/upnpdevice.cpp
#   backends/upnp/upnpdevicemanager.cpp
#   backends/upnp/upnpdeviceinterface.cpp
#   backends/upnp/upnpmediaserver.cpp
#   backends/upnp/upnpinternetgatewaydevice.cpp

#   )

   message(STATUS "-- Building Solid HAL backend." )
   set(solid_LIB_SRCS ${solid_LIB_SRCS}

   backends/hal/halacadapter.cpp
   backends/hal/halaudiointerface.cpp
   backends/hal/halbattery.cpp
   backends/hal/halblock.cpp
   backends/hal/halbutton.cpp
   backends/hal/halcamera.cpp
   backends/hal/halcdrom.cpp
   backends/hal/haldeviceinterface.cpp
   backends/hal/haldvbinterface.cpp
   backends/hal/halfstabhandling.cpp
   backends/hal/halgenericinterface.cpp
   backends/hal/haldevice.cpp
   backends/hal/halmanager.cpp
   backends/hal/halnetworkinterface.cpp
   backends/hal/halserialinterface.cpp
   backends/hal/halopticaldisc.cpp
   backends/hal/halportablemediaplayer.cpp
   backends/hal/halprocessor.cpp
   backends/hal/halstorageaccess.cpp
   backends/hal/halstorage.cpp
   backends/hal/halvideo.cpp
   backends/hal/halvolume.cpp
   backends/hal/halsmartcardreader.cpp

   )
endif(NOT WIN32 AND NOT APPLE)

if(APPLE)
   find_package(IOKit REQUIRED)

   message(STATUS "-- Building Solid IOKit backend." )
   set(solid_LIB_SRCS ${solid_LIB_SRCS}
   backends/iokit/iokitmanager.cpp
   backends/iokit/iokitdevice.cpp
   backends/iokit/cfhelper.cpp
   backends/iokit/iokitdeviceinterface.cpp
   backends/iokit/iokitgenericinterface.cpp
   backends/iokit/iokitprocessor.cpp
   backends/iokit/iokitnetworkinterface.cpp
   backends/iokit/iokitserialinterface.cpp
   backends/iokit/iokitbattery.cpp
   )
endif(APPLE)

if(WIN32)
   include(CheckIncludeFileCXX)
   check_include_file_cxx(wbemidl.h HAVE_WBEM)
   FIND_LIBRARY(WBEM_LIBRARIES NAMES wbemuuid wbemuuidd)
   if(HAVE_WBEM AND WBEM_LIBRARIES)
      set(HAVE_WBEM True)
      message(STATUS "Found wbemuuid library: ${WBEM_LIBRARIES}")
   else(HAVE_WBEM AND WBEM_LIBRARIES)
      set(HAVE_WBEM False)
   endif(HAVE_WBEM AND WBEM_LIBRARIES)
   if(HAVE_WBEM AND NOT WINCE)
       set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAVE_WBEM")
       message(STATUS "-- Building Solid WMI backend." )

       set(solid_LIB_SRCS ${solid_LIB_SRCS}
       backends/wmi/wmiacadapter.cpp
       backends/wmi/wmiaudiointerface.cpp
       backends/wmi/wmibattery.cpp
       backends/wmi/wmiblock.cpp
       backends/wmi/wmibutton.cpp
       backends/wmi/wmicamera.cpp
       backends/wmi/wmicdrom.cpp
       backends/wmi/wmideviceinterface.cpp
       backends/wmi/wmidvbinterface.cpp
       backends/wmi/wmigenericinterface.cpp
       backends/wmi/wmidevice.cpp
       backends/wmi/wmimanager.cpp
       backends/wmi/wminetworkinterface.cpp
       backends/wmi/wmiopticaldisc.cpp
       backends/wmi/wmiportablemediaplayer.cpp
       backends/wmi/wmiprocessor.cpp
       backends/wmi/wmiquery.cpp
       backends/wmi/wmistorageaccess.cpp
       backends/wmi/wmistorage.cpp
       backends/wmi/wmivideo.cpp
       backends/wmi/wmivolume.cpp

       )
    endif(HAVE_WBEM AND NOT WINCE)
endif(WIN32)

set_source_files_properties( org.freedesktop.PowerManagement.xml
                             org.freedesktop.PowerManagement.Inhibit.xml
                             PROPERTIES NO_NAMESPACE TRUE)

qt4_add_dbus_interfaces(solid_LIB_SRCS org.freedesktop.PowerManagement.xml
                        org.freedesktop.PowerManagement.Inhibit.xml )

qt4_add_dbus_interface(solid_LIB_SRCS org.kde.Solid.Networking.Client.xml
                       org_kde_solid_networking_client)

set(solid_OPTIONAL_LIBS)

if(WIN32)
   set(solid_OPTIONAL_LIBS ${solid_OPTIONAL_LIBS} ${KDEWIN_LIBRARY})
   if(HAVE_WBEM)
      set(solid_OPTIONAL_LIBS ${solid_OPTIONAL_LIBS} ${WBEM_LIBRARIES})
   endif(HAVE_WBEM)
endif(WIN32)

if(APPLE)
   set(solid_OPTIONAL_LIBS ${IOKIT_LIBRARY})
endif(APPLE)

kde4_add_library(solid SHARED ${solid_LIB_SRCS})

target_link_libraries(solid ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTGUI_LIBRARY} ${solid_OPTIONAL_LIBS} )#${HUpnp_LIBS} )
target_link_libraries(solid LINK_INTERFACE_LIBRARIES ${QT_CORE_LIBRARY} )

if (WINCE)
   target_link_libraries(solid ${WCECOMPAT_LIBRARIES})
endif(WINCE)

set_target_properties(solid PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
)
install(TARGETS solid EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES org.kde.Solid.Networking.Client.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})
install(FILES org.freedesktop.PowerManagement.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})
install(FILES org.freedesktop.PowerManagement.Inhibit.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

########### static lib for tests  ###############

kde4_add_library(solid_static STATIC ${solid_LIB_SRCS})

target_link_libraries(solid_static ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTGUI_LIBRARY} ${solid_OPTIONAL_LIBS})

if(WIN32)
   set_target_properties(solid_static PROPERTIES COMPILE_FLAGS -DSOLID_EXPORT=)
endif(WIN32)
########### install files ###############

install( FILES ${CMAKE_CURRENT_BINARY_DIR}/solid_export.h solidnamespace.h device.h
               devicenotifier.h deviceinterface.h genericinterface.h processor.h block.h
               storageaccess.h storagedrive.h opticaldrive.h storagevolume.h opticaldisc.h
               camera.h portablemediaplayer.h networkinterface.h acadapter.h battery.h
               button.h audiointerface.h dvbinterface.h predicate.h powermanagement.h
               networking.h video.h serialinterface.h smartcardreader.h
               DESTINATION ${INCLUDE_INSTALL_DIR}/solid COMPONENT Devel)

########### parser build ###############

set(lexer_FILE predicate_lexer)
set(parser_FILE predicate_parser)

find_package(Flex)
macro_log_feature(FLEX_FOUND "Flex" "Fast lexical analyser generator" "http://flex.sourceforge.net" FALSE "" "Needed to build the Solid predicate parser.")

find_program(BISON_EXECUTABLE bison)
macro_log_feature(BISON_EXECUTABLE "Bison" "Project parser generator (yacc replacement)" "http://www.gnu.org/software/bison" FALSE "" "Needed to build the Solid predicate parser.")
mark_as_advanced(BISON_EXECUTABLE)      # don't show it in the simple view in cmake-gui/ccmake

if (FLEX_EXECUTABLE AND BISON_EXECUTABLE)

   add_custom_target(UpdateSolidPredicateParser
                     COMMAND ${FLEX_EXECUTABLE} -P Solid -o${lexer_FILE}.c ${lexer_FILE}.l
                     COMMAND ${BISON_EXECUTABLE} -p Solid -d -b ${parser_FILE} ${parser_FILE}.y
                     COMMAND ${CMAKE_COMMAND} -E copy ${parser_FILE}.tab.c ${CMAKE_CURRENT_SOURCE_DIR}/${parser_FILE}.c
                     COMMAND ${CMAKE_COMMAND} -E copy ${parser_FILE}.tab.h ${CMAKE_CURRENT_SOURCE_DIR}/${parser_FILE}.h
                     COMMAND ${CMAKE_COMMAND} -E remove ${parser_FILE}.tab.c ${parser_FILE}.tab.h
                     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

else (FLEX_EXECUTABLE AND BISON_EXECUTABLE)
   add_custom_target(UpdateSolidPredicateParser
                     COMMAND echo "flex and/or bison not found, so target UpdateSolidPredicateParser inactive")
endif (FLEX_EXECUTABLE AND BISON_EXECUTABLE)

