
KDB2HTML = $(KDB2HTML_UNINSTALLED)
KDE_LANG = en
KDE_DOCS = AUTO

installdir = $(kde_htmldir)/en/api
install_DATA = install-dep.entities

index.cache.bz2: $(srcdir)/index.docbook $(KDE_XSL_STYLESHEET) install-dep.entities
	@-if test -n "$(MEINPROC)"; then \
	   if test ! -f index.docbook; then \
              rm index.docbook ;\
              echo $(LN_S) $(srcdir)/index.docbook . ;\
              $(LN_S) $(srcdir)/index.docbook . ;\
           fi ;\
	   $(MEINPROC) --check --cache index.cache.bz2 index.docbook; \
	fi
