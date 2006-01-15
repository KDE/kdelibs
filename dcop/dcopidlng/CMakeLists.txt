kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${KDE4_DATA_DIR}/dcopidl FILES Ast.pm Iter.pm kalyptusCxxToDcopIDL.pm kdocAstUtil.pm kdocParseDoc.pm kdocUtil.pm )

kde4_footer()



#original Makefile.am contents follow:

#bin_SCRIPTS = dcopidl
#
#dcopidllib_DATA = Ast.pm Iter.pm kalyptusCxxToDcopIDL.pm kdocAstUtil.pm kdocParseDoc.pm kdocUtil.pm
#dcopidllib_SCRIPTS = kalyptus
#dcopidllibdir = $(kde_datadir)/dcopidl
#
#check-local:
#	$(srcdir)/run_test.sh $(srcdir)
