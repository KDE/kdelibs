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

// kDebug() can't be turned off in kdeinit
#if 0
#define KSTARTUPINFO_ALL_DEBUG
#ifdef __GNUC__
#warning Extra KStartupInfo debug messages enabled.
#endif
#endif

#include "kstartupinfo.h"

#include <QtGui/QWidget>
#include <QtCore/QBool>

#include <config.h>

// need to resolve INT32(qglobal.h)<>INT32(Xlibint.h) conflict
#ifndef QT_CLEAN_NAMESPACE
#define QT_CLEAN_NAMESPACE
#endif

#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <QtCore/QTimer>
#include <QtGui/QActionEvent>
#ifdef Q_WS_X11
#include <qx11info_x11.h>
#include <netwm.h>
#endif
#include <kdebug.h>
#include <kapplication.h>
#include <signal.h>
#include <kstandarddirs.h>
#ifdef Q_WS_X11
#include <kwindowsystem.h>
#include <kxmessages.h>
#endif

static const char* const NET_STARTUP_MSG = "_NET_STARTUP_INFO";
static const char* const NET_STARTUP_WINDOW = "_NET_STARTUP_ID";
// DESKTOP_STARTUP_ID is used also in kinit/wrapper.c ,
// kdesu in both kdelibs and kdebase and who knows where else
static const char* const NET_STARTUP_ENV = "DESKTOP_STARTUP_ID";

static bool auto_app_started_sending = true;

static long get_num( const QString& item_P );
static unsigned long get_unum( const QString& item_P );
static QString get_str( const QString& item_P );
static QByteArray get_cstr( const QString& item_P );
static QStringList get_fields( const QString& txt_P );
static QString escape_str( const QString& str_P );

#ifdef Q_WS_X11
static Atom utf8_string_atom = None;
#endif

class KStartupInfo::Data
    : public KStartupInfoData
    {
    public:
        Data() : age(0) {} // just because it's in a QMap
        Data( const QString& txt_P )
            : KStartupInfoData( txt_P ), age( 0 ) {}
        unsigned int age;
    };

struct KStartupInfoId::Private
    {
    Private() : id( "" ) {}

        QString to_text() const;

    QByteArray id; // id
    };

struct KStartupInfoData::Private
    {
    Private() : desktop( 0 ), wmclass( "" ), hostname( "" ),
        silent( KStartupInfoData::Unknown ), timestamp( ~0U ), screen( -1 ), xinerama( -1 ), launched_by( 0 ) {}

        QString to_text() const;
        void remove_pid( pid_t pid );

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
    int xinerama;
    WId launched_by;
    QString application_id;
    };

class KStartupInfo::Private
    {
    public:
        // private slots
        void startups_cleanup();
        void startups_cleanup_no_age();
        void got_message( const QString& msg );
        void window_added( WId w );
        void slot_window_added( WId w );

        void init( int flags );
        void got_startup_info( const QString& msg_P, bool update_only_P );
        void got_remove_startup_info( const QString& msg_P );
        void new_startup_info_internal( const KStartupInfoId& id_P,
            Data& data_P, bool update_only_P );
        void remove_startup_info_internal( const KStartupInfoId& id_P );
        void remove_startup_pids( const KStartupInfoId& id, const KStartupInfoData& data );
        void remove_startup_pids( const KStartupInfoData& data );
        startup_t check_startup_internal( WId w, KStartupInfoId* id, KStartupInfoData* data );
        bool find_id( const QByteArray& id_P, KStartupInfoId* id_O,
            KStartupInfoData* data_O );
        bool find_pid( pid_t pid_P, const QByteArray& hostname, KStartupInfoId* id_O,
            KStartupInfoData* data_O );
        bool find_wclass( const QByteArray &res_name_P, const QByteArray &res_class_P,
            KStartupInfoId* id_O, KStartupInfoData* data_O );
        static QByteArray get_window_hostname( WId w_P );
        void startups_cleanup_internal( bool age_P );
        void clean_all_noncompliant();
        static QString check_required_startup_fields( const QString& msg,
            const KStartupInfoData& data, int screen );


        KStartupInfo *q;
        unsigned int timeout;
        QMap< KStartupInfoId, KStartupInfo::Data > startups;
	// contains silenced ASN's only if !AnnounceSilencedChanges
        QMap< KStartupInfoId, KStartupInfo::Data > silent_startups;
        // contains ASN's that had change: but no new: yet
        QMap< KStartupInfoId, KStartupInfo::Data > uninited_startups;
#ifdef Q_WS_X11
        KXMessages msgs;
#endif
	QTimer* cleanup;
	int flags;

	Private( int flags_P, KStartupInfo *q )
    	    : q( q ),
            timeout( 60 ),
#ifdef Q_WS_X11
	    msgs( NET_STARTUP_MSG, NULL, false ),
#endif
	      flags( flags_P )
        {
        }

        void createConnections()
        {
#ifdef Q_WS_X11
            // d == NULL means "disabled"
            if( !KApplication::kApplication())
                return;
            if( !QX11Info::display())
                return;

            if( !( flags & DisableKWinModule )) {
                QObject::connect( KWindowSystem::self(), SIGNAL( windowAdded( WId )), q, SLOT( slot_window_added( WId )));
#ifdef __GNUC__
#warning "systemTrayWindowAdded signal was remove from KWindowSystem class"
#endif		
                //QObject::connect( KWindowSystem::self(), SIGNAL( systemTrayWindowAdded( WId )), q, SLOT( slot_window_added( WId )));
            }
            QObject::connect( &msgs, SIGNAL( gotMessage( const QString& )), q, SLOT( got_message( const QString& )));
            cleanup = new QTimer( q );
            QObject::connect( cleanup, SIGNAL( timeout()), q, SLOT( startups_cleanup()));
#endif
        }
    };

