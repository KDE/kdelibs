/*
   This file is part of the KDB libraries
   Copyright (c) 2000 Praduroux Alessandro <pradu@thekompany.com>
 
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
 
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
 
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/     
#include "connectiondlg.h"
#include "connectionwidget.h"

#include <qlistview.h>
#include <qvbox.h>

#include <klocale.h>
#include <kiconloader.h>

#include "connectiondlg.moc"

using namespace KDB;


ConnectionDialog::ConnectionDialog(QWidget *parent, const char *name)
    :KDialogBase(parent, name, true, i18n(" Connection"), Help | Ok | Cancel, Ok, true)
{
    connectionWidget = new ConnectionWidget(this,"connectionWidget",false);
    setMainWidget(connectionWidget);
}

ConnectionDialog::~ConnectionDialog()
{
}

void
ConnectionDialog::setConnection(Connection *connection)
{
    connectionWidget->setConnection( connection );
}

Connection *
ConnectionDialog::connection()
{
    return connectionWidget->connection();
}

Connection *
ConnectionDialog::createConnection(QWidget *parent)
{
    Connection *connection = 0L;
    ConnectionDialog *dialog = new ConnectionDialog(parent);

    if ( dialog->exec() == KDialogBase::Accepted )
    {
        dialog->connectionWidget->createConnection();
	connection = dialog->connection();
    }

    delete dialog;
    return connection;
}

Connection *
ConnectionDialog::editConnection(Connection *connection, QWidget *parent)
{
    Connection * result = 0L;
    ConnectionDialog *dialog = new ConnectionDialog( parent );

    dialog->setConnection( connection );
    
    if ( dialog->exec() == KDialogBase::Accepted )
    {
	dialog->connectionWidget->createConnection();
        result = dialog->connection();
    }
	    
    delete dialog;
    return result;
}
