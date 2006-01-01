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
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

 */

#include "knotifyconfig.h"

#include <kconfig.h>
#include <kdebug.h>

KNotifyConfig::KNotifyConfig( const QString & _appname, const ContextList & _contexts, const QString & _eventid )
	: appname (_appname),
	eventsfile( _appname+"/"+_appname + ".notifyrc" , true, false, "data"),
	configfile( _appname+QString::fromAscii( ".notifyrc" ), true, false),
	contexts(_contexts) , eventid(_eventid) 
{
//	kdDebug() << k_funcinfo << appname << " , " << eventid << endl;
}

KNotifyConfig::~KNotifyConfig()
{
}

QString KNotifyConfig::readEntry( const QString & entry, bool path )
{
	QPair<QString , QString> context;
	foreach(  context , contexts )
	{
		const QString group="Event/" + eventid + "/" + context.first + "/" + context.second;
		if(configfile.hasGroup( group ) )
		{
			configfile.setGroup(group);
			QString p=path ?  configfile.readPathEntry(entry) : configfile.readEntry(entry,QString());
			if(!p.isNull())
				return p;
		}
	}
//	kdDebug() << k_funcinfo << entry << " not found in contexts " << endl;
	const QString group="Event/" + eventid ;
	if(configfile.hasGroup( group ) )
	{
		configfile.setGroup(group);
		QString p=path ?  configfile.readPathEntry(entry) : configfile.readEntry(entry,QString());
		if(!p.isNull())
			return p;
	}
//	kdDebug() << k_funcinfo << entry << " not found in config " << endl;
	if(eventsfile.hasGroup( group ) )
	{
		eventsfile.setGroup(group);
		QString p=path ?  eventsfile.readPathEntry(entry) : eventsfile.readEntry(entry, QString());
		if(!p.isNull())
			return p;
	}
//	kdDebug() << k_funcinfo << entry << " not found !!! " << endl;

	return QString();
}