KStartupInfo::KStartupInfo( int flags_P, QObject* parent_P )
    : QObject( parent_P ),
      d(new Private(flags_P, this))
    {
        d->createConnections();
    }

KStartupInfo::KStartupInfo( bool clean_on_cantdetect_P, QObject* parent_P )
    : QObject( parent_P ),
      d(new Private(clean_on_cantdetect_P ? CleanOnCantDetect : 0, this))
    {
        d->createConnections();
    }


KStartupInfo::~KStartupInfo()
    {
    delete d;
    }

void KStartupInfo::Private::got_message( const QString& msg_P )
    {
#ifdef Q_WS_X11
// TODO do something with SCREEN= ?
    kDebug( 172 ) << "got:" << msg_P;
    QString msg = msg_P.trimmed();
    if( msg.startsWith( QLatin1String("new:") )) // must match length below
        got_startup_info( msg.mid( 4 ), false );
    else if( msg.startsWith( QLatin1String("change:") )) // must match length below
        got_startup_info( msg.mid( 7 ), true );
    else if( msg.startsWith( QLatin1String("remove:") )) // must match length below
        got_remove_startup_info( msg.mid( 7 ));
#endif
    }

// if the application stops responding for a while, KWindowSystem may get
// the information about the already mapped window before KXMessages
// actually gets the info about the started application (depends
// on their order in X11 event filter in KApplication)
// simply delay info from KWindowSystem a bit
// SELI???
namespace
{
class DelayedWindowEvent
    : public QEvent
    {
    public:
	DelayedWindowEvent( WId w_P )
	    : QEvent( uniqueType() ), w( w_P ) {}
#ifdef Q_WS_X11
	Window w;
#else
	WId w;
#endif
	static Type uniqueType() { return Type(QEvent::User+15); }
    };
}

void KStartupInfo::Private::slot_window_added( WId w_P )
    {
    qApp->postEvent( q, new DelayedWindowEvent( w_P ));
    }

void KStartupInfo::customEvent( QEvent* e_P )
    {
#ifdef Q_WS_X11
    if( e_P->type() == DelayedWindowEvent::uniqueType() )
	d->window_added( static_cast< DelayedWindowEvent* >( e_P )->w );
    else
#endif
	QObject::customEvent( e_P );
    }

void KStartupInfo::Private::window_added( WId w_P )
    {
    KStartupInfoId id;
    KStartupInfoData data;
    startup_t ret = check_startup_internal( w_P, &id, &data );
    switch( ret )
        {
        case Match:
            kDebug( 172 ) << "new window match";
          break;
        case NoMatch:
          break; // nothing
        case CantDetect:
            if( flags & CleanOnCantDetect )
                clean_all_noncompliant();
          break;
        }
    }

void KStartupInfo::Private::got_startup_info( const QString& msg_P, bool update_P )
    {
    KStartupInfoId id( msg_P );
    if( id.none())
        return;
    KStartupInfo::Data data( msg_P );
    new_startup_info_internal( id, data, update_P );
    }

