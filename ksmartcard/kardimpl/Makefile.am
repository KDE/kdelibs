#    This file is part of the KDE libraries
#    Copyright (C) 2001 Fernando Llobregat <fernando.llobregat@free.fr>

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

INCLUDES= -I$(srcdir)/.. -I$(srcdir)/../kardlib -I$(top_srcdir) $(all_includes)


lib_LTLIBRARIES = libkscard_gsm.la libkscard_gemsafe.la

libkscard_gsm_la_LDFLAGS = $(all_libraries) ../kardlib/libksmartcard.la -module -avoid-version
libkscard_gsm_la_LIBADD = 
libkscard_gsm_la_SOURCES =kcardgsm_impl.h kcardgsm_impl.cc kcardimplementation.cc kcardimplementation.h


libkscard_gemsafe_la_LDFLAGS = $(all_libraries) ../kardlib/libksmartcard.la -module -avoid-version
libkscard_gemsafe_la_LIBADD = 
libkscard_gemsafe_la_SOURCES =kcardgemsafe_impl.h kcardgemsafe_impl.cc kcardimplementation.cc kcardimplementation.h

METASOURCES = AUTO

include_HEADERS = 

noinst_HEADERS =

services_DATA = kcardgsm.desktop kcardgemsafe.desktop
servicesdir = $(kde_servicesdir)/kscard

servicetype_DATA =  kcardimplsvc.desktop
servicetypedir = $(kde_servicetypesdir)
