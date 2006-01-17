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



#ifndef NOTIFYBYSOUND_H
#define NOTIFYBYSOUND_H

#include <knotifyplugin.h>

class KProcess;

class NotifyBySound : public KNotifyPlugin
{ Q_OBJECT
	public:
		NotifyBySound(QObject *parent=0l);
		virtual ~NotifyBySound();
		
		virtual QString optionName() { return "Sound"; };
		virtual void notify(int id , KNotifyConfig *config);
		
	private:
		class Private;
		Private* const d;
		void loadConfig();
		
	public:
		void setVolume( int v );
		bool event( QEvent *e );

		
	private Q_SLOTS:
		void slotPlayerProcessExited( KProcess *proc );
};

#endif