void KStartupInfo::Private::new_startup_info_internal( const KStartupInfoId& id_P,
                                                       KStartupInfo::Data& data_P, bool update_P )
    {
    if( id_P.none())
        return;
    if( startups.contains( id_P ))
        { // already reported, update
        startups[ id_P ].update( data_P );
        startups[ id_P ].age = 0; // CHECKME
        kDebug( 172 ) << "updating";
	if( startups[ id_P ].silent() == KStartupInfo::Data::Yes
	    && !( flags & AnnounceSilenceChanges ))
	    {
	    silent_startups[ id_P ] = startups[ id_P ];
	    startups.remove( id_P );
	    emit q->gotRemoveStartup( id_P, silent_startups[ id_P ] );
	    return;
	    }
        emit q->gotStartupChange( id_P, startups[ id_P ] );
        return;
        }
    if( silent_startups.contains( id_P ))
        { // already reported, update
        silent_startups[ id_P ].update( data_P );
        silent_startups[ id_P ].age = 0; // CHECKME
        kDebug( 172 ) << "updating silenced";
	if( silent_startups[ id_P ].silent() != Data::Yes )
	    {
	    startups[ id_P ] = silent_startups[ id_P ];
	    silent_startups.remove( id_P );
	    q->emit gotNewStartup( id_P, startups[ id_P ] );
	    return;
	    }
        emit q->gotStartupChange( id_P, silent_startups[ id_P ] );
        return;
        }
    if( uninited_startups.contains( id_P ))
        {
        uninited_startups[ id_P ].update( data_P );
        kDebug( 172 ) << "updating uninited";
        if( !update_P ) // uninited finally got new:
            {
            startups[ id_P ] = uninited_startups[ id_P ];
            uninited_startups.remove( id_P );
            emit q->gotNewStartup( id_P, startups[ id_P ] );
            return;
            }
        // no change announce, it's still uninited
        return;
        }
    if( update_P ) // change: without any new: first
        {
        kDebug( 172 ) << "adding uninited";
	uninited_startups.insert( id_P, data_P );
        }
    else if( data_P.silent() != Data::Yes || flags & AnnounceSilenceChanges )
	{
        kDebug( 172 ) << "adding";
        startups.insert( id_P, data_P );
	emit q->gotNewStartup( id_P, data_P );
	}
    else // new silenced, and silent shouldn't be announced
	{
        kDebug( 172 ) << "adding silent";
	silent_startups.insert( id_P, data_P );
	}
    cleanup->start( 1000 ); // 1 sec
    }

void KStartupInfo::Private::got_remove_startup_info( const QString& msg_P )
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

void KStartupInfo::Private::remove_startup_info_internal( const KStartupInfoId& id_P )
    {
    if( startups.contains( id_P ))
        {
	kDebug( 172 ) << "removing";
	emit q->gotRemoveStartup( id_P, startups[ id_P ]);
	startups.remove( id_P );
	}
    else if( silent_startups.contains( id_P ))
	{
	kDebug( 172 ) << "removing silent";
	silent_startups.remove( id_P );
	}
    else if( uninited_startups.contains( id_P ))
	{
	kDebug( 172 ) << "removing uninited";
	uninited_startups.remove( id_P );
	}
    return;
    }

