
#include "meinproc_common.h"

#include "xslt.h"

#include <QCoreApplication>
#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QTextCodec>
#include <QtCore/QFileInfo>
#include <QtCore/QList>

#include <libxml/xmlversion.h>
#include <libxml/xmlmemory.h>
#include <libxml/debugXML.h>
#include <libxml/HTMLtree.h>
#include <libxml/xmlIO.h>
#include <libxml/catalog.h>
#include <libxml/parserInternals.h>
#include <libxslt/xsltconfig.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#ifndef _WIN32
extern "C" int xmlLoadExtDtdDefaultValue;
#endif

int main(int argc, char **argv) {

    // xsltSetGenericDebugFunc(stderr, NULL);

    QCoreApplication app( argc, argv );

    const QStringList arguments = app.arguments();
    if ( arguments.count() != 4 ) {
        qCritical() << "wrong argument count";
        return ( 1 );
    }

    const QString srcDir = arguments[1];
    const QString xmllintPath = arguments[2];
    const QString checkFilename = arguments[3];
    const QString customizationCatalog = srcDir + "/customization/catalog.xml";
    
    if ( checkFile( checkFilename ) != CheckFileSuccess )
    {
        qCritical() << "checkFile failed in " << checkFilename;
        return ( 2 );
    }
    if ( checkFile( customizationCatalog ) != CheckFileSuccess )
    {
        qCritical() << "checkFile failed in " << customizationCatalog;
        return ( 2 );
    }

    QByteArray catalogs;
    catalogs += customizationCatalog.toUtf8();
    
    setenv( "XML_CATALOG_FILES", catalogs.constData(), 1 );
    xmlInitializeCatalog();

    LIBXML_TEST_VERSION

    if ( check( checkFilename, xmllintPath, catalogs ) != CheckSuccess )
    {
        qWarning() << "Check failed";
        return 3;
    }

    xmlSubstituteEntitiesDefault(1);
    xmlLoadExtDtdDefaultValue = 1;

    QVector<const char *> params;
    params.append( NULL );

    const QString tss = srcDir + "/customization/kde-chunk.xsl";
    QString output = transform(checkFilename , tss, params);
    if (output.isEmpty()) {
        fprintf(stderr, "unable to parse %s\n", checkFilename.toLocal8Bit().data());
        return(4);
    }

    doOutput(output, true, false, QString(), false /* replaceCharset */);

    xmlCleanupParser();
    xmlMemoryDump();
    return(0);
}

