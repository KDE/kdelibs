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

#ifndef KNOTIFY_H
#define KNOTIFY_H

#include <QObject>
#include <QPixmap>
#include <QHash>
#include <QString>
#include <QList>
#include <QPair>

#include <dbus/qdbus.h>

typedef QHash<QString,QString> Dict;
typedef QList< QPair<QString,QString> > ContextList;

class KNotifyPlugin;


class KNotify : public QObject
{
	Q_OBJECT

	public:
		KNotify(QObject *parent=0l);
		~KNotify();
		void addPlugin( KNotifyPlugin *p );

	public Q_SLOTS:
	
		void reconfigure();
		void closeNotification( int id);
		
		int event(const QString &event, const QString &fromApp, const ContextList& contexts ,
				   const QString &text, const QPixmap& pixmap,  const QStringList& actions , int winId = 0);
	Q_SIGNALS:
		void notificatonClosed( int id);
		void actionInvoked(int id,int action);
		
	private Q_SLOTS:
		void slotPluginFinished(int id);
		
	private:
		
		struct Event
		{
			int id;
			int ref;
		};
		
		int m_counter;
		QHash<QString, KNotifyPlugin *> m_plugins;
		QHash<int , Event > m_notifications;
		void loadConfig();
};

class KNotifyAdaptor : public QDBusAbstractAdaptor
{
	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "org.kde.KNotify")
	public:
		KNotifyAdaptor(QObject *parent);

	public Q_SLOTS:
	
		void reconfigure();
		void closeNotification( int id);
		
		int event(const QString &event, const QString &fromApp, const ContextList& contexts ,
				   const QString &text, const QPixmap& pixmap,  const QStringList& actions , int winId = 0);
	Q_SIGNALS:
		void notificatonClosed( int id);
		void actionInvoked(int id,int action);
};

#endif

