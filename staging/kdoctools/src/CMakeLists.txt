find_package(LibXslt)
set_package_properties(LibXslt PROPERTIES
                       URL "http://xmlsoft.org/XSLT"
                       TYPE REQUIRED
                       PURPOSE "Required by the KDE help system to process DocBook XML"
                      )

find_package(LibXml2)
set_package_properties(LibXml2 PROPERTIES
                       URL "http://xmlsoft.org"
                       TYPE REQUIRED
                       PURPOSE "Required by the KDE help system to process DocBook XML"
                      )

if(NOT LIBXML2_XMLLINT_EXECUTABLE)
  message(FATAL_ERROR "xmllint is required by the KDE help system to process DocBook XML")
endif()

include_directories( ${LIBXML2_INCLUDE_DIR} ${LIBXSLT_INCLUDE_DIR} )

find_package(DocBookXML)
set (DOCBOOKXML_VERSIONS "${DOCBOOKXML_CURRENTDTD_VERSION}")
if (DOCBOOKXML_OLDDTD_VERSION)
   set (DOCBOOKXML_VERSIONS "${DOCBOOKXML_VERSIONS} and ${DOCBOOKXML_OLDDTD_VERSION}")
endif (DOCBOOKXML_OLDDTD_VERSION)
set_package_properties(DocBookXML PROPERTIES DESCRIPTION "DocBook XML"
                       URL "http://www.oasis-open.org/docbook/xml/${DOCBOOKXML_CURRENTDTD_VERSION}"
                       TYPE REQUIRED
                       PURPOSE "Required by the KDE help system to process DocBook XML"
                      )

find_package(DocBookXSL)
set_package_properties(DocBookXSL PROPERTIES DESCRIPTION "DocBook XSL"
                       URL "http://docbook.sourceforge.net/release/xsl/current/"
                       TYPE REQUIRED
                       PURPOSE "Required by the KDE help system to process DocBook XML"
                      )

find_package(Qt5Core 5.2.0 REQUIRED NO_MODULE)

set (_custom_dtd_kdex "customization/dtd/kdex.dtd")
configure_file(${_custom_dtd_kdex}.cmake ${CMAKE_CURRENT_BINARY_DIR}/${_custom_dtd_kdex} )
# WARNING: this is due to severe limitations on the windows platform to keep the dtd relocateable
# see the .xsl.cmake files for a more verbose explanation below
if(NOT WIN32)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${_custom_dtd_kdex}
            DESTINATION ${DATA_INSTALL_DIR}/ksgmltools2/customization/dtd )
else(NOT WIN32)
    install(CODE
 "FILE(RELATIVE_PATH DOCBOOKXML_CURRENTDTD_DIR \${CMAKE_INSTALL_PREFIX}/${DATA_INSTALL_DIR}/ksgmltools2/customization/dtd ${DOCBOOKXML_CURRENTDTD_DIR})
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/${_custom_dtd_kdex}.cmake ${CMAKE_CURRENT_BINARY_DIR}/${_custom_dtd_kdex}.install)")
    get_filename_component(_custom_dtd_kdex_name ${_custom_dtd_kdex} NAME)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${_custom_dtd_kdex}.install DESTINATION ${DATA_INSTALL_DIR}/ksgmltools2/customization/dtd RENAME ${_custom_dtd_kdex_name})
endif(NOT WIN32)

########## next target ###############

set(xslt_PART_SRCS
    xslt.cpp
    xslt_kde.cpp
)

ADD_LIBRARY( xslt_kde STATIC
    ${xslt_PART_SRCS}
)

target_link_libraries(xslt_kde Qt5::Core ${KArchive_LIBRARIES})

########### next target ###############

set_source_files_properties(xslt.cpp PROPERTIES COMPILE_FLAGS -DSIMPLE_XSLT )

