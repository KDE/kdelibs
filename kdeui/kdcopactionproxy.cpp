/* This file is part of the KDE project
   Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>

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

#include "kdcopactionproxy.h"

#include <dcopclient.h>
#include <kapplication.h>
#include <kaction.h>
#include <kdebug.h>
#include <kdcoppropertyproxy.h>

#include <ctype.h>

class KDCOPActionProxy::KDCOPActionProxyPrivate
{
public:
  KDCOPActionProxyPrivate()
  {
  }
  ~KDCOPActionProxyPrivate()
  {
  }

  KActionCollection *m_actionCollection;
  DCOPObject *m_parent;
  DCOPCString m_prefix;
  int m_pos;
};

KDCOPActionProxy::KDCOPActionProxy( KActionCollection *actionCollection, DCOPObject *parent )
{
  init( actionCollection, parent );
}

KDCOPActionProxy::KDCOPActionProxy( DCOPObject *parent )
{
  init( 0, parent );
}

void KDCOPActionProxy::init( KActionCollection *collection, DCOPObject *parent )
{
  d = new KDCOPActionProxyPrivate;
  d->m_actionCollection = collection;
  d->m_parent = parent;
  d->m_prefix = parent->objId() + "/action/";
  d->m_pos = d->m_prefix.length();
}

KDCOPActionProxy::~KDCOPActionProxy()
{
  delete d;
}

QList<KAction *>KDCOPActionProxy::actions() const
{
  if ( !d->m_actionCollection )
    return QList<KAction *>();

  return d->m_actionCollection->actions();
}

KAction *KDCOPActionProxy::action( const char *name ) const
{
  if ( !d->m_actionCollection )
    return 0;

  return d->m_actionCollection->action( name );
}

DCOPCString KDCOPActionProxy::actionObjectId( const DCOPCString &name ) const
{
  return d->m_prefix + name;
}

QMap<DCOPCString,DCOPRef> KDCOPActionProxy::actionMap( const DCOPCString &appId ) const
{
  QMap<DCOPCString,DCOPRef> res;

  QByteArray id = appId;
  if ( id.isEmpty() )
    id = KApplication::dcopClient()->appId();

  QList<KAction *> lst = actions();
  foreach ( KAction*it, lst ) {
    res.insert( it->name(), DCOPRef( id, actionObjectId( it->name() ) ) );
  }

  return res;
}

bool KDCOPActionProxy::process( const DCOPCString &obj, const DCOPCString &fun, const QByteArray &data,
                                DCOPCString &replyType, QByteArray &replyData )
{
  if ( obj.left( d->m_pos ) != d->m_prefix )
    return false;

  KAction *act = action( obj.mid( d->m_pos ) );
  if ( !act )
    return false;

  return processAction( obj, fun, data, replyType, replyData, act );
}

bool KDCOPActionProxy::processAction( const DCOPCString&, const DCOPCString &fun, const QByteArray &data,
                                      DCOPCString &replyType, QByteArray &replyData, KAction *action )
{
  if ( fun == "activate()" )
  {
    replyType = "void";
    action->activate();
    return true;
  }

  if ( fun == "isPlugged()" )
  {
    replyType = "bool";
    QDataStream reply( &replyData, QIODevice::WriteOnly );
	reply.setVersion(QDataStream::Qt_3_1 );
    reply << (qint8)action->isPlugged();
    return true;
  }

  if ( fun == "functions()" )
  {
    DCOPCStringList res;
    res << "QValueList<QCString> functions()";
    res << "void activate()";
    res << "bool isPlugged()";

    res += KDCOPPropertyProxy::functions( action );

    replyType = "QValueList<QCString>";
    QDataStream reply( &replyData, QIODevice::WriteOnly );
	reply.setVersion(QDataStream::Qt_3_1 );
    reply << res;
    return true;
  }

  return KDCOPPropertyProxy::processPropertyRequest( fun, data, replyType, replyData, action );
}

void KDCOPActionProxy::virtual_hook( int id, void* data )
{ DCOPObjectProxy::virtual_hook( id, data ); }

