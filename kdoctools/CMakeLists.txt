project(kdoctools)
find_package(LibXml2 REQUIRED)
find_package(LibXslt REQUIRED)

include_directories( ${KDE4_KIO_INCLUDES} )
#include_directories( ${CMAKE_SOURCE_DIR}/kioslave ${CMAKE_SOURCE_DIR}/kioslave/bzip2 )

include_directories( ${LIBXML2_INCLUDE_DIR} ${LIBXSLT_INCLUDE_DIR} )

configure_file(checkXML.in.cmake ${CMAKE_CURRENT_BINARY_DIR}/checkXML )
configure_file(config-kdoctools.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kdoctools.h )
#macro_additional_clean_files( ${CMAKE_CURRENT_BINARY_DIR}/checkXML )


########### next target ###############

set(kio_help_PART_SRCS
   kio_help.cpp
   main.cpp
   xslt.cpp
)


kde4_add_plugin(kio_help ${kio_help_PART_SRCS})

target_link_libraries(kio_help  ${KDE4_KDECORE_LIBS} kio ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES} )

install(TARGETS kio_help  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### next target ###############

set(kio_ghelp_PART_SRCS kio_help.cpp main_ghelp.cpp xslt.cpp )


kde4_add_plugin(kio_ghelp ${kio_ghelp_PART_SRCS})

target_link_libraries(kio_ghelp  ${KDE4_KDECORE_LIBS} kio ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES} )

install(TARGETS kio_ghelp  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### next target ###############

set(meinproc_SRCS meinproc.cpp xslt.cpp )


kde4_add_executable(meinproc4 NOGUI RUN_UNINSTALLED ${meinproc_SRCS})

target_link_libraries(meinproc4 kio ${KDE4_KDECORE_LIBS} ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES})

if(KDE4_ENABLE_FINAL)
   set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/meinproc4_final_cpp.cpp PROPERTIES COMPILE_FLAGS -DSIMPLE_XSLT )
else(KDE4_ENABLE_FINAL)
   set_source_files_properties(xslt.cpp PROPERTIES COMPILE_FLAGS -DSIMPLE_XSLT )
endif(KDE4_ENABLE_FINAL)

install(TARGETS meinproc4 ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### next target ###############

set(genshortcutents_SRCS genshortcutents.cpp )


kde4_add_executable(genshortcutents NOGUI ${genshortcutents_SRCS})

target_link_libraries(genshortcutents ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS})


########### install files ###############

install( FILES help.protocol ghelp.protocol  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES kio_help.upd  DESTINATION  ${DATA_INSTALL_DIR}/kconf_update )

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/checkXML DESTINATION ${BIN_INSTALL_DIR} )

if(NOT WIN32)
  install( FILES kio_help.upd  DESTINATION  ${KCONF_UPDATE_INSTALL_DIR} )
  install( PROGRAMS move_kio_help_cache.sh  DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )
endif(NOT WIN32)

macro(INSTALL_CUSTOMIZE_DIR _expr _directory)
    FILE(GLOB _file_CUST "${_expr}" )
    foreach(_curentfile ${_file_CUST})
        install(FILES ${_curentfile} DESTINATION ${DATA_INSTALL_DIR}/ksgmltools2/${_directory} )
    endforeach(_curentfile ${_file_CUST})
endmacro(INSTALL_CUSTOMIZE_DIR _expr _directory)

set(docbookdir xml-dtd-4.1.2  xml-dtd-4.2  xsl )


set(expr "docbook/README.kde")
INSTALL_CUSTOMIZE_DIR( ${expr} docbook/)

