AUTOMAKE_OPTIONS = foreign 1.4

bin_SCRIPTS = kdb2html

# Don't add plain yet!  It may overwrite the customisations.
# lang-conv gives problems when added
SUBDIRS = customization

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(bindir)
	$(INSTALL_SCRIPT) $(DESTDIR)$(bindir)
# Removed "$(srcdir)/patches/db2ps" argument. Will it create problems, Stephan?
#	-patch $(TEX_MF)/web2c/texmf.cnf $(srcdir)/patches/texmf.cnf
  /usr/bin/install-catalog --add /etc/sgml/docbook-3.1.cat /usr/share/sgml/docbook/kde-customization-0.1/catalog

uninstall-local:
	-rm -f $(bindir)/db2ps
  /usr/bin/install-catalog --remove /etc/sgml/docbook-3.1.cat /usr/share/sgml/docbook/kde-customization-0.1/catalog
