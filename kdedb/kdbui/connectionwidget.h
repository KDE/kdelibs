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
#ifndef CONNECTIONWIDGET_H
#define CONNECTIONWIDGET_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "connectionbase.h"

namespace KDB {

class Database;
class Connection;
class DBListView;

/**
 *
 * This widget will create new connections and store them in
 * the kdb rc file
 *
 * @author Alessandro Praduroux <pradu@thekompany.com>
 * @author Michael Koch <m_kock@bigfoot.de>
 * @version kdbui 0.0.2
 */
class ConnectionWidget: public ConnectionBase {

    Q_OBJECT

public:

    ConnectionWidget(QWidget *parent, const char * name = 0, bool showDatabase = true);

    ~ConnectionWidget();

    Database *database();

    void setConnection(Connection *);
    
    Connection *connection();
    
    void createConnection();
   
 signals:

    void databaseSelected();

    void nonDatabaseSelected();

 protected:
 
    void slotDatabaseSelected();
    void slotCreateConnection();

 private:
    
    Connection *conn;
    DBListView *databaseList;
    
};

}
#endif
