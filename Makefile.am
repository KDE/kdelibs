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


COMPILE_FIRST = dcop libltdl kdecore kdeui kdesu kio kimgio kfile kparts kjs libkmid ksgmltools kssl

SUBDIRS = $(COMPILE_FIRST) $(TOPSUBDIRS)

$(top_srcdir)/acinclude.m4: $(top_srcdir)/admin/acinclude.m4.in $(top_srcdir)/admin/libtool.m4.in
	@cd $(top_srcdir) && cat admin/acinclude.m4.in admin/libtool.m4.in > acinclude.m4

$(top_srcdir)/configure.in: $(top_srcdir)/configure.files $(shell test -f $(top_srcdir)/configure.files && cat $(top_srcdir)/configure.files)
	cd $(top_srcdir) && $(MAKE) -f admin/Makefile.common configure.in
$(top_srcdir)/subdirs:
	cd $(top_srcdir) && $(MAKE) -f admin/Makefile.common subdirs
$(top_srcdir)/configure.files:
	cd $(top_srcdir) && $(MAKE) -f admin/Makefile.common configure.files

AUTOMAKE_OPTIONS = foreign 1.4

include_HEADERS = kde.pot

messages:
	dirs=`find . -type d | grep -v CVS`  ;\
	find $$dirs -maxdepth 1 -name "*.cpp" -print > files ;\
	find $$dirs -maxdepth 1 -name "*.cc" -print >> files ;\
	$(EXTRACTRC) `find $$dirs -maxdepth 1 -name "*.rc"` > rc.cpp	;\
	: > kde.pot ;\
	$(XGETTEXT) -x qt-messages.pot rc.cpp `cat files` && cat messages.po qt-messages.pot > $(podir)/kdelibs.pot;
	$(XGETTEXT) common_texts.cpp -o kde.pot
	rm -f rc.cpp

EXTRA_DIST = admin bsd-port debian kdelibs.spec.in kde.pot kde-config.in kde-config

dist-hook:
	cd $(top_distdir) && perl $(top_srcdir)/admin/am_edit -padmin

cvs-local:
	@echo "*** preparing libtool library"
	cd  libltdl && \
	rm -f acinclude.m4 && \
	cp ../admin/libtool.m4.in acinclude.m4 && \
	cat ltdl.m4 >> acinclude.m4 && \
	aclocal && \
	autoheader && \
	automake --gnits --add-missing && \
	autoconf

kde_conf_DATA = kde-config

kdedoc:
	makekdedoc -p'--skip-internal'
