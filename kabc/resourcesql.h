#ifndef KABC_RESOURCESQL_H
#define KABC_RESOURCESQL_H

#include "addressbook.h"
#include "resource.h"

class QSqlDatabase;

namespace KABC {

class ResourceSql : public Resource {
  public:
    ResourceSql( AddressBook *ab, const QString &user, const QString &password,
                 const QString &host );
  
    bool open();
    void close();
  
    bool load( AddressBook * );
    bool save();

    enum EntryKind { FamilyName = 1, GivenName , Email };
    
  private:
    QString mUser;
    QString mPassword;
    QString mHost;
  
    QSqlDatabase *mDb;
};

}

#endif
