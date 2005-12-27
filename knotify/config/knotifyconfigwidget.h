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

#ifndef KNOTIFYCONFIGWIDGET_H
#define KNOTIFYCONFIGWIDGET_H

#include <kvbox.h>
#include <QString>

class KNotifyConfigElement;

/**
 * Configure the notification for a given application / context
 * 
 * you must call setApplication before showing it
 * 
 * @author Olivier Goffart <ogoffart @ kde.org>
 */
class KDE_EXPORT KNotifyConfigWidget : public KVBox
{ Q_OBJECT
public:
	KNotifyConfigWidget( QWidget *parent);
	~KNotifyConfigWidget();
	
	/**
	 * Change the application and the context
	 * 
	 * @param appname name of the application.   if null kapp->instanceName() is used
	 * @param context_name the name of the context, if null , avery context are considered
	 * @param context_value the context value
	 */
	void setApplication( const QString & appname = QString::null ,
						 const QString & context_name = QString::null,
						 const QString & context_value = QString::null);
	
	/**
	 * save to the config file
	 */
	void save();

private:
	struct Private;
	Private *d;
private slots:
	void slotEventSelected( KNotifyConfigElement *e);
	
};

#endif
