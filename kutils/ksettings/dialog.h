/*  This file is part of the KDE project
    Copyright (C) 2003 Matthias Kretz <kretz@kde.org>

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

#ifndef KCONFIGUREDIALOG_H
#define KCONFIGUREDIALOG_H

#include <qobject.h>
#include <kservice.h>

class KConfigureDialog : public QObject
{
	Q_OBJECT
	public:
		/**
		 * Construct a new Preferences Dialog for the application.
		 */
		KConfigureDialog( QObject * parent = 0, const char * name = 0 );

		/**
		 * Construct a new Preferences Dialog with the pages for the selected
		 * instance names. For example if you want to have the configuration
		 * pages for the kviewviewer KPart and all it's plugins you would pass a
		 * QStringList consisting of only the name of the part "kviewviewer".
		 */
		KConfigureDialog( const QStringList & kcdparents, QObject * parent = 0, const char * name = 0 );

		void addKPartsPluginPage();

		~KConfigureDialog();

	public slots:
		void show();

	private:
		QValueList<KService::Ptr> instanceServices() const;
		QValueList<KService::Ptr> KCDParentsServices( const QStringList & ) const;
		void createDialogFromServices( const QValueList<KService::Ptr> & );
		class KConfigureDialogPrivate;
		KConfigureDialogPrivate * d;
};

#endif // KCONFIGUREDIALOG_H

// vim: sw=4 ts=4
