/* This file is part of the KDE libraries

    Copyright (C) 1999 Matthias Ettrich (ettrich@kde.org)

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
#include "qxembed.h"
#include "kapplet.h"
#include <unistd.h>
#include <dcopclient.h>
#include <kapp.h>
#include <qcstring.h>
#include <qdatastream.h>

class KAppletData
{
public:
    KAppletData()
    {
	orientation = Qt::Horizontal;
    };
    ~KAppletData()
    {
    }

    Qt::Orientation orientation;
};


KApplet::KApplet( QWidget* parent, const char* name  )
    : QWidget( parent, name), DCOPObject()
{
    QXEmbed::initialize();
    d = new KAppletData;
    s = Fixed;
}

KApplet::~KApplet()
{
    delete d;
}

void KApplet::init( int& /*argc*/, char ** /*argv*/ )
{
    int ideal_width = 42;
    int ideal_height = 42;

    DCOPClient* dcop = kapp->dcopClient();

    if (!dcop->isAttached() ) {
	if ( !dcop->attach() )
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
	    goto error;
    }

    return;

error:
    // do something, at least
    setupGeometry ( orientation(), ideal_width, ideal_height );
    show();
}


void KApplet::removeRequest()
{
    QByteArray data;
    QDataStream dataStream( data, IO_WriteOnly );
    dataStream << objId();
    kapp->dcopClient()->send("kicker", "appletArea", "removeMe(QCString)", data);
}

void KApplet::moveRequest()
{
    QByteArray data;
    QDataStream dataStream( data, IO_WriteOnly );
    dataStream << objId();
    kapp->dcopClient()->send("kicker", "appletArea", "moveMe(QCString)", data);
}

void KApplet::setStretch(Stretch size)
{
    s = size;
    QByteArray data;
    QDataStream dataStream( data, IO_WriteOnly );
    dataStream << objId();
    dataStream << (int)s;
    kapp->dcopClient()->send("kicker", "appletArea",
                             "setStretch(QCString,int)", data);
}


bool KApplet::process(const QCString &fun, const QByteArray &data,
	     QCString& replyType, QByteArray &replyData)
{
    if ( fun == "setupGeometry(int,int,int)" ) {
	QDataStream dataStream( data, IO_ReadOnly );
	int orient;
	int width, height;
	dataStream >> orient  >> width >> height;
	setupGeometry( Orientation(orient), width, height );
	return TRUE;
    } else if ( fun == "winId()" ) {
	QDataStream reply( replyData, IO_WriteOnly );
	reply << winId();
	replyType = "WId";
	return TRUE;
    } else if ( fun == "restartCommand()" ) {
	QDataStream reply( replyData, IO_WriteOnly );
	reply << QCString( kapp->argv()[0] );
	replyType = "QCString";
	return TRUE;
    }
    return FALSE;
}

void KApplet::setupGeometry( Orientation orientation , int width, int height  )
{
    QApplication::flushX();
    d->orientation = orientation;
    resize(width, height );
    updateGeometry();
}


QSize KApplet::sizeHint() const
{
    return size();
}

Qt::Orientation KApplet::orientation() const
{
    return d->orientation;
}

#include "kapplet.moc"

// $Log$
// Revision 1.11  1999/11/15 11:03:45  mosfet
// Added stretch methods for layouts. Will add docu when done.
//
// Revision 1.10  1999/11/14 05:53:32  ettrich
//
// Added KDockWindow, a simpler and more comfortable way of doing panel docking.
//
// Removed obsolete KWMModuleApplicaton as announced some time ago.
//
