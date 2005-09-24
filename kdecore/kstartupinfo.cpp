/****************************************************************************

 Copyright (C) 2001-2003 Lubos Lunak        <l.lunak@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

****************************************************************************/

// kdDebug() can't be turned off in kdeinit
#if 0
#define KSTARTUPINFO_ALL_DEBUG
#warning Extra KStartupInfo debug messages enabled.
#endif

#include <qwidget.h>

#include "config.h"
#ifdef Q_WS_X11
//#ifdef Q_WS_X11 // FIXME(E): Re-implement in a less X11 specific way
#include <qglobal.h>
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// need to resolve INT32(qglobal.h)<>INT32(Xlibint.h) conflict
#ifndef QT_CLEAN_NAMESPACE
#define QT_CLEAN_NAMESPACE
#endif

#include "kstartupinfo.h"

#include <QtGui/qx11info_x11.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <qtimer.h>
#include <qx11info_x11.h>
#ifdef Q_WS_X11
#include <netwm.h>
#endif
#include <kdebug.h>
#include <kapplication.h>
#include <signal.h>
#ifdef Q_WS_X11
#include <kwinmodule.h>
#include <kxmessages.h>
#include <kwin.h>
#endif

static const char* const NET_STARTUP_MSG = "_NET_STARTUP_INFO";
static const char* const NET_STARTUP_WINDOW = "_NET_STARTUP_ID";
// DESKTOP_STARTUP_ID is used also in kinit/wrapper.c
static const char* const NET_STARTUP_ENV = "DESKTOP_STARTUP_ID";

static bool auto_app_started_sending = true;

static long get_num( const QString& item_P );
static unsigned long get_unum( const QString& item_P );
static QString get_str( const QString& item_P );
static QByteArray get_cstr( const QString& item_P );
static QStringList get_fields( const QString& txt_P );
static QString escape_str( const QString& str_P );

static Atom utf8_string_atom = None;

class KStartupInfo::Data
    : public KStartupInfoData
    {
    public:
        Data() {}; // just because it's in a QMap
        Data( const QString& txt_P )
            : KStartupInfoData( txt_P ), age( 0 ) {};
        unsigned int age;
    };

struct KStartupInfoPrivate
    {
    public:
        QMap< KStartupInfoId, KStartupInfo::Data > startups;
	// contains silenced ASN's only if !AnnounceSilencedChanges
        QMap< KStartupInfoId, KStartupInfo::Data > silent_startups;
        // contains ASN's that had change: but no new: yet
        QMap< KStartupInfoId, KStartupInfo::Data > uninited_startups;
#ifdef Q_WS_X11
        KWinModule* wm_module;
        KXMessages msgs;
#endif
	QTimer* cleanup;
	int flags;
	KStartupInfoPrivate( int flags_P )
    	    :
#ifdef Q_WS_X11
	    msgs( NET_STARTUP_MSG, NULL, false ),
#endif
	      flags( flags_P ) {}
    };

KStartupInfo::KStartupInfo( int flags_P, QObject* parent_P )
    : QObject( parent_P ),
        timeout( 60 ), d( NULL )
    {
    init( flags_P );
    }

KStartupInfo::KStartupInfo( bool clean_on_cantdetect_P, QObject* parent_P )
    : QObject( parent_P ),
        timeout( 60 ), d( NULL )
    {
    init( clean_on_cantdetect_P ? CleanOnCantDetect : 0 );
    }

void KStartupInfo::init( int flags_P )
    {
    // d == NULL means "disabled"
    if( !KApplication::kApplication())
        return;
    if( !QX11Info::display())
        return;

    d = new KStartupInfoPrivate( flags_P );
#ifdef Q_WS_X11
    if( !( d->flags & DisableKWinModule ))
        {
        d->wm_module = new KWinModule( this );
        connect( d->wm_module, SIGNAL( windowAdded( WId )), SLOT( slot_window_added( WId )));
        connect( d->wm_module, SIGNAL( systemTrayWindowAdded( WId )), SLOT( slot_window_added( WId )));
        }
    else
        d->wm_module = NULL;
    connect( &d->msgs, SIGNAL( gotMessage( const QString& )), SLOT( got_message( const QString& )));
#endif
    d->cleanup = new QTimer( this );
    connect( d->cleanup, SIGNAL( timeout()), SLOT( startups_cleanup()));
    }

KStartupInfo::~KStartupInfo()
    {
    delete d;
    }

void KStartupInfo::got_message( const QString& msg_P )
    {
// TODO do something with SCREEN= ?
    kdDebug( 172 ) << "got:" << msg_P << endl;
    QString msg = msg_P.trimmed();
    if( msg.startsWith( "new:" )) // must match length below
        got_startup_info( msg.mid( 4 ), false );
    else if( msg.startsWith( "change:" )) // must match length below
        got_startup_info( msg.mid( 7 ), true );
    else if( msg.startsWith( "remove:" )) // must match length below
        got_remove_startup_info( msg.mid( 7 ));
    }

// if the application stops responding for a while, KWinModule may get
// the information about the already mapped window before KXMessages
// actually gets the info about the started application (depends
// on their order in X11 event filter in KApplication)
// simply delay info from KWinModule a bit
// SELI???
namespace
{
class DelayedWindowEvent
    : public QCustomEvent
    {
    public:
	DelayedWindowEvent( WId w_P )
	    : QCustomEvent( QEvent::User + 15 ), w( w_P ) {}
	Window w;
    };
}

void KStartupInfo::slot_window_added( WId w_P )
    {
    kapp->postEvent( this, new DelayedWindowEvent( w_P ));
    }

void KStartupInfo::customEvent( QEvent* e_P )
    {
    if( e_P->type() == QEvent::User + 15 )
	window_added( static_cast< DelayedWindowEvent* >( e_P )->w );
    else
	QObject::customEvent( e_P );
    }

