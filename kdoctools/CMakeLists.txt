project(kdoctools)

find_package(LibXslt)
macro_log_feature(LIBXSLT_FOUND "LibXSLT" "A library to transform XML files into other XML files" "http://xmlsoft.org/XSLT" TRUE "" "Required for building the KDE help system")

find_package(LibXml2)
macro_log_feature(LIBXML2_FOUND "LibXML2" "Libraries used to develop XML applications" "http://xmlsoft.org" TRUE "" "Needed for building the KDE help system.")

include_directories( ${KDE4_KIO_INCLUDES} )
#include_directories( ${CMAKE_SOURCE_DIR}/kioslave ${CMAKE_SOURCE_DIR}/kioslave/bzip2 )

include_directories( ${LIBXML2_INCLUDE_DIR} ${LIBXSLT_INCLUDE_DIR} )

configure_file(checkXML.in.cmake ${CMAKE_CURRENT_BINARY_DIR}/checkXML )
#macro_additional_clean_files( ${CMAKE_CURRENT_BINARY_DIR}/checkXML )

find_package(DocBookXML)
set (DOCBOOKXML_MSG "XML DTDs for DocBook ${DOCBOOKXML_CURRENTDTD_VERSION}")
if (DOCBOOKXML_OLDDTD_VERSION)
  set (DOCBOOKXML_MSG "${DOCBOOKXML_MSG} and ${DOCBOOKXML_OLDDTD_VERSION}")
endif (DOCBOOKXML_OLDDTD_VERSION)
macro_log_feature(DOCBOOKXML_FOUND "DocBook XML" "${DOCBOOKXML_MSG}" "http://www.oasis-open.org/docbook/xml/${DOCBOOKXML_CURRENTDTD_VERSION}" TRUE "" "Needed for building the KDE help system.")

find_package(DocBookXSL)
macro_log_feature(DOCBOOKXSL_FOUND "DocBook XSL" "Stylesheets for processing DocBook XML" "http://docbook.sourceforge.net/release/xsl/current/" TRUE "" "Needed for building the KDE help system.")

set (_custom_dtd_kdex "customization/dtd/kdex.dtd")
configure_file(${_custom_dtd_kdex}.cmake ${CMAKE_CURRENT_BINARY_DIR}/${_custom_dtd_kdex} )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${_custom_dtd_kdex}
        DESTINATION ${DATA_INSTALL_DIR}/ksgmltools2/customization/dtd )

configure_file(config-kdoctools.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kdoctools.h )

########### next target ###############

set(kio_help_PART_SRCS
   kio_help.cpp
   main.cpp
   xslt.cpp
   xslt_help.cpp
   xslt_kde.cpp
)


kde4_add_plugin(kio_help ${kio_help_PART_SRCS})

target_link_libraries(kio_help  ${KDE4_KDECORE_LIBS} kio ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES} )

install(TARGETS kio_help  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### next target ###############

set(kio_ghelp_PART_SRCS kio_help.cpp main_ghelp.cpp xslt.cpp xslt_help.cpp xslt_kde.cpp )


kde4_add_plugin(kio_ghelp ${kio_ghelp_PART_SRCS})

target_link_libraries(kio_ghelp  ${KDE4_KDECORE_LIBS} kio ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES} )

install(TARGETS kio_ghelp  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### next target ###############

if(KDE4_ENABLE_FINAL)
   set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/meinproc4_final_cpp.cpp PROPERTIES COMPILE_FLAGS -DSIMPLE_XSLT )
else(KDE4_ENABLE_FINAL)
   set_source_files_properties(xslt.cpp PROPERTIES COMPILE_FLAGS -DSIMPLE_XSLT )
endif(KDE4_ENABLE_FINAL)

