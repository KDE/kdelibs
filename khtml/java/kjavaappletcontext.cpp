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
#include "kjavaappletserver.h"
#include "kjavaprocess.h"
#include "kjavaapplet.h"
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <qmap.h>
#include <qguardedptr.h>
#include <qstringlist.h>
#include <qregexp.h>

// This file was using 6002, but kdebug.areas didn't know about that number
#define DEBUGAREA 6100

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
    connect(server->javaProcess(), SIGNAL(exited(int)), this, SLOT(javaProcessExited(int)));

    id = contextCount;
    server->createContext( id, this );

    ++contextCount;
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

bool KJavaAppletContext::create( KJavaApplet* applet )
{
    return server->createApplet( id, applet->appletId(),
                                applet->appletName(),
                                applet->appletClass(),
                                applet->baseURL(),
                                applet->user(),
                                applet->password(),
                                applet->authName(),
                                applet->codeBase(),
                                applet->archives(),
                                applet->size(),
                                applet->getParams(),
                                applet->getWindowName() );


}

void KJavaAppletContext::destroy( KJavaApplet* applet )
{
    const int appletId = applet->appletId();
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
	 && !arg.empty() )
    {
        QString tmp = arg.first();
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
              && !arg.empty() )
    {
        kdDebug(6100) << "url = " << arg.first() << endl;
        emit showDocument( arg.first(), "_top" );
    }
    else if ( cmd == QString::fromLatin1( "resizeapplet" )
              && arg.count() > 2 )
    {
        //arg[1] should be appletID
        //arg[2] should be new width
        //arg[3] should be new height
        bool ok;
        const int appletID = arg[0].toInt( &ok );
        const int width = arg[1].toInt( &ok );
        const int height = arg[2].toInt( &ok );

        if( !ok )
        {
            kdError(DEBUGAREA) << "could not parse out parameters for resize" << endl;
        }
        else
        {
            KJavaApplet* const tmp = d->applets[appletID];
            if (tmp)
                tmp->resizeAppletWidget( width, height );
        }
    }
    else if (cmd.startsWith(QString::fromLatin1("audioclip_"))) {
        kdDebug(DEBUGAREA) << "process Audio command (not yet implemented): " << cmd  << " " << arg[0] << endl;
    }
    else if ( cmd == QString::fromLatin1( "JS_Event" )
              && arg.count() > 2 )
    {
        bool ok;
        const int appletID = arg.first().toInt(&ok);
        KJavaApplet * applet;
        if (ok && (applet = d->applets[appletID]))
        {
            QStringList js_args(arg);
            js_args.pop_front();
            applet->jsData(js_args);
        }
        else
            kdError(DEBUGAREA) << "parse JS event " << arg[0] << " " << arg[1] << endl;
    }
    else if ( cmd == QString::fromLatin1( "AppletStateNotification" ) )
    {
        bool ok;
        const int appletID = arg.first().toInt(&ok);
        if (ok)
        {
            KJavaApplet* const applet = d->applets[appletID];
            if ( applet )
            {
                const int newState   = arg[1].toInt(&ok);
                if (ok)
                {
                    applet->stateChange(newState);
                    if (newState == KJavaApplet::INITIALIZED) {
                        kdDebug(DEBUGAREA) << "emit appletLoaded" << endl;
                        emit appletLoaded();
                    }
                } else
                    kdError(DEBUGAREA) << "AppletStateNotification: status is not numerical" << endl;
            } else
                kdWarning(DEBUGAREA) << "AppletStateNotification:  No such Applet with ID=" << arg[0] << endl;
        } else
            kdError(DEBUGAREA) << "AppletStateNotification: Applet ID is not numerical" << endl;
    }
    else if ( cmd == QString::fromLatin1( "AppletFailed" ) ) {
        bool ok;
        const int appletID = arg.first().toInt(&ok);
        if (ok)
        {
            KJavaApplet* const applet = d->applets[appletID];
            /*
            QString errorDetail(arg[1]);
            errorDetail.replace(QRegExp(":\\s*"), ":\n");
            KMessageBox::detailedError(0L, i18n("Java error while loading applet."), errorDetail);
            */
            if (applet)
                applet->setFailed();
            emit appletLoaded();
        }
    }
}

void KJavaAppletContext::javaProcessExited(int) {
    AppletMap::iterator it = d->applets.begin();
    const AppletMap::iterator itEnd = d->applets.end();
    for (; it != itEnd; ++it)
        if (!(*it).isNull() && (*it)->isCreated() && !(*it)->failed()) {
            (*it)->setFailed();
            if ((*it)->state() < KJavaApplet::INITIALIZED)
                emit appletLoaded();
        }
}

bool KJavaAppletContext::getMember(QStringList & args, QStringList & ret_args) {
    args.push_front( QString::number(id) );
    return server->getMember( args, ret_args );
}

bool KJavaAppletContext::putMember( QStringList & args ) {
    args.push_front( QString::number(id) );
    return server->putMember( args );
}

bool KJavaAppletContext::callMember(QStringList & args, QStringList &ret_args) {
    args.push_front( QString::number(id) );
    return server->callMember( args, ret_args );
}

void KJavaAppletContext::derefObject( QStringList & args ) {
    args.push_front( QString::number(id) );
    server->derefObject( args );
}

#include <kjavaappletcontext.moc>
