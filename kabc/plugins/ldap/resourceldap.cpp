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

#include <kdebug.h>
#include <klineedit.h>
#include <klocale.h>

#include <stdlib.h>

#include "resourceconfigwidget.h"
#include "resourceldap.h"
#include "resourceldapconfig.h"

using namespace KABC;

extern "C"
{
  ResourceConfigWidget *config_widget( QWidget *parent ) {
    return new ResourceLDAPConfig( parent, "ResourceLDAPConfig" );
  }

  Resource *resource( AddressBook *ab, const KConfig *config ) {
    return new ResourceLDAP( ab, config );
  }
}

void addModOp( LDAPMod ***pmods, const QString &attr, const QString &value );

ResourceLDAP::ResourceLDAP( AddressBook *ab, const KConfig *config )
  : Resource( ab )
{
  mLdap = 0;

  mUser = config->readEntry( "LdapUser" );
  mPassword = cryptStr( config->readEntry( "LdapPassword" ) );
  mDn = config->readEntry( "LdapDn" );
  mHost = config->readEntry( "LdapHost" );
  mPort = config->readEntry( "LdapPort" );
  mFilter = config->readEntry( "LdapFilter" );
}

ResourceLDAP::ResourceLDAP( AddressBook *ab, const QString &user,
	const QString &password, const QString &dn,
	const QString &host, const QString &port, const QString &filter )
    : Resource( ab )
{
  mLdap = 0;

  mUser = user;
  mPassword = password;
  mDn = dn;
  mHost = host;
  mPort = port;
  mFilter = filter;
}

Ticket *ResourceLDAP::requestSaveTicket()
{
  if ( !addressBook() ) {
	  kdDebug(5700) << "no addressbook" << endl;
    return 0;
  }

  return createTicket( this );
}

bool ResourceLDAP::open()
{
  if ( mLdap )
	  return false;

  if ( mPort.isEmpty() )
	  mPort = "389";

  mLdap = ldap_init( mHost.latin1(), mPort.toInt() );
  if ( !mLdap ) {
	  addressBook()->error( i18n( "Unable to connect to server '%1' on port '%2'" ).arg( mHost ).arg( mPort ) );
	  return false;
  }

  if ( !mUser.isEmpty() ) {
	  if ( ldap_simple_bind_s( mLdap, mUser.latin1(), mPassword.latin1() ) != LDAP_SUCCESS ) {
	    addressBook()->error( i18n( "Unable to bind to server '%1'" ).arg( mHost ) );
      return false;
    }

    kdDebug(5700) << "ResourceLDAP: bind to server successfully" << endl;
  }

  int deref = LDAP_DEREF_ALWAYS;
  if ( ldap_set_option( mLdap, LDAP_OPT_DEREF, (void *) &deref ) != LDAP_OPT_SUCCESS ) {
    kdDebug(5700) << "ResourceLDAP: can't set 'deref' option" << endl;
    return false;
  }

  if ( ldap_set_option( mLdap, LDAP_OPT_REFERRALS, LDAP_OPT_ON ) != LDAP_OPT_SUCCESS ) {
    kdDebug(5700) << "ResourceLDAP: can't set 'referrals' option" << endl;
    return false;
  }

  return true;
}

void ResourceLDAP::close()
{
  if ( ldap_unbind_s( mLdap ) != LDAP_SUCCESS ) {
    kdDebug(5700) << "ResourceLDAP: can't unbind from server" << endl;
    return;
  }

  mLdap = 0;
}