void KStartupInfo::window_added( WId w_P )
    {
    KStartupInfoId id;
    KStartupInfoData data;
    startup_t ret = check_startup_internal( w_P, &id, &data );
    switch( ret )
        {
        case Match:
            kdDebug( 172 ) << "new window match" << endl;
          break;
        case NoMatch:
          break; // nothing
        case CantDetect:
            if( d->flags & CleanOnCantDetect )
                clean_all_noncompliant();
          break;
        }
    }

void KStartupInfo::got_startup_info( const QString& msg_P, bool update_P )
    {
    KStartupInfoId id( msg_P );
    if( id.none())
        return;
    KStartupInfo::Data data( msg_P );
    new_startup_info_internal( id, data, update_P );
    }

void KStartupInfo::new_startup_info_internal( const KStartupInfoId& id_P,
    Data& data_P, bool update_P )
    {
    if( d == NULL )
        return;
    if( id_P.none())
        return;
    if( d->startups.contains( id_P ))
        { // already reported, update
        d->startups[ id_P ].update( data_P );
        d->startups[ id_P ].age = 0; // CHECKME
        kdDebug( 172 ) << "updating" << endl;
	if( d->startups[ id_P ].silent() == Data::Yes
	    && !( d->flags & AnnounceSilenceChanges ))
	    {
	    d->silent_startups[ id_P ] = d->startups[ id_P ];
	    d->startups.remove( id_P );
	    emit gotRemoveStartup( id_P, d->silent_startups[ id_P ] );
	    return;
	    }
        emit gotStartupChange( id_P, d->startups[ id_P ] );
        return;
        }
    if( d->silent_startups.contains( id_P ))
        { // already reported, update
        d->silent_startups[ id_P ].update( data_P );
        d->silent_startups[ id_P ].age = 0; // CHECKME
        kdDebug( 172 ) << "updating silenced" << endl;
	if( d->silent_startups[ id_P ].silent() != Data::Yes )
	    {
	    d->startups[ id_P ] = d->silent_startups[ id_P ];
	    d->silent_startups.remove( id_P );
	    emit gotNewStartup( id_P, d->startups[ id_P ] );
	    return;
	    }
        emit gotStartupChange( id_P, d->startups[ id_P ] );
        return;
        }
    if( d->uninited_startups.contains( id_P ))
        {
        d->uninited_startups[ id_P ].update( data_P );
        kdDebug( 172 ) << "updating uninited" << endl;
        if( !update_P ) // uninited finally got new:
            {
            d->startups[ id_P ] = d->uninited_startups[ id_P ];
            d->uninited_startups.remove( id_P );
            emit gotNewStartup( id_P, d->startups[ id_P ] );
            return;
            }
        // no change announce, it's still uninited
        return;
        }
    if( update_P ) // change: without any new: first
        {
        kdDebug( 172 ) << "adding uninited" << endl;
	d->uninited_startups.insert( id_P, data_P );
        }
    else if( data_P.silent() != Data::Yes || d->flags & AnnounceSilenceChanges )
	{
        kdDebug( 172 ) << "adding" << endl;
        d->startups.insert( id_P, data_P );
	emit gotNewStartup( id_P, data_P );
	}
    else // new silenced, and silent shouldn't be announced
	{
        kdDebug( 172 ) << "adding silent" << endl;
	d->silent_startups.insert( id_P, data_P );
	}
    d->cleanup->start( 1000 ); // 1 sec
    }

void KStartupInfo::got_remove_startup_info( const QString& msg_P )
    {
    KStartupInfoId id( msg_P );
    KStartupInfoData data( msg_P );
    if( data.pids().count() > 0 )
        {
        if( !id.none())
            remove_startup_pids( id, data );
        else
            remove_startup_pids( data );
        return;
        }
    remove_startup_info_internal( id );
    }

void KStartupInfo::remove_startup_info_internal( const KStartupInfoId& id_P )
    {
    if( d == NULL )
        return;
    if( d->startups.contains( id_P ))
        {
	kdDebug( 172 ) << "removing" << endl;
	emit gotRemoveStartup( id_P, d->startups[ id_P ]);
	d->startups.remove( id_P );
	}
    else if( d->silent_startups.contains( id_P ))
	{
	kdDebug( 172 ) << "removing silent" << endl;
	d->silent_startups.remove( id_P );
	}
    else if( d->uninited_startups.contains( id_P ))
	{
	kdDebug( 172 ) << "removing uninited" << endl;
	d->uninited_startups.remove( id_P );
	}
    return;
    }

void KStartupInfo::remove_startup_pids( const KStartupInfoData& data_P )
    { // first find the matching info
    if( d == NULL )
        return;
    for( QMap< KStartupInfoId, Data >::Iterator it = d->startups.begin();
         it != d->startups.end();
         ++it )
        {
        if( ( *it ).hostname() != data_P.hostname())
            continue;
        if( !( *it ).is_pid( data_P.pids().first()))
            continue; // not the matching info
        remove_startup_pids( it.key(), data_P );
        break;
        }
    }

void KStartupInfo::remove_startup_pids( const KStartupInfoId& id_P,
    const KStartupInfoData& data_P )
    {
    if( d == NULL )
        return;
    kdFatal( data_P.pids().count() == 0, 172 );
    Data* data = NULL;
    if( d->startups.contains( id_P ))
	data = &d->startups[ id_P ];
    else if( d->silent_startups.contains( id_P ))
	data = &d->silent_startups[ id_P ];
    else if( d->uninited_startups.contains( id_P ))
        data = &d->uninited_startups[ id_P ];
    else
	return;
    for( QList< pid_t >::ConstIterator it2 = data_P.pids().begin();
         it2 != data_P.pids().end();
         ++it2 )
	data->remove_pid( *it2 ); // remove all pids from the info
    if( data->pids().count() == 0 ) // all pids removed -> remove info
    	remove_startup_info_internal( id_P );
    }

