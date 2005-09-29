/*****************************************************************
Copyright (c) 2000 Matthias Ettrich <ettrich@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

// putenv() is not available on all platforms, so make sure the emulation
// wrapper is available in those cases by loading config.h!
#include <config.h>

#include <sys/types.h>
#include <pwd.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include <qbuffer.h>
#include <qcolor.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qimage.h>
#include <qmap.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qvariant.h>

#include "../dcopclient.h"
#include "../dcopref.h"
#include "../kdatastream.h"

#include "marshall.cpp"

#if defined Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#endif

typedef QMap<QString, QString> UserList;

static DCOPClient* dcop = 0;

static QTextStream cin_ ( stdin,  QIODevice::ReadOnly );
static QTextStream cout_( stdout, QIODevice::WriteOnly );
static QTextStream cerr_( stderr, QIODevice::WriteOnly );

/**
 * Session to send call to
 * DefaultSession - current session. Current KDE session when called without
 *                  --user or --all-users option. Otherwise this value ignores
 *                  all users with more than one active session.
 * AllSessions    - Send to all sessions found. requires --user or --all-users.
 * QuerySessions  - Don't call DCOP, return a list of available sessions.
 * CustomSession  - Use the specified session
 */
enum Session { DefaultSession = 0, AllSessions, QuerySessions, CustomSession };

bool startsWith(const DCOPCString &id, const char *str, int n)
{
  return !n || (strncmp(id.data(), str, n) == 0);
}

bool endsWith(QByteArray &id, char c)
{
   if (id.length() && (id[id.length()-1] == c))
   {
      id.truncate(id.length()-1);
      return true;
   }
   return false;
}

void queryApplications(const QByteArray &filter)
{
    int filterLen = filter.length();
    DCOPCStringList apps = dcop->registeredApplications();
    Q_FOREACH (DCOPCString clientId, apps)
    {
	if ( (clientId != dcop->appId()) &&
             !startsWith(clientId, "anonymous",9) &&
             startsWith(clientId, filter, filterLen)
           )
	    printf( "%s\n", clientId.data() );
    }

    if ( !dcop->isAttached() )
    {
	qWarning( "server not accessible" );
        exit(1);
    }
}

void queryObjects( const QByteArray &app, const QByteArray &filter )
{
    int filterLen = filter.length();
    bool ok = false;
    bool isDefault = false;
    DCOPCStringList objs = dcop->remoteObjects( app, &ok );
    Q_FOREACH (DCOPCString objId, objs)
    {
        if (objId == "default")
        {
           isDefault = true;
           continue;
        }

        if (startsWith(objId, filter, filterLen))
        {
            if (isDefault)
                printf( "%s (default)\n", objId.data() );
            else
                printf( "%s\n", objId.data() );
        }
        isDefault = false;
    }
    if ( !ok )
    {
        if (!dcop->isApplicationRegistered(app))
            qWarning( "No such application: '%s'", app.data());
        else
            qWarning( "Application '%s' not accessible", app.data() );
        exit(1);
    }
}

void queryFunctions( const char* app, const char* obj )
{
    bool ok = false;
    DCOPCStringList funcs = dcop->remoteFunctions( app, obj, &ok );
    for ( DCOPCStringList::Iterator it = funcs.begin(); it != funcs.end(); ++it ) {
	printf( "%s\n", (*it).data() );
    }
    if ( !ok )
    {
	qWarning( "object '%s' in application '%s' not accessible", obj, app );
	exit( 1 );
    }
}