bool ResourceLDAP::load()
{
  LDAPMessage *res;
  LDAPMessage *msg;
  BerElement *track;
  char *name;
  char **values;

  const char *LdapSearchAttr[ 5 ] = {
    "uid",
    "cn",
    "mail",
    "phoneNumber",
    0 };

  if ( ldap_search_s( mLdap, mDn.latin1(), LDAP_SCOPE_SUBTREE, QString( "(%1)" ).arg( mFilter ).latin1(),
      (char **)LdapSearchAttr, 0, &res ) != LDAP_SUCCESS ) {
    addressBook()->error( i18n( "Unable to search on server '%1'" ).arg( mHost ) );
    return false;
  }

  for ( msg = ldap_first_entry( mLdap, res ); msg; msg = ldap_next_entry( mLdap, msg ) ) {
    Addressee addr;
    addr.setResource( this );
    for ( name = ldap_first_attribute( mLdap, msg, &track ); name; name = ldap_next_attribute( mLdap, msg, track ) ) {
      values = ldap_get_values( mLdap, msg, name );
      for ( int i = 0; i < ldap_count_values( values ); ++i ) {
        if ( qstricmp( name, "uid" ) == 0 ) {
          addr.setUid( values[ i ] );
          continue;
        }
        if ( qstricmp( name, "cn" ) == 0 ) {
          addr.setNameFromString( values[ i ] );
          continue;
        }
        if ( qstricmp( name, "mail" ) == 0 ) {
          addr.insertEmail( values[ i ] );
          continue;
        }
        if ( qstricmp( name, "phoneNumber" ) == 0 ) {
          PhoneNumber phone;
          phone.setNumber( values[ i ] );
          addr.insertPhoneNumber( phone );
          break; // read only the home number
        }
      }
      ldap_value_free( values );
    }
    ber_free( track, 0 );

    addressBook()->insertAddressee( addr );
  }

  ldap_msgfree( res );

  return true;
}

bool ResourceLDAP::save( Ticket * )
{
  AddressBook::Iterator it;
  for ( it = addressBook()->begin(); it != addressBook()->end(); ++it ) {
    if ( (*it).resource() == this && (*it).changed() ) {
      LDAPMod **mods = NULL;
	
      addModOp( &mods, "objectClass", "person" );
      addModOp( &mods, "uid", (*it).uid() );
      addModOp( &mods, "cn", (*it).formattedName() );

      QStringList emails = (*it).emails();
      QStringList::ConstIterator mailIt;
      for ( mailIt = emails.begin(); mailIt != emails.end(); ++mailIt ) {
        QString email = (*mailIt);
        addModOp( &mods, "mail", email );
      }

      PhoneNumber number = (*it).phoneNumber( PhoneNumber::Home );
      addModOp( &mods, "phoneNumber", number.number() );

      QString dn = "uid=" + (*it).uid() + "," + mDn;

      int retval;
      if ( (retval = ldap_add_s( mLdap, dn.latin1(), mods )) != LDAP_SUCCESS )
         addressBook()->error( i18n( "Unable to modify '%1' on server '%2'" ).arg( (*it).uid() ).arg( mHost ) );

      ldap_mods_free( mods, 1 );

      // mark as unchanged
      (*it).setChanged( false );
    }
  }

  return true;
}

void ResourceLDAP::removeAddressee( const Addressee &addr )
{
  LDAPMessage *res;
  LDAPMessage *msg;

  QString filter = QString( "(&(uid=%1)(%2))" ).arg( addr.uid() ).arg( mFilter );

  kdDebug(5700) << "ldap:removeAddressee" << filter << endl;

  ldap_search_s( mLdap, mDn.latin1(), LDAP_SCOPE_SUBTREE, filter.latin1(),
      0, 0, &res );

  for ( msg = ldap_first_entry( mLdap, res ); msg; msg = ldap_next_entry( mLdap, msg ) ) {
    char *dn = ldap_get_dn( mLdap, msg );
    kdDebug(5700) << "found " << dn << endl;
    if ( ldap_delete_s( mLdap, dn ) != LDAP_SUCCESS )
      addressBook()->error( i18n( "Unable to delete '%1' on server '%2'" ).arg( dn ).arg( mHost ) );
    ldap_memfree( dn );
  }

  ldap_msgfree( res );
}

QString ResourceLDAP::identifier() const
{
  return mHost + "_" + mPort + "_" + mDn + "_" + mFilter;
}

void addModOp( LDAPMod ***pmods, const QString &attr, const QString &value )
{
  if ( value.isNull() )
    return;

  LDAPMod	**mods;

  mods = *pmods;

  uint i = 0;
  if ( mods != 0 )
    for ( ; mods[ i ] != 0; ++i );

  if (( mods = (LDAPMod **)realloc( mods, (i + 2) * sizeof( LDAPMod * ))) == 0 ) {
    kdError() << "ResourceLDAP: realloc" << endl;
    return;
  }

  *pmods = mods;
  mods[ i + 1 ] = 0;

  mods[ i ] = new LDAPMod;

  mods[ i ]->mod_op = 0;
  mods[ i ]->mod_type = strdup( attr.latin1() );
  mods[ i ]->mod_values = new char*[2];
  mods[ i ]->mod_values[0] = strdup( value.latin1() );
  mods[ i ]->mod_values[1] = 0;
}
