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
#ifndef CONNECTIONDLG_H
#define CONNECTIONDLG_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kdialogbase.h>

class QListView;

namespace KDB {

class ConnectionWidget;
class Connection;
 
/**
 * Connection dialog
 *
 * This dialog can be used to setup a connection 
 *  
 * @author Alessandro Praduroux <pradu@thekompany.com>
 * @author Michael Koch <m_kock@bigfoot.de>
 * @version kdbui 0.0.2
 */
class ConnectionDialog: public KDialogBase {

    Q_OBJECT

 public:

    ConnectionDialog(QWidget *parent = 0L, const char * name = 0L);

    ~ConnectionDialog();

    void setConnection(Connection *);
    
    Connection * connection();

    static Connection * createConnection(QWidget *parent = 0L);
    
    static Connection * editConnection(Connection *, QWidget *parent = 0L);
    
 private:

    ConnectionWidget *connectionWidget;
};

}
#endif