bool KStartupInfo::sendStartup( const KStartupInfoId& id_P, const KStartupInfoData& data_P )
    {
    if( id_P.none())
        return false;
    KXMessages msgs;
    QString msg = QString::fromLatin1( "new: %1 %2" )
        .arg( id_P.to_text()).arg( data_P.to_text());
	QX11Info inf;
    msg = check_required_startup_fields( msg, data_P, inf.screen());
    kdDebug( 172 ) << "sending " << msg << endl;
    msgs.broadcastMessage( NET_STARTUP_MSG, msg, -1, false );
    return true;
    }

bool KStartupInfo::sendStartupX( Display* disp_P, const KStartupInfoId& id_P,
    const KStartupInfoData& data_P )
    {
    if( id_P.none())
        return false;
    QString msg = QString::fromLatin1( "new: %1 %2" )
        .arg( id_P.to_text()).arg( data_P.to_text());
    msg = check_required_startup_fields( msg, data_P, DefaultScreen( disp_P ));
#ifdef KSTARTUPINFO_ALL_DEBUG
    kdDebug( 172 ) << "sending " << msg << endl;
#endif
    return KXMessages::broadcastMessageX( disp_P, NET_STARTUP_MSG, msg, -1, false );
    }

QString KStartupInfo::check_required_startup_fields( const QString& msg, const KStartupInfoData& data_P,
    int screen )
    {
    QString ret = msg;
    if( data_P.name().isEmpty())
        {
//        kdWarning( 172 ) << "NAME not specified in initial startup message" << endl;
        QString name = data_P.bin();
        if( name.isEmpty())
            name = "UNKNOWN";
        ret += QString( " NAME=\"%1\"" ).arg( escape_str( name ));
        }
    if( data_P.screen() == -1 ) // add automatically if needed
        ret += QString( " SCREEN=%1" ).arg( screen );
    return ret;
    }

bool KStartupInfo::sendChange( const KStartupInfoId& id_P, const KStartupInfoData& data_P )
    {
    if( id_P.none())
        return false;
    KXMessages msgs;
    QString msg = QString::fromLatin1( "change: %1 %2" )
        .arg( id_P.to_text()).arg( data_P.to_text());
    kdDebug( 172 ) << "sending " << msg << endl;
    msgs.broadcastMessage( NET_STARTUP_MSG, msg, -1, false );
    return true;
    }

bool KStartupInfo::sendChangeX( Display* disp_P, const KStartupInfoId& id_P,
    const KStartupInfoData& data_P )
    {
    if( id_P.none())
        return false;
    QString msg = QString::fromLatin1( "change: %1 %2" )
        .arg( id_P.to_text()).arg( data_P.to_text());
#ifdef KSTARTUPINFO_ALL_DEBUG
    kdDebug( 172 ) << "sending " << msg << endl;
#endif
    return KXMessages::broadcastMessageX( disp_P, NET_STARTUP_MSG, msg, -1, false );
    }

bool KStartupInfo::sendFinish( const KStartupInfoId& id_P )
    {
    if( id_P.none())
        return false;
    KXMessages msgs;
    QString msg = QString::fromLatin1( "remove: %1" ).arg( id_P.to_text());
    kdDebug( 172 ) << "sending " << msg << endl;
    msgs.broadcastMessage( NET_STARTUP_MSG, msg, -1, false );
    return true;
    }

bool KStartupInfo::sendFinishX( Display* disp_P, const KStartupInfoId& id_P )
    {
    if( id_P.none())
        return false;
    QString msg = QString::fromLatin1( "remove: %1" ).arg( id_P.to_text());
#ifdef KSTARTUPINFO_ALL_DEBUG
    kdDebug( 172 ) << "sending " << msg << endl;
#endif
    return KXMessages::broadcastMessageX( disp_P, NET_STARTUP_MSG, msg, -1, false );
    }

bool KStartupInfo::sendFinish( const KStartupInfoId& id_P, const KStartupInfoData& data_P )
    {
//    if( id_P.none()) // id may be none, the pids and hostname matter then
//        return false;
    KXMessages msgs;
    QString msg = QString::fromLatin1( "remove: %1 %2" )
        .arg( id_P.to_text()).arg( data_P.to_text());
    kdDebug( 172 ) << "sending " << msg << endl;
    msgs.broadcastMessage( NET_STARTUP_MSG, msg, -1, false );
    return true;
    }

bool KStartupInfo::sendFinishX( Display* disp_P, const KStartupInfoId& id_P,
    const KStartupInfoData& data_P )
    {
//    if( id_P.none()) // id may be none, the pids and hostname matter then
//        return false;
    QString msg = QString::fromLatin1( "remove: %1 %2" )
        .arg( id_P.to_text()).arg( data_P.to_text());
#ifdef KSTARTUPINFO_ALL_DEBUG
    kdDebug( 172 ) << "sending " << msg << endl;
#endif
    return KXMessages::broadcastMessageX( disp_P, NET_STARTUP_MSG, msg, -1, false );
    }

void KStartupInfo::appStarted()
    {
    if( kapp != NULL )  // KApplication constructor unsets the env. variable
        appStarted( kapp->startupId());
    else
        appStarted( KStartupInfo::currentStartupIdEnv().id());
    }

void KStartupInfo::appStarted( const QByteArray& startup_id )
    {
    KStartupInfoId id;
    id.initId( startup_id );
    if( id.none())
        return;
    if( kapp != NULL )
        KStartupInfo::sendFinish( id );
    else if( getenv( "DISPLAY" ) != NULL ) // don't rely on QX11Info::display()
        {
#ifdef Q_WS_X11
        Display* disp = XOpenDisplay( NULL );
        if( disp != NULL )
            {
            KStartupInfo::sendFinishX( disp, id );
            XCloseDisplay( disp );
            }
#endif
        }
    }

void KStartupInfo::disableAutoAppStartedSending( bool disable )
    {
    auto_app_started_sending = !disable;
    }

void KStartupInfo::silenceStartup( bool silence )
    {
    KStartupInfoId id;
    id.initId( kapp->startupId());
    if( id.none())
        return;
    KStartupInfoData data;
    data.setSilent( silence ? KStartupInfoData::Yes : KStartupInfoData::No );
    sendChange( id, data );
    }

