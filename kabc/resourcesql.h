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
