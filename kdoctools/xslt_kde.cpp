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

void fillInstance(KComponentData &ins, const QString &srcdir)
{
    QByteArray catalogs;

    if ( srcdir.isEmpty() ) {
        catalogs += KUrl::fromLocalFile( ins.dirs()->findResource("data", "ksgmltools2/customization/catalog.xml") ).toEncoded();
        ins.dirs()->addResourceType("dtd", "data", "ksgmltools2/");
    } else {
        catalogs += KUrl::fromLocalFile( srcdir +"/customization/catalog.xml" ).toEncoded();
        ins.dirs()->addResourceDir("dtd", srcdir);
    }

    setenv( "XML_CATALOG_FILES", catalogs.constData(), 1 );
    xmlInitializeCatalog();
}

QIODevice *getBZip2device(const QString &fileName )
{
    return KFilterDev::deviceForFile(fileName);
}

bool saveToCache( const QString &contents, const QString &filename )
{
    QIODevice *fd = ::getBZip2device(filename);
    if ( !fd )
        return false;

    if (!fd->open(QIODevice::WriteOnly))
    {
       delete fd;
       return false;
    }

    fd->write( contents.toUtf8() );
    fd->close();
    delete fd;
    return true;
}
