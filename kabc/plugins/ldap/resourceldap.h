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

#ifndef KABC_RESOURCELDAP_H
#define KABC_RESOURCELDAP_H

#include <kconfig.h>

#include <lber.h>
#include <ldap.h>

#include "addressbook.h"
#include "resource.h"

namespace KABC {

class ResourceLDAP : public Resource
{
public:

  ResourceLDAP( AddressBook *ab, const KConfig * );
  ResourceLDAP( AddressBook *ab, const QString &user, const QString &password,
      const QString &dn, const QString &host,
	    const QString &port, const QString &filter );
  
  bool open();
  void close();
  
  Ticket *requestSaveTicket();

  bool load();
  bool save( Ticket * );

  void removeAddressee( const Addressee& addr );

  QString identifier() const;

private:
  QString mUser;
  QString mPassword;
  QString mDn;
  QString mHost;
  QString mPort;
  QString mFilter;

  LDAP *mLdap;
};

}
#endif
