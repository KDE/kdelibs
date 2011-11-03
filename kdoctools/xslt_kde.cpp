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
#include <qurl.h>

void fillInstance(KComponentData &ins, const QString &srcdir)
{
    QByteArray catalogs;

    if ( srcdir.isEmpty() ) {
        catalogs += QUrl::fromLocalFile( ins.dirs()->findResource("data", "ksgmltools2/customization/catalog.xml") ).toEncoded();
        ins.dirs()->addResourceType("dtd", "data", "ksgmltools2/");
    } else {
        catalogs += QUrl::fromLocalFile( srcdir +"/customization/catalog.xml" ).toEncoded();
        ins.dirs()->addResourceDir("dtd", srcdir);
    }

    setenv( "XML_CATALOG_FILES", catalogs.constData(), 1 );
    xmlInitializeCatalog();
}

bool saveToCache( const QString &contents, const QString &filename )
{
    KFilterDev fd(filename);

    if (!fd.open(QIODevice::WriteOnly))
    {
       return false;
    }

    fd.write( contents.toUtf8() );
    fd.close();
    return true;
}
