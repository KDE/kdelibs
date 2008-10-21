#ifndef _WIN32
extern "C" int xmlLoadExtDtdDefaultValue;
#endif

#include <config-kdoctools.h>
#include <config.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
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
#include <QtCore/QString>
#include <kstandarddirs.h>
#include <kcomponentdata.h>
#include "xslt.h"
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <stdlib.h>
#include <kdebug.h>
#include <QtCore/QTextCodec>
#include <QtCore/QFileInfo>
#include <kshell.h>
#include <kurl.h>
#include <QtCore/QList>

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
            const QStringList wlist = words.simplified().split( ' ',QString::SkipEmptyParts );
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

    KCmdLineOptions options;
    options.add("stylesheet <xsl>", ki18n("Stylesheet to use"));
    options.add("stdout", ki18n("Output whole document to stdout"));
    options.add("o");
    options.add("output <file>", ki18n("Output whole document to file"));
    options.add("htdig", ki18n("Create a ht://dig compatible index"));
    options.add("check", ki18n("Check the document for validity"));
    options.add("cache <file>", ki18n("Create a cache file for the document"));
    options.add("srcdir <dir>", ki18n("Set the srcdir, for kdelibs"));
    options.add("param <key>=<value>", ki18n("Parameters to pass to the stylesheet"));
    options.add("+xml", ki18n("The file to transform"));

    KAboutData aboutData( "meinproc4", "kio_help4", ki18n("XML-Translator" ),
    "$Revision$",
    ki18n("KDE Translator for XML"));

    KCmdLineArgs::init(argc, argv, &aboutData, KCmdLineArgs::CmdLineArgKDE);
    KCmdLineArgs::addCmdLineOptions( options );

    KComponentData ins("kio_help4");
    KGlobal::locale();


    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if ( args->count() != 1 ) {
        args->usage();
        return ( 1 );
    }

    // Need to set SRCDIR before calling fillInstance
    QString srcdir;
    if ( args->isSet( "srcdir" ) )
        srcdir = QDir( args->getOption( "srcdir" ) ).absolutePath();
    fillInstance(ins,srcdir);

    LIBXML_TEST_VERSION

    QString checkFilename = args->arg( 0 );
    QFileInfo checkFile(checkFilename);
    if (!checkFile.exists())
    {
        kError() << "File '" << checkFilename << "' does not exist." << endl;
        return ( 2 );
    }
    if (!checkFile.isFile())
    {
        kError() << "'" << checkFilename << "' is not a file." << endl;
        return ( 2 );
    }
    if (!checkFile.isReadable())
    {
        kError() << "File '" << checkFilename << "' is not readable." << endl;
        return ( 2 );
    }

    if ( args->isSet( "check" ) ) {
        QString pwd_buffer = QDir::currentPath();
        QFileInfo file( args->arg( 0 ) );

        QByteArray catalogs;
        catalogs += KUrl::fromLocalFile( KStandardDirs::locate( "dtd", "customization/catalog.xml" ) ).toEncoded();
        catalogs += ' ';
        catalogs += KUrl::fromLocalFile( KStandardDirs::locate( "dtd", "docbook/xml-dtd-4.1.2/catalog.xml" ) ).toEncoded();

        setenv( "XML_CATALOG_FILES", catalogs.constData(), 1 );
        QString exe;
#if defined( XMLLINT )
        exe = XMLLINT;
#endif
        if ( !QFileInfo( exe ).isExecutable() ) {
            exe = KStandardDirs::findExe( "xmllint" );
            if (exe.isEmpty())
                exe = KStandardDirs::locate( "exe", "xmllint" );
        }
        if ( QFileInfo( exe ).isExecutable() ) {
            QDir::setCurrent( file.absolutePath() );
            QString cmd = exe;
            cmd += " --valid --noout ";
#ifdef Q_OS_WIN
            cmd += file.fileName();
#else
            cmd += KShell::quoteArg(file.fileName());
#endif
            cmd += " 2>&1";
            FILE *xmllint = popen( QFile::encodeName( cmd ).constData(), "r" );
            char buf[ 512 ];
            bool noout = true;
            unsigned int n;
            while ( ( n = fread(buf, 1, sizeof( buf ) - 1, xmllint ) ) ) {
                noout = false;
                buf[ n ] = '\0';
                fputs( buf, stderr );
            }
            pclose( xmllint );
            QDir::setCurrent( pwd_buffer );
            if ( !noout )
                return 1;
        } else {
            kWarning() << "couldn't find xmllint";
        }
    }

    xmlSubstituteEntitiesDefault(1);
    xmlLoadExtDtdDefaultValue = 1;

    QVector<const char *> params;