int callFunction( const char* app, const char* obj, const char* func, const DCOPCStringList args )
{
    QString f = func; // Qt is better with unicode strings, so use one.
    int left = f.indexOf( '(' );
    int right = f.indexOf( ')' );

    if ( right <  left )
    {
	qWarning( "parentheses do not match" );
	return( 1 );
    }

    if ( left < 0 ) {
	// try to get the interface from the server
	bool ok = false;
	DCOPCStringList funcs = dcop->remoteFunctions( app, obj, &ok );
	DCOPCString     realfunc;
	if ( !ok && args.isEmpty() )
	    goto doit;
	if ( !ok )
	{
	    qWarning( "object not accessible" );
	    return( 1 );
	}
	for ( DCOPCStringList::Iterator it = funcs.begin(); it != funcs.end(); ++it ) {
	    int l = (*it).indexOf( '(' );
	    int s;
	    if (l > 0)
	        s = (*it).lastIndexOf( ' ', l);
	    else
	        s = (*it).indexOf( ' ' );

	    if ( s < 0 )
		s = 0;
	    else
		s++;

	    if ( l > 0 && (*it).mid( s, l - s ) == func ) {
		realfunc = (*it).mid( s );
		const QString arguments = (*it).mid(l+1,(*it).indexOf( ')' )-l-1);
		int a = arguments.count(',');
		if ( (a==0 && !arguments.isEmpty()) || a>0)
			a++;
		if ( a == args.count()  )
		    break;
	    }
	}
	if ( realfunc.isEmpty() )
	{
	    qWarning("no such function");
	    return( 1 );
	}
	f = realfunc;
	left = f.indexOf( '(' );
	right = f.indexOf( ')' );
    }

 doit:
    if ( left < 0 )
	f += "()";

    // This may seem expensive but is done only once per invocation
    // of dcop, so it should be OK.
    //
    //
    QStringList intTypes;
    intTypes << "int" << "unsigned" << "long" << "bool" ;

    QStringList types;
    if ( left >0 && left + 1 < right - 1) {
	types = f.mid( left + 1, right - left - 1).split( ',', QString::SkipEmptyParts );
	for ( QStringList::Iterator it = types.begin(); it != types.end(); ++it ) {
	    QString lt = (*it).simplified();

	    int s = lt.indexOf(' ');

	    // If there are spaces in the name, there may be two
	    // reasons: the parameter name is still there, ie.
	    // "QString URL" or it's a complicated int type, ie.
	    // "unsigned long long int bool".
	    //
	    //
	    if ( s > 0 )
	    {
		QStringList partl = lt.split(' ' , QString::SkipEmptyParts);

		// The zero'th part is -- at the very least -- a
		// type part. Any trailing parts *might* be extra
		// int-type keywords, or at most one may be the
		// parameter name.
		//
		//
		s=1;

		while (s < static_cast<int>(partl.count()) && intTypes.contains(partl[s]))
		{
			s++;
		}

		if ( s < static_cast<int>(partl.count())-1)
		{
			qWarning("The argument `%s' seems syntactically wrong.",
				lt.toLatin1().constData());
		}
		if ( s == static_cast<int>(partl.count())-1)
		{
			partl.removeAll(partl.at(s));
		}

		lt = partl.join(" ");
		lt = lt.simplified();
	    }

	    (*it) = lt;
	}
	QString fc = f.left( left );
	fc += '(';
	bool first = true;
	for ( QStringList::Iterator it = types.begin(); it != types.end(); ++it ) {
	    if ( !first )
		fc +=",";
	    first = false;
	    fc += *it;
	}
	fc += ')';
	f = fc;
    }

    QByteArray data, replyData;
    DCOPCString replyType;
    QDataStream arg(&data, QIODevice::WriteOnly);
    arg.setVersion(QDataStream::Qt_3_1);

    int i = 0;
    for( QStringList::Iterator it = types.begin(); it != types.end(); ++it )
        marshall( arg, args, i, *it );

    if ( i != args.count() )
    {
	qWarning( "arguments do not match" );
	return( 1 );
    }

    if ( !dcop->call( app, obj, f.toLatin1().data(),  data, replyType, replyData) ) {
	qWarning( "call failed");
	return( 1 );
    } else {
	QDataStream reply(&replyData, QIODevice::ReadOnly);
	reply.setVersion(QDataStream::Qt_3_1);

        if ( replyType != "void" && replyType != "ASYNC" )
        {
            QByteArray replyString = demarshal( reply, replyType );
            if ( !replyString.isEmpty() )
                printf( "%s\n", replyString.data() );
            else
                printf("\n");
        }
    }
    return 0;
}

/**
 * Show command-line help and exit
 */
