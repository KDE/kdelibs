
project(kstyle-keramik)

add_definitions(-DQT_PLUGIN)

include_directories( ${CMAKE_SOURCE_DIR}/kdefx )

########### next target ###############

set(genembed_SRCS genembed.cpp )

kde4_automoc(${genembed_SRCS})

kde4_add_executable(genembed NOGUI RUN_UNINSTALLED ${genembed_SRCS})

target_link_libraries(genembed  ${KDE4_KDECORE_LIBS} kdefx )

#pixmaps.keramik keramikrc.h: genembed
#	pics=`ls $(srcdir)/pics/*.png 2>/dev/null` ;\
#	./genembed $$pics > pixmaps.keramik

file(GLOB keramikPics "${CMAKE_CURRENT_SOURCE_DIR}/pics/*.png")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/keramikPics.txt "" )
foreach(pics ${keramikPics})
   file(APPEND  ${CMAKE_CURRENT_BINARY_DIR}/keramikPics.txt "${pics}\n" )
endforeach(pics ${keramikPics})

# get the name of the generated wrapper script (which sets up LD_LIBRARY_PATH)
get_target_property(GENEMBED_EXECUTABLE genembed WRAPPER_SCRIPT)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/keramikrc.h ${CMAKE_CURRENT_BINARY_DIR}/pixmaps.keramik
  COMMAND ${GENEMBED_EXECUTABLE} --file ${CMAKE_CURRENT_BINARY_DIR}/keramikPics.txt > ${CMAKE_CURRENT_BINARY_DIR}/pixmaps.keramik
  DEPENDS genembed ${keramikPics}
)

########### next target ###############

set(keramik_PART_SRCS
   keramik.cpp
   pixmaploader.cpp
   gradients.cpp
   colorutil.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/keramikrc.h  # adding the generated header here will force the correct generation of the dependencies to genembed
)

# this actually shouldn't be required, maybe current cmake cvs handles it correctly, Alex
#set_source_files_properties(keramik.cpp PROPERTIES OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/keramikrc.h)

if (WIN32)
   set_source_files_properties(keramik.cpp PROPERTIES OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/keramikrc.h)
   # remove the ifdef when there are linker problems with mingw  
   if(MSVC)
      add_definitions(-DKDEFX_EXPORT=KDE_EXPORT)
   endif(MSVC)
endif (WIN32)

kde4_automoc(${keramik_PART_SRCS})

kde4_add_plugin(keramik ${keramik_PART_SRCS})

target_link_libraries(keramik  ${KDE4_KDECORE_LIBS} kdefx )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR}/plugins/styles keramik )
install(TARGETS keramik  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/styles/ )


########### install files ###############





