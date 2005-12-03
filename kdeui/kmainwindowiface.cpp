/* This file is part of the KDE project
   Copyright (C) 2001 Ian Reinhart Geiser <geiseri@yahoo.com>

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

#include "kmainwindowiface.h"

#include <dcopclient.h>
#include <kapplication.h>
#include <kdcopactionproxy.h>
#include <kdcoppropertyproxy.h>
#include <kmainwindow.h>
#include <kaction.h>
#include <qclipboard.h>


KMainWindowInterface::KMainWindowInterface(KMainWindow * mainWindow)
	: DCOPObject( mainWindow->name()) 
{
	m_MainWindow = mainWindow;
	m_dcopActionProxy = new KDCOPActionProxy( m_MainWindow->actionCollection(), this );
	m_dcopPropertyProxy = new KDCOPPropertyProxy(m_MainWindow);
}

KMainWindowInterface::~KMainWindowInterface()
{
	delete m_dcopActionProxy;
        delete m_dcopPropertyProxy;
}

DCOPCStringList KMainWindowInterface::actions()
{
	delete m_dcopActionProxy;
	m_dcopActionProxy = new KDCOPActionProxy( m_MainWindow->actionCollection(), this );
	DCOPCStringList tmp_actions;
	QList<KAction *> lst = m_dcopActionProxy->actions();
	foreach( KAction*it, lst ) {
		if (it->isPlugged())
			tmp_actions.append( it->name() );
	}
	return tmp_actions;
}

bool KMainWindowInterface::activateAction( const DCOPCString& action)
{
	delete m_dcopActionProxy;
	m_dcopActionProxy = new KDCOPActionProxy( m_MainWindow->actionCollection(), this );
	KAction *tmp_Action = m_dcopActionProxy->action(action);
	if (tmp_Action)
	{
		tmp_Action->activate();
		return true;
	}
	else
		return false;
}

bool KMainWindowInterface::disableAction( const DCOPCString& action)
{
	delete m_dcopActionProxy;
	m_dcopActionProxy = new KDCOPActionProxy( m_MainWindow->actionCollection(), this );
	KAction *tmp_Action = m_dcopActionProxy->action(action);
	if (tmp_Action)
	{
		tmp_Action->setEnabled(false);
		return true;
	}
	else
		return false;
}

bool KMainWindowInterface::enableAction( const DCOPCString& action)
{
	delete m_dcopActionProxy;
	m_dcopActionProxy = new KDCOPActionProxy( m_MainWindow->actionCollection(), this );
	KAction *tmp_Action = m_dcopActionProxy->action(action);
	if (tmp_Action)
	{
		tmp_Action->setEnabled(true);
		return true;
	}
	else
		return false;
}

bool KMainWindowInterface::actionIsEnabled( const DCOPCString& action)
{
	delete m_dcopActionProxy;
	m_dcopActionProxy = new KDCOPActionProxy( m_MainWindow->actionCollection(), this );
	KAction *tmp_Action = m_dcopActionProxy->action(action);
	if (tmp_Action)
	{
		return tmp_Action->isEnabled();
	}
	else
		return false;
}

DCOPCString KMainWindowInterface::actionToolTip( const DCOPCString& action)
{
	delete m_dcopActionProxy;
	m_dcopActionProxy = new KDCOPActionProxy( m_MainWindow->actionCollection(), this );
	KAction *tmp_Action = m_dcopActionProxy->action(action);
	if (tmp_Action)
	{
		return tmp_Action->toolTip().toUtf8();
	}
	else
		return "Error no such object!";
}

DCOPRef KMainWindowInterface::action( const DCOPCString& name )
{
	return DCOPRef( KApplication::dcopClient()->appId(), m_dcopActionProxy->actionObjectId( name ) );
}

QMap<DCOPCString,DCOPRef> KMainWindowInterface::actionMap()
{
	return m_dcopActionProxy->actionMap();
}

int KMainWindowInterface::getWinID()
{
	return (int) m_MainWindow->winId();
}

void KMainWindowInterface::grabWindowToClipBoard()
{
	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setPixmap(QPixmap::grabWidget(m_MainWindow));
}

void KMainWindowInterface::hide()
{
	m_MainWindow->hide();
}

void KMainWindowInterface::maximize()
{
	m_MainWindow->showMaximized();
}

void KMainWindowInterface::minimize()
{
	m_MainWindow->showMinimized();
}

void KMainWindowInterface::resize(int newX, int newY)
{
	m_MainWindow->resize(newX, newY);
}

void KMainWindowInterface::move(int newX, int newY)
{
	m_MainWindow->move(newX, newY);
}

void KMainWindowInterface::setGeometry(int newX, int newY, int newWidth, int newHeight)
{
	m_MainWindow->setGeometry(newX, newY, newWidth, newHeight);
}

void KMainWindowInterface::raise()
{
	m_MainWindow->raise();
}

void KMainWindowInterface::lower()
{
	m_MainWindow->lower();
}

void KMainWindowInterface::restore()
{
	m_MainWindow->showNormal();
}

void KMainWindowInterface::show()
{
	m_MainWindow->show();
}

DCOPCStringList KMainWindowInterface::functionsDynamic()
{
	return m_dcopPropertyProxy->functions();
}

bool KMainWindowInterface::processDynamic(const DCOPCString &fun, const QByteArray &data, DCOPCString& replyType, QByteArray &replyData)
{
	return m_dcopPropertyProxy->processPropertyRequest( fun, data, replyType, replyData);

}

