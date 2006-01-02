/* This file is part of the KDE libraries
   Copyright (C) 2005 Olivier Goffart <ogoffart @ kde.org>

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




#ifndef KNOTIFICATIONMANAGER_H
#define KNOTIFICATIONMANAGER_H

#include <dcopobject.h>
#include <knotification.h>

class KNotification;
class QPixmap;
class QStringList;

/**
 * @internal
 * class that communique with the KNotify server.
 * don't use this class dirrectly,  use KNotification instead
 * @author Olivier Goffart
 */
class KNotificationManager :  public DCOPObject
{  
	K_DCOP
	public: 
		static KNotificationManager* self();		
		~KNotificationManager();
		
		/**
		 * send the dcop call to the knotify server
		 * @return the identification number
		 */
		unsigned int notify(KNotification *n, const QPixmap& pix, const QStringList &action ,
							const KNotification::ContextList& contexts, const QString &appname);
		
		/**
		 * send the close dcop call to the knotify server for the notification with the identifier @p id .
		 * @param id the id of the notification
		 */
		void close( int id);
		
		/**
		 * remove the KNotification ptr from the internal map
		 * To be called in the KNotification destructor
		 */
		void remove( int id);	
		
	k_dcop:
			ASYNC notificationClosed( int id );
		ASYNC notificationActivated( int id,  int action);

	private:
		struct Private;
		Private *d;
		static KNotificationManager* s_self;
		KNotificationManager();

};

#endif