void KStartupInfo::handleAutoAppStartedSending()
    {
    if( auto_app_started_sending )
        appStarted();
    }

void KStartupInfo::setNewStartupId( QWidget* window, const QByteArray& startup_id )
    {
    long activate = true;
    kapp->setStartupId( startup_id );
    if( window != NULL )
        {
        if( !startup_id.isEmpty() && startup_id != "0" )
            {
            NETRootInfo i( QX11Info::display(), NET::Supported );
            if( i.isSupported( NET::WM2StartupId ))
                {
                KStartupInfo::setWindowStartupId( window->winId(), startup_id );
                activate = false; // WM will take care of it
                }
            }
        if( activate )
            {
            KWin::setOnDesktop( window->winId(), KWin::currentDesktop());
        // This is not very nice, but there's no way how to get any
        // usable timestamp without ASN, so force activating the window.
        // And even with ASN, it's not possible to get the timestamp here,
        // so if the WM doesn't have support for ASN, it can't be used either.
            KWin::forceActiveWindow( window->winId());
            }
        }
    KStartupInfo::handleAutoAppStartedSending();
    }

KStartupInfo::startup_t KStartupInfo::checkStartup( WId w_P, KStartupInfoId& id_O,
    KStartupInfoData& data_O )
    {
    return check_startup_internal( w_P, &id_O, &data_O );
    }

KStartupInfo::startup_t KStartupInfo::checkStartup( WId w_P, KStartupInfoId& id_O )
    {
    return check_startup_internal( w_P, &id_O, NULL );
    }

KStartupInfo::startup_t KStartupInfo::checkStartup( WId w_P, KStartupInfoData& data_O )
    {
    return check_startup_internal( w_P, NULL, &data_O );
    }

KStartupInfo::startup_t KStartupInfo::checkStartup( WId w_P )
    {
    return check_startup_internal( w_P, NULL, NULL );
    }

KStartupInfo::startup_t KStartupInfo::check_startup_internal( WId w_P, KStartupInfoId* id_O,
    KStartupInfoData* data_O )
    {
    if( d == NULL )
        return NoMatch;
    if( d->startups.count() == 0 )
        return NoMatch; // no startups
    // Strategy:
    //
    // Is this a compliant app ?
    //  - Yes - test for match
    //  - No - Is this a NET_WM compliant app ?
    //           - Yes - test for pid match
    //           - No - test for WM_CLASS match
    kdDebug( 172 ) << "check_startup" << endl;
    QByteArray id = windowStartupId( w_P );
    if( !id.isNull())
        {
        if( id.isEmpty() || id == "0" ) // means ignore this window
            {
            kdDebug( 172 ) << "ignore" << endl;
            return NoMatch;
            }
        return find_id( id, id_O, data_O ) ? Match : NoMatch;
        }
#ifdef Q_WS_X11
    NETWinInfo info( QX11Info::display(),  w_P, QX11Info::appRootWindow(),
        NET::WMWindowType | NET::WMPid | NET::WMState );
    pid_t pid = info.pid();
    if( pid > 0 )
        {
        QByteArray hostname = get_window_hostname( w_P );
        if( !hostname.isEmpty()
            && find_pid( pid, hostname, id_O, data_O ))
            return Match;
        // try XClass matching , this PID stuff sucks :(
        }
    XClassHint hint;
    if( XGetClassHint( QX11Info::display(), w_P, &hint ) != 0 )
        { // We managed to read the class hint
        QByteArray res_name = hint.res_name;
        QByteArray res_class = hint.res_class;
        XFree( hint.res_name );
        XFree( hint.res_class );
        if( find_wclass( res_name, res_class, id_O, data_O ))
            return Match;
        }
    // ignore NET::Tool and other special window types, if they can't be matched
    NET::WindowType type = info.windowType( NET::NormalMask | NET::DesktopMask
        | NET::DockMask | NET::ToolbarMask | NET::MenuMask | NET::DialogMask
        | NET::OverrideMask | NET::TopMenuMask | NET::UtilityMask | NET::SplashMask );
    if( type != NET::Normal
        && type != NET::Override
        && type != NET::Unknown
        && type != NET::Dialog
        && type != NET::Utility )
//        && type != NET::Dock ) why did I put this here?
	return NoMatch;
    // lets see if this is a transient
    Window transient_for;
    if( XGetTransientForHint( QX11Info::display(), static_cast< Window >( w_P ), &transient_for )
        && static_cast< WId >( transient_for ) != QX11Info::appRootWindow()
        && transient_for != None )
	return NoMatch;
#endif
    kdDebug( 172 ) << "check_startup:cantdetect" << endl;
    return CantDetect;
    }

bool KStartupInfo::find_id( const QByteArray& id_P, KStartupInfoId* id_O,
    KStartupInfoData* data_O )
    {
    if( d == NULL )
        return false;
    kdDebug( 172 ) << "find_id:" << id_P << endl;
    KStartupInfoId id;
    id.initId( id_P );
    if( d->startups.contains( id ))
        {
        if( id_O != NULL )
            *id_O = id;
        if( data_O != NULL )
            *data_O = d->startups[ id ];
        kdDebug( 172 ) << "check_startup_id:match" << endl;
        return true;
        }
    return false;
    }

bool KStartupInfo::find_pid( pid_t pid_P, const QByteArray& hostname_P,
    KStartupInfoId* id_O, KStartupInfoData* data_O )
    {
    if( d == NULL )
        return false;
    kdDebug( 172 ) << "find_pid:" << pid_P << endl;
    for( QMap< KStartupInfoId, Data >::Iterator it = d->startups.begin();
         it != d->startups.end();
         ++it )
        {
        if( ( *it ).is_pid( pid_P ) && ( *it ).hostname() == hostname_P )
            { // Found it !
            if( id_O != NULL )
                *id_O = it.key();
            if( data_O != NULL )
                *data_O = *it;
            // non-compliant, remove on first match
            remove_startup_info_internal( it.key());
            kdDebug( 172 ) << "check_startup_pid:match" << endl;
            return true;
            }
        }
    return false;
    }

