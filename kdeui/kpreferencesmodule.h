/*  This file is part of the KDE project
    Copyright (C) 2002 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#ifndef KPREFERENCESMODULE_H
#define KPREFERENCESMODULE_H

#include <qobject.h>

class QFrame;

/**
 * The base class for Preference Dialog modules.
 *
 * If the application uses a @ref KPreferencesDialog you can conveniently create
 * pages for the dialog by subclassing.
 *
 * @author Matthias Kretz (kretz@kde.org)
 * @version $Id$
 */
class KPreferencesModule : public QObject
{
	Q_OBJECT
	public:
		/**
		 * 
		 */
		KPreferencesModule( const QString & name, const QString & description, const QString & icon, QObject * parent, const char * name = 0 );
		virtual ~KPreferencesModule();

		/**
		 *
		 */
		virtual void load() = 0;

		/**
		 *
		 */
		virtual void save() = 0;

	protected:
		QFrame * m_page;

	private:
		struct KPreferencesModulePrivate;
		KPreferencesModulePrivate * d;
};

// vim:sw=4:ts=4

#endif // KPREFERENCESMODULE_H
