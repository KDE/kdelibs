/* This file is part of the KDE libraries
   Copyright (C) 2002-2003 Alexander Kellett <lypanov@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kbookmarkimporter_crash.h"

#include <kfiledialog.h>
#include <kstringhandler.h>
#include <klocale.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <qfile.h>
#include <qdir.h>
#include <qstring.h>
#include <qtextcodec.h>
#include <dcopclient.h>

#include <sys/types.h>
#include <stddef.h>
#include <dirent.h>
#include <sys/stat.h>

#define LINELIMIT 4096

typedef QMap<QString, QString> ViewMap;

// KDE 4.0: remove this BC keeping stub
void KCrashBookmarkImporter::parseCrashLog( QString /*filename*/, bool /*del*/ ) 
{
    ;
}

/* antlarr: KDE 4: Make it const QString & */
ViewMap KCrashBookmarkImporter::parseCrashLog_noemit( QString filename, bool del ) 
{
    QFile f( filename );
    ViewMap views;

    if ( !f.open( IO_ReadOnly ) )
        return views;

    QCString s( 4096 );

    QTextCodec * codec = QTextCodec::codecForName( "UTF-8" );
    Q_ASSERT( codec );
    if ( !codec ) 
        return views;

    while ( f.readLine( s.data(), LINELIMIT ) >=0 ) 
    {
        if ( s[s.length()-1] != '\n' )
        {
            kdWarning() << "Crash bookmarks contain a line longer than " << LINELIMIT << ". Skipping." << endl;
            continue;
        }
        QString t = codec->toUnicode( s.stripWhiteSpace() );
        QRegExp rx( "(.*)\\((.*)\\):(.*)$" );
        rx.setMinimal( TRUE );
        if ( rx.exactMatch( t ) ) 
        {
            if ( rx.cap(1) == "opened" )
                views[rx.cap(2)] = rx.cap(3);
            else if ( rx.cap(1) == "close" )
                views.remove( rx.cap(2) );
        }
    }

    f.close();

    if ( del ) 
        f.remove();

    return views;
}

QStringList KCrashBookmarkImporter::getCrashLogs() 
{
    QMap<QString, bool> activeLogs;

    DCOPClient* dcop = kapp->dcopClient();

    QCStringList apps = dcop->registeredApplications();
    for ( QCStringList::Iterator it = apps.begin(); it != apps.end(); ++it ) 
    {
        QCString &clientId = *it;

        if ( qstrncmp(clientId, "konqueror", 9) != 0 ) 
            continue;

        QByteArray data, replyData;
        QCString replyType;
        QDataStream arg( data, IO_WriteOnly );

        if ( !dcop->call( clientId.data(), "KonquerorIface", 
                          "crashLogFile()", data, replyType, replyData) ) 
        {
            kdWarning() << "can't find dcop function KonquerorIface::crashLogFile()" << endl;
            continue;
        }

        QDataStream reply( replyData, IO_ReadOnly );
        if ( replyType == "QString" ) 
        {
            QString ret;
            reply >> ret;
            activeLogs[ret] = true;
        }
    }

    QDir d( crashBookmarksDir() );
    d.setSorting( QDir::Time );
    d.setFilter( QDir::Files );
    d.setNameFilter( "konqueror-crash-*.log" );

    const QFileInfoList *list = d.entryInfoList();
    QFileInfoListIterator it( *list );

    QFileInfo *fi;
    QStringList crashFiles;

    int count = 0;
    for ( ; ( fi = it.current() ) != 0; ++it ) 
    {
        bool dead = !activeLogs.contains( fi->absFilePath() );
        if ( dead )
            crashFiles << fi->absFilePath();
        if ( count++ > 20 )
            break;
    }

    return crashFiles;
}

void KCrashBookmarkImporter::parseCrashBookmarks( bool del ) 
{
    QStringList crashFiles = KCrashBookmarkImporter::getCrashLogs();
    int count = 1;
    for ( QStringList::Iterator it = crashFiles.begin(); it != crashFiles.end(); ++it ) 
    {
        ViewMap views;
        views = parseCrashLog_noemit( *it, del );
        int outerFolder = ( crashFiles.count() > 1 ) && (views.count() > 0);
        if ( outerFolder )
            emit newFolder( QString("Konqueror Window %1").arg(count++), false, "" );
        for ( ViewMap::Iterator it = views.begin(); it != views.end(); ++it ) 
            emit newBookmark( it.data(), it.data().latin1(), QString("") );
        if ( outerFolder )
            emit endFolder();
    }
}

QString KCrashBookmarkImporter::crashBookmarksDir() 
{
    static KCrashBookmarkImporterImpl importer;
    return importer.findDefaultLocation();
}

void KCrashBookmarkImporterImpl::setShouldDelete( bool shouldDelete ) 
{
    m_shouldDelete = shouldDelete;
}

void KCrashBookmarkImporterImpl::parse() 
{
    KCrashBookmarkImporter importer( QString::null );
    setupSignalForwards( &importer, this );
    importer.parseCrashBookmarks( m_shouldDelete );
}

QString KCrashBookmarkImporterImpl::findDefaultLocation( bool ) const 
{
    return locateLocal( "tmp", "" );
}

#include "kbookmarkimporter_crash.moc"
