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


SUBDIRS = nana qk dcop libltdl kdecore kdeui kformula mimetypes \
	mediatool kspell kio kimgio kfile kab khtml kstyles \
	kparts doc toolbar interfaces kdetest pics

$(top_srcdir)/acinclude.m4: $(top_srcdir)/admin/acinclude.m4.in $(top_srcdir)/admin/libtool.m4.in
	@cd $(top_srcdir) && cat admin/acinclude.m4.in admin/libtool.m4.in > acinclude.m4

AUTOMAKE_OPTIONS = foreign 1.4

include_HEADERS = kde.pot

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x qt-messages.pot common_texts.cpp */*.cpp */*.cc && cat messages.po qt-messages.pot > $(podir)/kdelibs.pot; rm messages.po
	$(XGETTEXT) -C -ktranslate -ki18n common_texts.cpp && mv messages.po kde.pot

EXTRA_DIST = admin bsd-port debian kdelibs.spec.in kde.pot

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
