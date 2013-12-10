include_directories( ${LIBXML2_INCLUDE_DIR} ${LIBXSLT_INCLUDE_DIR} )

set (_custom_dtd_kdex "customization/dtd/kdex.dtd")
configure_file(${_custom_dtd_kdex}.cmake ${CMAKE_CURRENT_BINARY_DIR}/${_custom_dtd_kdex} )
# WARNING: this is due to severe limitations on the windows platform to keep the dtd relocateable
# see the .xsl.cmake files for a more verbose explanation below
if(NOT WIN32)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${_custom_dtd_kdex}
            DESTINATION ${DATA_INSTALL_DIR}/ksgmltools2/customization/dtd )
else()
    install(CODE
        "FILE(RELATIVE_PATH DOCBOOKXML_CURRENTDTD_DIR \${CMAKE_INSTALL_PREFIX}/${DATA_INSTALL_DIR}/ksgmltools2/customization/dtd ${DOCBOOKXML_CURRENTDTD_DIR})
        CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/${_custom_dtd_kdex}.cmake ${CMAKE_CURRENT_BINARY_DIR}/${_custom_dtd_kdex}.install)"
    )
    get_filename_component(_custom_dtd_kdex_name ${_custom_dtd_kdex} NAME)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${_custom_dtd_kdex}.install DESTINATION ${DATA_INSTALL_DIR}/ksgmltools2/customization/dtd RENAME ${_custom_dtd_kdex_name})
endif()

# needed by KIO, need to export it
add_library(KF5XsltKde STATIC xslt.cpp xslt_kde.cpp)
add_library(KF5::XsltKde ALIAS KF5XsltKde)
target_link_libraries(KF5XsltKde Qt5::Core KF5::Archive)
install(TARGETS KF5XsltKde ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
    xslt.h xslt_kde.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
install(TARGETS KF5XsltKde EXPORT KF5DocToolsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### next target ###############

# The cross compiling parts are commented out on purpose. Alex
if (CMAKE_CROSSCOMPILING)
#    set(IMPORT_MEINPROC4_EXECUTABLE "${KDE_HOST_TOOLS_PATH}/ImportMeinProc4Executable.cmake" CACHE FILEPATH "Point it to the export file of meinproc5 from a native build")
#    include(${IMPORT_MEINPROC4_EXECUTABLE})
else ()
    set_source_files_properties(xslt.cpp PROPERTIES COMPILE_FLAGS -DSIMPLE_XSLT )

    option(MEINPROC_NO_KARCHIVE "Compile meinproc without KArchive dependency" OFF)
    if(MEINPROC_NO_KARCHIVE)
        add_definitions(-DMEINPROC_NO_KARCHIVE) #we don't have saveToCache when compiling without KArchive, which is used in xslt_kde.cpp
    else ()
        set(meinproc_additional_SRCS xslt_kde.cpp)
        set(meinproc_additional_LIBS KF5::Archive)
    endif()

    add_executable(meinproc5 meinproc.cpp meinproc_common.cpp xslt.cpp ${meinproc_additional_SRCS})
    target_link_libraries(meinproc5 Qt5::Core ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES} ${LIBXSLT_EXSLT_LIBRARIES} ${meinproc_additional_LIBS})

    install(TARGETS meinproc5 EXPORT KF5DocToolsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
    add_executable(KF5::meinproc5 ALIAS meinproc5)

#    # export this binary for cross-compilation
#    export(TARGETS meinproc5 FILE ${CMAKE_BINARY_DIR}/ImportMeinProc4Executable.cmake)
endif ()

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
    else()
        install(CODE
            "FILE(RELATIVE_PATH DOCBOOKXSL_DIR \${CMAKE_INSTALL_PREFIX}/${DATA_INSTALL_DIR}/ksgmltools2/customization ${DOCBOOKXSL_DIR})
            CONFIGURE_FILE( ${_kdeincludexslt} ${_kdeincludexslt_binarypath}.install )")
        install(FILES ${_kdeincludexslt_binarypath}.install DESTINATION ${DATA_INSTALL_DIR}/ksgmltools2/customization RENAME ${_kdeincludexslt_out})
    endif()
endforeach( _kdeincludexslt ${_kdeincludexslt_ALL} )


########### install files ###############

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
add_executable( docbookl10nhelper ${docbookl10nhelper_SRCS} )
target_link_libraries( docbookl10nhelper Qt5::Core )

add_custom_command( TARGET docbookl10nhelper POST_BUILD
  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/docbookl10nhelper
  ${DOCBOOKXSL_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/customization/xsl
  ${CMAKE_CURRENT_BINARY_DIR}/customization/xsl
)
# all-l10n.xml is generated by docbookl10nhelper
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/customization/xsl/all-l10n.xml
  DESTINATION ${DATA_INSTALL_DIR}/ksgmltools2/customization/xsl/ )