void showHelp( int exitCode = 0 )
{
#ifdef DCOPQUIT
   cout_ << "Usage: dcopquit [options] [application]" << endl
#else
   cout_ << "Usage: dcop [options] [application [object [function [arg1] [arg2] ... ] ] ]" << endl
#endif
         << "" << endl
	 << "Console DCOP client" << endl
	 << "" << endl
	 << "Generic options:" << endl
	 << "  --help          Show help about options" << endl
	 << "" << endl
	 << "Options:" << endl
	 << "  --pipe          Call DCOP for each line read from stdin. The string '%1'" << endl
	 << "                  will be used in the argument list as a placeholder for" << endl
	 << "                  the substituted line." << endl
	 << "                  For example," << endl
	 << "                      dcop --pipe konqueror html-widget1 evalJS %1" << endl
	 << "                  is equivalent to calling" << endl
	 << "                      while read line ; do" << endl
	 << "                          dcop konqueror html-widget1 evalJS \"$line\"" << endl
	 << "                      done" << endl
	 << "                  in bash, but because no new dcop instance has to be started" << endl
	 << "                  for each line this is generally much faster, especially for" << endl
	 << "                  the slower GNU dynamic linkers." << endl
	 << "                  The '%1' placeholder cannot be used to replace e.g. the" << endl
	 << "                  program, object or method name." << endl
	 << "  --user <user>   Connect to the given user's DCOP server. This option will" << endl
	 << "                  ignore the values of the environment vars $DCOPSERVER and" << endl
	 << "                  $ICEAUTHORITY, even if they are set." << endl
	 << "                  If the user has more than one open session, you must also" << endl
	 << "                  use one of the --list-sessions, --session or --all-sessions" << endl
	 << "                  command-line options." << endl
	 << "  --all-users     Send the same DCOP call to all users with a running DCOP" << endl
	 << "                  server. Only failed calls to existing DCOP servers will" << endl
	 << "                  generate an error message. If no DCOP server is available" << endl
	 << "                  at all, no error will be generated." << endl
	 << "  --session <ses> Send to the given KDE session. This option can only be" << endl
	 << "                  used in combination with the --user option." << endl
	 << "  --all-sessions  Send to all sessions found. Only works with the --user" << endl
	 << "                  and --all-users options." << endl
	 << "  --list-sessions List all active KDE session for a user or all users." << endl
	 << "  --no-user-time  Don't update the user activity timestamp in the called" << endl
	 << "                  application (for usage in scripts running" << endl
	 << "                  in the background)." << endl
	 << endl;

    exit( exitCode );
}

/**
 * Return a list of all users and their home directories.
 * Returns an empty list if /etc/passwd cannot be read for some reason.
 */
static UserList userList()
{
    UserList result;

    while( passwd* pstruct = getpwent() )
    {
        result[ QString::fromLocal8Bit(pstruct->pw_name) ] = QFile::decodeName(pstruct->pw_dir);
    }

    return result;
}

/**
 * Return a list of available DCOP sessions for the specified user
 * An empty list means no sessions are available, or an error occurred.
 */
QStringList dcopSessionList( const QString &user, const QString &home )
{
    if( home.isEmpty() )
    {
	cerr_ << "WARNING: Cannot determine home directory for user "
	     << user << "!" << endl
	     << "Please check permissions or set the $DCOPSERVER variable manually before" << endl
	     << "calling dcop." << endl;
	return QStringList();
    }

    QStringList result;
    QFileInfo dirInfo( home );
    if( !dirInfo.exists() || !dirInfo.isReadable() )
	return result;

    QDir d( home );
    d.setFilter( QDir::Files | QDir::Hidden | QDir::NoSymLinks );
    d.setNameFilters( QStringList(".DCOP4server*") );

    const QList<QFileInfo> list = d.entryInfoList();
    if( !list.count() )
	return result;

    Q_FOREACH ( QFileInfo fi, list ) {
	if ( fi.isReadable() )
	    result.append( fi.fileName() );
    }
    return result;
}

void sendUserTime( const char* app )
{
#if defined Q_WS_X11
    static quint32 time = 0;
    if( time == 0 )
    {
        Display* dpy = XOpenDisplay( NULL );
        if( dpy != NULL )
        {
            Window w = XCreateSimpleWindow( dpy, DefaultRootWindow( dpy ), 0, 0, 1, 1, 0, 0, 0 );
            XSelectInput( dpy, w, PropertyChangeMask );
            unsigned char data[ 1 ];
            XChangeProperty( dpy, w, XA_ATOM, XA_ATOM, 8, PropModeAppend, data, 1 );
            XEvent ev;
            XWindowEvent( dpy, w, PropertyChangeMask, &ev );
            time = ev.xproperty.time;
            XDestroyWindow( dpy, w );
        }
    }
    DCOPRef( app, "MainApplication-Interface" ).call( "updateUserTimestamp", time );
#else
// ...
#endif
}

/**
 * Do the actual DCOP call
 */
