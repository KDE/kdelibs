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



#ifndef NOTIFYBYLOGFILE_H
#define NOTIFYBYLOGFILE_H

#include <knotifyplugin.h>


class NotifyByLogfile : public KNotifyPlugin
{ Q_OBJECT
	public:
		NotifyByLogfile(QObject *parent=0l);
		virtual ~NotifyByLogfile();
		
		virtual QString optionName() { return "Logfile"; };
		virtual void notify(int id , KNotifyConfig *config);
};

#endif
