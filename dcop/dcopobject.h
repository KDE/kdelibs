/*
   This file is part of the KDE libraries
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (c) 1999 Matthias Ettrich <ettrich@kde.org>

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

#ifndef _DCOPOBJECT_H
#define _DCOPOBJECT_H

#include <qobject.h>
#include <qmap.h>
#include <qstring.h>

class DCOPClient;


#define K_DCOP \
public:        \
  virtual bool process(const QCString &fun, const QByteArray &data, QCString& replyType, QByteArray &replyData); \
private:

#define k_dcop public

/**
 * DCOPObject provides an interface for receiving DCOP messages.
 *
 * This class provides an interface for receiving DCOP messages.  To use it,
 * simply multiply-inherit from DCOPObject and from some other class, and
 * then implement the DCOPObject::process method.  Because this method is
 * pure virtual, you <em>must</em> implement the method.
 *
 * @author Preston Brown <pbrown@kde.org>, Matthias Ettrich <ettrich@kde.org>
 */
class DCOPObject
{
 public:
  DCOPObject();
  DCOPObject(QObject *obj);
  DCOPObject(const QCString &objId);
  virtual ~DCOPObject();

  QCString objId() const;

  /**
   * dispatch a message.
   *
   * Note that @param fun is normalized.
   * @see DCOPClient::normalizeFunctionSignature
   */
  virtual bool process(const QCString &fun, const QByteArray &data,
		       QCString& replyType, QByteArray &replyData) = 0;

  static bool hasObject(const QCString &objId);
  static DCOPObject *find(const QCString &objId);
  static QCString objectName( QObject* obj );

 private:
  QCString ident;
};

class DCOPObjectProxy
{
public:
    DCOPObjectProxy( DCOPClient* client );
    virtual ~DCOPObjectProxy();

    virtual bool process( const QCString& obj, const QCString& fun, const QByteArray& data,
			  QCString& replyType, QByteArray &replyData );
private:
    DCOPClient* parent;
};



#endif
