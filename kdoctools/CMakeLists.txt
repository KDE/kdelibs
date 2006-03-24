kde4_header()

find_package(LibXml2 REQUIRED)
find_package(LibXslt REQUIRED)

macro_use_lib_kio()
#include_directories( ${CMAKE_SOURCE_DIR}/kioslave ${CMAKE_SOURCE_DIR}/kioslave/bzip2 )
include_directories( ${QT_INCLUDES} )

include_directories( ${LIBXML2_INCLUDE_DIR} ${LIBXSLT_INCLUDE_DIR} )

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/checkXML.in.cmake ${CMAKE_CURRENT_BINARY_DIR}/checkXML )
macro_additional_clean_files( ${CMAKE_CURRENT_BINARY_DIR}/checkXML )


########### next target ###############

set(kio_help_PART_SRCS
   kio_help.cpp
   main.cpp
   xslt.cpp
)

kde4_automoc(${kio_help_PART_SRCS})

kde4_add_plugin(kio_help ${kio_help_PART_SRCS})

target_link_libraries(kio_help  ${KDE4_KDECORE_LIBS} kio ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES} )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR}  kio_help )
install_targets(${PLUGIN_INSTALL_DIR} kio_help )


########### next target ###############

set(kio_ghelp_PART_SRCS kio_help.cpp main_ghelp.cpp xslt.cpp )

kde4_automoc(${kio_ghelp_PART_SRCS})

kde4_add_plugin(kio_ghelp ${kio_ghelp_PART_SRCS})

target_link_libraries(kio_ghelp  ${KDE4_KDECORE_LIBS} kio ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES} )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kio_ghelp )
install_targets(${PLUGIN_INSTALL_DIR} kio_ghelp )


########### next target ###############

set(meinproc_SRCS meinproc.cpp xslt.cpp )

kde4_automoc(${meinproc_SRCS})

kde4_add_executable(meinproc ${meinproc_SRCS})

target_link_libraries(meinproc kio ${KDE4_KDECORE_LIBS} ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES})
set_source_files_properties(xslt.cpp PROPERTIES COMPILE_FLAGS -DSIMPLE_XSLT )

install_targets(/bin meinproc )


########### next target ###############

set(genshortcutents_SRCS
genshortcutents.cpp
)



kde4_automoc(${genshortcutents_SRCS})

kde4_add_executable(genshortcutents ${genshortcutents_SRCS})

target_link_libraries(genshortcutents  ${KDE4_KDECORE_LIBS} )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES help.protocol ghelp.protocol )
install_files( ${DATA_INSTALL_DIR}/kconf_update FILES kio_help.upd )

install_files( /bin FILES checkXML )

install_files(${KCONF_UPDATE_INSTALL_DIR} FILES move_kio_help_cache.sh )


MACRO(INSTALL_CUSTOMIZE_DIR _expr _directory)
    FILE(GLOB _file_CUST "${_expr}" )
    foreach(_curentfile ${_file_CUST})
        install(FILES ${_curentfile} DESTINATION ${CMAKE_INSTALL_PREFIX}/${DATA_INSTALL_DIR}/ksgmltools2/${_directory} )
    endforeach(_curentfile ${_file_CUST})
ENDMACRO(INSTALL_CUSTOMIZE_DIR _expr _directory)

set(docbookdir xml-dtd-4.1.2  xml-dtd-4.2  xsl )

