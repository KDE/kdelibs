
#include "../config-kdoctools.h"
#include "xslt.h"
#include "meinproc_common.h"

#include <QCoreApplication>
#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QTextCodec>
#include <QtCore/QFileInfo>
#include <QtCore/QList>
#include <qstandardpaths.h>

#include <qurl.h>

#include <libxml/xmlversion.h>
#include <libxml/xmlmemory.h>
#include <libxml/debugXML.h>
#include <libxml/HTMLtree.h>
#include <libxml/xmlIO.h>
#include <libxml/parserInternals.h>
#include <libxslt/xsltconfig.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <libexslt/exslt.h>

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <qcommandlineparser.h>
#include <qcommandlineoption.h>

#ifndef _WIN32
extern "C" int xmlLoadExtDtdDefaultValue;
#endif

class MyPair {
public:
    QString word;
    int base;};

typedef QList<MyPair> PairList;

void parseEntry(PairList &list, xmlNodePtr cur, int base)
{
    if ( !cur )
        return;

    base += atoi( ( const char* )xmlGetProp(cur, ( const xmlChar* )"header") );
    if ( base > 10 ) // 10 is the maximum
        base = 10;

    /* We don't care what the top level element name is */
    cur = cur->xmlChildrenNode;
    while (cur != NULL) {

        if ( cur->type == XML_TEXT_NODE ) {
            QString words = QString::fromUtf8( ( char* )cur->content );
            const QStringList wlist = words.simplified().split( QLatin1Char(' '), QString::SkipEmptyParts );
            for ( QStringList::ConstIterator it = wlist.begin();
                  it != wlist.end(); ++it )
            {
                MyPair m;
                m.word = *it;
                m.base = base;
                list.append( m );
            }
        } else if ( !xmlStrcmp( cur->name, (const xmlChar *) "entry") )
            parseEntry( list, cur, base );

        cur = cur->next;
    }

}

