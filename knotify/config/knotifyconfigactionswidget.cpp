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
#include "knotifyconfigactionswidget.h"
#include "knotifyconfigelement.h"

#include <kiconloader.h>

KNotifyConfigActionsWidget::KNotifyConfigActionsWidget( QWidget * parent )
	: QWidget(parent)
{
	m_ui.setupUi(this);
	m_ui.Sound_play->setIcon( QIcon( SmallIcon("play")));
}



void KNotifyConfigActionsWidget::setConfigElement( KNotifyConfigElement * config )
{
	QString prstring=config->readEntry( "Action" );
	QStringList actions=prstring.split ("|");

	m_ui.Sound_check->setChecked( actions.contains("Sound") );
	m_ui.Popup_check->setChecked( actions.contains("Popup") );
	m_ui.Logfile_check->setChecked( actions.contains("Logfile") );
	m_ui.Execute_check->setChecked( actions.contains("Execute") );
	m_ui.Taskbar_check->setChecked( actions.contains("Taskbar") );
	
	m_ui.Sound_select->setURL( config->readEntry( "sound" , true ) );
	m_ui.Logfile_select->setURL( config->readEntry( "logfile" , true ) );
	m_ui.Execute_select->setURL( config->readEntry( "execute"  ) );
}

void KNotifyConfigActionsWidget::save( KNotifyConfigElement * config )
{
	QStringList actions;
	if(m_ui.Sound_check->isChecked())
		actions << "Sound";
	if(m_ui.Popup_check->isChecked())
		actions << "Popup";
	if(m_ui.Logfile_check->isChecked())
		actions << "Logfile";
	if(m_ui.Execute_check->isChecked())
		actions << "Execute";
	if(m_ui.Taskbar_check->isChecked())
		actions << "Taskbar";
	
	config->writeEntry( "Action" , actions.join("|") );
	
	config->writeEntry( "sound" , m_ui.Sound_select->url() );
	config->writeEntry( "logfile" , m_ui.Logfile_select->url() );
	config->writeEntry( "execute" , m_ui.Execute_select->url() );
}

