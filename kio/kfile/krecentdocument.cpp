/* -*- c++ -*-
 * Copyright (C)2000 Daniel M. Duley <mosfet@kde.org>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
#include <krecentdocument.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kurl.h>
#include <kdebug.h>
#include <kmimetype.h>

#include <qdir.h>
#include <qfileinfo.h>
#include <qtextstream.h>
#include <qstringlist.h>
#include <qregexp.h>

#include <sys/types.h>
#include <utime.h>

QString KRecentDocument::recentDocumentDirectory()
{
    // need to change this path, not sure where
    return locateLocal("data", QString::fromLatin1("RecentDocuments/"));
}

QStringList KRecentDocument::recentDocuments()
{
    QDir d(recentDocumentDirectory(), "*.desktop", QDir::Time,
           QDir::Files | QDir::Readable | QDir::Hidden);

    if (!d.exists())
        d.mkdir(recentDocumentDirectory());

    QStringList list = d.entryList();
    QStringList fullList;

    for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it) {
        fullList.append( d.absFilePath( *it ) );
    }

    return fullList;
}

void KRecentDocument::add(const KURL& url)
{
    KRecentDocument::add(url, qApp->argv()[0]); // ### argv[0] might not match the service filename!
}

void KRecentDocument::add(const KURL& url, const QString& desktopEntryName)
{
    QString openStr = url.url();
    openStr.replace( QRegExp("\\$"), "$$" ); // Desktop files with type "Link" are $-variable expanded

    kdDebug(250) << "KRecentDocument::add for " << openStr << endl;
    KConfig *config = KGlobal::config();
    QString oldGrp = config->group();
    config->setGroup(QString::fromLatin1("RecentDocuments"));
    bool useRecent = config->readBoolEntry(QString::fromLatin1("UseRecent"), true);
    int maxEntries = config->readNumEntry(QString::fromLatin1("MaxEntries"), 10);

    config->setGroup(oldGrp);
    if(!useRecent)
        return;

    QString path = recentDocumentDirectory();

    QString dStr = path + url.fileName();

    QString ddesktop = dStr + QString::fromLatin1(".desktop");

    int i=1;
    // check for duplicates
    while(QFile::exists(ddesktop)){
        // see if it points to the same file and application
        KSimpleConfig tmp(ddesktop);
        tmp.setDesktopGroup();
        if(tmp.readEntry(QString::fromLatin1("X-KDE-LastOpenedWith"))
	   == desktopEntryName)
	{
            utime(QFile::encodeName(ddesktop), NULL);
            return;
        }
        // if not append a (num) to it
        ++i;
        if ( i > maxEntries )
            break;
        ddesktop = dStr + QString::fromLatin1("[%1].desktop").arg(i);
    }

    QDir dir(path);
    // check for max entries, delete oldest files if exceeded
    QStringList list = dir.entryList(QDir::Files, QDir::Time | QDir::Reversed);
    i = list.count();
    if(i > maxEntries-1){
        QStringList::Iterator it;
        it = list.begin();
        while(i > maxEntries-1){
            QFile::remove(dir.absPath() + QString::fromLatin1("/") + (*it));
            --i, ++it;
        }
    }

    // create the applnk
    KSimpleConfig conf(ddesktop);
    conf.setDesktopGroup();
    conf.writeEntry( QString::fromLatin1("Type"), QString::fromLatin1("Link") );
    conf.writeEntry( QString::fromLatin1("URL"), openStr );
    // If you change the line below, change the test in the above loop
    conf.writeEntry( QString::fromLatin1("X-KDE-LastOpenedWith"), desktopEntryName );
    conf.writeEntry( QString::fromLatin1("Name"), url.fileName() );
    conf.writeEntry( QString::fromLatin1("Icon"), KMimeType::iconForURL( url ) );
}

void KRecentDocument::add(const QString &openStr, bool isUrl)
{
    if( isUrl ) {
        add( KURL( openStr ) );
    } else {
        KURL url;
        url.setPath( openStr );
        add( url );
    }
}

void KRecentDocument::clear()
{
  QStringList list = recentDocuments();
  QDir dir;
  for(QStringList::Iterator it = list.begin(); it != list.end() ; ++it)
    dir.remove(*it);
}

int KRecentDocument::maximumItems()
{
    KConfig *config = KGlobal::config();
    KConfigGroupSaver sa(config, QString::fromLatin1("RecentDocuments"));
    return config->readNumEntry(QString::fromLatin1("MaxEntries"), 10);
}


