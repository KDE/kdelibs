#include "xslt_help.h"
#include "xslt.h"

#include <libxslt/xsltconfig.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <libxml/xmlIO.h>
#include <libxml/parserInternals.h>
#include <libxml/catalog.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <QtCore/QDate>
#include <QtCore/QDir>
#include <QtCore/QRegExp>
#include <kcomponentdata.h>
#include <klocale.h>
#include <assert.h>
#include <kfilterbase.h>
#include <kfilterdev.h>
#include <QtCore/QTextCodec>
#include <stdlib.h>
#include <config.h>
#include <stdarg.h>
#include <klibloader.h>
#include <kcharsets.h>
#include <kurl.h>


static bool readCache( const QString &filename,
                       const QString &cache, QString &output)
{
    kDebug( 7119 ) << filename << " " << cache;
    KGlobal::dirs()->addResourceType("dtd", "data", "ksgmltools2/");
    if ( !compareTimeStamps( filename, cache ) )
        return false;
    if ( !compareTimeStamps( KStandardDirs::locate( "dtd", "customization/kde-chunk.xsl"), cache ) )
        return false;

    kDebug( 7119 ) << "create filter";
    QIODevice *fd = ::getBZip2device(cache);
    if ( !fd )
        return false;

    if (!fd->open(QIODevice::ReadOnly))
    {
       delete fd;
       QFile::remove(cache);
       return false;
    }

    kDebug( 7119 ) << "reading";

    char buffer[32000];
    int n;
    QByteArray text;
    // Also end loop in case of error, when -1 is returned
    while ( ( n = fd->read(buffer, 31900) ) > 0)
    {
        buffer[n] = 0;
        text += buffer;
    }
    kDebug( 7119 ) << "read " << text.length();
    fd->close();

    output = QString::fromUtf8( text );
    delete fd;

    if (n == -1)
        return false;

    kDebug( 7119 ) << "finished ";

    return true;
}

QString lookForCache( const QString &filename )
{
    kDebug() << "lookForCache " << filename;
    assert( filename.endsWith( QLatin1String(".docbook") ) );
    assert( QDir::isAbsolutePath(filename));
    QString cache = filename.left( filename.length() - 7 );
    QString output;
    if ( readCache( filename, cache + "cache.bz2", output) )
        return output;
#ifdef Q_WS_WIN
    QFileInfo fi(filename);
    // make sure filenames do not contain the base path, otherwise
    // accessing user data from another location invalids cached files.
    // Accessing user data under a different path is possible
    // when using usb sticks - this may affect unix/mac systems also
    cache = '/' + fi.absolutePath().remove(KStandardDirs::installPath("html"),Qt::CaseInsensitive).replace('/','_') + '_' + fi.baseName() + '.';
#endif
    if ( readCache( filename,
                    KStandardDirs::locateLocal( "cache",
                                 "kio_help" + cache +
                                 "cache.bz2" ), output ) )
        return output;

    return QString();
}

bool compareTimeStamps( const QString &older, const QString &newer )
{
    QFileInfo _older( older );
    QFileInfo _newer( newer );
    assert( _older.exists() );
    if ( !_newer.exists() )
        return false;
    return ( _newer.lastModified() > _older.lastModified() );
}
