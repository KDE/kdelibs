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

#include "kjavaappletcontext.h"
#include <kjavaappletserver.h>
#include <kjavaapplet.h>
#include <kdebug.h>
#include <qmap.h>
#include <qguardedptr.h>
#include <qstringlist.h>
#include <qregexp.h>

// For future expansion
class KJavaAppletContextPrivate
{
friend class KJavaAppletContext;
private:
    QMap< int, QGuardedPtr<KJavaApplet> > applets;
};


//  Static Factory Functions
int KJavaAppletContext::contextCount = 0;

/*  Class Implementation
 */
KJavaAppletContext::KJavaAppletContext()
    : QObject()
{
    d = new KJavaAppletContextPrivate;
    server = KJavaAppletServer::allocateJavaServer();

    id = contextCount;
    server->createContext( id, this );

    contextCount++;
}

KJavaAppletContext::~KJavaAppletContext()
{
    server->destroyContext( id );
    KJavaAppletServer::freeJavaServer();
    delete d;
}

int KJavaAppletContext::contextId()
{
    return id;
}

void KJavaAppletContext::setContextId( int _id )
{
    id = _id;
}

void KJavaAppletContext::create( KJavaApplet* applet )
{
    static int appletId = 0;

    server->createApplet( id, ++appletId,
                          applet->appletName(),
                          applet->appletClass(),
                          applet->baseURL(),
                          applet->codeBase(),
                          applet->archives(),
                          applet->size(),
                          applet->getParams(),
                          applet->getWindowName() );

    applet->setAppletId( appletId );
    d->applets.insert( appletId, applet );
}

void KJavaAppletContext::destroy( KJavaApplet* applet )
{
    int appletId = applet->appletId();
    d->applets.remove( appletId );

    server->destroyApplet( id, appletId );
}

void KJavaAppletContext::init( KJavaApplet* applet )
{
    server->initApplet( id, applet->appletId() );
}

void KJavaAppletContext::start( KJavaApplet* applet )
{
    server->startApplet( id, applet->appletId() );
}

void KJavaAppletContext::stop( KJavaApplet* applet )
{
    server->stopApplet( id, applet->appletId() );
}

void KJavaAppletContext::processCmd( QString cmd, QStringList args )
{
    received( cmd, args );
}

void KJavaAppletContext::received( const QString& cmd, const QStringList& arg )
{
    kdDebug(6100) << "KJavaAppletContext::received, cmd = >>" << cmd << "<<" << endl;
    kdDebug(6100) << "arg count = " << arg.count() << endl;

    if ( cmd == QString::fromLatin1("showstatus")
         && arg.count() > 0 )
    {
        QString tmp = arg[0];
        tmp.replace(QRegExp("[\n\r]"), "");
        kdDebug(6100) << "status message = " << tmp << endl;
        emit showStatus( tmp );
    }
    else if ( cmd == QString::fromLatin1( "showurlinframe" )
              && arg.count() > 1 )
    {
        kdDebug(6100) << "url = " << arg[0] << ", frame = " << arg[1] << endl;
        emit showDocument( arg[0], arg[1] );
    }
    else if ( cmd == QString::fromLatin1( "showdocument" )
              && arg.count() > 0 )
    {
        kdDebug(6100) << "url = " << arg[0] << endl;
        emit showDocument( arg[0], "_top" );
    }
    else if ( cmd == QString::fromLatin1( "resizeapplet" )
              && arg.count() > 0 )
    {
        //arg[1] should be appletID
        //arg[2] should be new width
        //arg[3] should be new height
        bool ok;
        int appletID = arg[0].toInt( &ok );
        int width = arg[1].toInt( &ok );
        int height = arg[2].toInt( &ok );

        if( !ok )
        {
            kdError(6002) << "could not parse out parameters for resize" << endl;
        }
        else
        {
            KJavaApplet* tmp = d->applets[appletID];
            tmp->resizeAppletWidget( width, height );
        }
    } 
    else if (cmd.startsWith(QString::fromLatin1("audioclip_"))) {
        kdDebug(6002) << "process Audio command (not yet implemented): " << cmd  << " " << arg[0] << endl;
    }
    else if ( cmd == QString::fromLatin1( "JS_Event" )
              && arg.count() > 2 )
    {
        bool ok;
        int appletID = arg[0].toInt(&ok);
        unsigned long objid = arg[1].toInt(&ok);
        if (ok) 
        {
            KJavaApplet * applet = d->applets[appletID];
            if (applet) 
            {
                KParts::LiveConnectExtension::ArgList arglist;
                for (unsigned i = 3; i < arg.count(); i += 2)
                    // take a deep breath here
                    arglist.push_back(KParts::LiveConnectExtension::ArgList::value_type((KParts::LiveConnectExtension::Type) arg[i].toInt(), arg[i+1]));

                emit static_cast<KJavaLiveConnect*>(applet->getLiveConnectExtension())->sendEvent(objid, arg[2], arglist);
            } 
            else
                kdError(6002) << "could find applet for JS event" << endl;
        } 
        else
            kdError(6002) << "could not parse applet ID for JS event " << arg[0] << " " << arg[1] << endl;
    } 
}

bool KJavaAppletContext::getMember(KJavaApplet * applet, const unsigned long objid, const QString & name, int & type, unsigned long & rid, QString & value) {
    return server->getMember(id, applet->appletId(), objid, name, type, rid, value);
}

bool KJavaAppletContext::putMember(KJavaApplet * applet, const unsigned long objid, const QString & name, const QString & value) {
    return server->putMember(id, applet->appletId(), objid, name, value);
}

bool KJavaAppletContext::callMember(KJavaApplet * applet, const unsigned long objid, const QString & name, const QStringList & args, int & type, unsigned long & retobjid, QString & value) {
    return server->callMember(id, applet->appletId(), objid, name, args, type, retobjid, value);
}

void KJavaAppletContext::derefObject(KJavaApplet * applet, const unsigned long jid) {
    server->derefObject(id, applet->appletId(), jid);
}

#include <kjavaappletcontext.moc>
