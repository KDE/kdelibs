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

#ifndef KCMODULEPROXY_H
#define KCMODULEPROXY_H

#include <qwidget.h>
#include <qstringlist.h>

class KCModuleInfo;
class KAboutData;
class KInstance;
class KCModule;

class KCModuleProxy : public QWidget
{
	Q_OBJECT
	public:
		KCModuleProxy( const KCModuleInfo & info, bool withfallback = false,
				QWidget * parent = 0, const char * name = 0,
				const QStringList & args = QStringList() );
		~KCModuleProxy();
		void load();
		void save();
		void defaults();
		QString quickHelp() const;
		const KAboutData * aboutData() const;
		int buttons() const;
		QString rootOnlyMsg() const;
		bool useRootOnlyMsg() const;
		KInstance * instance() const;
		bool changed() const;
		KCModule * realModule() const;
    const KCModuleInfo & moduleInfo() const;

	signals:
		void changed( bool );

	protected:
		void showEvent( QShowEvent * );

	private slots:
		void moduleChanged( bool );
		void moduleDestroyed();

	private:
		class KCModuleProxyPrivate;
		KCModuleProxyPrivate * d;
};

// vim: sw=4 ts=4 noet
#endif // KCMODULEPROXY_H