bool KStartupInfo::find_wclass( QByteArray res_name, QByteArray res_class,
    KStartupInfoId* id_O, KStartupInfoData* data_O )
    {
    if( d == NULL )
        return false;
    res_name = res_name.lower();
    res_class = res_class.lower();
    kdDebug( 172 ) << "find_wclass:" << res_name << ":" << res_class << endl;
    for( QMap< KStartupInfoId, Data >::Iterator it = d->startups.begin();
         it != d->startups.end();
         ++it )
        {
        const QByteArray wmclass = ( *it ).findWMClass();
        if( wmclass.lower() == res_name || wmclass.lower() == res_class )
            { // Found it !
            if( id_O != NULL )
                *id_O = it.key();
            if( data_O != NULL )
                *data_O = *it;
            // non-compliant, remove on first match
            remove_startup_info_internal( it.key());
            kdDebug( 172 ) << "check_startup_wclass:match" << endl;
            return true;
            }
        }
    return false;
    }

#ifdef Q_WS_X11
static Atom net_startup_atom = None;

static QByteArray read_startup_id_property( WId w_P )
    {
    QByteArray ret;
    unsigned char *name_ret;
    Atom type_ret;
    int format_ret;
    unsigned long nitems_ret = 0, after_ret = 0;
    if( XGetWindowProperty( QX11Info::display(), w_P, net_startup_atom, 0l, 4096,
            False, utf8_string_atom, &type_ret, &format_ret, &nitems_ret, &after_ret, &name_ret )
	    == Success )
        {
	if( type_ret == utf8_string_atom && format_ret == 8 && name_ret != NULL )
  	    ret = reinterpret_cast< char* >( name_ret );
        if ( name_ret != NULL )
            XFree( name_ret );
        }
    return ret;
    }

#endif

QByteArray KStartupInfo::windowStartupId( WId w_P )
    {
#ifdef Q_WS_X11
    if( net_startup_atom == None )
        net_startup_atom = XInternAtom( QX11Info::display(), NET_STARTUP_WINDOW, False );
    if( utf8_string_atom == None )
        utf8_string_atom = XInternAtom( QX11Info::display(), "UTF8_STRING", False );
    QByteArray ret = read_startup_id_property( w_P );
    if( ret.isEmpty())
        { // retry with window group leader, as the spec says
        XWMHints* hints = XGetWMHints( QX11Info::display(), w_P );
        if( hints && ( hints->flags & WindowGroupHint ) != 0 )
            ret = read_startup_id_property( hints->window_group );
        if( hints )
            XFree( hints );
        }
    return ret;
#else
    return QByteArray();
#endif
    }

void KStartupInfo::setWindowStartupId( WId w_P, const QByteArray& id_P )
    {
#ifdef Q_WS_X11
    if( id_P.isNull())
        return;
    if( net_startup_atom == None )
        net_startup_atom = XInternAtom( QX11Info::display(), NET_STARTUP_WINDOW, False );
    if( utf8_string_atom == None )
        utf8_string_atom = XInternAtom( QX11Info::display(), "UTF8_STRING", False );
    XChangeProperty( QX11Info::display(), w_P, net_startup_atom, utf8_string_atom, 8,
        PropModeReplace, reinterpret_cast< const unsigned char* >( id_P.data()), id_P.length());
#endif
    }

QByteArray KStartupInfo::get_window_hostname( WId w_P )
    {
#ifdef Q_WS_X11
    XTextProperty tp;
    char** hh;
    int cnt;
    if( XGetWMClientMachine( QX11Info::display(), w_P, &tp ) != 0
        && XTextPropertyToStringList( &tp, &hh, &cnt ) != 0 )
        {
        if( cnt == 1 )
            {
            QByteArray hostname = hh[ 0 ];
            XFreeStringList( hh );
            return hostname;
            }
        XFreeStringList( hh );
        }
#endif
    // no hostname
    return QByteArray();
    }

void KStartupInfo::setTimeout( unsigned int secs_P )
    {
    timeout = secs_P;
 // schedule removing entries that are older than the new timeout
    QTimer::singleShot( 0, this, SLOT( startups_cleanup_no_age()));
    }

void KStartupInfo::startups_cleanup_no_age()
    {
    startups_cleanup_internal( false );
    }

void KStartupInfo::startups_cleanup()
    {
    if( d == NULL )
        return;
    if( d->startups.count() == 0 && d->silent_startups.count() == 0
        && d->uninited_startups.count() == 0 )
        {
        d->cleanup->stop();
        return;
        }
    startups_cleanup_internal( true );
    }

void KStartupInfo::startups_cleanup_internal( bool age_P )
    {
    if( d == NULL )
        return;
    for( QMap< KStartupInfoId, Data >::Iterator it = d->startups.begin();
         it != d->startups.end();
         )
        {
        if( age_P )
            ( *it ).age++;
	unsigned int tout = timeout;
	if( ( *it ).silent() == Data::Yes ) // TODO
	    tout *= 20;
        if( ( *it ).age >= tout )
            {
            const KStartupInfoId& key = it.key();
            ++it;
            kdDebug( 172 ) << "entry timeout:" << key.id() << endl;
            remove_startup_info_internal( key );
            }
        else
            ++it;
        }
    for( QMap< KStartupInfoId, Data >::Iterator it = d->silent_startups.begin();
         it != d->silent_startups.end();
         )
        {
        if( age_P )
            ( *it ).age++;
	unsigned int tout = timeout;
	if( ( *it ).silent() == Data::Yes ) // TODO
	    tout *= 20;
        if( ( *it ).age >= tout )
            {
            const KStartupInfoId& key = it.key();
            ++it;
            kdDebug( 172 ) << "entry timeout:" << key.id() << endl;
            remove_startup_info_internal( key );
            }
        else
            ++it;
        }
    for( QMap< KStartupInfoId, Data >::Iterator it = d->uninited_startups.begin();
         it != d->uninited_startups.end();
         )
        {
        if( age_P )
            ( *it ).age++;
	unsigned int tout = timeout;
	if( ( *it ).silent() == Data::Yes ) // TODO
	    tout *= 20;
        if( ( *it ).age >= tout )
            {
            const KStartupInfoId& key = it.key();
            ++it;
            kdDebug( 172 ) << "entry timeout:" << key.id() << endl;
            remove_startup_info_internal( key );
            }
        else
            ++it;
        }
    }