int runDCOP( DCOPCStringList args, UserList users, Session session,
              const QString sessionName, bool readStdin, bool updateUserTime )
{
    bool DCOPrefmode=false;
    DCOPCString app;
    DCOPCString objid;
    DCOPCString function;
    DCOPCStringList params;
    DCOPClient *client = 0L;
    int retval = 0;
    if ( !args.isEmpty() && args[ 0 ].indexOf( "DCOPRef(" ) == 0 )
    {
	int delimPos = args[ 0 ].lastIndexOf( ',' );
	if( delimPos == -1 )
        {
	    cerr_ << "Error: '" << args[ 0 ]
		 << "' is not a valid DCOP reference." << endl;
	    exit( -1 );
        }
        app = args[ 0 ].mid( 8, delimPos-8 );
        delimPos++;
        objid = args[ 0 ].mid( delimPos, args[ 0 ].length()-delimPos-1 );
        if( args.count() > 1 )
	    function = args[ 1 ];
	if( args.count() > 2 )
	{
	    params = args;
	    params.erase( params.begin() );
	    params.erase( params.begin() );
	}
	DCOPrefmode=true;
    }
    else
    {
        if( !args.isEmpty() )
	    app = args[ 0 ];
        if( args.count() > 1 )
	    objid = args[ 1 ];
        if( args.count() > 2 )
	    function = args[ 2 ];
        if( args.count() > 3)
	{
	    params = args;
	    params.erase( params.begin() );
	    params.erase( params.begin() );
	    params.erase( params.begin() );
	}
    }

    bool firstRun = true;
    UserList::Iterator it;
    QStringList sessions;
    bool presetDCOPServer = false;
//    char *dcopStr = 0L;
    QString dcopServer;

    for( it = users.begin(); it != users.end() || firstRun; ++it )
    {
	firstRun = false;

	//cout_ << "Iterating '" << it.key() << "'" << endl;

	if( session == QuerySessions )
	{
	    QStringList sessions = dcopSessionList( it.key(), it.value() );
	    if( sessions.isEmpty() )
	    {
		if( users.count() <= 1 )
		{
		    cout_ << "No active sessions";
		    if( !( *it ).isEmpty() )
			cout_ << " for user " << *it;
		    cout_ << endl;
		}
	    }
	    else
	    {
		cout_ << "Active sessions ";
		if( !( *it ).isEmpty() )
		    cout_ << "for user " << *it << " ";
		cout_ << ":" << endl;

		QStringList::Iterator sIt = sessions.begin();
		for( ; sIt != sessions.end(); ++sIt )
		    cout_ << "  " << *sIt << endl;

		cout_ << endl;
	    }
	    continue;
	}

	if( getenv( "DCOPSERVER" ) )
	{
	    sessions.append( getenv( "DCOPSERVER" ) );
	    presetDCOPServer = true;
	}

	if( users.count() > 1 || ( users.count() == 1 &&
	    ( getenv( "DCOPSERVER" ) == 0 /*&& getenv( "DISPLAY" ) == 0*/ ) ) )
	{
	    sessions = dcopSessionList( it.key(), it.value() );
	    if( sessions.isEmpty() )
	    {
		if( users.count() > 1 )
		    continue;
		else
		{
		    cerr_ << "ERROR: No active KDE sessions!" << endl
			 << "If you are sure there is one, please set the $DCOPSERVER variable manually" << endl
			 << "before calling dcop." << endl;
		    exit( -1 );
		}
	    }
	    else if( !sessionName.isEmpty() )
	    {
		if( sessions.contains( sessionName ) )
		{
		    sessions.clear();
		    sessions.append( sessionName );
		}
		else
		{
		    cerr_ << "ERROR: The specified session doesn't exist!" << endl;
		    exit( -1 );
		}
	    }
	    else if( sessions.count() > 1 && session != AllSessions )
	    {
		cerr_ << "ERROR: Multiple available KDE sessions!" << endl
		     << "Please specify the correct session to use with --session or use the" << endl
		     << "--all-sessions option to broadcast to all sessions." << endl;
		exit( -1 );
	    }
	}

	if( users.count() > 1 || ( users.count() == 1 &&
	    ( getenv( "ICEAUTHORITY" ) == 0 || getenv( "DISPLAY" ) == 0 ) ) )
	{
	    // Check for ICE authority file and if the file can be read by us
	    QString home = it.value();
	    QString iceFile = it.value() + "/.ICEauthority";
	    QFileInfo fi( iceFile );
	    if( iceFile.isEmpty() )
	    {
		cerr_ << "WARNING: Cannot determine home directory for user "
		     << it.key() << "!" << endl
		     << "Please check permissions or set the $ICEAUTHORITY variable manually before" << endl
		     << "calling dcop." << endl;
	    }
	    else if( fi.exists() )
	    {
		if( fi.isReadable() )
		{
		    char *envStr = strdup( ( "ICEAUTHORITY=" + iceFile ).toAscii().constData() );
		    putenv( envStr );
		    //cerr_ << "ice: " << envStr << endl;
		}
		else
		{
		    cerr_ << "WARNING: ICE authority file " << iceFile
			 << "is not readable by you!" << endl
			 << "Please check permissions or set the $ICEAUTHORITY variable manually before" << endl
			 << "calling dcop." << endl;
		}
	    }
	    else
	    {
		if( users.count() > 1 )
		    continue;
		else
		{
		    cerr_ << "WARNING: Cannot find ICE authority file "
		         << iceFile << "!" << endl
			 << "Please check permissions or set the $ICEAUTHORITY"
			 << " variable manually before" << endl
			 << "calling dcop." << endl;
		}
	    }
	}

	// Main loop
	// If users is an empty list we're calling for the currently logged
	// in user. In this case we don't have a session, but still want
	// to iterate the loop once.
	QStringList::Iterator sIt = sessions.begin();
	for( ; sIt != sessions.end() || users.isEmpty(); ++sIt )
	{
	    if( !presetDCOPServer && !users.isEmpty() )
	    {
		QString dcopFile = it.value() + "/" + *sIt;
		QFile f( dcopFile );
		if( !f.open( QIODevice::ReadOnly ) )
		{
		    cerr_ << "Can't open " << dcopFile << " for reading!" << endl;
		    exit( -1 );
		}

		QList<QByteArray> l( f.readAll().split( '\n' ) );
		dcopServer = QString::fromLatin1( l.first() );

		if( dcopServer.isEmpty() )
		{
		    cerr_ << "WARNING: Unable to determine DCOP server for session "
			 << *sIt << "!" << endl
			 << "Please check permissions or set the $DCOPSERVER variable manually before" << endl
			 << "calling dcop." << endl;
		    exit( -1 );
		}
	    }

	    delete client;
	    client = new DCOPClient;
	    if( !dcopServer.isEmpty() )
		client->setServerAddress( dcopServer.toAscii().constData() );
	    bool success = client->attach();
	    if( !success )
	    {
		cerr_ << "ERROR: Couldn't attach to DCOP server!" << endl;
		retval = qMax( retval, 1 );
		if( users.isEmpty() )
		    break;
		else
		    continue;
	    }
	    dcop = client;

	    int argscount = args.count();
	    if ( DCOPrefmode )
	      argscount++;
	    switch ( argscount )
	    {
	    case 0:
		queryApplications("");
		break;
	    case 1:
		if (endsWith(app, '*'))
		   queryApplications(app);
		else
		   queryObjects( app, "" );
		break;
	    case 2:
		if (endsWith(objid, '*'))
		   queryObjects(app, objid);
		else
		   queryFunctions( app, objid );
		break;
	    case 3:
	    default:
                if( updateUserTime )
                    sendUserTime( app );
		if( readStdin )
		{
		    DCOPCStringList::Iterator replaceArg = params.end();

		    DCOPCStringList::Iterator it = params.begin();
		    for( ; it != params.end(); ++it )
			if( *it == "%1" )
			    replaceArg = it;

		    // Read from stdin until EOF and call function for each
		    // read line
		    while ( !cin_.atEnd() )
		    {
			QString buf = cin_.readLine();

			if( replaceArg != params.end() )
			    *replaceArg = buf.toLocal8Bit();

			if( !buf.isNull() )
			{
			    int res = callFunction( app, objid, function, params );
			    retval = qMax( retval, res );
			}
		    }
		}
		else
		{
		    // Just call function
//		    cout_ << "call " << app << ", " << objid << ", " << function << ", (params)" << endl;
		    int res = callFunction( app, objid, function, params );
		    retval = qMax( retval, res );
		}
		break;
	    }
	    // Another sIt++ would make the loop infinite...
	    if( users.isEmpty() )
		break;
	}

	// Another it++ would make the loop infinite...
	if( it == users.end() )
	    break;
    }

    return retval;
}

