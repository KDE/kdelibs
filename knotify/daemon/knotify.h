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

#include <QtDBus/QtDBus>


#include "knotifyconfig.h"


typedef QHash<QString,QString> Dict;


class KNotifyPlugin;


class KNotify : public QObject
{
	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "org.kde.KNotify")
	public:
		KNotify(QObject *parent=0l);
		~KNotify();
		void addPlugin( KNotifyPlugin *p );

	public Q_SLOTS:
		void reconfigure();
		void closeNotification( int id);
		
		int event(const QString &event, const QString &fromApp, const ContextList& contexts ,
				   const QString &text, const QPixmap& pixmap,  const QStringList& actions , WId winId = 0);
		
		void update(int id, const QString &text, const QPixmap& pixmap,  const QStringList& actions);
		void reemit(int id, const ContextList& contexts);
	Q_SIGNALS:
		void notificationClosed( int id);
		void notificationActivated(int id,int action);
		
	private Q_SLOTS:
		void slotPluginFinished(int id);
		
	private:
		
		struct Event
		{
			Event(const QString &appname, const ContextList &contexts , const QString &eventid)
				: config(appname, contexts , eventid) {} 
			int id;
			int ref;
			KNotifyConfig config;
		};
		
		int m_counter;
		QHash<QString, KNotifyPlugin *> m_plugins;
		QHash<int , Event* > m_notifications;
		void loadConfig();
		void emitEvent(Event *e);
};

class KNotifyAdaptor : public QDBusAbstractAdaptor
{
	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "org.kde.KNotify")
	Q_CLASSINFO("D-Bus Introspection", ""
					"<interface name=\"org.kde.KNotify\">"
						"<signal name=\"notificationClosed\">"
							"<arg name=\"id\" type=\"i\" direction=\"out\"/>"
						"</signal>"
						"<signal name=\"notificationActivated\">"
							"<arg name=\"id\" type=\"i\" direction=\"out\"/>"
							"<arg name=\"action\" type=\"i\" direction=\"out\"/>"
						"</signal>"
						"<method name=\"reconfigure\">"
						"</method>"
						"<method name=\"closeNotification\">"
							"<arg name=\"id\" type=\"i\" direction=\"in\"/>"
						"</method>"
						"<method name=\"event\">"
							"<arg type=\"i\" direction=\"out\"/>"
							"<arg name=\"event\" type=\"s\" direction=\"in\"/>"
							"<arg name=\"fromApp\" type=\"s\" direction=\"in\"/>"
							"<arg name=\"contexts\" type=\"a(ss)\" direction=\"in\"/>"
							"<arg name=\"text\" type=\"s\" direction=\"in\"/>"
							"<arg name=\"pixmap\" type=\"ay\" direction=\"in\"/>"
							"<arg name=\"actions\" type=\"as\" direction=\"in\"/>"
							"<arg name=\"winId\" type=\"x\" direction=\"in\"/>"
						"</method>"
						"<method name=\"update\">"
							"<arg name=\"id\" type=\"i\" direction=\"in\"/>"
							"<arg name=\"text\" type=\"s\" direction=\"in\"/>"
							"<arg name=\"pixmap\" type=\"ay\" direction=\"in\"/>"
							"<arg name=\"actions\" type=\"as\" direction=\"in\"/>"
						"</method>"
						"<method name=\"reemit\">"
							"<arg name=\"id\" type=\"i\" direction=\"in\"/>"
							"<arg name=\"contexts\" type=\"a(ss)\" direction=\"in\"/>"
						"</method>"

					"</interface>" )

	public:
		KNotifyAdaptor(QObject *parent);

	public Q_SLOTS:
	
		void reconfigure();
		void closeNotification( int id);
		
		int event(const QString &event, const QString &fromApp, const QVariantList& contexts ,
								const QString &text, const QByteArray& pixmap,  const QStringList& actions , qlonglong winId );
		
		void reemit(int id, const QVariantList& contexts);
		void update(int id, const QString &text, const QByteArray& pixmap,  const QStringList& actions );

	Q_SIGNALS:
		void notificationClosed( int id);
		void notificationActivated( int id,int action);
};

#endif