void KStartupInfo::clean_all_noncompliant()
    {
    if( d == NULL )
        return;
    for( QMap< KStartupInfoId, Data >::Iterator it = d->startups.begin();
         it != d->startups.end();
         )
        {
        if( ( *it ).WMClass() != "0" )
            {
            ++it;
            continue;
            }
        const KStartupInfoId& key = it.key();
        ++it;
        kdDebug( 172 ) << "entry cleaning:" << key.id() << endl;
        remove_startup_info_internal( key );
        }
    }

QByteArray KStartupInfo::createNewStartupId()
    {
    // Assign a unique id, use hostname+time+pid, that should be 200% unique.
    // Also append the user timestamp (for focus stealing prevention).
    struct timeval tm;
    gettimeofday( &tm, NULL );
    char hostname[ 256 ];
    hostname[ 0 ] = '\0';
    if (!gethostname( hostname, 255 ))
	hostname[sizeof(hostname)-1] = '\0';
#ifdef Q_WS_X11
    long qt_x_user_time = QX11Info::appUserTime();
#else
    long qt_x_user_time = 0;
#endif
    QByteArray id = QString( "%1;%2;%3;%4_TIME%5" ).arg( hostname ).arg( tm.tv_sec )
        .arg( tm.tv_usec ).arg( getpid()).arg( qt_x_user_time ).toUtf8();
    kdDebug( 172 ) << "creating: " << id << ":" << qAppName() << endl;
    return id;
    }


struct KStartupInfoIdPrivate
    {
    KStartupInfoIdPrivate() : id( "" ) {};
    QByteArray id; // id
    };

const QByteArray& KStartupInfoId::id() const
    {
    return d->id;
    }


QString KStartupInfoId::to_text() const
    {
    return QString::fromLatin1( " ID=\"%1\" " ).arg( escape_str( id()));
    }

KStartupInfoId::KStartupInfoId( const QString& txt_P )
    {
    d = new KStartupInfoIdPrivate;
    QStringList items = get_fields( txt_P );
    const QString id_str = QString::fromLatin1( "ID=" );
    for( QStringList::Iterator it = items.begin();
         it != items.end();
         ++it )
        {
        if( ( *it ).startsWith( id_str ))
            d->id = get_cstr( *it );
        }
    }

void KStartupInfoId::initId( const QByteArray& id_P )
    {
    if( !id_P.isEmpty())
        {
        d->id = id_P;
#ifdef KSTARTUPINFO_ALL_DEBUG
        kdDebug( 172 ) << "using: " << d->id << endl;
#endif
        return;
        }
    const char* startup_env = getenv( NET_STARTUP_ENV );
    if( startup_env != NULL && *startup_env != '\0' )
        { // already has id
        d->id = startup_env;
#ifdef KSTARTUPINFO_ALL_DEBUG
        kdDebug( 172 ) << "reusing: " << d->id << endl;
#endif
        return;
        }
    d->id = KStartupInfo::createNewStartupId();
    }

bool KStartupInfoId::setupStartupEnv() const
    {
    if( id().isEmpty())
        {
        unsetenv( NET_STARTUP_ENV );
        return false;
        }
    return setenv( NET_STARTUP_ENV, id(), true ) == 0;
    }

KStartupInfoId KStartupInfo::currentStartupIdEnv()
    {
    const char* startup_env = getenv( NET_STARTUP_ENV );
    KStartupInfoId id;
    if( startup_env != NULL && *startup_env != '\0' )
        id.d->id = startup_env;
    else
        id.d->id = "0";
    return id;
    }

void KStartupInfo::resetStartupEnv()
    {
    unsetenv( NET_STARTUP_ENV );
    }

KStartupInfoId::KStartupInfoId()
    {
    d = new KStartupInfoIdPrivate;
    }

KStartupInfoId::~KStartupInfoId()
    {
    delete d;
    }

KStartupInfoId::KStartupInfoId( const KStartupInfoId& id_P )
    {
    d = new KStartupInfoIdPrivate( *id_P.d );
    }

KStartupInfoId& KStartupInfoId::operator=( const KStartupInfoId& id_P )
    {
    if( &id_P == this )
        return *this;
    delete d;
    d = new KStartupInfoIdPrivate( *id_P.d );
    return *this;
    }

bool KStartupInfoId::operator==( const KStartupInfoId& id_P ) const
    {
    return id() == id_P.id();
    }

bool KStartupInfoId::operator!=( const KStartupInfoId& id_P ) const
    {
    return !(*this == id_P );
    }

// needed for QMap
bool KStartupInfoId::operator<( const KStartupInfoId& id_P ) const
    {
    return id() < id_P.id();
    }

bool KStartupInfoId::none() const
    {
    return d->id.isEmpty() || d->id == "0";
    }

unsigned long KStartupInfoId::timestamp() const
    {
    if( none())
        return 0;
    int pos = d->id.lastIndexOf( "_TIME" );
    if( pos >= 0 )
        {
        bool ok;
        long time = QString( d->id.mid( pos + 5 ) ).toLong( &ok );
        if( ok )
            return time;
        }
    // libstartup-notification style :
    // qsnprintf (s, len, "%s/%s/%lu/%d-%d-%s",
    //   canonicalized_launcher, canonicalized_launchee, (unsigned long) timestamp,
    //  (int) getpid (), (int) sequence_number, hostbuf);
    int pos1 = d->id.lastIndexOf( '/' );
    if( pos1 > 0 )
        {
        int pos2 = d->id.lastIndexOf( '/', pos1 - 1 );
        if( pos2 >= 0 )
            {
            bool ok;
            long time = QString( d->id.mid( pos2 + 1, pos1 - pos2 - 1 ) ).toLong( &ok );
            if( ok )
                return time;
            }
        }
    // bah ... old KStartupInfo or a problem
    return 0;
    }

