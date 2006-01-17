/*
    This file is part of libkabc.
    Copyright (c) 2004 Szombathelyi György <gyurco@freemail.hu>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General  Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KABC_LDAPURL_H
#define KABC_LDAPURL_H

#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>

#include <kurl.h>

namespace KABC {

/**
 * LDAPUrl

 * LDAPUrl implements an RFC 2255 compliant LDAP Url parser, with minimal
 * differences. LDAP Urls implemented by this class has the following format:
 * ldap[s]://[user[:password]@]hostname[:port]["/" [dn ["?" [attributes] 
 * ["?" [scope] ["?" [filter] ["?" extensions]]]]]]
 */


  class KABC_EXPORT LDAPUrl : public KUrl
  {
  public:

    struct Extension {
      QString value;
      bool critical;
    };
    
    typedef enum Scope { Base, One, Sub };

    /** Constructs an empty KLDAPUrl. */
    LDAPUrl();
    /** Constructs a KLDAPUrl from a KUrl. */
    LDAPUrl( const KUrl &url );
  
    /**
     * Returns the dn part of the LDAP Url (same as path(), but slash removed
     * from the beginning).
     */
    const QString& dn() const { return m_dn; };
    /** Sets the the dn part of the LDAP Url. */ 
    void setDn( const QString &dn );

    /** Returns the attributes part of the LDAP Url */
    const QStringList &attributes() { return m_attributes; }
    /** Sets the attributes part of the LDAP Url */
    void setAttributes( const QStringList &attributes ) 
      { m_attributes=attributes; updateQuery(); }

    /** Returns the scope part of the LDAP Url */
    Scope scope() const { return m_scope; };
    /** Sets the scope part of the LDAP Url */
    void setScope(Scope scope) { m_scope = scope; updateQuery(); }

    /** Returns the filter part of the LDAP Url */
    const QString &filter() const { return m_filter; }
    /** Sets the filter part of the LDAP Url */
    void setFilter( QString filter ) { m_filter = filter; updateQuery(); }

    /** Returns if the specified extension exists in the LDAP Url */
    bool hasExtension( const QString &key ) const;
    /** Returns the specified extension */
    Extension extension( const QString &key ) const;
    /** Returns the specified extension */
    QString extension( const QString &key, bool &critical ) const;
    /** Sets the specified extension key with the value and criticality in ext */
    void setExtension( const QString &key, const Extension &ext );
    /** Sets the specified extension key with the value and criticality specified */
    void setExtension( const QString &key, const QString &value, bool critical = false );
    /** Removes the specified extension */
    void removeExtension( const QString &key );
    /** Updates the query component from the attributes, scope, filter and extensions */
    void updateQuery();
    
  protected:
    void parseQuery();

  private:

    QMap<QString, Extension> m_extensions;
    QString m_dn;
    QStringList m_attributes;
    Scope m_scope;
    QString m_filter;
  };
}

#endif