void KStartupInfo::Private::remove_startup_pids( const KStartupInfoData& data_P )
    { // first find the matching info
    for( QMap< KStartupInfoId, KStartupInfo::Data >::Iterator it = startups.begin();
         it != startups.end();
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

void KStartupInfo::Private::remove_startup_pids( const KStartupInfoId& id_P,
    const KStartupInfoData& data_P )
    {
    kFatal( data_P.pids().count() == 0, 172 );
    Data* data = NULL;
    if( startups.contains( id_P ))
	data = &startups[ id_P ];
    else if( silent_startups.contains( id_P ))
	data = &silent_startups[ id_P ];
    else if( uninited_startups.contains( id_P ))
        data = &uninited_startups[ id_P ];
    else
	return;
    for( QList< pid_t >::ConstIterator it2 = data_P.pids().constBegin();
         it2 != data_P.pids().constEnd();
         ++it2 )
	data->d->remove_pid( *it2 ); // remove all pids from the info
    if( data->pids().count() == 0 ) // all pids removed -> remove info
    	remove_startup_info_internal( id_P );
    }

bool KStartupInfo::sendStartup( const KStartupInfoId& id_P, const KStartupInfoData& data_P )
    {
    if( id_P.none())
        return false;
#ifdef  Q_WS_X11
    KXMessages msgs;
    QString msg = QString::fromLatin1( "new: %1 %2" )
        .arg( id_P.d->to_text()).arg( data_P.d->to_text());
	QX11Info inf;
    msg = Private::check_required_startup_fields( msg, data_P, inf.screen());
    kDebug( 172 ) << "sending " << msg;
    msgs.broadcastMessage( NET_STARTUP_MSG, msg, -1, false );
#endif
    return true;
    }

bool KStartupInfo::sendStartupX( Display* disp_P, const KStartupInfoId& id_P,
    const KStartupInfoData& data_P )
    {
    if( id_P.none())
        return false;
#ifdef Q_WS_X11
    QString msg = QString::fromLatin1( "new: %1 %2" )
        .arg( id_P.d->to_text()).arg( data_P.d->to_text());
    msg = Private::check_required_startup_fields( msg, data_P, DefaultScreen( disp_P ));
#ifdef KSTARTUPINFO_ALL_DEBUG
    kDebug( 172 ) << "sending " << msg;
#endif
    return KXMessages::broadcastMessageX( disp_P, NET_STARTUP_MSG, msg, -1, false );
#else
    return true;
#endif
    }

QString KStartupInfo::Private::check_required_startup_fields( const QString& msg, const KStartupInfoData& data_P,
    int screen )
    {
    QString ret = msg;
    if( data_P.name().isEmpty())
        {
//        kWarning( 172 ) << "NAME not specified in initial startup message";
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
#ifdef Q_WS_X11
    KXMessages msgs;
    QString msg = QString::fromLatin1( "change: %1 %2" )
        .arg( id_P.d->to_text()).arg( data_P.d->to_text());
    kDebug( 172 ) << "sending " << msg;
    msgs.broadcastMessage( NET_STARTUP_MSG, msg, -1, false );
#endif
    return true;
    }

bool KStartupInfo::sendChangeX( Display* disp_P, const KStartupInfoId& id_P,
    const KStartupInfoData& data_P )
    {
    if( id_P.none())
        return false;
#ifdef Q_WS_X11
    QString msg = QString::fromLatin1( "change: %1 %2" )
        .arg( id_P.d->to_text()).arg( data_P.d->to_text());
#ifdef KSTARTUPINFO_ALL_DEBUG
    kDebug( 172 ) << "sending " << msg;
#endif
    return KXMessages::broadcastMessageX( disp_P, NET_STARTUP_MSG, msg, -1, false );
#else
    return true;
#endif
    }

bool KStartupInfo::sendFinish( const KStartupInfoId& id_P )
    {
    if( id_P.none())
        return false;
#ifdef Q_WS_X11
    KXMessages msgs;
    QString msg = QString::fromLatin1( "remove: %1" ).arg( id_P.d->to_text());
    kDebug( 172 ) << "sending " << msg;
    msgs.broadcastMessage( NET_STARTUP_MSG, msg, -1, false );
#endif
    return true;
    }

bool KStartupInfo::sendFinishX( Display* disp_P, const KStartupInfoId& id_P )
    {
    if( id_P.none())
        return false;
#ifdef Q_WS_X11
    QString msg = QString::fromLatin1( "remove: %1" ).arg( id_P.d->to_text());
#ifdef KSTARTUPINFO_ALL_DEBUG
    kDebug( 172 ) << "sending " << msg;
#endif
    return KXMessages::broadcastMessageX( disp_P, NET_STARTUP_MSG, msg, -1, false );
#else
    return true;
#endif
    }

bool KStartupInfo::sendFinish( const KStartupInfoId& id_P, const KStartupInfoData& data_P )
    {
//    if( id_P.none()) // id may be none, the pids and hostname matter then
//        return false;
#ifdef Q_WS_X11
    KXMessages msgs;
    QString msg = QString::fromLatin1( "remove: %1 %2" )
        .arg( id_P.d->to_text()).arg( data_P.d->to_text());
    kDebug( 172 ) << "sending " << msg;
    msgs.broadcastMessage( NET_STARTUP_MSG, msg, -1, false );
#endif
    return true;
    }

bool KStartupInfo::sendFinishX( Display* disp_P, const KStartupInfoId& id_P,
    const KStartupInfoData& data_P )
    {
//    if( id_P.none()) // id may be none, the pids and hostname matter then
//        return false;
#ifdef Q_WS_X11
    QString msg = QString::fromLatin1( "remove: %1 %2" )
        .arg( id_P.d->to_text()).arg( data_P.d->to_text());
#ifdef KSTARTUPINFO_ALL_DEBUG
    kDebug( 172 ) << "sending " << msg;
#endif
    return KXMessages::broadcastMessageX( disp_P, NET_STARTUP_MSG, msg, -1, false );
#else
    return true;
#endif
    }

void KStartupInfo::appStarted()
    {
    if( kapp != NULL )  // KApplication constructor unsets the env. variable
        {
        appStarted( kapp->startupId());
        kapp->clearStartupId(); // reset the id, no longer valid (must use clearStartupId() to avoid infinite loop)
        }
    else
        {
        appStarted( currentStartupIdEnv().id());
        resetStartupEnv();
        }
    }

void KStartupInfo::appStarted( const QByteArray& startup_id )
    {
    KStartupInfoId id;
    id.initId( startup_id );
    if( id.none())
        return;
    if( kapp != NULL )
        KStartupInfo::sendFinish( id );
    else if( !qgetenv( "DISPLAY" ).isEmpty() ) // don't rely on QX11Info::display()
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
    bool activate = true;
    kapp->setStartupId( startup_id );
#ifdef Q_WS_X11
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
            KWindowSystem::setOnDesktop( window->winId(), KWindowSystem::currentDesktop());
        // This is not very nice, but there's no way how to get any
        // usable timestamp without ASN, so force activating the window.
        // And even with ASN, it's not possible to get the timestamp here,
        // so if the WM doesn't have support for ASN, it can't be used either.
            KWindowSystem::forceActiveWindow( window->winId());
            }
        }
#endif
    KStartupInfo::handleAutoAppStartedSending();
    }

KStartupInfo::startup_t KStartupInfo::checkStartup( WId w_P, KStartupInfoId& id_O,
    KStartupInfoData& data_O )
    {
    return d->check_startup_internal( w_P, &id_O, &data_O );
    }

KStartupInfo::startup_t KStartupInfo::checkStartup( WId w_P, KStartupInfoId& id_O )
    {
    return d->check_startup_internal( w_P, &id_O, NULL );
    }

KStartupInfo::startup_t KStartupInfo::checkStartup( WId w_P, KStartupInfoData& data_O )
    {
    return d->check_startup_internal( w_P, NULL, &data_O );
    }

KStartupInfo::startup_t KStartupInfo::checkStartup( WId w_P )
    {
    return d->check_startup_internal( w_P, NULL, NULL );
    }

KStartupInfo::startup_t KStartupInfo::Private::check_startup_internal( WId w_P, KStartupInfoId* id_O,
    KStartupInfoData* data_O )
    {
    if( startups.count() == 0 )
        return NoMatch; // no startups
    // Strategy:
    //
    // Is this a compliant app ?
    //  - Yes - test for match
    //  - No - Is this a NET_WM compliant app ?
    //           - Yes - test for pid match
    //           - No - test for WM_CLASS match
    kDebug( 172 ) << "check_startup";
    QByteArray id = windowStartupId( w_P );
    if( !id.isNull())
        {
        if( id.isEmpty() || id == "0" ) // means ignore this window
            {
            kDebug( 172 ) << "ignore";
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
    kDebug( 172 ) << "check_startup:cantdetect";
    return CantDetect;
    }

bool KStartupInfo::Private::find_id( const QByteArray& id_P, KStartupInfoId* id_O,
    KStartupInfoData* data_O )
    {
    kDebug( 172 ) << "find_id:" << id_P;
    KStartupInfoId id;
    id.initId( id_P );
    if( startups.contains( id ))
        {
        if( id_O != NULL )
            *id_O = id;
        if( data_O != NULL )
            *data_O = startups[ id ];
        kDebug( 172 ) << "check_startup_id:match";
        return true;
        }
    return false;
    }

bool KStartupInfo::Private::find_pid( pid_t pid_P, const QByteArray& hostname_P,
    KStartupInfoId* id_O, KStartupInfoData* data_O )
    {
    kDebug( 172 ) << "find_pid:" << pid_P;
    for( QMap< KStartupInfoId, KStartupInfo::Data >::Iterator it = startups.begin();
         it != startups.end();
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
            kDebug( 172 ) << "check_startup_pid:match";
            return true;
            }
        }
    return false;
    }

bool KStartupInfo::Private::find_wclass( const QByteArray &_res_name, const QByteArray &_res_class,
    KStartupInfoId* id_O, KStartupInfoData* data_O )
    {
    QByteArray res_name = _res_name.toLower();
    QByteArray res_class = _res_class.toLower();
    kDebug( 172 ) << "find_wclass:" << res_name << ":" << res_class;
    for( QMap< KStartupInfoId, Data >::Iterator it = startups.begin();
         it != startups.end();
         ++it )
        {
        const QByteArray wmclass = ( *it ).findWMClass();
        if( wmclass.toLower() == res_name || wmclass.toLower() == res_class )
            { // Found it !
            if( id_O != NULL )
                *id_O = it.key();
            if( data_O != NULL )
                *data_O = *it;
            // non-compliant, remove on first match
            remove_startup_info_internal( it.key());
            kDebug( 172 ) << "check_startup_wclass:match";
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

QByteArray KStartupInfo::Private::get_window_hostname( WId w_P )
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
    d->timeout = secs_P;
 // schedule removing entries that are older than the new timeout
    QTimer::singleShot( 0, this, SLOT( startups_cleanup_no_age()));
    }

void KStartupInfo::Private::startups_cleanup_no_age()
    {
    startups_cleanup_internal( false );
    }

void KStartupInfo::Private::startups_cleanup()
    {
    if( startups.count() == 0 && silent_startups.count() == 0
        && uninited_startups.count() == 0 )
        {
        cleanup->stop();
        return;
        }
    startups_cleanup_internal( true );
    }

void KStartupInfo::Private::startups_cleanup_internal( bool age_P )
    {
    for( QMap< KStartupInfoId, KStartupInfo::Data >::Iterator it = startups.begin();
         it != startups.end();
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
            kDebug( 172 ) << "entry timeout:" << key.id();
            remove_startup_info_internal( key );
            }
        else
            ++it;
        }
    for( QMap< KStartupInfoId, KStartupInfo::Data >::Iterator it = silent_startups.begin();
         it != silent_startups.end();
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
            kDebug( 172 ) << "entry timeout:" << key.id();
            remove_startup_info_internal( key );
            }
        else
            ++it;
        }
    for( QMap< KStartupInfoId, KStartupInfo::Data >::Iterator it = uninited_startups.begin();
         it != uninited_startups.end();
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
            kDebug( 172 ) << "entry timeout:" << key.id();
            remove_startup_info_internal( key );
            }
        else
            ++it;
        }
    }

void KStartupInfo::Private::clean_all_noncompliant()
    {
    for( QMap< KStartupInfoId, KStartupInfo::Data >::Iterator it = startups.begin();
         it != startups.end();
         )
        {
        if( ( *it ).WMClass() != "0" )
            {
            ++it;
            continue;
            }
        const KStartupInfoId& key = it.key();
        ++it;
        kDebug( 172 ) << "entry cleaning:" << key.id();
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
    unsigned long qt_x_user_time = QX11Info::appUserTime();
#else
    unsigned long qt_x_user_time = 0;
#endif
    QByteArray id = QString::fromLatin1( "%1;%2;%3;%4_TIME%5" ).arg( hostname ).arg( tm.tv_sec )
        .arg( tm.tv_usec ).arg( getpid()).arg( qt_x_user_time ).toUtf8();
    kDebug( 172 ) << "creating: " << id << ":" << (qApp ? qAppName() : QString("unnamed app") /* e.g. kdeinit */);
    return id;
    }


const QByteArray& KStartupInfoId::id() const
    {
    return d->id;
    }


QString KStartupInfoId::Private::to_text() const
    {
    return QString::fromLatin1( " ID=\"%1\" " ).arg( escape_str( id));
    }

KStartupInfoId::KStartupInfoId( const QString& txt_P ) : d(new Private)
    {
    const QStringList items = get_fields( txt_P );
    const QString id_str = QLatin1String( "ID=" );
    for( QStringList::ConstIterator it = items.begin();
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
        kDebug( 172 ) << "using: " << d->id;
#endif
        return;
        }
    const QByteArray startup_env = qgetenv( NET_STARTUP_ENV );
    if( !startup_env.isEmpty() )
        { // already has id
        d->id = startup_env;
#ifdef KSTARTUPINFO_ALL_DEBUG
        kDebug( 172 ) << "reusing: " << d->id;
#endif
        return;
        }
    d->id = KStartupInfo::createNewStartupId();
    }

bool KStartupInfoId::setupStartupEnv() const
    {
    if( none())
        {
        unsetenv( NET_STARTUP_ENV );
        return false;
        }
    return setenv( NET_STARTUP_ENV, id(), true ) == 0;
    }

KStartupInfoId KStartupInfo::currentStartupIdEnv()
    {
    const QByteArray startup_env = qgetenv( NET_STARTUP_ENV );
    KStartupInfoId id;
    if( !startup_env.isEmpty() )
        id.d->id = startup_env;
    else
        id.d->id = "0";
    return id;
    }

void KStartupInfo::resetStartupEnv()
    {
    unsetenv( NET_STARTUP_ENV );
    }

KStartupInfoId::KStartupInfoId() : d(new Private)
    {
    }

KStartupInfoId::~KStartupInfoId()
    {
    delete d;
    }

KStartupInfoId::KStartupInfoId( const KStartupInfoId& id_P ) : d(new Private(*id_P.d))
    {
    }

KStartupInfoId& KStartupInfoId::operator=( const KStartupInfoId& id_P )
    {
    if( &id_P == this )
        return *this;
    *d = *id_P.d;
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

// KDE5 TODO: rename to isNull ?
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
        unsigned long time = QString( d->id.mid( pos + 5 ) ).toULong( &ok );
        if( !ok && d->id[ pos + 5 ] == '-' ) // try if it's as a negative signed number perhaps
            time = QString( d->id.mid( pos + 5 ) ).toLong( &ok );
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
            unsigned long time = QString( d->id.mid( pos2 + 1, pos1 - pos2 - 1 ) ).toULong( &ok );
            if( !ok && d->id[ pos2 + 1 ] == '-' )
                time = QString( d->id.mid( pos2 + 1, pos1 - pos2 - 1 ) ).toLong( &ok );
            if( ok )
                return time;
            }
        }
    // bah ... old KStartupInfo or a problem
    return 0;
    }