struct KStartupInfoDataPrivate
    {
    KStartupInfoDataPrivate() : desktop( 0 ), wmclass( "" ), hostname( "" ),
	silent( KStartupInfoData::Unknown ), timestamp( -1U ), screen( -1 ) {};
    QString bin;
    QString name;
    QString description;
    QString icon;
    int desktop;
    QList< pid_t > pids;
    QByteArray wmclass;
    QByteArray hostname;
    KStartupInfoData::TriState silent;
    unsigned long timestamp;
    int screen;
    };

QString KStartupInfoData::to_text() const
    {
    QString ret = "";
    if( !d->bin.isEmpty())
        ret += QString::fromLatin1( " BIN=\"%1\"" ).arg( escape_str( d->bin ));
    if( !d->name.isEmpty())
        ret += QString::fromLatin1( " NAME=\"%1\"" ).arg( escape_str( d->name ));
    if( !d->description.isEmpty())
        ret += QString::fromLatin1( " DESCRIPTION=\"%1\"" ).arg( escape_str( d->description ));
    if( !d->icon.isEmpty())
        ret += QString::fromLatin1( " ICON=%1" ).arg( d->icon );
    if( d->desktop != 0 )
        ret += QString::fromLatin1( " DESKTOP=%1" )
            .arg( d->desktop == NET::OnAllDesktops ? NET::OnAllDesktops : d->desktop - 1 ); // spec counts from 0
    if( !d->wmclass.isEmpty())
        ret += QString::fromLatin1( " WMCLASS=\"%1\"" ).arg( QString( d->wmclass ) );
    if( !d->hostname.isEmpty())
        ret += QString::fromLatin1( " HOSTNAME=%1" ).arg( QString( d->hostname ) );
    for( QList< pid_t >::ConstIterator it = d->pids.begin();
         it != d->pids.end();
         ++it )
        ret += QString::fromLatin1( " PID=%1" ).arg( *it );
    if( d->silent != Unknown )
	ret += QString::fromLatin1( " SILENT=%1" ).arg( d->silent == Yes ? 1 : 0 );
    if( d->timestamp != -1U )
        ret += QString::fromLatin1( " TIMESTAMP=%1" ).arg( d->timestamp );
    if( d->screen != -1 )
        ret += QString::fromLatin1( " SCREEN=%1" ).arg( d->screen );
    return ret;
    }

KStartupInfoData::KStartupInfoData( const QString& txt_P )
    {
    d = new KStartupInfoDataPrivate;
    QStringList items = get_fields( txt_P );
    const QString bin_str = QString::fromLatin1( "BIN=" );
    const QString name_str = QString::fromLatin1( "NAME=" );
    const QString description_str = QString::fromLatin1( "DESCRIPTION=" );
    const QString icon_str = QString::fromLatin1( "ICON=" );
    const QString desktop_str = QString::fromLatin1( "DESKTOP=" );
    const QString wmclass_str = QString::fromLatin1( "WMCLASS=" );
    const QString hostname_str = QString::fromLatin1( "HOSTNAME=" ); // SELI nonstd
    const QString pid_str = QString::fromLatin1( "PID=" );  // SELI nonstd
    const QString silent_str = QString::fromLatin1( "SILENT=" );
    const QString timestamp_str = QString::fromLatin1( "TIMESTAMP=" );
    const QString screen_str = QString::fromLatin1( "SCREEN=" );
    for( QStringList::Iterator it = items.begin();
         it != items.end();
         ++it )
        {
        if( ( *it ).startsWith( bin_str ))
            d->bin = get_str( *it );
        else if( ( *it ).startsWith( name_str ))
            d->name = get_str( *it );
        else if( ( *it ).startsWith( description_str ))
            d->description = get_str( *it );
        else if( ( *it ).startsWith( icon_str ))
            d->icon = get_str( *it );
        else if( ( *it ).startsWith( desktop_str ))
            {
            d->desktop = get_num( *it );
            if( d->desktop != NET::OnAllDesktops )
                ++d->desktop; // spec counts from 0
            }
        else if( ( *it ).startsWith( wmclass_str ))
            d->wmclass = get_cstr( *it );
        else if( ( *it ).startsWith( hostname_str ))
            d->hostname = get_cstr( *it );
        else if( ( *it ).startsWith( pid_str ))
            addPid( get_num( *it ));
        else if( ( *it ).startsWith( silent_str ))
            d->silent = get_num( *it ) != 0 ? Yes : No;
        else if( ( *it ).startsWith( timestamp_str ))
            d->timestamp = get_unum( *it );
        else if( ( *it ).startsWith( screen_str ))
            d->screen = get_num( *it );
        }
    }

KStartupInfoData::KStartupInfoData( const KStartupInfoData& data )
{
    d = new KStartupInfoDataPrivate( *data.d );
}

KStartupInfoData& KStartupInfoData::operator=( const KStartupInfoData& data )
{
    if( &data == this )
        return *this;
    delete d;
    d = new KStartupInfoDataPrivate( *data.d );
    return *this;
}

void KStartupInfoData::update( const KStartupInfoData& data_P )
    {
    if( !data_P.bin().isEmpty())
        d->bin = data_P.bin();
    if( !data_P.name().isEmpty() && name().isEmpty()) // don't overwrite
        d->name = data_P.name();
    if( !data_P.description().isEmpty() && description().isEmpty()) // don't overwrite
        d->description = data_P.description();
    if( !data_P.icon().isEmpty() && icon().isEmpty()) // don't overwrite
        d->icon = data_P.icon();
    if( data_P.desktop() != 0 && desktop() == 0 ) // don't overwrite
        d->desktop = data_P.desktop();
    if( !data_P.d->wmclass.isEmpty())
        d->wmclass = data_P.d->wmclass;
    if( !data_P.d->hostname.isEmpty())
        d->hostname = data_P.d->hostname;
    for( QList< pid_t >::ConstIterator it = data_P.d->pids.begin();
         it != data_P.d->pids.end();
         ++it )
        addPid( *it );
    if( data_P.silent() != Unknown )
	d->silent = data_P.silent();
    if( data_P.timestamp() != -1U && timestamp() == -1U ) // don't overwrite
        d->timestamp = data_P.timestamp();
    if( data_P.screen() != -1 )
        d->screen = data_P.screen();
    }

