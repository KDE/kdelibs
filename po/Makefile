# Generated automatically from Makefile.in by configure.

# This has to be set. The rest should be reusable
DE_POFILES = de_kde.po 
DE_GMFILES = de_kde.gmo

FR_POFILES = fr_kde.po
FR_GMFILES = fr_kde.gmo

POFILES = $(DE_POFILES) $(FR_POFILES)
GMOFILES= $(DE_GMFILES) $(FR_GMFILES)

# more general options
srcdir = .
top_srcdir = ..
prefix = /home/coolo/prod/kde
exec_prefix = ${prefix}
localedir = $(prefix)/locale

INSTALL = /usr/bin/install -c
INSTALL_DATA = ${INSTALL} -m 644
MKINSTALLDIRS = $(SHELL) $(top_srcdir)/mkinstalldirs
SHELL = /bin/sh

GMSGFMT = /usr/bin/msgfmt
MSGFMT = /usr/bin/msgfmt
XGETTEXT = /usr/bin/xgettext
DISTFILES = Makefile.in $(POFILES) $(GMOFILES)

# files:

default: all

all: all-yes
install: install-yes

Makefile: Makefile.in
	cd .. && \
	CONFIG_FILES=po/$@ CONFIG_HEADERS= $(SHELL) ./config.status

.SUFFIXES: .po .gmo

.po.mo:
	$(MSGFMT) -o $@ $<

.po.gmo:
	file=$(srcdir)/`echo $* | sed 's,.*/,,'`.gmo \
	&& rm -f $$file && $(GMSGFMT) -o $$file $< 

all-yes: $(GMOFILES)
all-no:

install-yes: all
	@catalogs='$(GMOFILES)'; \
	for cat in $$catalogs; do \
	destdir=$(localedir); \
	lang=`echo $$cat | sed 's/_.*$$//'`; \
	name=`echo $$cat | sed -e 's/^.._//' | sed -e 's/\.gmo/\.mo/'` \
	dir=$$destdir/$$lang/LC_MESSAGES; \
	$(MKINSTALLDIRS) $$dir; \
	$(INSTALL_DATA) $$cat $$dir/$$name; \
	  echo "installing $$cat as $$dir/$$name"; \
	done

install-no:

distclean:
	rm -f *~ Makefile

clean:
