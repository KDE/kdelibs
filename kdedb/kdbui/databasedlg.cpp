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
#include "databasedlg.h"
#include "connectionwidget.h"
#include "dblistview.h"

#include <qlistview.h>
#include <qvbox.h>

#include <klocale.h>
#include <kiconloader.h>

#include <kdb/dbengine.h>
#include <kdb/plugin.h>
#include <kdb/connection.h>
#include <kdb/database.h>

#include "databasedlg.moc"

using namespace KDB;


DatabaseDialog::DatabaseDialog(QWidget *parent, const char *name)
    :KDialogBase(KDialogBase::Tabbed, i18n("Open Database"), Help | Ok | Cancel, Ok, parent, name, true, true)
{
    enableButtonOK(false);

    // first page: select from an existing connection
    QVBox *choosePage = addVBoxPage(i18n("Existing Connection"));

    connectionList = new DBListView(choosePage,"ConnectionList");

    connect( connectionList,
             SIGNAL( databaseSelected( KDB::Database * ) ),
             SLOT( slotEnableOK() ) );

    connect( connectionList,
             SIGNAL( nonDatabaseSelected() ),
	     SLOT( slotDisableOK() ) );

    
    // second page: create new connection
    QVBox *createPage = addVBoxPage(i18n("New Connection"));

    createWidget = new ConnectionWidget(createPage);

    connect( createWidget,
             SIGNAL( databaseSelected() ),
             SLOT( slotEnableOK() ) );
    
}

DatabaseDialog::~DatabaseDialog()
{
}

void
DatabaseDialog::accept()
{
    // try to figure out the database and open it.
    switch (activePageIndex()) {
    case 0: // choose page
        {
            db = connectionList->selectedDatabase();
            break;
        }
    case 1: // build page
        {
            db = createWidget->database();
            break;
        }
    default:
        break;
    }
                
    done(Accepted);
}


void
DatabaseDialog::slotEnableOK()
{
    enableButtonOK(true);
}

void
DatabaseDialog::slotDisableOK()
{
    enableButtonOK(false);
}

Database*
DatabaseDialog::database()
{
    return db;
}

Database*
DatabaseDialog::openDatabase(QWidget *parent)
{
    Database* result = 0L;
    DatabaseDialog *dialog = new DatabaseDialog(parent);

    if (dialog->exec() == QDialog::Accepted) {
        result = dialog->database();
        result->open();
    }

    delete dialog;
    return result;
}


