
project(kdeprint)

add_definitions(-D_KDEPRINT_COMPILE ${QDBUS_DEFINITIONS})

# Needed for the subdirs
include_directories( ${KDE4_KIO_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )


# you can find macro_optional_find_package() in kdelibs/cmake/modules/OptionalFindPackage.cmake
# it is the same as FIND_PACKAGE(<name>) but additionally creates an OPTION(WITH_<name>)
# so the checking for the software can be disabled via ccmake or -DWITH_<name>=OFF
macro_optional_find_package(Cups)

add_subdirectory( management )
add_subdirectory( lpdunix )
add_subdirectory( lpr )
add_subdirectory( rlpr )
add_subdirectory( ext )
add_subdirectory( filters )
add_subdirectory( pics )
add_subdirectory( tools )
add_subdirectory( tests )

if(CUPS_FOUND)
   add_subdirectory(cups)
endif(CUPS_FOUND)


include_directories( ${ZLIB_INCLUDE_DIR} )


########### next target ###############

set(kded_kdeprintd_PART_SRCS kdeprintd.cpp kprintprocess.cpp )

kde4_automoc(${kded_kdeprintd_PART_SRCS})

kde4_add_plugin(kded_kdeprintd ${kded_kdeprintd_PART_SRCS})

target_link_libraries(kded_kdeprintd  ${KDE4_KDECORE_LIBS} kio )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kded_kdeprintd )
install_targets(${PLUGIN_INSTALL_DIR} kded_kdeprintd )


########### next target ###############

set(kdeprint_LIB_SRCS
   kmfactory.cpp
   kmfactoryiface.cpp
   kmfactoryadaptor.cpp
   kmjob.cpp
   kmjobmanager.cpp
   kmmanager.cpp
   kmprinter.cpp
   kmuimanager.cpp
   kmvirtualmanager.cpp
   driver.cpp
   driveritem.cpp
   droptionview.cpp
   driverview.cpp
   kmdbentry.cpp
   kprintpreview.cpp
   kprintdialog.cpp
   kprinter.cpp
   kprinterimpl.cpp
   kprintdialogpage.cpp
   kpcopiespage.cpp
   kpdriverpage.cpp
   kprinterpropertydialog.cpp
   kpgeneralpage.cpp
   kpqtpage.cpp
   kmthreadjob.cpp
   kpipeprocess.cpp
   kpfilterpage.cpp
   kmspecialmanager.cpp
   kdeprintcheck.cpp
   plugincombobox.cpp
   kpreloadobject.cpp
   kfilelist.cpp
   kprintaction.cpp
   kpfileselectpage.cpp
   treecombobox.cpp
   kxmlcommand.cpp
   util.cpp
   printerfilter.cpp
   marginpreview.cpp
   marginvaluewidget.cpp
   marginwidget.cpp
   kpmarginpage.cpp
   messagewindow.cpp
   posterpreview.cpp
   kpposterpage.cpp
   foomatic2loader.cpp
   fooparser.cpp
   fooscanner.cpp
   ppdparser.cpp
   ppdscanner.cpp
   ppdloader.cpp
   )

kde4_automoc(${kdeprint_LIB_SRCS})

kde4_add_library(kdeprint SHARED ${kdeprint_LIB_SRCS})

target_link_libraries(kdeprint  ${KDE4_KDECORE_LIBS} kparts )

set_target_properties(kdeprint PROPERTIES VERSION 5.0.0 SOVERSION 5 )
install_targets(${LIB_INSTALL_DIR} kdeprint )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kdeprint FILES testprint.ps specials.desktop )
install_files( ${CONFIG_INSTALL_DIR} FILES kdeprintrc )
install_files( ${SERVICES_INSTALL_DIR}/kded FILES kdeprintd.desktop )
install_files( ${CONFIG_INSTALL_DIR}/ui FILES kprintpreviewui.rc )
install_files( ${DATA_INSTALL_DIR}/kconf_update FILES kdeprintrc.upd )
install_files( /include FILES kprinter.h kprintaction.h )
install_files( /include/kdeprint FILES kprintdialogpage.h kpreloadobject.h kmmanager.h kmprinter.h kmobject.h kprintdialog.h driver.h kmjobmanager.h kmjob.h )




