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
#ifndef DBLISTVIEW_H
#define DBLISTVIEW_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qlistview.h>

class KConfig;

namespace KDB {

class Object;
class Plugin;
class Database;
class Connection;

class DBListViewItem: public QListViewItem {

public:
    DBListViewItem(QListView * parent, Object *obj);
    DBListViewItem(QListViewItem * parent, Object *obj);

    void init();

    Object *itemObject();

private:
    Object * m_obj;
};

class DBListViewPrivate; 
/**
 * Database list view.
 *
 * This is a list view that will display all accessible databases.
 *
 * It can be used  in 'auto' mode, 'semiauto' mode  or in 'manual' mode.
 *
 * in 'auto' mode will load all available plugins, connections, and
 * databases for available connections.
 *
 * in 'semiauto' mode to display the database list the function addConnection
 * must be called to add single connection objects to the display.
 * addConnection also tries to find all available database using the connection.
 *
 * in 'manual' mode to display the database list the function addConnection
 * must be called to add single connection objects to the display.
 *
 * 
 * @author Alessandro Praduroux <pradu@thekompany.com>
 * @author Michael Koch <m_kock@bigfoot.de>
 * @version kdbui 0.0.2
 */
class DBListView: public QListView {

    Q_OBJECT

 public:
    enum { Auto, SemiAuto, Manual };

    DBListView(QWidget *parent, const char * name = 0, bool displayTree = true, int mode = Auto);

    ~DBListView();

    Database * selectedDatabase();

    virtual DBListViewItem * addConnection(Connection *conn);

    virtual DBListViewItem * addPlugin(Plugin *plugin);

    virtual DBListViewItem * addDatabase(Database *database);

 signals:

    void databaseSelected(KDB::Database *);

    void nonDatabaseSelected();

 protected slots:

    void slotItemSelected(QListViewItem *);

 private:

    DBListViewItem * addConnection(Connection *conn, DBListViewItem *item);

    bool m_tree;
    int m_mode;

    Database * currentDB;
    DBListViewPrivate *d;
};

}
#endif

