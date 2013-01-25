include (CheckCXXSourceCompiles)

if(MSVC)
    check_cxx_source_compiles("int main() { __asm { pxor mm0, mm0 }; }" HAVE_X86_MMX)
    check_cxx_source_compiles("int main() { __asm { xorps xmm0, xmm0 }; }" HAVE_X86_SSE)
    check_cxx_source_compiles("int main() { __asm { xorpd xmm0, xmm0 }; }" HAVE_X86_SSE2)
    check_cxx_source_compiles("int main() { __asm { femms }; }" HAVE_X86_3DNOW)
else(MSVC)
    check_cxx_source_compiles("	    #ifdef __SUNPRO_CC
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
endif(MSVC)
check_cxx_source_compiles("     #ifdef __SUNPRO_CC
                                        #define __asm__ asm
                                #endif
				int main() { __asm__(\"mtspr 256, %0; vand %%v0, %%v0, %%v0\" : : \"r\"(-1) ); }" HAVE_PPC_ALTIVEC)

configure_file(config-processor.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-processor.h )



########### next target ###############

file(MAKE_DIRECTORY
   ${CMAKE_CURRENT_BINARY_DIR}/backends/fakehw
   ${CMAKE_CURRENT_BINARY_DIR}/backends/hal
   ${CMAKE_CURRENT_BINARY_DIR}/backends/udev
   ${CMAKE_CURRENT_BINARY_DIR}/backends/wmi
)

if(WIN32)
    add_definitions(-DYY_NO_UNISTD_H)
endif(WIN32)

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
   networkshare.cpp
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
   internetgateway.cpp

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
   ifaces/networkshare.cpp
   ifaces/serialinterface.cpp
   ifaces/opticaldisc.cpp
   ifaces/portablemediaplayer.cpp
   ifaces/processor.cpp
   ifaces/storagedrive.cpp
   ifaces/storagevolume.cpp
   ifaces/storageaccess.cpp
   ifaces/video.cpp
   ifaces/smartcardreader.cpp
   ifaces/internetgateway.cpp

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
   backends/fakehw/fakenetworkshare.cpp
   backends/fakehw/fakenetworkinterface.cpp
   backends/fakehw/fakeopticaldisc.cpp
   backends/fakehw/fakeportablemediaplayer.cpp
   backends/fakehw/fakeprocessor.cpp
   backends/fakehw/fakestorage.cpp
   backends/fakehw/fakestorageaccess.cpp
   backends/fakehw/fakevideo.cpp
   backends/fakehw/fakevolume.cpp
   backends/fakehw/fakesmartcardreader.cpp

   backends/shared/rootdevice.cpp
   backends/shared/cpufeatures.cpp
)

if ( HUPNP_FOUND )
  include_directories( ${HUPNP_INCLUDE_DIR} )
  message(STATUS "Building Solid UPnP backend." )
  set(solid_LIB_SRCS ${solid_LIB_SRCS}
      backends/upnp/upnpdevice.cpp
      backends/upnp/upnpdevicemanager.cpp
      backends/upnp/upnpdeviceinterface.cpp
      backends/upnp/upnpmediaserver.cpp
      backends/upnp/upnpinternetgateway.cpp
      backends/upnp/upnpcontrolpoint.cpp
  )
endif ()

if(NOT WIN32 AND NOT APPLE)
#   message(STATUS "Building Solid KUPnP backend." )
#   set(solid_LIB_SRCS ${solid_LIB_SRCS}

#   backends/kupnp/cagibidbuscodec.cpp
#   backends/kupnp/cagibidevice.cpp
#   backends/kupnp/kupnpstorageaccess.cpp
#   backends/kupnp/kupnpdeviceinterface.cpp
#   backends/kupnp/mediaserver1.cpp
#   backends/kupnp/mediaserver2.cpp
#   backends/kupnp/mediaserver3.cpp
#   backends/kupnp/internetgatewaydevice1.cpp
#   backends/kupnp/kupnpdevice.cpp
#   backends/kupnp/kupnprootdevice.cpp
#   backends/kupnp/kupnpmanager.cpp

#   )

   if ( UDEV_FOUND )
      message(STATUS "Building Solid UDev backend." )
      set(solid_LIB_SRCS ${solid_LIB_SRCS}
      backends/udev/udevdevice.cpp
      backends/udev/udevmanager.cpp
      backends/udev/udevdeviceinterface.cpp
      backends/udev/udevgenericinterface.cpp
      backends/udev/cpuinfo.cpp
      backends/udev/udevprocessor.cpp
      backends/udev/udevcamera.cpp
      backends/udev/udevvideo.cpp
      backends/udev/udevaudiointerface.cpp
      backends/udev/udevnetworkinterface.cpp
      backends/udev/udevserialinterface.cpp
      backends/udev/udevaudiointerface_p.cpp
      backends/udev/udevportablemediaplayer.cpp
      backends/udev/udevdvbinterface.cpp
      backends/udev/udevblock.cpp
      backends/shared/udevqtclient.cpp
      backends/shared/udevqtdevice.cpp
      )

      # check for media-player-info (runtime-only optional dependency)
      find_package(MediaPlayerInfo)
      set_package_properties(MediaPlayerInfo PROPERTIES
         DESCRIPTION "Enables identification and querying of portable media players"
         PURPOSE "Runtime-only dependency of the udev solid backend. Support for m-p-i is included even if not found during build"
         URL "http://www.freedesktop.org/wiki/Software/media-player-info"
         TYPE RUNTIME
      )
   endif( UDEV_FOUND )


   message(STATUS "Building Solid HAL backend." )
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

   message(STATUS "Building Solid UPower backend." )
   set(solid_LIB_SRCS ${solid_LIB_SRCS}
      backends/upower/upowermanager.cpp
      backends/upower/upowerdevice.cpp
      backends/upower/upowerbattery.cpp
      backends/upower/upoweracadapter.cpp
      backends/upower/upowerdeviceinterface.cpp
      backends/upower/upowergenericinterface.cpp
   )

   # FIXME: this should work on more Unix systems
   if (CMAKE_SYSTEM_NAME MATCHES Linux)
       if ( WITH_SOLID_UDISKS2 )
           message(STATUS "Building Solid UDisks2 backend." )
           add_definitions(-DWITH_SOLID_UDISKS2)
           set(solid_LIB_SRCS ${solid_LIB_SRCS}
           backends/udisks2/udisksmanager.cpp
           backends/udisks2/udisksdevice.cpp
           backends/udisks2/udisksdevicebackend.cpp
           backends/udisks2/udisksblock.cpp
           backends/udisks2/udisksstoragevolume.cpp
           backends/udisks2/udisksdeviceinterface.cpp
           backends/udisks2/udisksopticaldisc.cpp
           backends/udisks2/udisksopticaldrive.cpp
           backends/udisks2/udisksstoragedrive.cpp
           backends/udisks2/udisksstorageaccess.cpp
           backends/udisks2/udisksgenericinterface.cpp
           backends/udisks2/dbus/manager.cpp
           )
       else ( WITH_SOLID_UDISKS2 )
           message(STATUS "Building Solid UDisks backend." )
           set(solid_LIB_SRCS ${solid_LIB_SRCS}
           backends/udisks/udisksmanager.cpp
           backends/udisks/udisksdevice.cpp
           backends/udisks/udisksblock.cpp
           backends/udisks/udisksstoragevolume.cpp
           backends/udisks/udisksdeviceinterface.cpp
           backends/udisks/udisksopticaldisc.cpp
           backends/udisks/udisksopticaldrive.cpp
           backends/udisks/udisksstoragedrive.cpp
           backends/udisks/udisksstorageaccess.cpp
           backends/udisks/udisksgenericinterface.cpp
          )
       endif ( WITH_SOLID_UDISKS2 )
   endif (CMAKE_SYSTEM_NAME MATCHES Linux)

   message(STATUS "Building Solid fstab backend." )
   set(solid_LIB_SRCS ${solid_LIB_SRCS}
      backends/fstab/fstabmanager.cpp
      backends/fstab/fstabdevice.cpp
      backends/fstab/fstabnetworkshare.cpp
      backends/fstab/fstabstorageaccess.cpp
      backends/fstab/fstabhandling.cpp
      backends/fstab/fstabwatcher.cpp
   )

endif(NOT WIN32 AND NOT APPLE)

include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckCXXSourceCompiles)

check_include_files(mntent.h      HAVE_MNTENT_H)
check_include_files(sys/types.h   HAVE_SYS_TYPES_H)
check_include_files(sys/param.h   HAVE_SYS_PARAM_H)
check_include_files("stdio.h;sys/mnttab.h"  HAVE_SYS_MNTTAB_H)
check_include_files("sys/param.h;sys/mount.h"  HAVE_SYS_MOUNT_H)

check_function_exists(getmntinfo HAVE_GETMNTINFO)

check_function_exists(setmntent  HAVE_SETMNTENT)

check_cxx_source_compiles("
  #include <sys/types.h>
  #include <sys/statvfs.h>
  int main(){
    struct statvfs *mntbufp;
    int flags;
    return getmntinfo(&mntbufp, flags);
  }
" GETMNTINFO_USES_STATVFS )

configure_file( config-solid.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-solid.h )

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

   if(WBEM_FOUND)
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
    endif()
endif(WIN32)

set_source_files_properties( org.freedesktop.PowerManagement.xml
                             org.freedesktop.PowerManagement.Inhibit.xml
                             org.kde.Solid.PowerManagement.PolicyAgent.xml
                             PROPERTIES NO_NAMESPACE TRUE)

qt4_add_dbus_interfaces(solid_LIB_SRCS org.freedesktop.PowerManagement.xml
                        org.freedesktop.PowerManagement.Inhibit.xml
                        org.kde.Solid.PowerManagement.PolicyAgent.xml)

qt4_add_dbus_interface(solid_LIB_SRCS org.kde.Solid.Networking.Client.xml
                       org_kde_solid_networking_client)

set(solid_OPTIONAL_LIBS)

if(WIN32)
   if(WBEM_FOUND)
      set(solid_OPTIONAL_LIBS ${solid_OPTIONAL_LIBS} ${WBEM_LIBRARIES})
   endif()
endif()

if(APPLE)
   set(solid_OPTIONAL_LIBS ${IOKIT_LIBRARY})
endif()

if ( HUPNP_FOUND )
   set(solid_OPTIONAL_LIBS ${solid_OPTIONAL_LIBS} ${HUPNP_LIBS})
endif()

if ( UDEV_FOUND )
   set(solid_OPTIONAL_LIBS ${solid_OPTIONAL_LIBS} ${UDEV_LIBS})
endif ()

add_library(Solid  ${solid_LIB_SRCS})
target_include_directories(Solid PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..;${CMAKE_CURRENT_BINARY_DIR}/..>")
generate_export_header(Solid)

target_link_libraries(Solid LINK_PUBLIC ${QT_QTCORE_LIBRARY}
                            LINK_PRIVATE ${QT_QTDBUS_LIBRARY}
                                         ${QT_QTXML_LIBRARY}
                                         Qt5::Widgets
                                         ${solid_OPTIONAL_LIBS}
 )

set_target_properties(Solid PROPERTIES  VERSION ${SOLID_VERSION_STRING}
                                        SOVERSION ${SOLID_SOVERSION} )

install(TARGETS Solid EXPORT SolidTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES org.kde.Solid.Networking.Client.xml
              org.kde.Solid.PowerManagement.PolicyAgent.xml
              org.freedesktop.PowerManagement.xml
              org.freedesktop.PowerManagement.Inhibit.xml
        DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

########### static lib for tests  ###############

add_library(Solid_static STATIC ${solid_LIB_SRCS})
set_target_properties(Solid_static PROPERTIES COMPILE_FLAGS -DSOLID_STATIC_DEFINE=1)

target_link_libraries(Solid_static LINK_PUBLIC ${QT_QTCORE_LIBRARY})
target_link_libraries(Solid_static ${QT_QTDBUS_LIBRARY} ${QT_QTXML_LIBRARY} Qt5::Widgets ${solid_OPTIONAL_LIBS})
target_include_directories(Solid_static PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..;${CMAKE_CURRENT_BINARY_DIR}/..>")

########### install files ###############

install( FILES ${CMAKE_CURRENT_BINARY_DIR}/solid_export.h solidnamespace.h device.h
               devicenotifier.h deviceinterface.h genericinterface.h processor.h block.h
               storageaccess.h storagedrive.h opticaldrive.h storagevolume.h opticaldisc.h
               camera.h portablemediaplayer.h networkinterface.h acadapter.h battery.h
               button.h audiointerface.h dvbinterface.h predicate.h powermanagement.h
               networking.h video.h serialinterface.h smartcardreader.h internetgateway.h
               networkshare.h
         DESTINATION ${INCLUDE_INSTALL_DIR}/solid COMPONENT Devel)

########### parser build ###############

set(lexer_FILE predicate_lexer)
set(parser_FILE predicate_parser)

find_package(FLEX)
set_package_properties(FLEX PROPERTIES DESCRIPTION "Allows the Solid predicate parser to be updated"
                       URL "http://flex.sourceforge.net"
                       TYPE OPTIONAL
                       PURPOSE "Required by the UpdateSolidPredicateParser target (mainly useful for developers)"
                      )

find_package(BISON)
set_package_properties(BISON PROPERTIES
    URL "http://www.gnu.org/software/bison"
    DESCRIPTION "Allows the Solid predicate parser to be updated"
    TYPE OPTIONAL
    PURPOSE "Required by the UpdateSolidPredicateParser target (mainly useful for developers)"
  )

if (FLEX_FOUND AND BISON_FOUND)

   add_custom_target(UpdateSolidPredicateParser
                     COMMAND ${FLEX_EXECUTABLE} -P Solid -o${lexer_FILE}.c ${lexer_FILE}.l
                     COMMAND ${BISON_EXECUTABLE} -p Solid -d -b ${parser_FILE} ${parser_FILE}.y
                     COMMAND ${CMAKE_COMMAND} -E copy ${parser_FILE}.tab.c ${CMAKE_CURRENT_SOURCE_DIR}/${parser_FILE}.c
                     COMMAND ${CMAKE_COMMAND} -E copy ${parser_FILE}.tab.h ${CMAKE_CURRENT_SOURCE_DIR}/${parser_FILE}.h
                     COMMAND ${CMAKE_COMMAND} -E remove ${parser_FILE}.tab.c ${parser_FILE}.tab.h
                     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

else ()
   add_custom_target(UpdateSolidPredicateParser
                     COMMAND echo "flex and/or bison not found, so target UpdateSolidPredicateParser inactive")
endif ()

