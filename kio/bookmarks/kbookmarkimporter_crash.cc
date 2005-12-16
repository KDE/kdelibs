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
#include <dcopclient.h>
#include <qfile.h>
#include <qdir.h>
#include <qstring.h>
#include <qtextcodec.h>
#include <qset.h>

#include <sys/types.h>
#include <stddef.h>
#include <dirent.h>
#include <sys/stat.h>

typedef QMap<QString, QString> ViewMap;


ViewMap KCrashBookmarkImporterImpl::parseCrashLog_noemit( const QString & filename, bool del )
{
    static const int g_lineLimit = 16*1024;

    QFile f( filename );
    ViewMap views;

    if ( !f.open( QIODevice::ReadOnly ) )
        return views;

    QByteArray s( g_lineLimit );

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
        QString t = codec->toUnicode( s.trimmed() );
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

QStringList KCrashBookmarkImporterImpl::getCrashLogs()
{
    QMap<QString, bool> activeLogs;

    DCOPClient* dcop = KApplication::dcopClient();

    DCOPCStringList apps = dcop->registeredApplications();
    foreach ( DCOPCString clientId, apps )
    {
        if ( qstrncmp(clientId, "konqueror", 9) != 0 )
            continue;

        QByteArray data, replyData;
        DCOPCString replyType;
        QDataStream arg( &data, QIODevice::WriteOnly );

        if ( !dcop->call( clientId, "KonquerorIface",
                          "crashLogFile()", data, replyType, replyData) )
        {
            kdWarning() << "can't find dcop function KonquerorIface::crashLogFile()" << endl;
            continue;
        }

        if ( replyType != "QString" )
            continue;

        QDataStream reply( replyData );
        QString ret;
        reply >> ret;
        activeLogs[ret] = true;
    }

    QDir d( KCrashBookmarkImporterImpl().findDefaultLocation() );
    d.setSorting( QDir::Time );
    d.setFilter( QDir::Files );
    d.setNameFilter( "konqueror-crash-*.log" );

    QFileInfoList list = d.entryInfoList();
    QListIterator<QFileInfo> it( list );

    QStringList crashFiles;

    int count = 0;
    while ( it.hasNext() && count < 20 )
    {
        count++;
        QString path = it.next().absoluteFilePath();
        bool stillAlive = activeLogs.contains( path );
        if ( !stillAlive )
            crashFiles << path;
    }
    // Delete remaining ones
    while ( it.hasNext() )
    {
        QFile::remove( it.next().absoluteFilePath() );
    }

    return crashFiles;
}

void KCrashBookmarkImporterImpl::parse()
{
    QSet<QString> signatureSet;
    QStringList crashFiles = KCrashBookmarkImporterImpl::getCrashLogs();
    int count = 1;
    for ( QStringList::Iterator it = crashFiles.begin(); it != crashFiles.end(); ++it )
    {
        ViewMap views;
        views = parseCrashLog_noemit( *it, m_shouldDelete );
        QString signature;
        for ( ViewMap::Iterator vit = views.begin(); vit != views.end(); ++vit )
            signature += "|"+vit.data();
        if (signatureSet.contains(signature))
        {
            // Duplicate... throw away and skip
            QFile::remove(*it);
            continue;
        }

        signatureSet.insert(signature);

        int outerFolder = ( crashFiles.count() > 1 ) && (views.count() > 0);
        if ( outerFolder )
            emit newFolder( QString("Konqueror Window %1").arg(count++), false, "" );
        for ( ViewMap::Iterator vit = views.begin(); vit != views.end(); ++vit )
            emit newBookmark( vit.data(), vit.data(), QString("") );
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
