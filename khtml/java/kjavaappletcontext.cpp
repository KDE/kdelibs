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
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <qmap.h>
#include <qguardedptr.h>
#include <qstringlist.h>
#include <qregexp.h>

typedef QMap< int, QGuardedPtr<KJavaApplet> > AppletMap;

// For future expansion
class KJavaAppletContextPrivate
{
friend class KJavaAppletContext;
private:
    AppletMap applets;
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

void KJavaAppletContext::registerApplet( KJavaApplet* applet )
{
    static int appletId = 0;

    applet->setAppletId( ++appletId );
    d->applets.insert( appletId, applet );
}

void KJavaAppletContext::create( KJavaApplet* applet )
{
    server->createApplet( id, applet->appletId(),
                          applet->appletName(),
                          applet->appletClass(),
                          applet->baseURL(),
                          applet->codeBase(),
                          applet->archives(),
                          applet->size(),
                          applet->getParams(),
                          applet->getWindowName() );

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
    else if ( cmd == QString::fromLatin1( "AppletStateNotification" ) )
    {
        bool ok;
        int appletID = arg[0].toInt(&ok);
        if (ok)
        {
            KJavaApplet * applet = d->applets[appletID];
            if ( applet )
            {
                int newState   = arg[1].toInt(&ok);
                if (ok)
                {
                    applet->stateChange(newState);
                    if (newState == KJavaApplet::INITIALIZED) {
                        kdDebug(6002) << "emit appletLoaded" << endl;
                        emit appletLoaded();
                    }
                } else
                    kdError(6002) << "AppletStateNotification: status is not numerical" << endl;
            } else
                kdWarning(6002) << "AppletStateNotification:  No such Applet with ID=" << arg[0] << endl;
        } else
            kdError(6002) << "AppletStateNotification: Applet ID is not numerical" << endl;
    }
    else if ( cmd == QString::fromLatin1( "AppletFailed" ) ) {
        bool ok;
        int appletID = arg[0].toInt(&ok);
        if (ok)
        {
            KJavaApplet * applet = d->applets[appletID];
            /*
            QString errorDetail(arg[1]);
            errorDetail.replace(QRegExp(":\\s*"), ":\n");
            KMessageBox::detailedError(0L, i18n("Java error while loading applet."), errorDetail);
            */
            applet->setFailed();
        }
    }
}

bool KJavaAppletContext::appletsLoaded() const {
    AppletMap::const_iterator it = d->applets.begin();
    for (; it != d->applets.end(); it++) {
        if (!(*it).isNull()) {
            if (!(*it)->isAlive()) {
                return false;
            }
        }
    }
    return true;
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
