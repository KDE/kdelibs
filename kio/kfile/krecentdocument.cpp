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
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kurl.h>
#include <kdebug.h>
#include <kmimetype.h>
#include <kdesktopfile.h>
#include <kde_file.h>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QTextIStream>
#include <QtCore/QMutableStringListIterator>
#include <QtCore/QRegExp>

#include <sys/types.h>
#include <kconfiggroup.h>

QString KRecentDocument::recentDocumentDirectory()
{
    // need to change this path, not sure where
    return KStandardDirs::locateLocal("data", QLatin1String("RecentDocuments/"));
}

QStringList KRecentDocument::recentDocuments()
{
    QDir d(recentDocumentDirectory(), "*.desktop", QDir::Time,
           QDir::Files | QDir::Readable | QDir::Hidden);

    if (!d.exists())
        d.mkdir(recentDocumentDirectory());

    const QStringList list = d.entryList();
    QStringList fullList;

    for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it) {
       QString fileName = *it ;
       QString pathDesktop;
       if (fileName.startsWith(":")) {
       // FIXME: Remove when Qt will be fixed
       // http://bugreports.qt.nokia.com/browse/QTBUG-11223
           pathDesktop = KRecentDocument::recentDocumentDirectory() + *it ;
       }
       else {
           pathDesktop = d.absoluteFilePath( *it );
       }
       KDesktopFile tmpDesktopFile( pathDesktop );
       KUrl urlDesktopFile(tmpDesktopFile.desktopGroup().readPathEntry("URL", QString()));
       if (urlDesktopFile.isLocalFile() && !QFile(urlDesktopFile.toLocalFile()).exists()) {
           d.remove(pathDesktop);
       } else {
           fullList.append( pathDesktop );
       }
    }

    return fullList;
}

void KRecentDocument::add(const KUrl& url)
{
    KRecentDocument::add(url, KGlobal::mainComponent().componentName());
    // ### componentName might not match the service filename...
}

void KRecentDocument::add(const KUrl& url, const QString& desktopEntryName)
{
    if ( url.isLocalFile() && KGlobal::dirs()->relativeLocation( "tmp", url.toLocalFile() ) != url.toLocalFile() )
      return; // inside tmp resource, do not save

    QString openStr = url.url();
    openStr.replace( QRegExp("\\$"), "$$" ); // Desktop files with type "Link" are $-variable expanded

    kDebug(250) << "KRecentDocument::add for " << openStr;
    KConfigGroup config = KGlobal::config()->group(QByteArray("RecentDocuments"));
    bool useRecent = config.readEntry(QLatin1String("UseRecent"), true);
    int maxEntries = config.readEntry(QLatin1String("MaxEntries"), 10);

    if(!useRecent || maxEntries <= 0)
        return;

    QString path = recentDocumentDirectory();

    QString dStr = path + url.fileName();

    QString ddesktop = dStr + QLatin1String(".desktop");

    int i=1;
    // check for duplicates
    while(QFile::exists(ddesktop)){
        // see if it points to the same file and application
        KDesktopFile tmp(ddesktop);
        if ( tmp.desktopGroup().readEntry("X-KDE-LastOpenedWith") == desktopEntryName ) {
            KDE::utime(ddesktop, NULL);
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
    const QStringList list = dir.entryList(QDir::Files | QDir::Hidden, QFlags<QDir::SortFlag>(QDir::Time | QDir::Reversed));
    i = list.count();
    if(i > maxEntries-1){
        QStringList::ConstIterator it;
        it = list.begin();
        while(i > maxEntries-1){
            QFile::remove(dir.absolutePath() + QLatin1String("/") + (*it));
            --i, ++it;
        }
    }

    // create the applnk
    KDesktopFile configFile(ddesktop);
    KConfigGroup conf = configFile.desktopGroup();
    conf.writeEntry( "Type", QString::fromLatin1("Link") );
    conf.writePathEntry( "URL", openStr );
    // If you change the line below, change the test in the above loop
    conf.writeEntry( "X-KDE-LastOpenedWith", desktopEntryName );
    conf.writeEntry( "Name", url.fileName() );
    conf.writeEntry( "Icon", KMimeType::iconNameForUrl( url ) );
}

void KRecentDocument::add(const QString &openStr, bool isUrl)
{
    if( isUrl ) {
        add( KUrl( openStr ) );
    } else {
        KUrl url;
        url.setPath( openStr );
        add( url );
    }
}

void KRecentDocument::clear()
{
  const QStringList list = recentDocuments();
  QDir dir;
  for(QStringList::ConstIterator it = list.begin(); it != list.end() ; ++it)
    dir.remove(*it);
}

int KRecentDocument::maximumItems()
{
    KConfigGroup cg(KGlobal::config(), QLatin1String("RecentDocuments"));
    return cg.readEntry(QLatin1String("MaxEntries"), 10);
}