foreach (_currentdocbookdir ${docbookdir})
	set(expr "docbook/${_currentdocbookdir}/*.mod")
	install_customize_dir( ${expr} docbook/${_currentdocbookdir}/)
	set(expr "docbook/${_currentdocbookdir}/*.txt")
	install_customize_dir( ${expr} docbook/${_currentdocbookdir}/)
	set(expr "docbook/${_currentdocbookdir}/*.dtd")
    install_customize_dir( ${expr} docbook/${_currentdocbookdir}/)
	set(expr "docbook/${_currentdocbookdir}/*.cat")
    install_customize_dir( ${expr} docbook/${_currentdocbookdir}/)
	set(expr "docbook/${_currentdocbookdir}/README*")
	install_customize_dir( ${expr} docbook/${_currentdocbookdir}/)
	set(expr "docbook/${_currentdocbookdir}/Changelog*")
	install_customize_dir( ${expr} docbook/${_currentdocbookdir}/)
	set(expr "docbook/${_currentdocbookdir}/*.xml")
	install_customize_dir( ${expr} docbook/${_currentdocbookdir}/)
	set(expr "docbook/${_currentdocbookdir}/VERSION*")
	install_customize_dir( ${expr} docbook/${_currentdocbookdir}/)
	set(expr "docbook/${_currentdocbookdir}/What*")
	install_customize_dir( ${expr} docbook/${_currentdocbookdir}/)
	
	set(expr "docbook/${_currentdocbookdir}/ent/*.ent")
	install_customize_dir( ${expr} docbook/${_currentdocbookdir}/ent/)

	set(expr "docbook/${_currentdocbookdir}/common/*.xsl")
	install_customize_dir( ${expr} docbook/${_currentdocbookdir}/common/)
	set(expr "docbook/${_currentdocbookdir}/common/*.xml")
	install_customize_dir( ${expr} docbook/${_currentdocbookdir}/common/)
	set(expr "docbook/${_currentdocbookdir}/common/*.dtd")
	install_customize_dir( ${expr} docbook/${_currentdocbookdir}/common/)
	set(expr "docbook/${_currentdocbookdir}/common/Changelog")
	install_customize_dir( ${expr} docbook/${_currentdocbookdir}/common/)

    set(expr "docbook/${_currentdocbookdir}/html/*.xsl")
    install_customize_dir( ${expr} docbook/${_currentdocbookdir}/html/)
    set(expr "docbook/${_currentdocbookdir}/html/*.xml")
    install_customize_dir( ${expr} docbook/${_currentdocbookdir}/html/)
    set(expr "docbook/${_currentdocbookdir}/html/*.dtd")
    install_customize_dir( ${expr} docbook/${_currentdocbookdir}/html/)
	set(expr "docbook/${_currentdocbookdir}/html/Changelog")
	install_customize_dir( ${expr} docbook/${_currentdocbookdir}/html/)
	set(expr "docbook/${_currentdocbookdir}/html/*.ent")
	install_customize_dir( ${expr} docbook/${_currentdocbookdir}/html/)
	set(expr "docbook/${_currentdocbookdir}/html/*.xweb")
	install_customize_dir( ${expr} docbook/${_currentdocbookdir}/html/)

    set(expr "docbook/${_currentdocbookdir}/images/*.png")
    install_customize_dir( ${expr} docbook/${_currentdocbookdir}/images/)
	set(expr "docbook/${_currentdocbookdir}/images/*.gif")
	install_customize_dir( ${expr} docbook/${_currentdocbookdir}/images/)
	set(expr "docbook/${_currentdocbookdir}/images/*.tif")
	install_customize_dir( ${expr} docbook/${_currentdocbookdir}/images/)
	set(expr "docbook/${_currentdocbookdir}/images/Changelog")
	install_customize_dir( ${expr} docbook/${_currentdocbookdir}/images/)

	set(expr "docbook/${_currentdocbookdir}/images/callouts/*.png")
	install_customize_dir( ${expr} docbook/${_currentdocbookdir}/images/callouts/)
    set(expr "docbook/${_currentdocbookdir}/images/callouts/*.gif")
    install_customize_dir( ${expr} docbook/${_currentdocbookdir}/images/callouts/)
	set(expr "docbook/${_currentdocbookdir}/images/callouts/Changelog")
	install_customize_dir( ${expr} docbook/${_currentdocbookdir}/images/callouts/)

	set(expr "docbook/${_currentdocbookdir}/lib/lib.*")
	install_customize_dir( ${expr} docbook/${_currentdocbookdir}/lib/)
	set(expr "docbook/${_currentdocbookdir}/lib/Changel*")
	install_customize_dir( ${expr} docbook/${_currentdocbookdir}/lib/)
	set(expr "docbook/${_currentdocbookdir}/lib/README*")
	install_customize_dir( ${expr} docbook/${_currentdocbookdir}/lib/)

	set(expr "docbook/${_currentdocbookdir}/manpages/*.xsl")
	install_customize_dir( ${expr} docbook/${_currentdocbookdir}/manpages/)

	set(expr "docbook/${_currentdocbookdir}/params/*.xml")
	install_customize_dir( ${expr} docbook/${_currentdocbookdir}/params/)
	set(expr "docbook/${_currentdocbookdir}/params/Changelog")
	install_customize_dir( ${expr} docbook/${_currentdocbookdir}/params/)
	

endforeach(_currentdocbookdir ${docbookdir})


set(expr "customization/*.xsl")
install_customize_dir( ${expr} customization/)

set(expr "customization/README")
install_customize_dir( ${expr} customization/)

set(expr "customization/catalog.xml")
install_customize_dir( ${expr} customization/)

set(customizedir af bg ca cs da de dtd el en-GB en entities eo es et fi fo fr gl he hu id it ja ko lt nl nn no obsolete pl pt-BR pt ro ru sk sl sr sr@latin sv th tr uk wa xh xsl xx zh-CN zh-TW)
 
foreach (_currentcustomizedir ${customizedir})
	set(expr "customization/${_currentcustomizedir}/README")
	install_customize_dir( ${expr} customization/${_currentcustomizedir})
	set(expr "customization/${_currentcustomizedir}/*.entities")
	install_customize_dir( ${expr} customization/${_currentcustomizedir})
	set(expr "customization/${_currentcustomizedir}/*.xsl")
	install_customize_dir( ${expr} customization/${_currentcustomizedir})
	set(expr "customization/${_currentcustomizedir}/obsolete/*.elements")
	install_customize_dir( ${expr} customization/${_currentcustomizedir}/obsolete/)
	set(expr "customization/${_currentcustomizedir}/obsolete/*.dtd")
	install_customize_dir( ${expr} customization/${_currentcustomizedir}/obsolete/)
    set(expr "customization/${_currentcustomizedir}/entities/*.docbook")
	install_customize_dir( ${expr} customization/${_currentcustomizedir}/entities/ )
	set(expr "customization/${_currentcustomizedir}/entities/*.entities")
	install_customize_dir( ${expr} customization/${_currentcustomizedir}/entities/ )
    set(expr "customization/${_currentcustomizedir}/*.xml")
    install_customize_dir(${expr} customization/${_currentcustomizedir} )
	set(expr "customization/${_currentcustomizedir}/*.dtd")
	install_customize_dir(${expr} customization/${_currentcustomizedir}/)
	set(expr "customization/${_currentcustomizedir}/*.elements")
	install_customize_dir(${expr} customization/${_currentcustomizedir}/)
	
endforeach(_currentcustomizedir ${customizedir}) 
	  




