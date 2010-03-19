/* This file is part of the KDE libraries
   Copyright (C) 2000 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2001 Stephan Kulow <coolo@kde.org>
   Copyright (C) 2003 Cornelius Schumacher <schumacher@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later versio

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


#include <config.h>

#include "kio_help.h"
#include "xslt.h"
#include "xslt_help.h"

#include <kdebug.h>
#include <kde_file.h>
#include <kurl.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kcomponentdata.h>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#include <QtCore/QRegExp>
#include <QtCore/QTextCodec>
#include <QtGui/QTextDocument>


#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

#include <errno.h>
#include <fcntl.h>
#ifdef HAVE_STDIO_H
# include <stdio.h>
#endif
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif

#include <libxslt/xsltutils.h>
#include <libxslt/transform.h>

using namespace KIO;

QString HelpProtocol::langLookup(const QString &fname)
{
    QStringList search;

    // assemble the local search paths
    const QStringList localDoc = KGlobal::dirs()->resourceDirs("html");

    QStringList langs = KGlobal::locale()->languageList();
    langs.append( "en" );
    langs.removeAll( "C" );

    // this is kind of compat hack as we install our docs in en/ but the
    // default language is en_US
    for (QStringList::Iterator it = langs.begin(); it != langs.end(); ++it)
        if ( *it == "en_US" )
            *it = "en";

    // look up the different languages
    int ldCount = localDoc.count();
    for (int id=0; id < ldCount; id++)
    {
        QStringList::ConstIterator lang;
        for (lang = langs.constBegin(); lang != langs.constEnd(); ++lang)
            search.append(QString("%1%2/%3").arg(localDoc[id], *lang, fname));
    }

    // try to locate the file
    for (QStringList::ConstIterator it = search.constBegin(); it != search.constEnd(); ++it)
    {
        kDebug( 7119 ) << "Looking for help in: " << *it;

        QFileInfo info(*it);
        if (info.exists() && info.isFile() && info.isReadable())
            return *it;

        if ( ( *it ).endsWith( QLatin1String(".html") ) )
        {
            QString file = (*it).left((*it).lastIndexOf('/')) + "/index.docbook";
            kDebug( 7119 ) << "Looking for help in: " << file;
            info.setFile(file);
            if (info.exists() && info.isFile() && info.isReadable())
                return *it;
        }
    }


    return QString();
}


QString HelpProtocol::lookupFile(const QString &fname,
                                 const QString &query, bool &redirect)
{
    redirect = false;

    const QString path = fname;

    QString result = langLookup(path);
    if (result.isEmpty())
    {
        result = langLookup(path+"/index.html");
        if (!result.isEmpty())
	{
            KUrl red( "help:/" );
            red.setPath( path + "/index.html" );
            red.setQuery( query );
            redirection(red);
            kDebug( 7119 ) << "redirect to " << red.url();
            redirect = true;
	}
        else
	{
	    unicodeError( i18n("There is no documentation available for %1." , Qt::escape(path)) );
            return QString();
	}
    } else
        kDebug( 7119 ) << "result " << result;

    return result;
}


void HelpProtocol::unicodeError( const QString &t )
{
#ifdef Q_WS_WIN
   QString encoding = "UTF-8";
#else
   QString encoding = QTextCodec::codecForLocale()->name();
#endif   
   data(fromUnicode( QString(
        "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=%1\"></head>\n"
        "%2</html>" ).arg( encoding, Qt::escape(t) ) ) );
     
}

HelpProtocol *slave = 0;

HelpProtocol::HelpProtocol( bool ghelp, const QByteArray &pool, const QByteArray &app )
  : SlaveBase( ghelp ? "ghelp" : "help", pool, app ), mGhelp( ghelp )
{
    slave = this;
}

void HelpProtocol::get( const KUrl& url )
{
    kDebug( 7119 ) << "path=" << url.path()
                   << "query=" << url.query();

    bool redirect;
    QString doc = QDir::cleanPath(url.path());
    if (doc.contains("..")) {
        error( KIO::ERR_DOES_NOT_EXIST, url.url() );
        return;
    }

    if ( !mGhelp ) {
        if (!doc.startsWith('/'))
            doc = doc.prepend(QLatin1Char('/'));

        if (doc.endsWith('/'))
            doc += "index.html";
    }

    infoMessage(i18n("Looking up correct file"));

    if ( !mGhelp ) {
      doc = lookupFile(doc, url.query(), redirect);

      if (redirect)
      {
          finished();
          return;
      }
    }

    if (doc.isEmpty())
    {
        error( KIO::ERR_DOES_NOT_EXIST, url.url() );
        return;
    }

    mimeType("text/html");
    KUrl target;
    target.setPath(doc);
    if (url.hasHTMLRef())
        target.setHTMLRef(url.htmlRef());

    kDebug( 7119 ) << "target " << target.url();

    QString file = target.scheme() == "file" ? target.toLocalFile() : target.path();
    
    if ( mGhelp ) {
      if ( !file.endsWith( QLatin1String( ".xml" ) ) ) {
         get_file( target );
         return;
      }
    } else {
        QString docbook_file = file.left(file.lastIndexOf('/')) + "/index.docbook";
        if (!KStandardDirs::exists(file)) {
            file = docbook_file;
        } else {
            QFileInfo fi(file);
            if (fi.isDir()) {
                file = file + "/index.docbook";
            } else {
                if ( !file.endsWith( QLatin1String( ".html" ) ) || !compareTimeStamps( file, docbook_file ) ) {
                    get_file( target );
                    return;
                } else
                    file = docbook_file;
            }
        }
    }

    infoMessage(i18n("Preparing document"));

    if ( mGhelp ) {
        QString xsl = "customization/kde-nochunk.xsl";
        mParsed = transform(file, KStandardDirs::locate("dtd", xsl));

        kDebug( 7119 ) << "parsed " << mParsed.length();

        if (mParsed.isEmpty()) {
            unicodeError( i18n( "The requested help file could not be parsed:<br />%1" ,  file ) );
        } else {
            int pos1 = mParsed.indexOf( "charset=" );
            if ( pos1 > 0 ) {
              int pos2 = mParsed.indexOf( '"', pos1 );
              if ( pos2 > 0 ) {
                mParsed.replace( pos1, pos2 - pos1, "charset=UTF-8" );
              }
            }
            data( mParsed.toUtf8() );
        }
    } else {

        kDebug( 7119 ) << "look for cache for " << file;

        mParsed = lookForCache( file );

        kDebug( 7119 ) << "cached parsed " << mParsed.length();

        if ( mParsed.isEmpty() ) {
            mParsed = transform(file, KStandardDirs::locate("dtd", "customization/kde-chunk.xsl"));
            if ( !mParsed.isEmpty() ) {
                infoMessage( i18n( "Saving to cache" ) );
#ifdef Q_WS_WIN
                QFileInfo fi(file);
                // make sure filenames do not contain the base path, otherwise
                // accessing user data from another location invalids cached files
                // Accessing user data under a different path is possible
                // when using usb sticks - this may affect unix/mac systems also
                QString cache = '/' + fi.absolutePath().remove(KStandardDirs::installPath("html"),Qt::CaseInsensitive).replace('/','_') + '_' + fi.baseName() + '.';
#else
                QString cache = file.left( file.length() - 7 );
#endif
                saveToCache( mParsed, KStandardDirs::locateLocal( "cache",
                                                                  "kio_help" + cache +
                                                                  "cache.bz2" ) );
            }
        } else infoMessage( i18n( "Using cached version" ) );

        kDebug( 7119 ) << "parsed " << mParsed.length();

        if (mParsed.isEmpty()) {
            unicodeError( i18n( "The requested help file could not be parsed:<br />%1" ,  file ) );
        } else {
            QString query = url.query(), anchor;

            // if we have a query, look if it contains an anchor
            if (!query.isEmpty())
                if (query.startsWith(QLatin1String("?anchor="))) {
                    anchor = query.mid(8).toLower();

			    KUrl redirURL(url);

			    redirURL.setQuery(QString());
			    redirURL.setHTMLRef(anchor);
			    redirection(redirURL);
			    finished();
			    return;
		    }
            if (anchor.isEmpty() && url.hasHTMLRef())
	        anchor = url.htmlRef();

            kDebug( 7119 ) << "anchor: " << anchor;

            if ( !anchor.isEmpty() )
            {
                int index = 0;
                while ( true ) {
                    index = mParsed.indexOf( QRegExp( "<a name=" ), index);
                    if ( index == -1 ) {
                        kDebug( 7119 ) << "no anchor\n";
                        break; // use whatever is the target, most likely index.html
                    }

                    if ( mParsed.mid( index, 11 + anchor.length() ).toLower() ==
                         QString( "<a name=\"%1\">" ).arg( anchor ) )
                    {
                        index = mParsed.lastIndexOf( "<FILENAME filename=", index ) +
                                 strlen( "<FILENAME filename=\"" );
                        QString filename=mParsed.mid( index, 2000 );
                        filename = filename.left( filename.indexOf( '\"' ) );
                        QString path = target.path();
                        path = path.left( path.lastIndexOf( '/' ) + 1) + filename;
                        target.setPath( path );
                        kDebug( 7119 ) << "anchor found in " << target.url();
                        break;
                    }
                    index++;
                }
            }
            emitFile( target );
        }
    }

    finished();
}

void HelpProtocol::emitFile( const KUrl& url )
{
    infoMessage(i18n("Looking up section"));

    QString filename = url.path().mid(url.path().lastIndexOf('/') + 1);

    int index = mParsed.indexOf(QString("<FILENAME filename=\"%1\"").arg(filename));
    if (index == -1) {
        if ( filename == "index.html" ) {
            data( fromUnicode( mParsed ) );
            return;
        }

        unicodeError( i18n("Could not find filename %1 in %2.", filename,  url.url() ) );
        return;
    }

    QString filedata = splitOut(mParsed, index);
    replaceCharsetHeader( filedata );

    data( fromUnicode( filedata ) );
    data( QByteArray() );
}

void HelpProtocol::mimetype( const KUrl &)
{
    mimeType("text/html");
    finished();
}

// Copied from kio_file to avoid redirects

#define MAX_IPC_SIZE (1024*32)

void HelpProtocol::get_file( const KUrl& url )
{
    kDebug( 7119 ) << "get_file " << url.url();

#ifdef Q_WS_WIN
    QFile f( url.toLocalFile() );
    if ( !f.exists() ) {
        error( KIO::ERR_DOES_NOT_EXIST, url.url() );
        return;
    }
    if ( !f.open(QIODevice::ReadOnly) ) {
        error( KIO::ERR_CANNOT_OPEN_FOR_READING, url.path() );
        return;
    }
    int processed_size = 0;
    totalSize( f.size() );

    QByteArray array;
    array.resize(MAX_IPC_SIZE);

    while( 1 )
    {
        qint64 n = f.read(array.data(),array.size());
        if (n == -1) {
            error( KIO::ERR_COULD_NOT_READ, url.path());
            f.close();
            return;
       }
       if (n == 0)
            break; // Finished

       data( array );

       processed_size += n;
       processedSize( processed_size );
    }

    data( QByteArray() );
    f.close();

    processedSize( f.size() );
    finished();
#else
    QByteArray _path( QFile::encodeName(url.path()));
    KDE_struct_stat buff;
    if ( KDE_stat( _path.data(), &buff ) == -1 ) {
        if ( errno == EACCES )
           error( KIO::ERR_ACCESS_DENIED, url.url() );
        else
           error( KIO::ERR_DOES_NOT_EXIST, url.url() );
        return;
    }

    if ( S_ISDIR( buff.st_mode ) ) {
        error( KIO::ERR_IS_DIRECTORY, url.path() );
        return;
    }
    if ( S_ISFIFO( buff.st_mode ) || S_ISSOCK ( buff.st_mode ) ) {
        error( KIO::ERR_CANNOT_OPEN_FOR_READING, url.path() );
        return;
    }

    int fd = KDE_open( _path.data(), O_RDONLY);
    if ( fd < 0 ) {
        error( KIO::ERR_CANNOT_OPEN_FOR_READING, url.path() );
        return;
    }

    totalSize( buff.st_size );
    int processed_size = 0;

    char buffer[ MAX_IPC_SIZE ];
    QByteArray array;

    while( 1 )
    {
       int n = ::read( fd, buffer, MAX_IPC_SIZE );
       if (n == -1)
       {
          if (errno == EINTR)
              continue;
          error( KIO::ERR_COULD_NOT_READ, url.path());
          ::close(fd);
          return;
       }
       if (n == 0)
          break; // Finished

       array = array.fromRawData(buffer, n);
       data( array );
       array = array.fromRawData(buffer, n);

       processed_size += n;
       processedSize( processed_size );
    }

    data( QByteArray() );

    ::close( fd );

    processedSize( buff.st_size );

    finished();
#endif
}