foreach (_currentdocbookdir ${docbookdir})
	set(expr "docbook/${_currentdocbookdir}/*.mod")
	INSTALL_CUSTOMIZE_DIR( ${expr} docbook/${_currentdocbookdir}/)
	set(expr "docbook/${_currentdocbookdir}/*.txt")
	INSTALL_CUSTOMIZE_DIR( ${expr} docbook/${_currentdocbookdir}/)
	set(expr "docbook/${_currentdocbookdir}/*.dtd")
    INSTALL_CUSTOMIZE_DIR( ${expr} docbook/${_currentdocbookdir}/)
	set(expr "docbook/${_currentdocbookdir}/*.cat")
    INSTALL_CUSTOMIZE_DIR( ${expr} docbook/${_currentdocbookdir}/)
	set(expr "docbook/${_currentdocbookdir}/README*")
	INSTALL_CUSTOMIZE_DIR( ${expr} docbook/${_currentdocbookdir}/)
	set(expr "docbook/${_currentdocbookdir}/Changelog*")
	INSTALL_CUSTOMIZE_DIR( ${expr} docbook/${_currentdocbookdir}/)
	set(expr "docbook/${_currentdocbookdir}/*.xml")
	INSTALL_CUSTOMIZE_DIR( ${expr} docbook/${_currentdocbookdir}/)
	set(expr "docbook/${_currentdocbookdir}/VERSION*")
	INSTALL_CUSTOMIZE_DIR( ${expr} docbook/${_currentdocbookdir}/)
	set(expr "docbook/${_currentdocbookdir}/What*")
	INSTALL_CUSTOMIZE_DIR( ${expr} docbook/${_currentdocbookdir}/)
	
	set(expr "docbook/${_currentdocbookdir}/ent/*.ent")
	INSTALL_CUSTOMIZE_DIR( ${expr} docbook/${_currentdocbookdir}/ent/)

	set(expr "docbook/${_currentdocbookdir}/common/*.xsl")
	INSTALL_CUSTOMIZE_DIR( ${expr} docbook/${_currentdocbookdir}/common/)
	set(expr "docbook/${_currentdocbookdir}/common/*.xml")
	INSTALL_CUSTOMIZE_DIR( ${expr} docbook/${_currentdocbookdir}/common/)
	set(expr "docbook/${_currentdocbookdir}/common/*.dtd")
	INSTALL_CUSTOMIZE_DIR( ${expr} docbook/${_currentdocbookdir}/common/)
	set(expr "docbook/${_currentdocbookdir}/common/Changelog")
	INSTALL_CUSTOMIZE_DIR( ${expr} docbook/${_currentdocbookdir}/common/)

    set(expr "docbook/${_currentdocbookdir}/html/*.xsl")
    INSTALL_CUSTOMIZE_DIR( ${expr} docbook/${_currentdocbookdir}/html/)
    set(expr "docbook/${_currentdocbookdir}/html/*.xml")
    INSTALL_CUSTOMIZE_DIR( ${expr} docbook/${_currentdocbookdir}/html/)
    set(expr "docbook/${_currentdocbookdir}/html/*.dtd")
    INSTALL_CUSTOMIZE_DIR( ${expr} docbook/${_currentdocbookdir}/html/)
	set(expr "docbook/${_currentdocbookdir}/html/Changelog")
	INSTALL_CUSTOMIZE_DIR( ${expr} docbook/${_currentdocbookdir}/html/)
	set(expr "docbook/${_currentdocbookdir}/html/*.ent")
	INSTALL_CUSTOMIZE_DIR( ${expr} docbook/${_currentdocbookdir}/html/)
	set(expr "docbook/${_currentdocbookdir}/html/*.xweb")
	INSTALL_CUSTOMIZE_DIR( ${expr} docbook/${_currentdocbookdir}/html/)

    set(expr "docbook/${_currentdocbookdir}/images/*.png")
    INSTALL_CUSTOMIZE_DIR( ${expr} docbook/${_currentdocbookdir}/images/)
	set(expr "docbook/${_currentdocbookdir}/images/*.gif")
	INSTALL_CUSTOMIZE_DIR( ${expr} docbook/${_currentdocbookdir}/images/)
	set(expr "docbook/${_currentdocbookdir}/images/*.tif")
	INSTALL_CUSTOMIZE_DIR( ${expr} docbook/${_currentdocbookdir}/images/)
	set(expr "docbook/${_currentdocbookdir}/images/Changelog")
	INSTALL_CUSTOMIZE_DIR( ${expr} docbook/${_currentdocbookdir}/images/)

	set(expr "docbook/${_currentdocbookdir}/images/callouts/*.png")
	INSTALL_CUSTOMIZE_DIR( ${expr} docbook/${_currentdocbookdir}/images/callouts/)
    set(expr "docbook/${_currentdocbookdir}/images/callouts/*.gif")
    INSTALL_CUSTOMIZE_DIR( ${expr} docbook/${_currentdocbookdir}/images/callouts/)
	set(expr "docbook/${_currentdocbookdir}/images/callouts/Changelog")
	INSTALL_CUSTOMIZE_DIR( ${expr} docbook/${_currentdocbookdir}/images/callouts/)

	set(expr "docbook/${_currentdocbookdir}/lib/lib.*")
	INSTALL_CUSTOMIZE_DIR( ${expr} docbook/${_currentdocbookdir}/lib/)
	set(expr "docbook/${_currentdocbookdir}/lib/Changel*")
	INSTALL_CUSTOMIZE_DIR( ${expr} docbook/${_currentdocbookdir}/lib/)
	set(expr "docbook/${_currentdocbookdir}/lib/README*")
	INSTALL_CUSTOMIZE_DIR( ${expr} docbook/${_currentdocbookdir}/lib/)

	set(expr "docbook/${_currentdocbookdir}/manpages/*.xsl")
	INSTALL_CUSTOMIZE_DIR( ${expr} docbook/${_currentdocbookdir}/manpages/)

	set(expr "docbook/${_currentdocbookdir}/params/*.xml")
	INSTALL_CUSTOMIZE_DIR( ${expr} docbook/${_currentdocbookdir}/params/)
	set(expr "docbook/${_currentdocbookdir}/params/Changelog")
	INSTALL_CUSTOMIZE_DIR( ${expr} docbook/${_currentdocbookdir}/params/)
	

endforeach(_currentdocbookdir ${docbookdir})


set(customizedir af bg ca cs da de dtd el en-GB en entities eo es et fi fo fr he hu id it ja ko lt nl nn no obsolete pl pt-BR pt ro ru sk sl sr sv tr uk wa xh xsl xx zh-CN zh-TW)
 
foreach (_currentcustomizedir ${customizedir})
	set(expr "customization/${_currentcustomizedir}/README")
	INSTALL_CUSTOMIZE_DIR( ${expr} customization/${_currentcustomizedir})
	set(expr "customization/${_currentcustomizedir}/*.entities")
	INSTALL_CUSTOMIZE_DIR( ${expr} customization/${_currentcustomizedir})
	set(expr "customization/${_currentcustomizedir}/*.xsl")
	INSTALL_CUSTOMIZE_DIR( ${expr} customization/${_currentcustomizedir})
	set(expr "customization/${_currentcustomizedir}/obsolete/*.elements")
	INSTALL_CUSTOMIZE_DIR( ${expr} customization/${_currentcustomizedir}/obsolete/)
	set(expr "customization/${_currentcustomizedir}/obsolete/*.dtd")
	INSTALL_CUSTOMIZE_DIR( ${expr} customization/${_currentcustomizedir}/obsolete/)
    set(expr "customization/${_currentcustomizedir}/entities/*.docbook")
	INSTALL_CUSTOMIZE_DIR( ${expr} customization/${_currentcustomizedir}/entities/ )
	set(expr "customization/${_currentcustomizedir}/entities/*.entities")
	INSTALL_CUSTOMIZE_DIR( ${expr} customization/${_currentcustomizedir}/entities/ )
    set(expr "customization/${_currentcustomizedir}/catalog")
    INSTALL_CUSTOMIZE_DIR(${expr} customization/${_currentcustomizedir} )
    set(expr "customization/${_currentcustomizedir}/*.xml")
    INSTALL_CUSTOMIZE_DIR(${expr} customization/${_currentcustomizedir} )
	set(expr "customization/${_currentcustomizedir}/dtd/*.dtd")
	INSTALL_CUSTOMIZE_DIR(${expr} customization/${_currentcustomizedir}/dtd/)
	set(expr "customization/${_currentcustomizedir}/dtd/*.elements")
	INSTALL_CUSTOMIZE_DIR(${expr} customization/${_currentcustomizedir}/dtd/)
	
endforeach(_currentcustomizedir ${customizedir}) 
	  

kde4_create_doxygen_docs(  )

kde4_footer()



#original Makefile.am contents follow:

#
#INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/kioslave -I$(top_srcdir)/kioslave/bzip2 $(all_includes) $(LIBXML_CFLAGS) $(LIBXSLT_CFLAGS)
#AM_LDFLAGS = $(KDE_RPATH) $(LIBXML_RPATH) $(LIBXSLT_RPATH) $(all_libraries)
#
######## Files
#
#kde_module_LTLIBRARIES = kio_help.la kio_ghelp.la
#
#kio_help_la_SOURCES = kio_help.cpp main.cpp xslt.cpp
#kio_help_la_LIBADD = $(LIBXSLT_LIBS) $(LIBXML_LIBS) $(LIB_KIO)
#kio_help_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#
#kio_ghelp_la_SOURCES = kio_help.cpp main_ghelp.cpp xslt.cpp
#kio_ghelp_la_LIBADD = $(LIBXSLT_LIBS) $(LIBXML_LIBS) $(LIB_KIO)
#kio_ghelp_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#
#noinst_HEADERS = kio_help.h
#
#kdelnk_DATA = help.protocol ghelp.protocol
#kdelnkdir = $(kde_servicesdir)
#
#bin_PROGRAMS = meinproc 
#meinproc_LDADD = xslt_pure.o $(LIB_KDECORE) $(LIBXSLT_LIBS) $(LIBXML_LIBS) 
#meinproc_SOURCES = meinproc.cpp
#
#noinst_PROGRAMS = genshortcutents
#genshortcutents_SOURCES = genshortcutents.cpp
#genshortcutents_LDADD = $(LIB_KDECORE)
#
#kde-standard-accels.entities: genshortcutents
#	$(top_builddir)/kdoctools/genshortcutents -o $(top_builddir)/kdoctools/kde-standard-accels.entities
#
#xslt_pure.o: xslt.cpp
#	-rm -f xslt_pure.cpp
#	$(LN_S) $(srcdir)/xslt.cpp xslt_pure.cpp
#	$(CXX) $(DEFS) $(DEFAULT_INCLUDES) -DSIMPLE_XSLT $(INCLUDES) $(CPPFLAGS) $(AM_CPPFLAGS) $(CXXFLAGS) -c xslt_pure.cpp
#
#bin_SCRIPTS = checkXML
#
#METASOURCES = AUTO
#KDE_CXXFLAGS = -UQT3_SUPPORT
#
#update_DATA = kio_help.upd
#update_SCRIPTS = move_kio_help_cache.sh
#updatedir = $(kde_datadir)/kconf_update
#
#messages:
#	$(XGETTEXT) *.cpp -o $(podir)/kio_help.pot
#
#install-data-local: kde-standard-accels.entities
#	@(for subdir in customization docbook; do \
#	   for dir in `( cd $(srcdir)/$$subdir && find . -type d ! -name CVS | grep -v '\.svn' )`; do \
#		$(mkinstalldirs) $(DESTDIR)$(kde_datadir)/ksgmltools2/$$subdir/$$dir ;\
#		for file in `(cd $(srcdir)/$$subdir && ls -1 $$dir/*)`; do \
#		    if test -f $(srcdir)/$$subdir/$$file; then \
#			$(INSTALL_DATA) $(srcdir)/$$subdir/$$file  $(DESTDIR)$(kde_datadir)/ksgmltools2/$$subdir/$$file; fi ;\
#	   	done ;\
#	   done ;\
#	done;\
#	$(INSTALL_DATA) $(top_builddir)/kdoctools/kde-standard-accels.entities $(DESTDIR)$(kde_datadir)/ksgmltools2/customization/entities )
#
#uninstall-local:
#	-rm -rf $(DESTDIR)$(kde_datadir)/ksgmltools2/customization	
#	-rm -rf $(DESTDIR)$(kde_datadir)/ksgmltools2/docbook
#
#CLEANFILES = xslt_pure.o xslt_pure.cpp checkXML kde-standard-accels.entities genshortcutents
#
#include $(top_srcdir)/admin/Doxyfile.am
#
