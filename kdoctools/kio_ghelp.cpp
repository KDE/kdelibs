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
#include <qregexp.h>

#include <kshred.h>
#include <kdebug.h>
#include <kurl.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kprotocolmanager.h>
#include <kinstance.h>
#include <limits.h>
#include <qtextcodec.h>
#include <qregexp.h>

#include "kio_ghelp.h"
#include <libxslt/xsltutils.h>
#include <libxslt/transform.h>
#include "xslt.h"

using namespace KIO;

void GnomeHelpProtocol::notFound()
{
    data( fromUnicode( i18n("<html>The requested help file could not be found. Check that "
                                                          "you have installed the documentation.</html>" ) ) );
    finished();
}

GnomeHelpProtocol *slave = 0;

GnomeHelpProtocol::GnomeHelpProtocol( const QCString &pool, const QCString &app )
  : SlaveBase( "ghelp", pool, app )
{
    slave = this;
}

void GnomeHelpProtocol::get( const KURL& url )
{
    QString query = url.query();

    kdDebug( 7119 ) << "get: path=" << url.path()
              << " query=" << query << endl;

    QString doc = url.path();

    infoMessage(i18n("Looking up correct file"));

    if (doc.isEmpty())
    {
        error( KIO::ERR_DOES_NOT_EXIST, url.url() );
        return;
    }

    mimeType("text/html");
    KURL target;
    target.setPath(doc);
    if (url.hasHTMLRef())
        target.setHTMLRef(url.htmlRef());

    kdDebug( 7119 ) << "target " << target.url() << endl;

    QString file = target.path();
    QString docbook_file = file.left( file.findRev( '.' ) ) + ".xml";
    if (!KStandardDirs::exists(file)) {
        file = docbook_file;
    } else {
        QFileInfo fi(file);
        if (fi.isDir()) {
            file = file + "/index.docbook";
        } else {
            if ( file.right( 5 ) != ".html" || !compareTimeStamps( file, docbook_file ) ) {
                get_file( target );
                return;
            } else
                file = docbook_file;
        }
    }

    infoMessage(i18n("Preparing document"));

    QString xsl = "customization/kde-nochunk.xsl";
    mParsed = transform(file, locate("dtd", xsl));
    
    kdDebug( 7119 ) << "parsed " << mParsed.length() << endl;

    if (mParsed.isEmpty()) {
        data(fromUnicode( QString(
            "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=%1\"></head>\n"
            "%2<br>%3</html>" ).arg( QTextCodec::codecForLocale()->name() ).
                                                        arg( i18n( "The requested help file could not be parsed:" ) ).arg( file ) ) );
    } else {
        data( fromUnicode( mParsed ) );
    }

    finished();
}

void GnomeHelpProtocol::mimetype( const KURL &)
{
    mimeType("text/html");
    finished();
}

// Copied from kio_file to avoid redirects

#define MAX_IPC_SIZE (1024*32)

void GnomeHelpProtocol::get_file( const KURL& url )
{
    kdDebug( 7119 ) << "get_file " << url.url() << endl;

    QCString _path( QFile::encodeName(url.path()));
    struct stat buff;
    if ( ::stat( _path.data(), &buff ) == -1 ) {
        if ( errno == EACCES )
           error( KIO::ERR_ACCESS_DENIED, url.path() );
        else
           error( KIO::ERR_DOES_NOT_EXIST, url.path() );
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

    int fd = open( _path.data(), O_RDONLY);
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
          close(fd);
          return;
       }
       if (n == 0)
          break; // Finished

       array.setRawData(buffer, n);
       data( array );
       array.resetRawData(buffer, n);

       processed_size += n;
       processedSize( processed_size );
    }

    data( QByteArray() );

    close( fd );

    processedSize( buff.st_size );

    finished();
}