# The cross compiling parts are commented out on purpose. Alex
# if (CMAKE_CROSSCOMPILING)
#    set(IMPORT_MEINPROC4_EXECUTABLE "${KDE_HOST_TOOLS_PATH}/ImportMeinProc4Executable.cmake" CACHE FILEPATH "Point it to the export file of meinproc4 from a native build")
#    include(${IMPORT_MEINPROC4_EXECUTABLE})
#
# else (CMAKE_CROSSCOMPILING)

   set(meinproc_SRCS meinproc.cpp meinproc_common.cpp xslt.cpp xslt_kde.cpp )

   kde4_add_executable(meinproc4 NOGUI ${meinproc_SRCS})

   target_link_libraries(meinproc4 ${KDE4_KDECORE_LIBS} ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES})

   # "export" this target too so we can use the LOCATION property of the imported target in
   # FindKDE4Internal.cmake to get the full path to the installed executable instead of using FIND_PROGRAM(), Alex
   install(TARGETS meinproc4  EXPORT kdelibsToolsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

#    # export this binary for cross-compilation
#    export(TARGETS meinproc4 FILE ${CMAKE_BINARY_DIR}/ImportMeinProc4Executable.cmake)
# 
# endif (CMAKE_CROSSCOMPILING)

########### next target ###############

if(KDE4_ENABLE_FINAL)
   set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/meinproc4_simple_final_cpp.cpp PROPERTIES COMPILE_FLAGS -DSIMPLE_XSLT )
else(KDE4_ENABLE_FINAL)
   set_source_files_properties(xslt.cpp PROPERTIES COMPILE_FLAGS -DSIMPLE_XSLT )
endif(KDE4_ENABLE_FINAL)

if (NOT CMAKE_CROSSCOMPILING)
   set(meinproc_qt_SRCS meinproc_simple.cpp meinproc_common.cpp xslt.cpp )

   kde4_add_executable(meinproc4_simple NOGUI ${meinproc_qt_SRCS})

   target_link_libraries(meinproc4_simple ${QT_QTCORE_LIBRARIES} ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES})

   install(TARGETS meinproc4_simple ${INSTALL_TARGETS_DEFAULT_ARGS})

endif (NOT CMAKE_CROSSCOMPILING)

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

macro(INSTALL_CUSTOMIZE_L_DIR _expr _directory)
    FILE(GLOB _file_CUST "${_expr}" )
    foreach(_curentfile ${_file_CUST})
        # copy the file also under CMAKE_CURRENT_BINARY_DIR - those
        # files are needed to generate the documentation without installing
        # kdelibs (i.e. when kdelibs itself compiled).
        get_filename_component(_currentfile_name ${_curentfile} NAME)
        set(_currentfile_binary_path ${CMAKE_CURRENT_BINARY_DIR}/${_directory}/${_currentfile_name})
        configure_file(${_curentfile} ${_currentfile_binary_path} COPYONLY)

        install(FILES ${_currentfile_binary_path}
                DESTINATION ${DATA_INSTALL_DIR}/ksgmltools2/${_directory} )
    endforeach(_curentfile ${_file_CUST})
endmacro(INSTALL_CUSTOMIZE_L_DIR _expr _directory)

