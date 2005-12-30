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

#include "knotifyconfigelement.h"

#include <kconfig.h>
#include <kdebug.h>

KNotifyConfigElement::KNotifyConfigElement(const QString &eventid, KConfigBase *defaultConfig , KConfigBase *localConfig)
	: m_config( localConfig , "Event/" + eventid ) ,
	  m_default( defaultConfig , "Event/" + eventid )
{
}


QString KNotifyConfigElement::readEntry( const QString & entry, bool path )
{
	if(m_cache.contains(entry))
		return m_cache[entry];
	QString p=path ?  m_config.readPathEntry(entry) : m_config.readEntry(entry, QString::null);
	if(!p.isNull())
		return p;
	
	return path ?  m_default.readPathEntry(entry) : m_default.readEntry(entry, QString::null);
}

void KNotifyConfigElement::writeEntry( const QString & entry, const QString &data )
{
	m_cache[entry]=data;
	QMap<QString, QString>::const_iterator it = m_cache.constBegin();
}

void KNotifyConfigElement::save(  )
{
	QMap<QString, QString>::const_iterator it = m_cache.constBegin();
	for ( ; it != m_cache.constEnd() ; ++it)
	{
		m_config.writeEntry(it.key() , it.data() );
	}
	m_config.sync();
}