# The cross compiling parts are commented out on purpose. Alex
# if (CMAKE_CROSSCOMPILING)
#    set(IMPORT_MEINPROC4_EXECUTABLE "${KDE_HOST_TOOLS_PATH}/ImportMeinProc4Executable.cmake" CACHE FILEPATH "Point it to the export file of meinproc4 from a native build")
#    include(${IMPORT_MEINPROC4_EXECUTABLE})
#
# else (CMAKE_CROSSCOMPILING)

   set(meinproc_SRCS meinproc.cpp meinproc_common.cpp xslt.cpp xslt_kde.cpp )

   kde4_add_executable(meinproc4 NOGUI ${meinproc_SRCS})

   target_link_libraries(meinproc4 kdeqt5staging ki18n ${KArchive_LIBRARIES} ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES} ${LIBXSLT_EXSLT_LIBRARIES} )

   add_dependencies(meinproc4 docbookl10nhelper)

   # "export" this target too so we can use the LOCATION property of the imported target in
   # FindKDE4Internal.cmake to get the full path to the installed executable instead of using FIND_PROGRAM(), Alex
   install(TARGETS meinproc4  EXPORT kdelibsToolsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

#    # export this binary for cross-compilation
#    export(TARGETS meinproc4 FILE ${CMAKE_BINARY_DIR}/ImportMeinProc4Executable.cmake)
#
# endif (CMAKE_CROSSCOMPILING)

########### next target ###############

set_source_files_properties(xslt.cpp PROPERTIES COMPILE_FLAGS -DSIMPLE_XSLT )

if (NOT CMAKE_CROSSCOMPILING)
   set(meinproc_qt_SRCS meinproc_simple.cpp meinproc_common.cpp xslt.cpp )

   kde4_add_executable(meinproc4_simple NOGUI ${meinproc_qt_SRCS})

   target_link_libraries(meinproc4_simple kdeqt5staging Qt5::Core ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES} ${LIBXSLT_EXSLT_LIBRARIES})

   add_dependencies(meinproc4_simple docbookl10nhelper)

   install(TARGETS meinproc4_simple ${INSTALL_TARGETS_DEFAULT_ARGS})

endif (NOT CMAKE_CROSSCOMPILING)

########### customize includes for xslt ###############
# WARNING: this has severe limitations on the windows platform since the .xsl
# & docbook files need to be relocatable. We cannot just replace the paths in
# the .xsl with relative paths because these files are not installed while
# building kdelibs. Instead, we will reconfigure those files at install time
# with relative paths.

file( GLOB _kdeincludexslt_ALL "customization/kde-include*.xsl.cmake" )
foreach( _kdeincludexslt ${_kdeincludexslt_ALL} )
    get_filename_component( _kdeincludexslt_name ${_kdeincludexslt} NAME )
    string( REGEX REPLACE ".cmake$" "" _kdeincludexslt_out "${_kdeincludexslt_name}" )
    set( _kdeincludexslt_binarypath "${CMAKE_CURRENT_BINARY_DIR}/customization/${_kdeincludexslt_out}" )
    configure_file( ${_kdeincludexslt} ${_kdeincludexslt_binarypath} )
    if(NOT WIN32)
        install(FILES ${_kdeincludexslt_binarypath}
                DESTINATION ${DATA_INSTALL_DIR}/ksgmltools2/customization )
    else(NOT WIN32)
        install(CODE 
 "FILE(RELATIVE_PATH DOCBOOKXSL_DIR \${CMAKE_INSTALL_PREFIX}/${DATA_INSTALL_DIR}/ksgmltools2/customization ${DOCBOOKXSL_DIR})
  CONFIGURE_FILE( ${_kdeincludexslt} ${_kdeincludexslt_binarypath}.install )")
        install(FILES ${_kdeincludexslt_binarypath}.install DESTINATION ${DATA_INSTALL_DIR}/ksgmltools2/customization RENAME ${_kdeincludexslt_out})
    endif(NOT WIN32)
endforeach( _kdeincludexslt ${_kdeincludexslt_ALL} )


########### install files ###############

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/checkXML DESTINATION ${BIN_INSTALL_DIR} )

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

########### l10n xml helper ###############

set( docbookl10nhelper_SRCS docbookl10nhelper.cpp )
kde4_add_executable( docbookl10nhelper NOGUI ${docbookl10nhelper_SRCS} )
target_link_libraries( docbookl10nhelper Qt5::Core )

add_custom_command( TARGET docbookl10nhelper POST_BUILD
  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/docbookl10nhelper
  ${DOCBOOKXSL_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/customization/xsl
  ${CMAKE_CURRENT_BINARY_DIR}/customization/xsl
)
# all-l10n.xml is generated by docbookl10nhelper
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/customization/xsl/all-l10n.xml
  DESTINATION ${DATA_INSTALL_DIR}/ksgmltools2/customization/xsl/ )

install( FILES
    KDocToolsMacros.cmake
    DESTINATION ${CMAKECONFIG_INSTALL_DIR} COMPONENT Devel )