QString KStartupInfoData::Private::to_text() const
    {
    QString ret;
    if( !bin.isEmpty())
        ret += QString::fromLatin1( " BIN=\"%1\"" ).arg( escape_str( bin ));
    if( !name.isEmpty())
        ret += QString::fromLatin1( " NAME=\"%1\"" ).arg( escape_str( name ));
    if( !description.isEmpty())
        ret += QString::fromLatin1( " DESCRIPTION=\"%1\"" ).arg( escape_str( description ));
    if( !icon.isEmpty())
        ret += QString::fromLatin1( " ICON=%1" ).arg( icon );
    if( desktop != 0 )
        ret += QString::fromLatin1( " DESKTOP=%1" )
#ifdef Q_WS_X11
            .arg( desktop == NET::OnAllDesktops ? NET::OnAllDesktops : desktop - 1 ); // spec counts from 0
#else
            .arg( 0 ); // spec counts from 0
#endif
    if( !wmclass.isEmpty())
        ret += QString::fromLatin1( " WMCLASS=\"%1\"" ).arg( QString( wmclass ) );
    if( !hostname.isEmpty())
        ret += QString::fromLatin1( " HOSTNAME=%1" ).arg( QString( hostname ) );
    for( QList< pid_t >::ConstIterator it = pids.begin();
         it != pids.end();
         ++it )
        ret += QString::fromLatin1( " PID=%1" ).arg( *it );
    if( silent != KStartupInfoData::Unknown )
	ret += QString::fromLatin1( " SILENT=%1" ).arg( silent == KStartupInfoData::Yes ? 1 : 0 );
    if( timestamp != ~0U )
        ret += QString::fromLatin1( " TIMESTAMP=%1" ).arg( timestamp );
    if( screen != -1 )
        ret += QString::fromLatin1( " SCREEN=%1" ).arg( screen );
    if( xinerama != -1 )
        ret += QString::fromLatin1( " XINERAMA=%1" ).arg( xinerama );
    if( launched_by != 0 )
        ret += QString::fromLatin1( " LAUNCHED_BY=%1" ).arg( (qptrdiff)launched_by );
    if( !application_id.isEmpty())
        ret += QString::fromLatin1( " APPLICATION_ID=%1" ).arg( application_id );
    return ret;
    }

