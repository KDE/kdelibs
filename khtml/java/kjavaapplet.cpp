/* This file is part of the KDE project
 *
 * Copyright (C) 2000 Richard Moore <rich@kde.org>
 *               2000 Wynn Wilkes <wynnw@caldera.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "kjavaappletwidget.h"
#include "kjavaappletcontext.h"

#include <kdebug.h>

class KJavaAppletPrivate
{
friend class KJavaApplet;
private:
   bool    reallyExists;
   QString className;
   QString appName;
   QString baseURL;
   QString codeBase;
   QString archives;
   QSize   size;
   QString windowName;

   KJavaAppletWidget* UIwidget;
};


KJavaApplet::KJavaApplet( KJavaAppletWidget* _parent,
                          KJavaAppletContext* _context )
    : params(), liveconnect( new KJavaLiveConnect( _context, this ) )
{
    d = new KJavaAppletPrivate;

    d->UIwidget = _parent;

    if( _context )
        context = _context;
    else
        context = new KJavaAppletContext();

    d->reallyExists = false;
    id = -1;
}

KJavaApplet::~KJavaApplet()
{
    if ( d->reallyExists )
        context->destroy( this );

    delete d;
    delete liveconnect;
}

bool KJavaApplet::isCreated()
{
    return d->reallyExists;
}

void KJavaApplet::setAppletClass( const QString& _className )
{
    d->className = _className;
}

QString& KJavaApplet::appletClass()
{
    return d->className;
}

QString& KJavaApplet::parameter( const QString& name )
{
    return params[ name ];
}

void KJavaApplet::setParameter( const QString& name, const QString& value )
{
    params.insert( name, value );
}

QMap<QString,QString>& KJavaApplet::getParams()
{
    return params;
}

void KJavaApplet::setBaseURL( const QString& baseURL )
{
    d->baseURL = baseURL;
}

QString& KJavaApplet::baseURL()
{
    return d->baseURL;
}

void KJavaApplet::setCodeBase( const QString& codeBase )
{
    d->codeBase = codeBase;
}

QString& KJavaApplet::codeBase()
{
    return d->codeBase;
}

void KJavaApplet::setSize( QSize size )
{
    d->size = size;
}

QSize KJavaApplet::size()
{
    return d->size;
}

void KJavaApplet::setArchives( const QString& _archives )
{
    d->archives = _archives;
}

QString& KJavaApplet::archives()
{
    return d->archives;
}

void KJavaApplet::resizeAppletWidget( int width, int height )
{
    kdDebug(6100) << "KJavaApplet, id = " << id << ", ::resizeAppletWidget to " << width << ", " << height << endl;

    if( d->UIwidget )
        d->UIwidget->resize( width, height );
}

void KJavaApplet::setAppletName( const QString& name )
{
    d->appName = name;
}

void KJavaApplet::setWindowName( const QString& title )
{
    d->windowName = title;
}

QString& KJavaApplet::getWindowName()
{
    return d->windowName;
}

QString& KJavaApplet::appletName()
{
    return d->appName;
}

void KJavaApplet::create( )
{
    context->create( this );
    d->reallyExists = true;
}

void KJavaApplet::init()
{
    context->init( this );
}

void KJavaApplet::start()
{
    context->start( this );
}

void KJavaApplet::stop()
{
    context->stop( this );
}

int KJavaApplet::appletId()
{
    return id;
}

void KJavaApplet::setAppletId( int _id )
{
    id = _id;
}

KJavaLiveConnect::KJavaLiveConnect(KJavaAppletContext* c, KJavaApplet* a) 
    : KParts::LiveConnectExtension(0), context(c), applet(a) {
}

bool KJavaLiveConnect::get(const unsigned long objid, const QString & field, KParts::LiveConnectExtension::Type & type, unsigned long & rid, QString & value )
{
    //return context->getMember(applet, objid, field, (int&) type, rid, value);
    int itype;
    bool ret = context->getMember(applet, objid, field, itype, rid, value);
    type = (KParts::LiveConnectExtension::Type) itype;
    return ret;
}

bool KJavaLiveConnect::put(const unsigned long objid, const QString & name, const QString & value)
{
    return context->putMember(applet, objid, name, value);
}

bool KJavaLiveConnect::call( const unsigned long objid, const QString & func, const QStringList & args, KParts::LiveConnectExtension::Type & type, unsigned long & retobjid, QString & value )
{
    int itype;
    bool ret = context->callMember(applet, objid, func, args, itype, retobjid, value);
    type = (KParts::LiveConnectExtension::Type) itype;
    return ret;
}

void KJavaLiveConnect::unregister(const unsigned long objid)
{
    context->derefObject(applet, objid);
}

#include "kjavaapplet.moc"
