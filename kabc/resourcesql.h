/*
    This file is part of libkabc.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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

#ifndef KABC_RESOURCESQL_H
#define KABC_RESOURCESQL_H

#include <kconfig.h>

#include "addressbook.h"
#include "resource.h"

class QSqlDatabase;

namespace KABC {

class ResourceSql : public Resource
{
public:
    ResourceSql( AddressBook *ab, const QString &user, const QString &password,
	    const QString &db, const QString &host );
    ResourceSql( AddressBook *ab, const KConfig * );
  
    bool open();
    void close();
  
    Ticket *requestSaveTicket();

    bool load();
    bool save( Ticket * );

    QString identifier() const;
    QString typeInfo() const;

private:
    void init(const QString &user, const QString &password,
	    const QString &db, const QString &host );

    QString mUser;
    QString mPassword;
    QString mDbName;
    QString mHost;

    QSqlDatabase *mDb;
};

}

#endif