KStartupInfoData::KStartupInfoData( const QString& txt_P ) : d(new Private)
    {
    const QStringList items = get_fields( txt_P );
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
    const QString xinerama_str = QString::fromLatin1( "XINERAMA=" );
    const QString launched_by_str = QString::fromLatin1( "LAUNCHED_BY=" );
    const QString application_id_str = QString::fromLatin1( "APPLICATION_ID=" );
    for( QStringList::ConstIterator it = items.begin();
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
#ifdef Q_WS_X11
            if( d->desktop != NET::OnAllDesktops )
#endif
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
        else if( ( *it ).startsWith( xinerama_str ))
            d->xinerama = get_num( *it );
        else if( ( *it ).startsWith( launched_by_str ))
            d->launched_by = ( WId ) get_num( *it );
        else if( ( *it ).startsWith( application_id_str ))
            d->application_id = get_str( *it );
        }
    }

KStartupInfoData::KStartupInfoData( const KStartupInfoData& data ) : d(new Private(*data.d))
{
}

KStartupInfoData& KStartupInfoData::operator=( const KStartupInfoData& data )
{
    if( &data == this )
        return *this;
    *d = *data.d;
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
    for( QList< pid_t >::ConstIterator it = data_P.d->pids.constBegin();
         it != data_P.d->pids.constEnd();
         ++it )
        addPid( *it );
    if( data_P.silent() != Unknown )
	d->silent = data_P.silent();
    if( data_P.timestamp() != ~0U && timestamp() == ~0U ) // don't overwrite
        d->timestamp = data_P.timestamp();
    if( data_P.screen() != -1 )
        d->screen = data_P.screen();
    if( data_P.xinerama() != -1 && xinerama() != -1 ) // don't overwrite
        d->xinerama = data_P.xinerama();
    if( data_P.launchedBy() != 0 && launchedBy() != 0 ) // don't overwrite
        d->launched_by = data_P.launchedBy();
    if( !data_P.applicationId().isEmpty() && applicationId().isEmpty()) // don't overwrite
        d->application_id = data_P.applicationId();
    }

