# This Makefile is no serious work: it shouldn't fix problems by patching other packages
# Please direct any insult to ebisch@cybercable.tm.fr.

AUTOMAKE_OPTIONS = foreign 1.4

bin_SCRIPTS = kdb2html

# Don't add plain yet!  It may overwrite the customisations.
# lang-conv gives problems when added
SUBDIRS = customization

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(bindir)
	$(INSTALL_SCRIPT) $(srcdir)/patches/db2ps $(DESTDIR)$(bindir)
#	-patch $(TEX_MF)/web2c/texmf.cnf $(srcdir)/patches/texmf.cnf

uninstall-local:
	-rm -f $(bindir)/db2ps