KStartupInfoData::KStartupInfoData()
{
    d = new KStartupInfoDataPrivate;
}

KStartupInfoData::~KStartupInfoData()
{
    delete d;
}

void KStartupInfoData::setBin( const QString& bin_P )
    {
    d->bin = bin_P;
    }

const QString& KStartupInfoData::bin() const
    {
    return d->bin;
    }

void KStartupInfoData::setName( const QString& name_P )
    {
    d->name = name_P;
    }

const QString& KStartupInfoData::name() const
    {
    return d->name;
    }

const QString& KStartupInfoData::findName() const
    {
    if( !name().isEmpty())
        return name();
    return bin();
    }

void KStartupInfoData::setDescription( const QString& desc_P )
    {
    d->description = desc_P;
    }

const QString& KStartupInfoData::description() const
    {
    return d->description;
    }

const QString& KStartupInfoData::findDescription() const
    {
    if( !description().isEmpty())
        return description();
    return name();
    }

void KStartupInfoData::setIcon( const QString& icon_P )
    {
    d->icon = icon_P;
    }

const QString& KStartupInfoData::findIcon() const
    {
    if( !icon().isEmpty())
        return icon();
    return bin();
    }

const QString& KStartupInfoData::icon() const
    {
    return d->icon;
    }

void KStartupInfoData::setDesktop( int desktop_P )
    {
    d->desktop = desktop_P;
    }

int KStartupInfoData::desktop() const
    {
    return d->desktop;
    }

void KStartupInfoData::setWMClass( const QByteArray& wmclass_P )
    {
    d->wmclass = wmclass_P;
    }

const QByteArray KStartupInfoData::findWMClass() const
    {
    if( !WMClass().isEmpty() && WMClass() != "0" )
        return WMClass();
    return bin().toUtf8();
    }

const QByteArray& KStartupInfoData::WMClass() const
    {
    return d->wmclass;
    }

void KStartupInfoData::setHostname( const QByteArray& hostname_P )
    {
    if( !hostname_P.isNull())
        d->hostname = hostname_P;
    else
        {
        char tmp[ 256 ];
        tmp[ 0 ] = '\0';
        if (!gethostname( tmp, 255 ))
	    tmp[sizeof(tmp)-1] = '\0';
        d->hostname = tmp;
        }
    }

const QByteArray& KStartupInfoData::hostname() const
    {
    return d->hostname;
    }

void KStartupInfoData::addPid( pid_t pid_P )
    {
    if( !d->pids.contains( pid_P ))
        d->pids.append( pid_P );
    }

void KStartupInfoData::remove_pid( pid_t pid_P )
    {
    d->pids.remove( pid_P );
    }

const QList< pid_t >& KStartupInfoData::pids() const
    {
    return d->pids;
    }

bool KStartupInfoData::is_pid( pid_t pid_P ) const
    {
    return d->pids.contains( pid_P );
    }

void KStartupInfoData::setSilent( TriState state_P )
    {
    d->silent = state_P;
    }

KStartupInfoData::TriState KStartupInfoData::silent() const
    {
    return d->silent;
    }

void KStartupInfoData::setTimestamp( unsigned long time )
    {
    d->timestamp = time;
    }

unsigned long KStartupInfoData::timestamp() const
    {
    return d->timestamp;
    }

void KStartupInfoData::setScreen( int screen )
    {
    d->screen = screen;
    }

int KStartupInfoData::screen() const
    {
    return d->screen;
    }

static
long get_num( const QString& item_P )
    {
    unsigned int pos = item_P.find( '=' );
    return item_P.mid( pos + 1 ).toLong();
    }

static
unsigned long get_unum( const QString& item_P )
    {
    unsigned int pos = item_P.find( '=' );
    return item_P.mid( pos + 1 ).toULong();
    }

static
QString get_str( const QString& item_P )
    {
    int pos = item_P.find( '=' );
    if( item_P.length() > pos + 2 && item_P[ pos + 1 ] == '\"' )
        {
        int pos2 = item_P.left( pos + 2 ).find( '\"' );
        if( pos2 < 0 )
            return QString::null;                      // 01234
        return item_P.mid( pos + 2, pos2 - 2 - pos );  // A="C"
        }
    return item_P.mid( pos + 1 );
    }

static
QByteArray get_cstr( const QString& item_P )
    {
    return get_str( item_P ).toUtf8();
    }

static
QStringList get_fields( const QString& txt_P )
    {
    QString txt = txt_P.simplified();
    QStringList ret;
    QString item = "";
    bool in = false;
    bool escape = false;
    for( int pos = 0;
         pos < txt.length();
         ++pos )
        {
        if( escape )
            {
            item += txt[ pos ];
            escape = false;
            }
        else if( txt[ pos ] == '\\' )
            escape = true;
        else if( txt[ pos ] == '\"' )
            in = !in;
        else if( txt[ pos ] == ' ' && !in )
            {
            ret.append( item );
            item = "";
            }
        else
            item += txt[ pos ];
        }
    ret.append( item );
    return ret;
    }

static QString escape_str( const QString& str_P )
    {
    QString ret = "";
    for( int pos = 0;
	 pos < str_P.length();
	 ++pos )
	{
	if( str_P[ pos ] == '\\'
	    || str_P[ pos ] == '"' )
	    ret += '\\';
	ret += str_P[ pos ];
	}
    return ret;
    }

#include "kstartupinfo.moc"
#endif
