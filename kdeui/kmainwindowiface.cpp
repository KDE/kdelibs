/* This file is part of the KDE project
   Copyright (C) 2001 Ian Reinhart Geiser <geiseri@yahoo.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kmainwindowiface.h"

#include <dcopclient.h>
#include <kapp.h>
#include <kdcopactionproxy.h>
#include <kmainwindow.h>
#include <kaction.h>

KMainWindowInterface::KMainWindowInterface(KMainWindow * mainWindow)
	: DCOPObject( mainWindow->name())
{
	m_dcopActionProxy = new KDCOPActionProxy( mainWindow->actionCollection(), this );
}

KMainWindowInterface::~KMainWindowInterface()
{
  delete m_dcopActionProxy;
}

QCStringList KMainWindowInterface::actions()
{
	QCStringList tmp_actions;
	QValueList<KAction *> lst = m_dcopActionProxy->actions();
	QValueList<KAction *>::ConstIterator it = lst.begin();
	QValueList<KAction *>::ConstIterator end = lst.end();
	for (; it != end; ++it )
		if ( (*it)->isEnabled())
			tmp_actions.append( (QCString)(*it)->name() );
	return tmp_actions;
}
bool KMainWindowInterface::activate( QCString action)
{
	KAction *tmp_Action = m_dcopActionProxy->action(action);
	if (tmp_Action)
	{
		tmp_Action->activate();
		return true;
	}
	else
		return false;
}
bool KMainWindowInterface::disable( QCString action)
{

}
bool KMainWindowInterface::enable( QCString action)
{

}
bool KMainWindowInterface::status( QCString action)
{

}
QCString KMainWindowInterface::tooltip( QCString action)
{

}

/*
QCStringList KMainWindowInterface::interfaces()
{
	QCStringList tmp_DCOPinterfaces = DCOPObject::interfaces();

	return tmp_DCOPinterfaces;
}
bool KMainWindowInterface::process( const QCString &functionName, const QByteArray &argData, QCString &returnType, QByteArray &returnData)
{
	QCString tmp_actionName = functionName;
	tmp_actionName.truncate( functionName.length()-2 );
	KAction *tmp_Action = m_dcopActionProxy->action(tmp_actionName);
	if (tmp_Action->isEnabled())
	{
		tmp_Action->activate();
		return true;
	}
	else
		return DCOPObject::process( functionName, argData, returnType, returnData );
}
*/