#ifdef Q_OS_WIN
# define main kdemain
#endif

int main( int argc, char** argv )
{
    bool readStdin = false;
    int numOptions = 0;
    QString user;
    Session session = DefaultSession;
    QString sessionName;
    bool updateUserTime = true;

    cin_.setCodec( QTextCodec::codecForLocale() );

    // Scan for command-line options first
    for( int pos = 1 ; pos <= argc - 1 ; pos++ )
    {
	if( strcmp( argv[ pos ], "--help" ) == 0 )
	    showHelp( 0 );
	else if( strcmp( argv[ pos ], "--pipe" ) == 0 )
	{
	    readStdin = true;
	    numOptions++;
	}
	else if( strcmp( argv[ pos ], "--user" ) == 0 )
	{
	    if( pos <= argc - 2 )
	    {
		user = QString::fromLocal8Bit( argv[ pos + 1] );
		numOptions +=2;
		pos++;
	    }
	    else
	    {
		cerr_ << "Missing username for '--user' option!" << endl << endl;
		showHelp( -1 );
	    }
	}
	else if( strcmp( argv[ pos ], "--session" ) == 0 )
	{
	    if( session == AllSessions )
	    {
		cerr_ << "ERROR: --session cannot be mixed with --all-sessions!" << endl << endl;
		showHelp( -1 );
	    }
	    else if( pos <= argc - 2 )
	    {
		sessionName = QString::fromLocal8Bit( argv[ pos + 1] );
		numOptions +=2;
		pos++;
	    }
	    else
	    {
		cerr_ << "Missing session name for '--session' option!" << endl << endl;
		showHelp( -1 );
	    }
	}
	else if( strcmp( argv[ pos ], "--all-users" ) == 0 )
	{
	    user = "*";
	    numOptions ++;
	}
	else if( strcmp( argv[ pos ], "--list-sessions" ) == 0 )
	{
	    session = QuerySessions;
	    numOptions ++;
	}
	else if( strcmp( argv[ pos ], "--all-sessions" ) == 0 )
	{
	    if( !sessionName.isEmpty() )
	    {
		cerr_ << "ERROR: --session cannot be mixed with --all-sessions!" << endl << endl;
		showHelp( -1 );
	    }
	    session = AllSessions;
	    numOptions ++;
	}
        else if( strcmp( argv[ pos ], "--no-user-time" ) == 0 )
        {
            updateUserTime = false;
            numOptions ++;
        }
	else if( argv[ pos ][ 0 ] == '-' )
	{
	    cerr_ << "Unknown command-line option '" << argv[ pos ]
		 << "'." << endl << endl;
	    showHelp( -1 );
	}
	else
	    break;	// End of options
    }

    argc -= numOptions;

    DCOPCStringList args;

#ifdef DCOPQUIT
    if (argc > 1)
    {
       QByteArray prog = argv[ numOptions + 1 ];

       if (!prog.isEmpty())
       {
          args.append( prog );

          // Pass as-is if it ends with a wildcard
          if (prog[prog.length()-1] != '*')
          {
             // Strip a trailing -<PID> part.
             int i = prog.lastIndexOf('-');
             if (i >= 0)
             {
                prog = prog.left(i);
             }
             args.append( "qt/"+prog );
             args.append( "quit()" );
          }
       }
    }
#else
    for( int i = numOptions; i < argc + numOptions - 1; i++ )
       args.append( argv[ i + 1 ] );
#endif

    if( readStdin && args.count() < 3 )
    {
	cerr_ << "--pipe option only supported for function calls!" << endl << endl;
	showHelp( -1 );
    }

    if( user == "*" && args.count() < 3 && session != QuerySessions )
    {
	cerr_ << "ERROR: The --all-users option is only supported for function calls!" << endl << endl;
	showHelp( -1 );
    }

    if( session == QuerySessions && !args.isEmpty() )
    {
	cerr_ << "ERROR: The --list-sessions option cannot be used for actual DCOP calls!" << endl << endl;
	showHelp( -1 );
    }

    if( session == QuerySessions && user.isEmpty() )
    {
	cerr_ << "ERROR: The --list-sessions option can only be used with the --user or" << endl
	     << "--all-users options!" << endl << endl;
	showHelp( -1 );
    }

    if( session != DefaultSession && session != QuerySessions &&
        args.count() < 3 )
    {
	cerr_ << "ERROR: The --session and --all-sessions options are only supported for function" << endl
	     << "calls!" << endl << endl;
	showHelp( -1 );
    }

    UserList users;
    if( user == "*" )
	users = userList();
    else if( !user.isEmpty() )
	users[ user ] = userList()[ user ];

    int retval = runDCOP( args, users, session, sessionName, readStdin, updateUserTime );

    return retval;
}

// vim: set ts=8 sts=4 sw=4 noet:

