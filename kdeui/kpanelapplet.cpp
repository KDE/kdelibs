/*
   This file is part of the KDE libraries

   Copyright (c) 2000 Matthias Elter   <elter@kde.org>
   base on code written 1999 by Matthias Ettrich <ettrich@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

//#include <unistd.h>

#include <qcstring.h>
#include <qdatastream.h>

#include <kapp.h>
#include <dcopclient.h>

#include "qxembed.h"
#include "kpanelapplet.h"

KPanelApplet::KPanelApplet( QWidget* parent, const char* name  )
  : QWidget( parent, name)
  , DCOPObject()
  , _stretch (false)
{
  QXEmbed::initialize();
}

KPanelApplet::~KPanelApplet() {}

void KPanelApplet::init( int& /*argc*/, char ** /*argv*/ ) 
{
  DCOPClient* dcop = kapp->dcopClient();

  if (!dcop->isAttached())
	if (!dcop->attach())
      {
        qDebug("Error: Failed to attach to DCOP server.");
        goto error;
      }

  // tell kicker that we are here and want be docked
  {
	QCString replyType;
	QByteArray data, replyData;
	QDataStream dataStream( data, IO_WriteOnly );
	dataStream << objId();
    
	// we use "call" to know whether it was really sucessful
	if ( !dcop->call("kicker", "appletArea", "dockMe(QCString)", data, replyType, replyData ) )
      {
        qDebug("Error: Failed to dock into Kicker.");
        goto error;
      }
  }
  return;
 
 error:
  // do something, at least
  resize(48,heightForWidth(48));
  show();
}

void KPanelApplet::setStretch(bool s)
{
  _stretch = s;
  
  QByteArray data;
  QDataStream dataStream( data, IO_WriteOnly );
  dataStream << objId();
  dataStream << static_cast<int>(_stretch);
  kapp->dcopClient()->send("kicker", "appletArea",
                           "setStretch(QCString,int)", data);
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
  else if ( fun == "restartCommand()" )
    {
      QDataStream reply( replyData, IO_WriteOnly );
      reply << QCString( kapp->argv()[0] );
      replyType = "QCString";
      return true;
    }
  return false;
}

KPanelApplet::Position KPanelApplet::position()
{
  QByteArray data;
  QCString replyType;
  QByteArray replyData;

  if (kapp->dcopClient()->call( "kicker", "appletArea", "position()", data, replyType, replyData ) )
    {
      int pos;
      QDataStream reply( replyData, IO_ReadOnly );
      reply >> pos;
      return static_cast<Position>(pos);
    }
  return Bottom; // Should never happen.
}

Qt::Orientation KPanelApplet::orientation()
{
  QByteArray data;
  QCString replyType;
  QByteArray replyData;

  if (kapp->dcopClient()->call( "kicker", "appletArea", "orientation()", data, replyType, replyData ) )
    {
      int orient;
      QDataStream reply( replyData, IO_ReadOnly );
      reply >> orient;
      return static_cast<Orientation>(orient);
    }
  return Horizontal; // Should never happen.
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
