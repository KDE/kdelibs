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

#ifndef KNOTIFY_H
#define KNOTIFY_H

#include <QObject>
#include <dcopobject.h>
#include <QPixmap>
#include <QHash>
#include <QString>
#include <QList>
#include <QPair>


typedef QHash<QString,QString> Dict;
typedef QList< QPair<QString,QString> > ContextList;

class KNotifyPlugin;


class KNotify : public QObject, public DCOPObject
{
	Q_OBJECT
	K_DCOP

	public:
		KNotify(QObject *parent=0l);
		~KNotify();
		void addPlugin( KNotifyPlugin *p );

	protected:
	k_dcop:
	
		void reconfigure();
		void closeNotification( int id);
		
		int event(const QString &event, const QString &fromApp, const ContextList& contexts ,
				   const QString &text, const QPixmap& pixmap,  const QStringList& actions , int winId = 0);
	k_dcop_signals: protected slots: 
		void notificatonClosed( int id);
		void actionInvoked(int id,int action);
		
	private slots:
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


#endif