KStartupInfoData::KStartupInfoData() : d(new Private)
{
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

QByteArray KStartupInfoData::WMClass() const
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

QByteArray KStartupInfoData::hostname() const
    {
    return d->hostname;
    }

void KStartupInfoData::addPid( pid_t pid_P )
    {
    if( !d->pids.contains( pid_P ))
        d->pids.append( pid_P );
    }

void KStartupInfoData::Private::remove_pid( pid_t pid_P )
    {
        pids.removeAll( pid_P );
    }

QList< pid_t > KStartupInfoData::pids() const
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

void KStartupInfoData::setScreen( int _screen )
    {
    d->screen = _screen;
    }

int KStartupInfoData::screen() const
    {
    return d->screen;
    }

void KStartupInfoData::setXinerama( int xinerama )
    {
    d->xinerama = xinerama;
    }

int KStartupInfoData::xinerama() const
    {
    return d->xinerama;
    }

void KStartupInfoData::setLaunchedBy( WId window )
    {
    d->launched_by = window;
    }

WId KStartupInfoData::launchedBy() const
    {
    return d->launched_by;
    }

void KStartupInfoData::setApplicationId( const QString& desktop )
    {
    if( desktop.startsWith( '/' ))
        {
        d->application_id = desktop;
        return;
        }
    // the spec requires this is always a full path, in order for everyone to be able to find it
    QString desk = KStandardDirs::locate( "apps", desktop );
    if( desk.isEmpty())
        desk = KStandardDirs::locate( "services", desktop );
    if( desk.isEmpty())
        return;
    d->application_id = desk;
    }

QString KStartupInfoData::applicationId() const
    {
    return d->application_id;
    }

static
long get_num( const QString& item_P )
    {
    unsigned int pos = item_P.indexOf( QLatin1Char('=') );
    return item_P.mid( pos + 1 ).toLong();
    }

static
unsigned long get_unum( const QString& item_P )
    {
    unsigned int pos = item_P.indexOf( QLatin1Char('=') );
    return item_P.mid( pos + 1 ).toULong();
    }

static
QString get_str( const QString& item_P )
    {
    int pos = item_P.indexOf( QLatin1Char('=') );
    if( item_P.length() > pos + 2 && item_P.at( pos + 1 ) == QLatin1Char('\"') )
        {
        int pos2 = item_P.left( pos + 2 ).indexOf( QLatin1Char('\"') );
        if( pos2 < 0 )
            return QString();                      // 01234
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
