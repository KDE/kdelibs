/*
  This file is part of the KDE libraries

  Copyright (c) 2000 Matthias Elter   <elter@kde.org>
  base on code written 1999 by Matthias Ettrich <ettrich@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License version 2 as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA 02111-1307, USA.
*/

#include <string.h>

#include <qcstring.h>
#include <qdatastream.h>

#include <kapp.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <klocale.h>
#include <dcopclient.h>

#include "qxembed.h"
#include "kpanelapplet.h"

KPanelApplet::KPanelApplet( QWidget* parent, const char* name  )
  : QWidget( parent, name)
  , DCOPObject() , _actions (0) , _flags (0) , _orient (Horizontal) , _pos (Bottom)
{
  // init QXEmbed
  QXEmbed::initialize();

  // always use active palette
  QPalette pal(palette());
  if(pal.active().mid() != pal.inactive().mid()){
	pal.setInactive(pal.active());
	setPalette(pal);
  }
}

bool KPanelApplet::dock( const QCString appletId )
{
  // try to attach to DCOP server
  DCOPClient* dcop = kapp->dcopClient();
  
  if (!dcop->isAttached())
	if (!dcop->attach())
      {
	    kdError() << "Failed to attach to DCOP server." << endl;
        return false;
      }
  
  // tell kicker that we are here and want be docked
  {
	QCString replyType;
	QByteArray data, replyData;
	QDataStream dataStream( data, IO_WriteOnly );
	dataStream << objId() << appletId;
    
	// we use "call" to know whether it was sucessful
	if ( !dcop->call("kicker", "appletArea", "dockRequest(QCString,QCString)",
                     data, replyType, replyData ) )
      {
	    kdError() << "Failed to dock into the panel." << endl;
	    return false;
      }
  }
  return true;
}

void KPanelApplet::setFlags(int f)
{
  _flags = f;

  QByteArray data;
  QDataStream dataStream( data, IO_WriteOnly );
  dataStream << objId();
  dataStream << _flags;
  kapp->dcopClient()->send("kicker", "appletArea",
                           "setFlags(QCString,int)", data);
}

void KPanelApplet::setActions(int a)
{
  _actions = a;

  QByteArray data;
  QDataStream dataStream( data, IO_WriteOnly );
  dataStream << objId();
  dataStream << _actions;
  kapp->dcopClient()->send("kicker", "appletArea",
                           "setActions(QCString,int)", data);
}

void KPanelApplet::updateLayout()
{
  QByteArray data;
  kapp->dcopClient()->send("kicker", "appletArea", "updateLayout()", data);
}

bool KPanelApplet::process(const QCString &fun, const QByteArray &data,
                           QCString& replyType, QByteArray &replyData)
{
  if ( fun == "winId()" )
    {
      QDataStream reply( replyData, IO_WriteOnly );
      reply << winId();
      replyType = "WId";
      return true;
    }
  else if ( fun == "widthForHeight(int)" )
    {
      QDataStream dataStream( data, IO_ReadOnly );
      int height;
      dataStream >> height;
      QDataStream reply( replyData, IO_WriteOnly );
      reply << widthForHeight(height);
      replyType = "int";
      return true;
    }
  else if ( fun == "heightForWidth(int)" )
    {
      QDataStream dataStream( data, IO_ReadOnly );
      int width;
      dataStream >> width;
      QDataStream reply( replyData, IO_WriteOnly );
      reply << heightForWidth(width);
      replyType = "int";
      return true;
    }
  else if ( fun == "setPosition(int)" )
    {
      QDataStream dataStream( data, IO_ReadOnly );
      int pos;
      dataStream >> pos;
      _pos = static_cast<Position>(pos);
      resizeEvent(0);
      return true;
    }
  else if ( fun == "setOrientation(int)" )
    {
      QDataStream dataStream( data, IO_ReadOnly );
      int orient;
      dataStream >> orient;
      _orient = static_cast<Qt::Orientation>(orient);
      resizeEvent(0);
      return true;
    }
  else if ( fun == "removedFromPanel()" )
    {
      kapp->quit();
      return true;
    }
  else if ( fun == "about()" )
    {
      about();
      return true;
    }
  else if ( fun == "help()" )
    {
      help();
      return true;
    }
  else if ( fun == "preferences()" )
    {
      preferences();
      return true;
    }
  else if ( fun == "restartCommand()" )
    {
      QDataStream reply( replyData, IO_WriteOnly );
      reply << QCString( kapp->argv()[0] );
      replyType = "QCString";
      return true;
    }
  else if ( fun == "actions()" )
    {
      QDataStream reply( replyData, IO_WriteOnly );
      reply << _actions;
      replyType = "int";
      return true;
    }
  else if ( fun == "flags()" )
    {
      QDataStream reply( replyData, IO_WriteOnly );
      reply << _flags;
      replyType = "int";
      return true;
    }
  return false;
}

int KPanelApplet::heightForWidth(int width)
{
  // default to a quadratic shape
  return width;
}

int KPanelApplet::widthForHeight(int height)
{
  // default to a quadratic shape
  return height;
}