set(docbookdir xsl )

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
	install_customize_l_dir( ${expr} docbook/${_currentdocbookdir}/)
	set(expr "docbook/${_currentdocbookdir}/What*")
	install_customize_dir( ${expr} docbook/${_currentdocbookdir}/)
	
	set(expr "docbook/${_currentdocbookdir}/ent/*.ent")
	install_customize_dir( ${expr} docbook/${_currentdocbookdir}/ent/)

	set(expr "docbook/${_currentdocbookdir}/common/*.xsl")
	install_customize_l_dir( ${expr} docbook/${_currentdocbookdir}/common/)
	set(expr "docbook/${_currentdocbookdir}/common/*.xml")
	install_customize_l_dir( ${expr} docbook/${_currentdocbookdir}/common/)
	set(expr "docbook/${_currentdocbookdir}/common/*.dtd")
	install_customize_l_dir( ${expr} docbook/${_currentdocbookdir}/common/)
	set(expr "docbook/${_currentdocbookdir}/common/Changelog")
	install_customize_l_dir( ${expr} docbook/${_currentdocbookdir}/common/)

    set(expr "docbook/${_currentdocbookdir}/html/*.xsl")
    install_customize_l_dir( ${expr} docbook/${_currentdocbookdir}/html/)
    set(expr "docbook/${_currentdocbookdir}/html/*.xml")
    install_customize_l_dir( ${expr} docbook/${_currentdocbookdir}/html/)
    set(expr "docbook/${_currentdocbookdir}/html/*.dtd")
    install_customize_l_dir( ${expr} docbook/${_currentdocbookdir}/html/)
	set(expr "docbook/${_currentdocbookdir}/html/Changelog")
	install_customize_l_dir( ${expr} docbook/${_currentdocbookdir}/html/)
	set(expr "docbook/${_currentdocbookdir}/html/*.ent")
	install_customize_l_dir( ${expr} docbook/${_currentdocbookdir}/html/)
	set(expr "docbook/${_currentdocbookdir}/html/*.xweb")
	install_customize_l_dir( ${expr} docbook/${_currentdocbookdir}/html/)

    set(expr "docbook/${_currentdocbookdir}/images/*.png")
    install_customize_l_dir( ${expr} docbook/${_currentdocbookdir}/images/)
	set(expr "docbook/${_currentdocbookdir}/images/*.gif")
	install_customize_l_dir( ${expr} docbook/${_currentdocbookdir}/images/)
	set(expr "docbook/${_currentdocbookdir}/images/*.tif")
	install_customize_l_dir( ${expr} docbook/${_currentdocbookdir}/images/)
	set(expr "docbook/${_currentdocbookdir}/images/Changelog")
	install_customize_l_dir( ${expr} docbook/${_currentdocbookdir}/images/)

	set(expr "docbook/${_currentdocbookdir}/images/callouts/*.png")
	install_customize_l_dir( ${expr} docbook/${_currentdocbookdir}/images/callouts/)
    set(expr "docbook/${_currentdocbookdir}/images/callouts/*.gif")
    install_customize_l_dir( ${expr} docbook/${_currentdocbookdir}/images/callouts/)
	set(expr "docbook/${_currentdocbookdir}/images/callouts/Changelog")
	install_customize_l_dir( ${expr} docbook/${_currentdocbookdir}/images/callouts/)

	set(expr "docbook/${_currentdocbookdir}/lib/lib.*")
	install_customize_l_dir( ${expr} docbook/${_currentdocbookdir}/lib/)
	set(expr "docbook/${_currentdocbookdir}/lib/Changel*")
	install_customize_l_dir( ${expr} docbook/${_currentdocbookdir}/lib/)
	set(expr "docbook/${_currentdocbookdir}/lib/README*")
	install_customize_l_dir( ${expr} docbook/${_currentdocbookdir}/lib/)

	set(expr "docbook/${_currentdocbookdir}/manpages/*.xsl")
	install_customize_l_dir( ${expr} docbook/${_currentdocbookdir}/manpages/)

	set(expr "docbook/${_currentdocbookdir}/params/*.xml")
	install_customize_l_dir( ${expr} docbook/${_currentdocbookdir}/params/)
	set(expr "docbook/${_currentdocbookdir}/params/Changelog")
	install_customize_l_dir( ${expr} docbook/${_currentdocbookdir}/params/)
	

endforeach(_currentdocbookdir ${docbookdir})


set(expr "customization/*.xsl")
install_customize_l_dir( ${expr} customization/)

set(expr "customization/README")
install_customize_l_dir( ${expr} customization/)

set(expr "customization/catalog.xml")
install_customize_l_dir( ${expr} customization/)

set(customizedir af bg ca cs da de dtd el en-GB en entities eo es et fi fo fr gl he hu id it ja ko lt nds nl nn no pl pt-BR pt ro ru sk sl sr sr@latin sr@ijekavian sr@ijekavianlatin sv th tr uk wa xh xsl xx zh-CN zh-TW)
 
foreach (_currentcustomizedir ${customizedir})
	set(expr "customization/${_currentcustomizedir}/README")
	install_customize_l_dir( ${expr} customization/${_currentcustomizedir})
	set(expr "customization/${_currentcustomizedir}/*.entities")
	install_customize_l_dir( ${expr} customization/${_currentcustomizedir})
	set(expr "customization/${_currentcustomizedir}/*.xsl")
	install_customize_l_dir( ${expr} customization/${_currentcustomizedir})
    set(expr "customization/${_currentcustomizedir}/entities/*.docbook")
	install_customize_l_dir( ${expr} customization/${_currentcustomizedir}/entities/ )
	set(expr "customization/${_currentcustomizedir}/entities/*.entities")
	install_customize_l_dir( ${expr} customization/${_currentcustomizedir}/entities/ )
    set(expr "customization/${_currentcustomizedir}/*.xml")
    install_customize_l_dir(${expr} customization/${_currentcustomizedir} )
	set(expr "customization/${_currentcustomizedir}/*.dtd")
	install_customize_l_dir(${expr} customization/${_currentcustomizedir}/)
	set(expr "customization/${_currentcustomizedir}/*.elements")
	install_customize_l_dir(${expr} customization/${_currentcustomizedir}/)
	
endforeach(_currentcustomizedir ${customizedir}) 
	  