int main(int argc, char **argv) {

    // xsltSetGenericDebugFunc(stderr, NULL);


    /*options.add("stylesheet <xsl>", ki18n("Stylesheet to use"));
    options.add("stdout", ki18n("Output whole document to stdout"));
    options.add("o");
    options.add("output <file>", ki18n("Output whole document to file"));
    options.add("htdig", ki18n("Create a ht://dig compatible index"));
    options.add("check", ki18n("Check the document for validity"));
    options.add("cache <file>", ki18n("Create a cache file for the document"));
    options.add("srcdir <dir>", ki18n("Set the srcdir, for kdelibs"));
    options.add("param <key>=<value>", ki18n("Parameters to pass to the stylesheet"));
    options.add("+xml", ki18n("The file to transform"));*/

    QCoreApplication app( argc, argv );
    app.setApplicationName(QStringLiteral("meinproc"));
    app.setApplicationVersion(QStringLiteral("5.0"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::translate("main", "KDE Translator for XML"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("stylesheet"), QCoreApplication::translate("main", "Stylesheet to use"), QStringLiteral("xsl")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("stdout"), QCoreApplication::translate("main", "Output whole document to stdout")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("o") << QStringLiteral("output"), QCoreApplication::translate("main", "Output whole document to file"), QStringLiteral("file")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("htdig"), QCoreApplication::translate("main", "Create a ht://dig compatible index")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("check"), QCoreApplication::translate("main", "Check the document for validity")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("cache"), QCoreApplication::translate("main", "Create a cache file for the document"), QStringLiteral("file")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("srcdir"), QCoreApplication::translate("main", "Set the srcdir, for kdelibs"), QStringLiteral("dir")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("param"), QCoreApplication::translate("main", "Parameters to pass to the stylesheet"), QStringLiteral("key=value")));
    parser.addPositionalArgument(QStringLiteral("xml"), QCoreApplication::translate("main", "The file to transform"));
    parser.process(app);

    if (parser.positionalArguments().count() != 1) {
        parser.showHelp();
        return ( 1 );
    }

    exsltRegisterAll();

    // Need to set SRCDIR before calling setupStandardDirs
    QString srcdir;
    if (parser.isSet(QStringLiteral("srcdir")))
        srcdir = QDir(parser.value(QStringLiteral("srcdir"))).absolutePath();
    setupStandardDirs(srcdir);

    LIBXML_TEST_VERSION

    const QString checkFilename = parser.positionalArguments().first();
    CheckFileResult ckr = checkFile( checkFilename );
    if ( ckr != CheckFileSuccess )
    {
        if ( ckr == CheckFileDoesNotExist ) qWarning() << "File '" << checkFilename << "' does not exist.";
        else if ( ckr == CheckFileIsNotFile ) qWarning() << "'" << checkFilename << "' is not a file.";
        else if ( ckr == CheckFileIsNotReadable ) qWarning() << "File '" << checkFilename << "' is not readable.";
        return ( 2 );
    }

    if (parser.isSet(QStringLiteral("check"))) {

        QByteArray catalogs;
        catalogs += QUrl::fromLocalFile(locateFileInDtdResource(QStringLiteral("customization/catalog.xml"))).toEncoded();

        QString exe;
#if defined( XMLLINT )
        exe = QStringLiteral(XMLLINT);
#endif
        if ( !QFileInfo( exe ).isExecutable() ) {
            exe = QStandardPaths::findExecutable(QStringLiteral("xmllint"));
        }

        CheckResult cr = check( checkFilename, exe, catalogs );
        if ( cr != CheckSuccess )
        {
            if ( cr == CheckNoXmllint ) qWarning() << "couldn't find xmllint";
            return 1;
        }
    }

    xmlSubstituteEntitiesDefault(1);
    xmlLoadExtDtdDefaultValue = 1;

    QVector<const char *> params;
#ifndef Q_OS_WIN
    // libxslt parses the path given to outputFile as XPath expression which fails
    // see libxslt/xsltEvalUserParams
    // this parameter is used only by share/apps/ksgmltools2/docbook/xsl/html/math.xsl
    // and is not supported on windows yet
    if (parser.isSet(QStringLiteral("output"))) {
        params.append(qstrdup("outputFile"));
        params.append(qstrdup(parser.value(QStringLiteral("output")).toLocal8Bit().constData()));
    }
#endif
    {
        const QStringList paramList = parser.values(QStringLiteral("param"));
        QStringList::ConstIterator it = paramList.constBegin();
        QStringList::ConstIterator end = paramList.constEnd();
        for ( ; it != end; ++it ) {
            const QString tuple = *it;
            const int ch = tuple.indexOf( QLatin1Char('=') );
            if ( ch == -1 ) {
                qWarning() << "Key-Value tuple '" << tuple << "' lacks a '='!";
                return( 2 );
            }
            params.append( qstrdup( tuple.left( ch ).toUtf8().constData() ) );
            params.append( qstrdup( tuple.mid( ch + 1 ).toUtf8().constData() )  );
        }
    }
    params.append( NULL );

    bool index = parser.isSet(QStringLiteral("htdig"));
    QString tss = parser.value(QStringLiteral("stylesheet"));
    if ( tss.isEmpty() )
        tss = QStringLiteral("customization/kde-chunk.xsl");
    if ( index )
        tss = QStringLiteral("customization/htdig_index.xsl");

    tss = locateFileInDtdResource(tss);
    const QString cache = parser.value(QStringLiteral("cache"));
    const bool usingStdOut = parser.isSet(QStringLiteral("stdout"));
    const bool usingOutput = parser.isSet(QStringLiteral("output"));
    const QString outputOption = parser.value(QStringLiteral("output"));

    if ( index ) {
        xsltStylesheetPtr style_sheet =
            xsltParseStylesheetFile((const xmlChar *)tss.toLatin1().data());

        if (style_sheet != NULL) {

            xmlDocPtr doc = xmlParseFile( QFile::encodeName( checkFilename ).constData() );

            xmlDocPtr res = xsltApplyStylesheet(style_sheet, doc, &params[0]);

            xmlFreeDoc(doc);
            xsltFreeStylesheet(style_sheet);
            if (res != NULL) {
                xmlNodePtr cur = xmlDocGetRootElement(res);
                if (!cur || xmlStrcmp(cur->name, (const xmlChar *) "entry")) {
                    fprintf(stderr,"document of the wrong type, root node != entry");
                    xmlFreeDoc(res);
                    return(1);
                }
                PairList list;
                parseEntry( list, cur, 0 );
                int wi = 0;
                for ( PairList::ConstIterator it = list.constBegin(); it != list.constEnd();
                      ++it, ++wi )
                    fprintf( stdout, "w\t%s\t%d\t%d\n", ( *it ).word.toUtf8().data(),
                             1000*wi/list.count(), ( *it ).base );

                xmlFreeDoc(res);
            } else {
                qWarning() << "couldn't parse document " << checkFilename;
            }
        } else {
            qWarning() << "couldn't parse style sheet " << tss;
        }

    } else {
        QString output = transform(checkFilename , tss, params);
        if (output.isEmpty()) {
            fprintf(stderr, "unable to parse %s\n", checkFilename.toLocal8Bit().data());
            return(1);
        }

        if ( !cache.isEmpty() ) {
            if ( !saveToCache( output, cache ) ) {
                qWarning() << QCoreApplication::translate("main", "Could not write to cache file %1.").arg(cache);
            }
            goto end;
        }

        doOutput(output, usingStdOut, usingOutput, outputOption, true /* replaceCharset */);
    }
 end:
    xmlCleanupParser();
    xmlMemoryDump();
    return(0);
}

