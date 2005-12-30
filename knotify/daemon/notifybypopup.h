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

#ifndef NOTIFYBYPOPUP_H
#define NOTIFYBYPOPUP_H

#include "knotifyplugin.h"
#include <QHash>

class KPassivePopup;

class NotifyByPopup : public KNotifyPlugin
{ Q_OBJECT
	public:
		NotifyByPopup(QObject *parent=0l);
		virtual ~NotifyByPopup();
		
		virtual QString optionName() { return "Popup"; }
		virtual void notify(int id , KNotifyConfig *config);
		virtual void close( int id );
		
	private:
		QHash<int, KPassivePopup * > m_popups;
		
	private slots:
		void slotPopupDestroyed();
		void slotLinkClicked(const QString & );

};

#endif
