// $Id$

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#include <qvaluelist.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qtextstream.h>

#include <kshred.h>
#include <kdebug.h>
#include <kurl.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstddirs.h>
#include <kprotocolmanager.h>
#include <kinstance.h>
#include <qfile.h>
#include <limits.h>
#include <qtextcodec.h>

#include "kio_help.h"
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <xslt.h>

using namespace KIO;

QString HelpProtocol::langLookup(QString fname)
{
    QStringList search;

    // assemble the local search paths
    const QStringList localDoc = KGlobal::dirs()->resourceDirs("html");

    // look up the different languages
    for (int id=localDoc.count()-1; id >= 0; --id)
    {
        QStringList langs = KGlobal::locale()->languageList();
        langs.append("en");
        QStringList::ConstIterator lang;
        for (lang = langs.begin(); lang != langs.end(); ++lang)
            search.append(QString("%1%2/%3").arg(localDoc[id]).arg(*lang).arg(fname));
    }

    // try to locate the file
    QStringList::Iterator it;
    for (it = search.begin(); it != search.end(); ++it)
    {
        kdDebug() << "Looking for help in: " << *it << endl;

        QFileInfo info(*it);
        if (info.exists() && info.isFile() && info.isReadable())
            return *it;

        QString file = (*it).left((*it).findRev('/')) + "/index.docbook";
        kdDebug() << "Looking for help in: " << file << endl;
        info.setFile(file);
        if (info.exists() && info.isFile() && info.isReadable())
            return *it;
    }

    return QString::null;
}


QString HelpProtocol::lookupFile(const QString &fname,
                                 const QString &query, bool &redirect)
{
    redirect = false;

    QString path, result;

    path = fname;

    result = langLookup(path);
    if (result.isEmpty())
    {
        result = langLookup(path+"/index.html");
        if (!result.isEmpty())
	{
            KURL red( "help:/" );
            red.setPath( path + "/index.html" );
            red.setQuery( query );
            redirection(red);
            kdDebug() << "redirect to " << red.url() << endl;
            redirect = true;
	}
        else
	{
            result = langLookup("khelpcenter/index.html");
            if (!result.isEmpty())
	    {
                redirection(KURL("help:/khelpcenter/index.html"));
                redirect = true;
                return QString::null;
	    }

            notFound();
            return QString::null;
	}
    } else
        kdDebug() << "result " << result << endl;

    return result;
}


void HelpProtocol::notFound()
{
    data( QTextCodec::codecForLocale()->fromUnicode( i18n("<html>The requested help file could not be found. Check that "
                                                          "you have installed the documentation.</html>" ) ) );
    finished();
}

HelpProtocol *slave = 0;

HelpProtocol::HelpProtocol( const QCString &pool, const QCString &app )
  : SlaveBase( "help", pool, app )
{
    slave = this;
}

void HelpProtocol::get( const KURL& url )
{
    kdDebug() << "get: path=" << url.path()
              << " query=" << url.query() << endl;

    bool redirect;
    QString doc;
    doc = url.path();
    if (doc.at(doc.length() - 1) == '/')
        doc += "index.html";

    infoMessage(i18n("Looking up correct file"));

    doc = lookupFile(doc, url.query(), redirect);

    if (redirect)
    {
        finished();
        return;
    }

    if (doc.isEmpty())
    {
        error( KIO::ERR_DOES_NOT_EXIST, url.url() );
        return;
    }

    KURL target;
    target.setPath(doc);
    if (url.hasHTMLRef())
        target.setHTMLRef(url.htmlRef());

    kdDebug() << "target " << target.url() << endl;

    QString file = target.path();
    if (!KStandardDirs::exists(file) || file.right(4) == "html") {
        file = file.left(file.findRev('/')) + "/index.docbook";
    } else {
        QFileInfo fi(file);
        if (fi.isDir()) {
            file = file + "/index.docbook";
        } else {
            kdDebug() << "emitFile redirection " << url.url().latin1() << endl;
            redirection(target);
            finished();
            return;
        }
    }

    infoMessage(i18n("Preparing document"));

    kdDebug() << "look for cache " << endl;

    parsed = lookForCache( file );

    kdDebug() << "cached parsed " << parsed.length() << endl;

    if ( parsed.isEmpty() ) {
        parsed = transform(file, locate("dtd", "customization/kde-chunk.xsl"));
        if ( !parsed.isEmpty() ) {
            infoMessage( i18n( "Saving to cache" ) );
            QString cache = file.left( file.length() - 7 );
            if ( !saveToCache( parsed, locateLocal( "data",
                                                    "kio_help/cache" + cache +
                                                    "cache.bz2" ) ) )
                saveToCache( parsed, locateLocal( "data",
                                                  "kio_help/cache" + cache +
                                                  "cache.gz" ) );
        }
    } else infoMessage( i18n( "Using cached version" ) );

    kdDebug() << "parsed " << parsed.length() << endl;

    if (parsed.isEmpty()) {
        data(QTextCodec::codecForLocale()->fromUnicode( QString(
            "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=%1\"></head>\n"
            "%2<br>%3</html>" ).arg( QTextCodec::codecForLocale()->name() ).
                                                        arg( i18n( "The requested help file could not be parsed:" ) ).arg( file ) ) );
    } else {
        QString query = url.query(), anchor;

        // if we have a query, look if it contains an anchor
        if (!query.isEmpty())
            if (query.left(8) == "?anchor=")
                anchor = query.mid(8).lower();

        kdDebug() << "anchor: " << anchor << endl;

        if ( !anchor.isEmpty() )
        {
            int index = 0;
            while ( true ) {
                index = parsed.find( QRegExp( "<a name=" ), index);
                if ( index == -1 ) {
                    kdDebug() << "no anchor\n";
                    break; // use whatever is the target, most likely index.html
                }

                if ( parsed.mid( index, 11 + anchor.length() ).lower() ==
                     QString( "<a name=\"%1\">" ).arg( anchor ) )
                {
                    index = parsed.findRev( "<FILENAME filename=", index ) +
                             strlen( "<FILENAME filename=\"" );
                    QString filename=parsed.mid( index, 2000 );
                    filename = filename.left( filename.find( '\"' ) );
                    QString path = target.path();
                    path = path.left( path.findRev( '/' ) + 1) + filename;
                    kdDebug() << "anchor found in " << path <<endl;
                    target.setPath( path );
                    break;
                }
                index++;
            }
        }
        emitFile( target);
    }

    finished();
}

void HelpProtocol::emitFile( const KURL& url )
{
    infoMessage(i18n("Looking up section"));

    QString filename = url.path().mid(url.path().findRev('/') + 1);

    int index = parsed.find(QString("<FILENAME filename=\"%1\"").arg(filename));
    if (index == -1) {
        if ( filename == "index.html" ) {
            data( QTextCodec::codecForLocale()->fromUnicode( parsed ) );
            return;
        }

        data(QTextCodec::codecForLocale()->fromUnicode( i18n("<html>Couldn't find filename %1 in %2</html>").arg(filename).arg(url.url() ) ) );
        return;
    }

    QString filedata = splitOut(parsed, index);

    filedata.replace( QRegExp( "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">" ),
                      QString( "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=%1\">" ).arg(  QTextCodec::codecForLocale()->name() ) );

    data( QTextCodec::codecForLocale()->fromUnicode( filedata ) );
}

void HelpProtocol::mimetype( const KURL &)
{
    mimeType("text/html");
    finished();
}
