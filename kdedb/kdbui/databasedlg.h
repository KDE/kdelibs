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

#ifndef DATABASEDLG_H
#define DATABASEDLG_H "$Id$"

#include <kdialogbase.h>

class QListView;

namespace KDB {

class Database;
class ConnectionWidget;
class DBListView;

class DatabaseDialogPrivate; 
/**
 * Open database dialog.
 *
 * This dialog can be used to open a database.
 * It will let the user to choose from a saved connection or to
 * create a new connection.
 *
 * @author Alessandro Praduroux <pradu@thekompany.com>
 * @author Michael Koch <m_kock@bigfoot.de>
 * @version kdbui 0.0.2
 */
class DatabaseDialog: public KDialogBase {

    Q_OBJECT

 public:

    DatabaseDialog(QWidget *parent = 0, const char * name = 0);

    ~DatabaseDialog();

    Database * database();

    static Database * openDatabase(QWidget *parent);

 protected:

    void accept();

 protected slots:

    void slotEnableOK();

    void slotDisableOK();
    
 private:

    Database * db;
    DBListView *connectionList;
    ConnectionWidget *createWidget;

    DatabaseDialogPrivate *d;
};

}
#endif
