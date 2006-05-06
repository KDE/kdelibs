kde4_header()



########### install files ###############

install(PROGRAMS ${KDE4_DCOPIDL_EXECUTABLE} DESTINATION bin)

install_files( ${DATA_INSTALL_DIR}/dcopidl FILES Ast.pm Iter.pm kalyptusCxxToDcopIDL.pm kdocAstUtil.pm kdocParseDoc.pm kdocUtil.pm kalyptus )

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
