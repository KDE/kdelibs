/*
   Copyright (C) 2005-2006 by Olivier Goffart <ogoffart at kde.org>


   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 */

#ifndef KNOTIFYCONFIG_H
#define KNOTIFYCONFIG_H

#include "knotify.h"
#include "kconfig.h"

/**
 * Represent the config for an event
	@author Olivier Goffart <ogoffart@kde.org>
*/
class KNotifyConfig
{
	public:
		KNotifyConfig(const QString &appname, const ContextList &_contexts , const QString &_eventid);
		~KNotifyConfig();
	
		QString readEntry(const QString& entry , bool path=false);
		
		QString text;
		QPixmap pix;
		int winId;
		QStringList actions;
		QString appname;

		KConfig eventsfile,configfile;
		ContextList contexts;
		QString eventid;
};

#endif
