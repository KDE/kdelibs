#    This file is part of the KDE libraries
#    Copyright (C) 1996-1997 Matthias Kalle Dalheimer (kalle@kde.org)
#              (C) 1997,1998 Stephan Kulow (coolo@kde.org)

#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Library General Public
#    License as published by the Free Software Foundation; either
#    version 2 of the License, or (at your option) any later version.

#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Library General Public License for more details.

#    You should have received a copy of the GNU Library General Public License
#    along with this library; see the file COPYING.LIB.  If not, write to
#    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
#    Boston, MA 02111-1307, USA.


COMPILE_FIRST = dcop libltdl kdecore interfaces kdeui kdesu kssl kjs kio kimgio kfile kparts kdeprint libkmid kconf_update 
COMPILE_LAST = doc

SUBDIRS = $(COMPILE_FIRST) $(TOPSUBDIRS) $(COMPILE_LAST)

$(top_srcdir)/acinclude.m4: $(top_srcdir)/admin/acinclude.m4.in $(top_srcdir)/admin/libtool.m4.in
	@cd $(top_srcdir) && cat admin/acinclude.m4.in admin/libtool.m4.in > acinclude.m4

$(top_srcdir)/configure.in: $(top_srcdir)/subdirs $(top_srcdir)/configure.files $(CONF_FILES)
	cd $(top_srcdir) && $(MAKE) -f admin/Makefile.common configure.in
$(top_srcdir)/subdirs:
	cd $(top_srcdir) && $(MAKE) -f admin/Makefile.common subdirs
$(top_srcdir)/configure.files:
	cd $(top_srcdir) && $(MAKE) -f admin/Makefile.common configure.files

AUTOMAKE_OPTIONS = foreign 1.4

include_HEADERS = kde.pot

messages:
	find . -type d | grep -v CVS | sed -e 's,$$,/,' > dirs
	mfs=`find . -name Makefile.am | xargs egrep -l '^messages:'` ;\
	for dir in $$mfs; do \
	  if test ! "$$dir" = "./Makefile.am"; then \
	    dir=`dirname $$dir` ;\
	    egrep -v "^$$dir" dirs > dirs.new && mv dirs.new dirs ;\
	  fi ;\
	done
	dirs=`cat dirs` ;\
	find $$dirs -maxdepth 1 -name "*.cpp" -print > files ;\
	find $$dirs -maxdepth 1 -name "*.cc" -print >> files ;\
	$(EXTRACTRC) `find $$dirs -maxdepth 1 -name "*.rc"` > rc.cpp	;\
	: > kde.pot ;\
	$(XGETTEXT) -x qt-messages.pot rc.cpp `cat files` && cat messages.po qt-messages.pot > $(podir)/kdelibs.pot;
	$(XGETTEXT) common_texts.cpp -o kde.pot
	rm -f rc.cpp dirs

EXTRA_DIST = admin bsd-port debian kdelibs.spec.in kde.pot configure.in.bot \
	COMPILING COMPILING.html COPYING.BSD DEBUG \
	KDE2PORTING.html NAMING common_texts.cpp kdelibs.lsm kdoc.rules \
	qt-messages.pot configure.in.in configure.in.mid configure.in.bot

MAINTAINERCLEANFILES = configure.files subdirs

dist-hook:
	cd $(top_distdir) && perl $(top_srcdir)/admin/am_edit -padmin

cvs-local:
	@echo "*** preparing libtool library"
	cd  libltdl && \
	rm -f acinclude.m4 && \
	cat ../admin/libtool.m4.in > acinclude.m4 && \
	cat ltdl.m4 >> acinclude.m4 && \
	aclocal && \
	autoheader && \
	automake --gnits --add-missing && \
	autoconf

cvs-dist-local:
	cd libltdl && automake --gnits --add-missing -i &&  autoconf

# Rule for generating HTML documentation [you need kdoc installed]
# Don't forget to run qt2kdoc first (qt2kdoc $QTDIR/doc/html)
# The kdelibs docu will end up in ./srcdoc/
kdedoc:
	cd $(top_srcdir) && \
	makekdedoc -p'--skip-internal' --outputdir=$(top_builddir)/srcdoc

