/* This file is part of the KDE project
   Copyright (C) 2001 Ian Reinhart Geiser <geiseri@yahoo.com>
   Copyright (C) 2006 Thiago Macieira <thiago@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the Lesser GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the Lesser GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kmainwindowiface_p.h"

#include <kactioncollection.h>
#include <kapplication.h>
#include <kmainwindow.h>
#include <kaction.h>
#include <qclipboard.h>


KMainWindowInterface::KMainWindowInterface(KMainWindow * mainWindow)
        : QDBusAbstractAdaptor(mainWindow)
{
	m_MainWindow = mainWindow;
	//m_dcopActionProxy = new KDCOPActionProxy( m_MainWindow->actionCollection(), this );
}

KMainWindowInterface::~KMainWindowInterface()
{
        //delete m_dcopActionProxy;
}

QStringList KMainWindowInterface::actions()
{
	QStringList tmp_actions;
	QList<KAction *> lst = m_MainWindow->actionCollection()->actions();
	foreach( KAction* it, lst ) {
		if (it->isPlugged())
			tmp_actions.append( it->objectName() );
	}
	return tmp_actions;
}

bool KMainWindowInterface::activateAction( const QString& action )
{
	KAction *tmp_Action = m_MainWindow->actionCollection()->action(action);
	if (tmp_Action)
	{
		tmp_Action->trigger();
		return true;
	}
	else
		return false;
}

bool KMainWindowInterface::disableAction( const QString& action)
{
	KAction *tmp_Action = m_MainWindow->actionCollection()->action(action);
	if (tmp_Action)
	{
		tmp_Action->setEnabled(false);
		return true;
	}
	else
		return false;
}

bool KMainWindowInterface::enableAction( const QString& action)
{
	KAction *tmp_Action = m_MainWindow->actionCollection()->action(action);
	if (tmp_Action)
	{
		tmp_Action->setEnabled(true);
		return true;
	}
	else
		return false;
}

bool KMainWindowInterface::actionIsEnabled( const QString& action)
{
	KAction *tmp_Action = m_MainWindow->actionCollection()->action(action);
	if (tmp_Action)
	{
		return tmp_Action->isEnabled();
	}
	else
		return false;
}

QString KMainWindowInterface::actionToolTip( const QString& action)
{
	KAction *tmp_Action = m_MainWindow->actionCollection()->action(action);
	if (tmp_Action)
	{
		return tmp_Action->toolTip().toUtf8();
	}
	else
		return "Error no such object!";
}

qlonglong KMainWindowInterface::winId()
{
	return qlonglong(m_MainWindow->winId());
}

void KMainWindowInterface::grabWindowToClipBoard()
{
	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setPixmap(QPixmap::grabWidget(m_MainWindow));
}

#include "kmainwindowiface_p.moc"
