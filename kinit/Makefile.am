#	This file is part of the KDE libraries
#    Copyright (C) 1999 Waldo Bastian (bastian@kde.org)

#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Library General Public
#    License version 2 as published by the Free Software Foundation.

#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Library General Public License for more details.

#    You should have received a copy of the GNU Library General Public License
#    along with this library; see the file COPYING.LIB.  If not, write to
#    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
#    Boston, MA 02111-1307, USA.

INCLUDES = -I$(srcdir)/../libltdl/ -I$(srcdir)/../kio/klauncher/ $(all_includes) -Wall

SUBDIRS = .

bin_PROGRAMS = kinit

kinit_SOURCES = kinit.c setproctitle.c

kinit_LDFLAGS = $(QT_LDFLAGS) $(X_LDFLAGS) $(USER_LDFLAGS) 
kinit_LDADD = $(LIB_DCOP) $(LIB_DMALLOC) $(LIB_KDECORE) $(LIB_KIO) $(LIB_KIMGIO)

kinit_METASOURCES = AUTO

