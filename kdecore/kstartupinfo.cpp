/****************************************************************************

 $Id$

 Copyright (C) 2001 Lubos Lunak        <l.lunak@kde.org>

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
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "kstartupinfo.h"

#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <qtimer.h>
#include <netwm.h>
#include <kdebug.h>
#include <kapp.h>
#include <signal.h>
#include <kwinmodule.h>
#include <kxmessages.h>
#ifndef KDE_USE_FINAL
#include <X11/Xlibint.h> // cannot be included multiple times
#endif

#undef Data // crappy X11

#ifndef None  // CHECKME
#define None 0
#endif

static const char* const KDE_STARTUP_INFO = "KDE_STARTUP_INFO";
static const char* const KDE_STARTUP_ID = "KDE_STARTUP_ID";
// KDE_STARTUP_ENV is used also in kinit/wrapper.c
static const char* const KDE_STARTUP_ENV = "KDE_STARTUP_ENV";

static int get_num( const QString& item_P );
static QString get_str( const QString& item_P );
static QCString get_cstr( const QString& item_P );
static QStringList get_fields( const QString& txt_P );
static QString escape_str( const QString& str_P );

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
        KWinModule* wm_module;
        KXMessages msgs;
	QTimer* cleanup;
       KStartupInfoPrivate() : msgs( KDE_STARTUP_INFO ) {}
    };
    
KStartupInfo::KStartupInfo( bool clean_on_cantdetect_P, QObject* parent_P, const char* name_P )
    : QObject( parent_P, name_P ), 
        d(0L),
        clean_on_cantdetect( clean_on_cantdetect_P ), timeout( 60 )
    {
    if (!KApplication::kApplication()) return;
    if (!KApplication::kApplication()->getDisplay()) return;

    d = new KStartupInfoPrivate;
    d->wm_module = new KWinModule( this );
    connect( d->wm_module, SIGNAL( windowAdded( WId )), SLOT( window_added( WId )));
    connect( d->wm_module, SIGNAL( systemTrayWindowAdded( WId )), SLOT( window_added( WId )));
    connect( &d->msgs, SIGNAL( gotMessage( const QString& )), SLOT( got_message( const QString& )));
    d->cleanup = new QTimer( this );
    connect( d->cleanup, SIGNAL( timeout()), SLOT( startups_cleanup()));
    }

KStartupInfo::~KStartupInfo()
    {
    if (d) delete d;
    }

void KStartupInfo::got_message( const QString& msg_P )
    {
//    kdDebug( 172 ) << "got:" << msg_P << endl;
    QString msg = msg_P.stripWhiteSpace();
    if( msg.startsWith( "new:" )) // must match length below
        got_startup_info( msg.mid( 4 ), false );
    else if( msg.startsWith( "change:" )) // must match length below
        got_startup_info( msg.mid( 7 ), true );
    else if( msg.startsWith( "remove:" )) // must match length below
        got_remove_startup_info( msg.mid( 7 ));
    }

void KStartupInfo::window_added( WId w_P )
    {
    KStartupInfoId id;
    startup_t ret = check_startup_internal( w_P, &id, NULL, false );
    switch( ret )
        {
        case Match:
//            kdDebug( 172 ) << "new window match" << endl;
            remove_startup_info_internal( id );
          break;
        case NoMatch:
          break; // nothing
        case CantDetect:
            if( clean_on_cantdetect )
                clean_all_noncompliant();
          break;
        }
    }
        
void KStartupInfo::got_startup_info( const QString& msg_P, bool update_only_P )
    {
    KStartupInfoId id( msg_P );
    if( id.none())
        return;
    KStartupInfo::Data data( msg_P );
    new_startup_info_internal( id, data, update_only_P );
    }

void KStartupInfo::new_startup_info_internal( const KStartupInfoId& id_P,
    Data& data_P, bool update_only_P )
    {
    if (!d) return;
    if( id_P.none())
        return;
    if( d->startups.contains( id_P ))
        { // already reported, update
        d->startups[ id_P ].update( data_P );
        d->startups[ id_P ].age = 0; // CHECKME
//        kdDebug( 172 ) << "updating" << endl;
        emit gotStartupChange( id_P, d->startups[ id_P ] );
        return;
        }
    if( update_only_P )
        return;
    d->startups.insert( id_P, data_P );
//    kdDebug( 172 ) << "adding" << endl;
    emit gotNewStartup( id_P, data_P );
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
    if (!d) return;
    if( !d->startups.contains( id_P ))
        return;
//    kdDebug( 172 ) << "removing" << endl;
    emit gotRemoveStartup( id_P, d->startups[ id_P ]);
    d->startups.remove( id_P );
    return;
    }

void KStartupInfo::remove_startup_pids( const KStartupInfoData& data_P )
    { // first find the matching info
    if (!d) return;
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
    if (!d) return;
    kdFatal( data_P.pids().count() == 0, 172 );
    if( !d->startups.contains( id_P ))
        return;
    Data& data = d->startups[ id_P ];
    for( QValueList< pid_t >::ConstIterator it2 = data_P.pids().begin();
         it2 != data_P.pids().end();
         ++it2 )
        data.remove_pid( *it2 ); // remove all pids from the info
    if( data.pids().count() == 0 ) // all pids removed -> remove info
        remove_startup_info_internal( id_P );
    }

bool KStartupInfo::sendStartup( const KStartupInfoId& id_P, const KStartupInfoData& data_P )
    {
    if( id_P.none())
        return false;
    KXMessages msgs;
    QString msg = QString::fromLatin1( "new: %1 %2" )
        .arg( id_P.to_text()).arg( data_P.to_text());
//    kdDebug( 172 ) << "sending " << msg << endl;
    msgs.broadcastMessage( KDE_STARTUP_INFO, msg );
    return true;
    }

bool KStartupInfo::sendStartupX( Display* disp_P, const KStartupInfoId& id_P,
    const KStartupInfoData& data_P )
    {
    if( id_P.none())
        return false;
    QString msg = QString::fromLatin1( "new: %1 %2" )
        .arg( id_P.to_text()).arg( data_P.to_text());
#ifdef KSTARTUPINFO_ALL_DEBUG
    kdDebug( 172 ) << "sending " << msg << endl;
#endif
    return KXMessages::broadcastMessageX( disp_P, KDE_STARTUP_INFO, msg );
    }

bool KStartupInfo::sendChange( const KStartupInfoId& id_P, const KStartupInfoData& data_P )
    {
    if( id_P.none())
        return false;
    KXMessages msgs;
    QString msg = QString::fromLatin1( "change: %1 %2" )
        .arg( id_P.to_text()).arg( data_P.to_text());
//    kdDebug( 172 ) << "sending " << msg << endl;
    msgs.broadcastMessage( KDE_STARTUP_INFO, msg );
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
    return KXMessages::broadcastMessageX( disp_P, KDE_STARTUP_INFO, msg );
    }

bool KStartupInfo::sendFinish( const KStartupInfoId& id_P )
    {
    if( id_P.none())
        return false;
    KXMessages msgs;
    QString msg = QString::fromLatin1( "remove: %1" ).arg( id_P.to_text());
//    kdDebug( 172 ) << "sending " << msg << endl;
    msgs.broadcastMessage( KDE_STARTUP_INFO, msg );
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
    return KXMessages::broadcastMessageX( disp_P, KDE_STARTUP_INFO, msg );
    }

bool KStartupInfo::sendFinish( const KStartupInfoId& id_P, const KStartupInfoData& data_P )
    {
//    if( id_P.none()) // id may be none, the pids and hostname matter then
//        return false;
    KXMessages msgs;
    QString msg = QString::fromLatin1( "remove: %1 %2" )
        .arg( id_P.to_text()).arg( data_P.to_text());
//    kdDebug( 172 ) << "sending " << msg << endl;
    msgs.broadcastMessage( KDE_STARTUP_INFO, msg );
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
    return KXMessages::broadcastMessageX( disp_P, KDE_STARTUP_INFO, msg );
    }

void KStartupInfo::appStarted()
    {
    if( kapp != NULL ) // KApplication constructor unsets the env. variable
        {
        KStartupInfoId id;
        id.initId( kapp->startupId());
        if( !id.none())
            KStartupInfo::sendFinish( id );
        }
    else
        {
        KStartupInfoId id = KStartupInfo::currentStartupIdEnv();
        if( !id.none())
            { // no KApplication instance, don't rely on qt_xdisplay()
            Display* disp = XOpenDisplay( NULL );
            if( disp != NULL )
                {
                KStartupInfo::sendFinishX( disp, id );
                XCloseDisplay( disp );
                }
            }
        }
    }

KStartupInfo::startup_t KStartupInfo::checkStartup( WId w_P, KStartupInfoId& id_O,
    KStartupInfoData& data_O )
    {
    return check_startup_internal( w_P, &id_O, &data_O, true );
    }

KStartupInfo::startup_t KStartupInfo::checkStartup( WId w_P, KStartupInfoId& id_O )
    {
    return check_startup_internal( w_P, &id_O, NULL, true );
    }

KStartupInfo::startup_t KStartupInfo::checkStartup( WId w_P, KStartupInfoData& data_O )
    {
    return check_startup_internal( w_P, NULL, &data_O, true );
    }

KStartupInfo::startup_t KStartupInfo::checkStartup( WId w_P )
    {
    return check_startup_internal( w_P, NULL, NULL, true );
    }
    
KStartupInfo::startup_t KStartupInfo::check_startup_internal( WId w_P, KStartupInfoId* id_O,
    KStartupInfoData* data_O, bool remove_P )
    {
    if (!d) return NoMatch;
    if( d->startups.count() == 0 )
        return NoMatch; // no startups
    NETWinInfo info( qt_xdisplay(),  w_P, qt_xrootwin(),
        NET::WMWindowType | NET::WMPid | NET::WMState );
    // ignore NET::Tool and other special window types
    if( info.windowType() != NET::Normal
        && info.windowType() != NET::Override
        && info.windowType() != NET::Unknown
        && info.windowType() != NET::Dock )
	return NoMatch;
    // lets see if this is a transient
    Window transient_for;
    if( XGetTransientForHint( qt_xdisplay(), static_cast< Window >( w_P ), &transient_for )
        && static_cast< WId >( transient_for ) != qt_xrootwin()
        && transient_for != None )
	return NoMatch;
#if 0 // CHECKME
        Task* t = findTask((WId) transient_for);
	if (t) {
	    if (t->window() != w) {
		t->addTransient(w);
                // kdDebug() << "TM: Transient " << w << " added for Task: " << t->window() << endl;
            }
	    return;
	}
#endif
    // Strategy:
    //
    // Is this a compliant app ?
    //  - Yes - test for match
    //  - No - Is this a NET_WM compliant app ?
    //           - Yes - test for pid match
    //           - No - test for WM_CLASS match
//    kdDebug( 172 ) << "check_startup" << endl;
    QCString id = windowStartupId( w_P );
    if( !id.isNull())
        {
        if( id.isEmpty() || id == "0" ) // means ignore this window
            {
//            kdDebug( 172 ) << "ignore" << endl;
            return NoMatch;
            }
        return find_id( id, id_O, data_O, remove_P ) ? Match : NoMatch;
        }
    // _NET_WM_PID apps are also considered compliant
    pid_t pid = info.pid();
    if( pid > 0 )
        {
        QCString hostname = get_window_hostname( w_P );
        if( !hostname.isEmpty())
            return find_pid( pid, hostname, id_O, data_O, remove_P ) ? Match : NoMatch;
        }
        // no hostname !?
    // Hard - this app is not even NET_WM compliant
    XClassHint hint;
    if( XGetClassHint( qt_xdisplay(), w_P, &hint ) != 0 )
        { // We managed to read the class hint
        if( find_wclass( hint.res_name, hint.res_class, id_O, data_O, remove_P ))
            return Match;
        }
//    kdDebug( 172 ) << "check_startup:cantdetect" << endl;
    return CantDetect;
    }

bool KStartupInfo::find_id( const QCString& id_P, KStartupInfoId* id_O,
    KStartupInfoData* data_O, bool remove_P )
    {
    if (!d) return false;
//    kdDebug( 172 ) << "find_id:" << id_P << endl;
    KStartupInfoId id;
    id.initId( id_P );
    if( d->startups.contains( id ))
        {
        if( id_O != NULL )
            *id_O = id;
        if( data_O != NULL )
            *data_O = d->startups[ id ];
        if( remove_P )
            d->startups.remove( id );
//        kdDebug( 172 ) << "check_startup_id:match" << endl;
        return true;
        }
    return false;
    }
    
bool KStartupInfo::find_pid( pid_t pid_P, const QCString& hostname_P, 
    KStartupInfoId* id_O, KStartupInfoData* data_O, bool remove_P )
    {
    if (!d) return false;
//    kdDebug( 172 ) << "find_pid:" << pid_P << endl;
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
            if( remove_P )
                d->startups.remove( it );
//            kdDebug( 172 ) << "check_startup_pid:match" << endl;
            return true;
            }
        }
    return false;
    }

bool KStartupInfo::find_wclass( QCString res_name, QCString res_class,
    KStartupInfoId* id_O, KStartupInfoData* data_O, bool remove_P )
    {
    if (!d) return false;
    res_name = res_name.lower();
    res_class = res_class.lower();
//    kdDebug( 172 ) << "find_wclass:" << res_name << ":" << res_class << endl;
    for( QMap< KStartupInfoId, Data >::Iterator it = d->startups.begin();
         it != d->startups.end();
         ++it )
        {
        const QCString wmclass = ( *it ).findWMClass();
        if( wmclass.lower() == res_name || wmclass.lower() == res_class )
            { // Found it !
            if( id_O != NULL )
                *id_O = it.key();
            if( data_O != NULL )
                *data_O = *it;
            if( remove_P )
                d->startups.remove( it );
//            kdDebug( 172 ) << "check_startup_wclass:match" << endl;
            return true;
            }
        }
    return false;
    }

static Atom kde_startup_atom = None;

QCString KStartupInfo::windowStartupId( WId w_P )
    {
    if( kde_startup_atom == None )
        kde_startup_atom = XInternAtom( qt_xdisplay(), KDE_STARTUP_ID, False );
    unsigned char *name_ret;
    QCString ret;
    Atom type_ret;
    int format_ret;
    unsigned long nitems_ret = 0, after_ret = 0;
    if( XGetWindowProperty( qt_xdisplay(), w_P, kde_startup_atom, 0l, (long) BUFSIZE,
            False, XA_STRING, &type_ret, &format_ret, &nitems_ret, &after_ret, &name_ret )
	    == Success )
        {
	if( type_ret == XA_STRING && format_ret == 8 && name_ret != NULL )
	    ret = reinterpret_cast< char* >( name_ret );
        if ( name_ret != NULL )
            XFree( name_ret );
        }
    return ret;
    }

void KStartupInfo::setWindowStartupId( WId w_P, const QCString& id_P )
    {
    if( id_P.isNull())
        return;
    if( kde_startup_atom == None )
        kde_startup_atom = XInternAtom( qt_xdisplay(), KDE_STARTUP_ID, False );
    XChangeProperty( qt_xdisplay(), w_P, kde_startup_atom, XA_STRING, 8,
        PropModeReplace, reinterpret_cast< unsigned char* >( id_P.data()), id_P.length());
    }

QCString KStartupInfo::get_window_hostname( WId w_P )
    {
    XTextProperty tp;
    char** hh;
    int cnt;
    if( XGetWMClientMachine( qt_xdisplay(), w_P, &tp ) != 0
        && XTextPropertyToStringList( &tp, &hh, &cnt ) != 0 )
        {
        if( cnt == 1 )
            {
            QCString hostname = hh[ 0 ];
            XFreeStringList( hh );
            return hostname;
            }
        XFreeStringList( hh );
        }
    // no hostname
    return QCString();
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
    if (!d) return;
    if( d->startups.count() == 0 )
        {
        d->cleanup->stop();
        return;
        }
    startups_cleanup_internal( true );
    }
    
void KStartupInfo::startups_cleanup_internal( bool age_P )
    {
    if (!d) return;
    for( QMap< KStartupInfoId, Data >::Iterator it = d->startups.begin();
         it != d->startups.end();
         )
        {
        if( age_P )
            ( *it ).age++;
        if( ( *it ).age >= timeout )
            {
            const KStartupInfoId& key = it.key();
            ++it;
//            kdDebug( 172 ) << "entry timeout:" << key.id() << endl;
            remove_startup_info_internal( key );
            }
        else
            ++it;
        }
    }
    
void KStartupInfo::clean_all_noncompliant()
    {
    if (!d) return;
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
//        kdDebug( 172 ) << "entry cleaning:" << key.id() << endl;
        remove_startup_info_internal( key );
        }
    }
    
struct KStartupInfoIdPrivate
    {
    KStartupInfoIdPrivate() : id( "" ) {};
    QCString id; // id
    };

const QCString& KStartupInfoId::id() const
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

void KStartupInfoId::initId( const QCString& id_P )
    {
    if( !id_P.isEmpty())
        {
        d->id = id_P;
#ifdef KSTARTUPINFO_ALL_DEBUG
        kdDebug( 172 ) << "using: " << d->id << endl;
#endif
        return;
        }
    const char* startup_env = getenv( KDE_STARTUP_ENV );
    if( startup_env != NULL && *startup_env != '\0' )
        { // already has id
        d->id = startup_env;
#ifdef KSTARTUPINFO_ALL_DEBUG
        kdDebug( 172 ) << "reusing: " << d->id << endl;
#endif
        return;
        }
    // assign a unique id  CHECKME
    // use hostname+time+pid, that should be 200% unique
    // hmm, probably something 99.9% unique and much shorter would be enough
    struct timeval tm;
    gettimeofday( &tm, NULL );
    char hostname[ 256 ];
    hostname[ 0 ] = '\0';
    gethostname( hostname, 255 );
    d->id = QString( "%1;%2;%3;%4" ).arg( hostname ).arg( tm.tv_sec )
        .arg( tm.tv_usec ).arg( getpid()).latin1();
#ifdef KSTARTUPINFO_ALL_DEBUG
    kdDebug( 172 ) << "creating: " << d->id << endl;
#endif
    }

bool KStartupInfoId::setupStartupEnv() const
    {
    if( id().isEmpty())
        {
        unsetenv( KDE_STARTUP_ENV );
        return false;
        }
    return setenv( KDE_STARTUP_ENV, id(), true ) == 0;
    }

KStartupInfoId KStartupInfo::currentStartupIdEnv()
    {
    const char* startup_env = getenv( KDE_STARTUP_ENV );
    KStartupInfoId id;
    if( startup_env != NULL && *startup_env != '\0' )
        id.d->id = startup_env;
    else
        id.d->id = "0";
    return id;
    }
    
void KStartupInfo::resetStartupEnv()
    {
    unsetenv( KDE_STARTUP_ENV );
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
    
struct KStartupInfoDataPrivate
    {
    KStartupInfoDataPrivate() : desktop( 0 ), wmclass( "" ), hostname( "" ) {};
    QString bin;
    QString name;
    QString icon;
    int desktop;
    QValueList< pid_t > pids;
    QCString wmclass;
    QCString hostname;
    };
    
QString KStartupInfoData::to_text() const
    {
    QString ret = "";
    if( !d->bin.isEmpty())
        ret += QString::fromLatin1( " BIN=\"%1\"" ).arg( escape_str( d->bin ));
    if( !d->name.isEmpty())
        ret += QString::fromLatin1( " NAME=\"%1\"" ).arg( escape_str( d->name ));
    if( !d->icon.isEmpty())
        ret += QString::fromLatin1( " ICON=%1" ).arg( d->icon );
    if( d->desktop != 0 )
        ret += QString::fromLatin1( " DESKTOP=%1" ).arg( d->desktop );
    if( !d->wmclass.isEmpty())
        ret += QString::fromLatin1( " WMCLASS=%1" ).arg( d->wmclass );
    if( !d->hostname.isEmpty())
        ret += QString::fromLatin1( " HOSTNAME=%1" ).arg( d->hostname );
    for( QValueList< pid_t >::ConstIterator it = d->pids.begin();
         it != d->pids.end();
         ++it )
        ret += QString::fromLatin1( " PID=%1" ).arg( *it );
    return ret;
    }

KStartupInfoData::KStartupInfoData( const QString& txt_P )
    {
    d = new KStartupInfoDataPrivate;
    QStringList items = get_fields( txt_P );
    const QString bin_str = QString::fromLatin1( "BIN=" );
    const QString name_str = QString::fromLatin1( "NAME=" );
    const QString icon_str = QString::fromLatin1( "ICON=" );
    const QString desktop_str = QString::fromLatin1( "DESKTOP=" );
    const QString wmclass_str = QString::fromLatin1( "WMCLASS=" );
    const QString hostname_str = QString::fromLatin1( "HOSTNAME=" );
    const QString pid_str = QString::fromLatin1( "PID=" );
    for( QStringList::Iterator it = items.begin();
         it != items.end();
         ++it )
        {
        if( ( *it ).startsWith( bin_str ))
            d->bin = get_str( *it );
        else if( ( *it ).startsWith( name_str ))
            d->name = get_str( *it );
        else if( ( *it ).startsWith( icon_str ))
            d->icon = get_str( *it );
        else if( ( *it ).startsWith( desktop_str ))
            d->desktop = get_num( *it );
        else if( ( *it ).startsWith( wmclass_str ))
            d->wmclass = get_cstr( *it );
        else if( ( *it ).startsWith( hostname_str ))
            d->hostname = get_cstr( *it );
        else if( ( *it ).startsWith( pid_str ))
            addPid( get_num( *it ));
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
    if( !data_P.icon().isEmpty() && icon().isEmpty()) // don't overwrite
        d->icon = data_P.icon();
    if( data_P.desktop() != 0 && desktop() == 0 ) // don't overwrite
        d->desktop = data_P.desktop();
    if( !data_P.d->wmclass.isEmpty())
        d->wmclass = data_P.d->wmclass;
    if( !data_P.d->hostname.isEmpty())
        d->hostname = data_P.d->hostname;
    for( QValueList< pid_t >::ConstIterator it = data_P.d->pids.begin();
         it != data_P.d->pids.end();
         ++it )
        addPid( *it );
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
    
const QString& KStartupInfoData::findName() const
    {
    if( !name().isEmpty())
        return name();
    return bin();
    }

const QString& KStartupInfoData::name() const
    {
    return d->name;
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
    
void KStartupInfoData::setWMClass( const QCString& wmclass_P )
    {
    d->wmclass = wmclass_P;
    }

const QCString KStartupInfoData::findWMClass() const
    {
    if( !WMClass().isEmpty() && WMClass() != "0" )
        return WMClass();
    return bin().latin1(); // CHECKME
    }
    
const QCString& KStartupInfoData::WMClass() const
    {
    return d->wmclass;
    }
    
void KStartupInfoData::setHostname( const QCString& hostname_P )
    {
    if( !hostname_P.isNull())
        d->hostname = hostname_P;
    else
        {
        char tmp[ 256 ];
        tmp[ 0 ] = '\0';
        gethostname( tmp, 255 );
        d->hostname = tmp;
        }
    }

const QCString& KStartupInfoData::hostname() const
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
        
const QValueList< pid_t >& KStartupInfoData::pids() const
    {
    return d->pids;
    }

bool KStartupInfoData::is_pid( pid_t pid_P ) const
    {
    return d->pids.contains( pid_P );
    }

static
int get_num( const QString& item_P )
    {
    unsigned int pos = item_P.find( '=' );
    return item_P.mid( pos + 1 ).toInt();
    }

static
QString get_str( const QString& item_P )
    {
    unsigned int pos = item_P.find( '=' );
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
QCString get_cstr( const QString& item_P )
    {
    return get_str( item_P ).latin1(); // CHECKME
    }
    
static
QStringList get_fields( const QString& txt_P )
    {
    QString txt = txt_P.simplifyWhiteSpace();
    QStringList ret;
    QString item = "";
    bool in = false;
    bool escape = false;
    for( unsigned int pos = 0;
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
    for( unsigned int pos = 0;
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

#undef None

#include "kstartupinfo.moc"
