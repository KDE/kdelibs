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



#ifndef KNOTIFYPLUGIN_H
#define KNOTIFYPLUGIN_H

#include <qobject.h>

class KNotifyConfig;


/**
 * abstract class for action
 * @author Olivier Goffart <ogoffart at kde.org>
*/
class KNotifyPlugin : public QObject
{ Q_OBJECT
	public:
		KNotifyPlugin(QObject *parent=0l);
		virtual ~KNotifyPlugin();
		
		virtual QString optionName() =0;
		virtual void notify(int id , KNotifyConfig *config )=0;
		virtual void close(int id) { emit finished(id);}
	
	protected:
		void finish(int id) { emit finished(id); }
	
	signals:
		void finished(int id);
		void actionInvoked(int id , int action);
};

#endif
