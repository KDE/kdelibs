/*
    This file is part of libkabc.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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

#ifndef KABC_Messaging_H
#define KABC_Messaging_H

#include <qstring.h>
#include <qvaluelist.h>

namespace KABC {

/**
  @short Instant messaging data
  
  This class stores data for instant messaging.
*/
class Messaging
{
    friend QDataStream &operator<<( QDataStream &, const Messaging& );
    friend QDataStream &operator>>( QDataStream &, Messaging& );

  public:
    typedef QValueList<Messaging> List;

    /**
      Construct invalid Messaging object.
    */
    Messaging();

    /**
      Construct Messaging object of the given service type.
      
      @param serviceType  The service type.
    */
    Messaging( const QString &serviceType );

    /**
      Sets the uid.
     */
    void setId( const QString &id );

    /**
      Returns the uid.
     */
    QString id() const;

    /**
      Sets the service type.
    */
    void setServiceType( const QString &serviceType );

    /**
      Returns the service type.
    */
    QString serviceType() const;
    
    /**
      Sets the account id.
    */
    void setAccountId( const QString &account );

    /**
      Returns the account id.
    */
    QString accountId() const;

    /**
      Sets the sender account id.
    */
    void setSenderAccountId( const QString &account );

    /**
      Returns the sender account id.
    */
    QString senderAccountId() const;
  
    /**
      Returns, if this object is valid.
    */
    bool isValid() const;

    bool operator==( const Messaging & ) const;
    bool operator!=( const Messaging & ) const;

    /**
      Returns string representation of Messaging object.
    */
    QString asString() const;
      
  private:
    QString mId;
    QString mServiceType;
    QString mAccountId;
    QString mSenderAccountId;

    class MessagingPrivate;
    MessagingPrivate *d;
};

QDataStream &operator<<( QDataStream &, const Messaging & );
QDataStream &operator>>( QDataStream &, Messaging & );

}

#endif
