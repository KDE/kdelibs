/* This file is part of the KDE libraries
   Copyright (C) 2005 Olivier Goffart <ogoffart at kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KNOTIFYCONFIGWIDGET_H
#define KNOTIFYCONFIGWIDGET_H

#include <knotifyconfig_export.h>
#include <QtGui/QWidget>
#include <QtCore/QString>

class KNotifyConfigElement;

/**
 * Configure the notification for a given application / context
 * 
 * You probably will want to use the static function configure
 * 
 * If you create the widget yourself, you must call setApplication before showing it
 * 
 * @author Olivier Goffart <ogoffart @ kde.org>
 */
class KNOTIFYCONFIG_EXPORT KNotifyConfigWidget : public QWidget
{ 
	Q_OBJECT
public:
	KNotifyConfigWidget( QWidget *parent);
	~KNotifyConfigWidget();
	
	/**
	 * Show a dialog with the widget.
	 * @param parent the parent widget of the dialog
	 * @param appname the application name,  if null, it is autodetected
	 * @return the widget itself    the topLevelWidget of it is probably a KDialog
	 */
	static KNotifyConfigWidget *configure(QWidget *parent = 0l, const QString &appname=QString());
	
	/**
	 * Change the application and the context
	 * 
	 * @param appname name of the application.   if null KGlobal::mainComponent().componentName() is used
	 * @param context_name the name of the context, if null , avery context are considered
	 * @param context_value the context value
	 */
	void setApplication( const QString & appname = QString() ,
						 const QString & context_name = QString(),
						 const QString & context_value = QString());
	
public Q_SLOTS:
	/**
	 * save to the config file
	 */
	void save();
	
Q_SIGNALS:
	/**
	 * Indicate that the state of the modules contents has changed. 
	 * This signal is emitted whenever the state of the configuration changes. 
	 * @see KCModule::changed
	 */
	void changed(bool state);

private:
	struct Private;
	Private* const d;
private Q_SLOTS:
	void slotEventSelected( KNotifyConfigElement *e);
	void slotActionChanged();
	
};

#endif
