AUTOMAKE_OPTIONS = foreign 1.4

bin_SCRIPTS = kdb2html

# Don't add plain yet!  It may overwrite the customisations.
# lang-conv gives problems when added
SUBDIRS = customization

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(bindir)
#	-patch $(TEX_MF)/web2c/texmf.cnf $(srcdir)/patches/texmf.cnf
	/usr/bin/install-catalog --add /etc/sgml/sgml-docbook-3.1.cat /usr/share/sgml/docbook/kde-customization-0.1/catalog

uninstall-local:
	/usr/bin/install-catalog --remove /etc/sgml/sgml-docbook-3.1.cat /usr/share/sgml/docbook/kde-customization-0.1/catalog
