/*  This file is part of the KDE project
   Copyright (C) 2005-2006 by Olivier Goffart <ogoffart at kde.org>


   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

 */

#ifndef KNOTIFYCONFIGELEMENT_H
#define KNOTIFYCONFIGELEMENT_H

#include <QtCore/QMap>

class KConfig;
class KConfigGroup;

/**
 * Represent the config for an event
        @author Olivier Goffart <ogoffart@kde.org>
 */
class KNotifyConfigElement
{
	public:
		KNotifyConfigElement( const QString &eventid, KConfig *config);
		~KNotifyConfigElement();

		QString readEntry(const QString& entry, bool path=false);
		void writeEntry(const QString& entry, const QString & data);
		
		void save();
		
		/**
		 * return wither kttsd is installed or not.
		 */
		static bool have_kttsd();
		
	private:
		QMap<QString,QString> m_cache;
		KConfigGroup* m_config;
};

#endif