#ifndef Q_WS_WIN 
    // libxslt parses the path given to outputFile as XPath expression which fails 
    // see libxslt/xsltEvalUserParams
    // this parameter is used only by share/apps/ksgmltools2/docbook/xsl/html/math.xsl 
    // and is not supported on windows yet
    if (args->isSet( "output" ) ) {
        params.append( qstrdup( "outputFile" ) );
        params.append( qstrdup( args->getOption( "output" ).toLocal8Bit() ) );
    }
#endif
    {
        const QStringList paramList = args->getOptionList( "param" );
        QStringList::ConstIterator it = paramList.begin();
        QStringList::ConstIterator end = paramList.end();
        for ( ; it != end; ++it ) {
            const QString tuple = *it;
            const int ch = tuple.indexOf( '=' );
            if ( ch == -1 ) {
                kError() << "Key-Value tuple '" << tuple << "' lacks a '='!" << endl;
                return( 2 );
            }
            params.append( qstrdup( tuple.left( ch ).toUtf8() ) );
            params.append( qstrdup( tuple.mid( ch + 1 ).toUtf8() )  );
        }
    }
    params.append( NULL );

    bool index = args->isSet( "htdig" );
    QString tss = args->getOption( "stylesheet" );
    if ( tss.isEmpty() )
        tss =  "customization/kde-chunk.xsl";
    if ( index )
        tss = "customization/htdig_index.xsl" ;

    tss = KStandardDirs::locate( "dtd", tss );

    if ( index ) {
        xsltStylesheetPtr style_sheet =
            xsltParseStylesheetFile((const xmlChar *)tss.toLatin1().data());

        if (style_sheet != NULL) {

            xmlDocPtr doc = xmlParseFile( QFile::encodeName( args->arg( 0 ) ).constData() );

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
                kDebug() << "couldn't parse document " << args->arg( 0 );
            }
        } else {
            kDebug() << "couldn't parse style sheet " << tss;
        }

    } else {
        QString output = transform(args->arg( 0 ) , tss, params);
        if (output.isEmpty()) {
            fprintf(stderr, "unable to parse %s\n", args->arg( 0 ).toLocal8Bit().data());
            return(1);
        }

        QString cache = args->getOption( "cache" );
        if ( !cache.isEmpty() ) {
            if ( !saveToCache( output, cache ) ) {
                kError() << i18n( "Could not write to cache file %1." ,  cache ) << endl;
            }
            goto end;
        }

        if (output.indexOf( "<FILENAME " ) == -1 || args->isSet( "stdout" ) || args->isSet("output") )
        {
            QFile file;
            if (args->isSet( "stdout" ) ) {
                file.open( stdout, QIODevice::WriteOnly );
            } else {
                if (args->isSet( "output" ) )
                   file.setFileName( args->getOption( "output" ));
                else
                   file.setFileName( "index.html" );
                file.open(QIODevice::WriteOnly);
            }
            replaceCharsetHeader( output );

            QByteArray data = output.toLocal8Bit();
            file.write(data.data(), data.length());
            file.close();
        } else {
            int index = 0;
            while (true) {
                index = output.indexOf("<FILENAME ", index);
                if (index == -1)
                    break;
                int filename_index = index + strlen("<FILENAME filename=\"");

                QString filename = output.mid(filename_index,
                                              output.indexOf("\"", filename_index) -
                                              filename_index);

                QString filedata = splitOut(output, index);
                QFile file(filename);
                file.open(QIODevice::WriteOnly);
                replaceCharsetHeader( filedata );
                QByteArray data = fromUnicode( filedata );
                file.write(data.data(), data.length());
                file.close();

                index += 8;
            }
        }
    }
 end:
    xmlCleanupParser();
    xmlMemoryDump();
    return(0);
}

