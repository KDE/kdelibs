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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include <qcolor.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qmap.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qvariant.h>

// putenv() is not available on all platforms, so make sure the emulation
// wrapper is available in those cases by loading config.h!
#include <config.h>

#include "../dcopclient.h"
#include "../dcopref.h"
#include "../kdatastream.h"

#include "marshall.cpp"

typedef QMap<QString, QString> UserList;

static DCOPClient* dcop = 0;

static QTextStream cin_ ( stdin,  IO_ReadOnly );
static QTextStream cout_( stdout, IO_WriteOnly );
static QTextStream cerr_( stderr, IO_WriteOnly );

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

bool startsWith(const QCString &id, const char *str, int n)
{
  return !n || (strncmp(id.data(), str, n) == 0);
}

bool endsWith(QCString &id, char c)
{
   if (id.length() && (id[id.length()-1] == c))
   {
      id.truncate(id.length()-1);
      return true;
   }
   return false;
}

void queryApplications(const QCString &filter)
{
    int filterLen = filter.length();
    QCStringList apps = dcop->registeredApplications();
    for ( QCStringList::Iterator it = apps.begin(); it != apps.end(); ++it )
    {
        QCString &clientId = *it;
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

void queryObjects( const QCString &app, const QCString &filter )
{
    int filterLen = filter.length();
    bool ok = false;
    bool isDefault = false;
    QCStringList objs = dcop->remoteObjects( app, &ok );
    for ( QCStringList::Iterator it = objs.begin(); it != objs.end(); ++it )
    {
        QCString &objId = *it;

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
    QCStringList funcs = dcop->remoteFunctions( app, obj, &ok );
    for ( QCStringList::Iterator it = funcs.begin(); it != funcs.end(); ++it ) {
	printf( "%s\n", (*it).data() );
    }
    if ( !ok )
    {
	qWarning( "object '%s' in application '%s' not accessible", obj, app );
        exit(1);
    }
}

void callFunction( const char* app, const char* obj, const char* func, const QCStringList args )
{
    QString f = func; // Qt is better with unicode strings, so use one.
    int left = f.find( '(' );
    int right = f.find( ')' );

    if ( right <  left )
    {
	qWarning( "parentheses do not match" );
        exit(1);
    }

    if ( left < 0 ) {
	// try to get the interface from the server
	bool ok = false;
	QCStringList funcs = dcop->remoteFunctions( app, obj, &ok );
	QCString realfunc;
	if ( !ok && args.isEmpty() )
	    goto doit;
	if ( !ok )
        {
	    qWarning( "object not accessible" );
            exit(1);
        }
	for ( QCStringList::Iterator it = funcs.begin(); it != funcs.end(); ++it ) {
	    int l = (*it).find( '(' );
	    int s = (*it).find( ' ');
	    if ( s < 0 )
		s = 0;
	    else
		s++;

	    if ( l > 0 && (*it).mid( s, l - s ) == func ) {
		realfunc = (*it).mid( s );
		uint a = (*it).contains(',');
		if ( ( a == 0 && args.isEmpty() ) || ( a > 0 && a + 1 == args.count() ) )
		    break;
	    }
	}
	if ( realfunc.isEmpty() )
	{
	    qWarning("no such function");
//            exit(1);
	    return;
	}
	f = realfunc;
	left = f.find( '(' );
	right = f.find( ')' );
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
	types = QStringList::split( ',', f.mid( left + 1, right - left - 1) );
	for ( QStringList::Iterator it = types.begin(); it != types.end(); ++it ) {
	    QString lt = (*it).simplifyWhiteSpace();

	    int s = lt.find(' ');

	    // If there are spaces in the name, there may be two
	    // reasons: the parameter name is still there, ie.
	    // "QString URL" or it's a complicated int type, ie.
	    // "unsigned long long int bool".
	    //
	    //
	    if ( s > 0 )
	    {
		QStringList partl = QStringList::split(' ' , lt);

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
				lt.latin1());
		}
		if ( s == static_cast<int>(partl.count())-1)
		{
			partl.remove(partl.at(s));
		}

		lt = partl.join(" ");
		lt = lt.simplifyWhiteSpace();
	    }

	    (*it) = lt;
	}
	QString fc = f.left( left );
	fc += '(';
	bool first = TRUE;
	for ( QStringList::Iterator it = types.begin(); it != types.end(); ++it ) {
	    if ( !first )
		fc +=",";
	    first = FALSE;
	    fc += *it;
	}
	fc += ')';
	f = fc;
    }

    QByteArray data, replyData;
    QCString replyType;
    QDataStream arg(data, IO_WriteOnly);

    uint i = 0;
    for( QStringList::Iterator it = types.begin(); it != types.end(); ++it )
        marshall( arg, args, i, *it );

    if ( i != args.count() )
    {
	qWarning( "arguments do not match" );
	exit(1);
    }

    if ( !dcop->call( app, obj, f.latin1(),  data, replyType, replyData) ) {
	qWarning( "call failed");
        exit(1);
    } else {
	QDataStream reply(replyData, IO_ReadOnly);

        if ( replyType != "void" && replyType != "ASYNC" )
        {
            QCString replyString = demarshal( reply, replyType );
            printf( "%s\n", replyString.data() );
        }
    }
}

/**
 * Show command-line help and exit
 */
void showHelp( int exitCode = 0 )
{
    cout_ << "Usage: dcop [options] [application [object [function [arg1] [arg2] ... ] ] ]" << endl
	 << "" << endl
	 << "Console DCOP client" << endl
	 << "" << endl
	 << "Generic options:" << endl
	 << "  --help          Show help about options" << endl
	 << "" << endl
	 << "Options:" << endl
	 << "  --pipe          Call DCOP for each line read from stdin" << endl
	 << "                  This is roughly equivalent to calling" << endl
	 << "                      'while read line ; do dcop $line ; done'" << endl
	 << "                  but because no new dcop instance has to be started for" << endl
	 << "                  each line this is generally much faster, especially" << endl
	 << "                  for the slower GNU dynamic linkers." << endl
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

    QFile f( "/etc/passwd" );

    if( !f.open( IO_ReadOnly ) )
    {
	cerr_ << "Can't open /etc/passwd for reading!" << endl;
	return result;
    }

    QStringList l( QStringList::split( '\n', f.readAll() ) );

    for( QStringList::ConstIterator it( l.begin() ); it != l.end(); ++it )
    {
	QStringList userInfo( QStringList::split( ':', *it, true ) );
	result[ userInfo[ 0 ] ] = userInfo[ 5 ];
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
    d.setNameFilter( ".DCOPserver*" );

    const QFileInfoList *list = d.entryInfoList();
    if( !list )
	return result;

    QFileInfoListIterator it( *list );
    QFileInfo *fi;

    while ( ( fi = it.current() ) != 0 )
    {
	if( fi->isReadable() )
	    result.append( fi->fileName() );
	++it;
    }
    return result;
}

/**
 * Do the actual DCOP call
 */
void runDCOP( QCStringList args, UserList users, Session session,
              const QString sessionName, bool readStdin )
{
    bool DCOPrefmode=false;
    QCString app;
    QCString objid;
    QCString function;
    QCStringList params;
    DCOPClient *client = 0L;
    if ( !args.isEmpty() && args[ 0 ].find( "DCOPRef(" ) == 0 )
    {
	int delimPos = args[ 0 ].findRev( ',' );
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
	    params.remove( params.begin() );
	    params.remove( params.begin() );
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
	    params.remove( params.begin() );
	    params.remove( params.begin() );
	    params.remove( params.begin() );
	}
    }

    bool firstRun = true;
    UserList::Iterator it;
    QStringList sessions;
    bool presetDCOPServer = false;
//    char *dcopStr = 0L;
    QString dcopServer;

    for( it = users.begin(); it != users.end() || firstRun; it++ )
    {
	firstRun = false;

	//cout_ << "Iterating '" << it.key() << "'" << endl;

	if( session == QuerySessions )
	{
	    QStringList sessions = dcopSessionList( it.key(), it.data() );
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

		QStringList::Iterator sIt;
		for( sIt = sessions.begin(); sIt != sessions.end(); sIt++ )
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
	    sessions = dcopSessionList( it.key(), it.data() );
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
	    QString home = it.data();
	    QString iceFile = it.data() + "/.ICEauthority";
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
		    char *envStr = strdup( ( "ICEAUTHORITY=" + iceFile ).ascii() );
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
	for( ; sIt != sessions.end() || users.isEmpty(); sIt++ )
	{
	    if( !presetDCOPServer && !users.isEmpty() )
	    {
		QString dcopFile = it.data() + "/" + *sIt;
		QFile f( dcopFile );
		if( !f.open( IO_ReadOnly ) )
		{
		    cerr_ << "Can't open " << dcopFile << " for reading!" << endl;
		    exit( -1 );
		}

		QStringList l( QStringList::split( '\n', f.readAll() ) );
		dcopServer = l.first();

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
		client->setServerAddress( dcopServer.ascii() );
	    bool success = client->attach();
	    if( !success )
	    {
		cerr_ << "ERROR: Couldn't attach to DCOP server!" << endl;
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
		if( readStdin )
		{
		    QCStringList::Iterator replaceArg = params.end();

		    QCStringList::Iterator it;
		    for( it = params.begin(); it != params.end(); it++ )
			if( *it == "%1" )
			    replaceArg = it;

		    // Read from stdin until EOF and call function for each
		    // read line
		    while ( !cin_.atEnd() )
		    {
			QString buf = cin_.readLine();

			if( replaceArg != params.end() )
			    *replaceArg = buf.local8Bit();

			if( !buf.isNull() )
			    callFunction( app, objid, function, params );
		    }
		}
		else
		{
		    // Just call function
//		    cout_ << "call " << app << ", " << objid << ", " << function << ", (params)" << endl;
		    callFunction( app, objid, function, params );
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
}


int main( int argc, char** argv )
{
    bool readStdin = false;
    int numOptions = 0;
    QString user;
    Session session = DefaultSession;
    QString sessionName;

    cin_.setEncoding( QTextStream::Locale );

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

    QCStringList args;
    for( int i = numOptions; i < argc + numOptions - 1; i++ )
	args.append( argv[ i + 1 ] );

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

    runDCOP( args, users, session, sessionName, readStdin );

    return 0;
}

// vim: set ts=8 sts=4 sw=4 noet:

