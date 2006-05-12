
find_package(LibXml2 REQUIRED)
find_package(LibXslt REQUIRED)

include_directories( ${KDE4_KIO_INCLUDES} )
#include_directories( ${CMAKE_SOURCE_DIR}/kioslave ${CMAKE_SOURCE_DIR}/kioslave/bzip2 )

include_directories( ${LIBXML2_INCLUDE_DIR} ${LIBXSLT_INCLUDE_DIR} )

configure_file(checkXML.in.cmake ${CMAKE_CURRENT_BINARY_DIR}/checkXML )
#macro_additional_clean_files( ${CMAKE_CURRENT_BINARY_DIR}/checkXML )


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

kde4_add_executable(meinproc NOGUI ${meinproc_SRCS})

target_link_libraries(meinproc kio ${KDE4_KDECORE_LIBS} ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES})
set_source_files_properties(xslt.cpp PROPERTIES COMPILE_FLAGS -DSIMPLE_XSLT )

install(TARGETS meinproc DESTINATION bin)


########### next target ###############

set(genshortcutents_SRCS genshortcutents.cpp )

kde4_automoc(${genshortcutents_SRCS})

kde4_add_executable(genshortcutents NOGUI ${genshortcutents_SRCS})

target_link_libraries(genshortcutents ${KDE4_KDECORE_LIBS} )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES help.protocol ghelp.protocol )
install_files( ${DATA_INSTALL_DIR}/kconf_update FILES kio_help.upd )

install_files( /bin FILES checkXML )

install_files(${KCONF_UPDATE_INSTALL_DIR} FILES move_kio_help_cache.sh )


macro(INSTALL_CUSTOMIZE_DIR _expr _directory)
    FILE(GLOB _file_CUST "${_expr}" )
    foreach(_curentfile ${_file_CUST})
        install(FILES ${_curentfile} DESTINATION ${CMAKE_INSTALL_PREFIX}/${DATA_INSTALL_DIR}/ksgmltools2/${_directory} )
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

set(customizedir af bg ca cs da de dtd el en-GB en entities eo es et fi fo fr he hu id it ja ko lt nl nn no obsolete pl pt-BR pt ro ru sk sl sr sv tr uk wa xh xsl xx zh-CN zh-TW)
 
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
    set(expr "customization/${_currentcustomizedir}/catalog")
    install_customize_dir(${expr} customization/${_currentcustomizedir} )
    set(expr "customization/${_currentcustomizedir}/*.xml")
    install_customize_dir(${expr} customization/${_currentcustomizedir} )
	set(expr "customization/${_currentcustomizedir}/*.dtd")
	install_customize_dir(${expr} customization/${_currentcustomizedir}/)
	set(expr "customization/${_currentcustomizedir}/*.elements")
	install_customize_dir(${expr} customization/${_currentcustomizedir}/)
	
endforeach(_currentcustomizedir ${customizedir}) 
	  




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
