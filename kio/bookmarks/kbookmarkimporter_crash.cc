//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
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

typedef QMap<QString, QString> ViewMap;

// KDE 4.0: remove this BC keeping stub
void KCrashBookmarkImporter::parseCrashLog( QString /*filename*/, bool /*del*/ ) 
{
    ;
}

ViewMap KCrashBookmarkImporterImpl::parseCrashLog_noemit( const QString & filename, bool del ) 
{
    static const int g_lineLimit = 16*1024;

    QFile f( filename );
    ViewMap views;

    if ( !f.open( IO_ReadOnly ) )
        return views;

    QCString s( g_lineLimit );

    QTextCodec * codec = QTextCodec::codecForName( "UTF-8" );
    Q_ASSERT( codec );
    if ( !codec ) 
        return views;

    while ( f.readLine( s.data(), g_lineLimit ) >=0 ) 
    {
        if ( s[s.length()-1] != '\n' )
        {
            kdWarning() << "Crash bookmarks contain a line longer than " << g_lineLimit << ". Skipping." << endl;
            continue;
        }
        QString t = codec->toUnicode( s.stripWhiteSpace() );
        QRegExp rx( "(.*)\\((.*)\\):(.*)$" );
        rx.setMinimal( true );
        if ( !rx.exactMatch( t ) ) 
            continue;
        if ( rx.cap(1) == "opened" )
            views[rx.cap(2)] = rx.cap(3);
        else if ( rx.cap(1) == "close" )
            views.remove( rx.cap(2) );
    }

    f.close();

    if ( del ) 
        f.remove();

    return views;
}

QStringList KCrashBookmarkImporter::getCrashLogs() 
{
    return KCrashBookmarkImporterImpl::getCrashLogs();
}

QStringList KCrashBookmarkImporterImpl::getCrashLogs() 
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

        if ( replyType != "QString" ) 
            continue;

        QDataStream reply( replyData, IO_ReadOnly );
        QString ret;
        reply >> ret;
        activeLogs[ret] = true;
    }

    QDir d( KCrashBookmarkImporterImpl().findDefaultLocation() );
    d.setSorting( QDir::Time );
    d.setFilter( QDir::Files );
    d.setNameFilter( "konqueror-crash-*.log" );

    const QFileInfoList *list = d.entryInfoList();
    QFileInfoListIterator it( *list );

    QFileInfo *fi;
    QStringList crashFiles;

    int count = 0;
    for ( ; (( fi = it.current() ) != 0) && (count < 20); ++it, ++count ) 
    {
        bool stillAlive = activeLogs.contains( fi->absFilePath() );
        if ( !stillAlive )
            crashFiles << fi->absFilePath();
    }
    // Delete remaining ones
    for ( ; ( fi = it.current() ) != 0; ++it ) 
    {
        QFile::remove( fi->absFilePath() );
    }

    return crashFiles;
}

void KCrashBookmarkImporterImpl::parse() 
{
    QDict<bool> signatureMap;
    QStringList crashFiles = KCrashBookmarkImporterImpl::getCrashLogs();
    int count = 1;
    for ( QStringList::Iterator it = crashFiles.begin(); it != crashFiles.end(); ++it ) 
    {
        ViewMap views;
        views = parseCrashLog_noemit( *it, m_shouldDelete );
        QString signature;
        for ( ViewMap::Iterator vit = views.begin(); vit != views.end(); ++vit ) 
            signature += "|"+vit.data();
        if (signatureMap[signature])
        {
            // Duplicate... throw away and skip
            QFile::remove(*it);
            continue;
        }
            
        signatureMap.insert(signature, (bool *) true); // hack

        int outerFolder = ( crashFiles.count() > 1 ) && (views.count() > 0);
        if ( outerFolder )
            emit newFolder( QString("Konqueror Window %1").arg(count++), false, "" );
        for ( ViewMap::Iterator vit = views.begin(); vit != views.end(); ++vit ) 
            emit newBookmark( vit.data(), vit.data().latin1(), QString("") );
        if ( outerFolder )
            emit endFolder();
    }
}

QString KCrashBookmarkImporter::crashBookmarksDir() 
{
    static KCrashBookmarkImporterImpl *p = 0;
    if (!p)
        p = new KCrashBookmarkImporterImpl;
    return p->findDefaultLocation();
}

void KCrashBookmarkImporterImpl::setShouldDelete( bool shouldDelete ) 
{
    m_shouldDelete = shouldDelete;
}

void KCrashBookmarkImporter::parseCrashBookmarks( bool del ) 
{
    KCrashBookmarkImporterImpl importer;
    importer.setFilename( m_fileName );
    importer.setShouldDelete( del );
    importer.setupSignalForwards( &importer, this );
    importer.parse();
}

QString KCrashBookmarkImporterImpl::findDefaultLocation( bool ) const 
{
    return locateLocal( "tmp", "" );
}

#include "kbookmarkimporter_crash.moc"
