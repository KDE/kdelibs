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
#include <qstring.h>
#include <kstddirs.h>
#include <kinstance.h>
#include <xslt.h>
#include <qfile.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <stdlib.h>
#include <kdebug.h>
#include <qtextcodec.h>
#include <qfileinfo.h>

extern int xmlLoadExtDtdDefaultValue;

class MyPair {
public:
    QString word;
    int base;};

typedef QValueList<MyPair> PairList;

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
            QStringList wlist = QStringList::split( ' ',  words.simplifyWhiteSpace() );
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

static KCmdLineOptions options[] =
{
    { "stylesheet <xsl>",  I18N_NOOP( "Stylesheet to use" ), 0 },
    { "stdout", I18N_NOOP( "output whole document to stdout" ), 0 },
    { "htdig", I18N_NOOP( "create a ht://dig compatible index" ), 0 },
    { "check", I18N_NOOP( "check the document for validity" ), 0 },
    { "cache <file>", I18N_NOOP( "create a cache file for the document" ), 0},
    { "+xml", I18N_NOOP("The file to transform"), 0},
    { 0, 0, 0 } // End of options.
};




int main(int argc, char **argv) {

    // xsltSetGenericDebugFunc(stderr, NULL);

    KAboutData aboutData( "meinproc", I18N_NOOP("XML-Translator" ),
	"$Id$",
	I18N_NOOP("KDE Translator for XML"));

    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineArgs::addCmdLineOptions( options );

    KLocale::setMainCatalogue("kio_help");
    KInstance ins("meinproc");
    KGlobal::locale();

    fillInstance(ins);

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if ( args->count() != 1 ) {
        args->usage();
        return ( 1 );
    }

    LIBXML_TEST_VERSION

    QString checkFilename = QFile::decodeName(args->arg( 0 ));
    QFileInfo checkFile(checkFilename);
    if (!checkFile.exists())
    {
        kdError() << "File '" << checkFilename << "' does not exist." << endl;
        return ( 2 );
    }
    if (!checkFile.isFile())
    {
        kdError() << "'" << checkFilename << "' is not a file." << endl;
        return ( 2 );
    }
    if (!checkFile.isReadable())
    {
        kdError() << "File '" << checkFilename << "' is not readable." << endl;
        return ( 2 );
    }

    if ( args->isSet( "check" ) ) {
        char pwd_buffer[250];
        QFileInfo file( QFile::decodeName(args->arg( 0 )) );
        getcwd( pwd_buffer, 250 );
        chdir( QFile::encodeName( file.dirPath( true ) ) );

        QString catalogs;
        catalogs += locate( "dtd", "customization/catalog" );
        catalogs += ":";
        catalogs += locate( "dtd", "docbook/xml-dtd-4.1.2/docbook.cat" );

        setenv( "SGML_CATALOG_FILES", QFile::encodeName( catalogs ).data(), 1);
        QString exe;
#if defined( XMLLINT )
        exe = XMLLINT;
#endif
        if ( ::access( QFile::encodeName( exe ), X_OK ) ) {
            exe = KStandardDirs::findExe( "xmllint" );
            if (exe.isEmpty())
                exe = locate( "exe", "xmllint" );
        }
        if ( !::access( QFile::encodeName( exe ), X_OK ) ) {
            FILE *xmllint = popen( QString( exe + " --catalogs --valid --noout %1 2>&1" ).arg( file.fileName() ).local8Bit().data(), "r");
            bool noout = true;
            while ( !feof( xmllint ) ) {
                int c;
                c = fgetc( xmllint );
                if ( c != EOF ) {
                    fputc( c, stderr );
                    noout = false;
                }
            }
            pclose( xmllint );
            chdir( pwd_buffer );
            if ( !noout )
                return 1;
        } else {
            kdWarning() << "couldn't find xmllint" << endl;
        }
    }

    xmlSubstituteEntitiesDefault(1);
    xmlLoadExtDtdDefaultValue = 1;
    xmlSetExternalEntityLoader(meinExternalEntityLoader);

    bool index = args->isSet( "htdig" );
    QString tss = args->getOption( "stylesheet" );
    if ( tss.isEmpty() )
        tss =  "customization/kde-chunk.xsl";
    if ( index )
        tss = "customization/htdig_index.xsl" ;

    tss = locate( "dtd", tss );

    if ( index ) {
        xsltStylesheetPtr style_sheet =
            xsltParseStylesheetFile((const xmlChar *)tss.latin1());

        if (style_sheet != NULL) {

            xmlDocPtr doc = xmlParseFile( QFile::encodeName( args->arg( 0 ) ) );

            // the params can be used to customize it more flexible
            const char *params[16 + 1];
            params[0] = NULL;
            xmlDocPtr res = xsltApplyStylesheet(style_sheet, doc, params);

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
                for ( PairList::ConstIterator it = list.begin(); it != list.end();
                      ++it, ++wi )
                    fprintf( stdout, "w\t%s\t%d\t%d\n", ( *it ).word.utf8().data(),
                             1000*wi/list.count(), ( *it ).base );

                xmlFreeDoc(res);
            } else {
                kdDebug() << "couldn't parse document " << args->arg( 0 ) << endl;
            }
        } else {
            kdDebug() << "couldn't parse style sheet " << tss << endl;
        }

    } else {
        QString output = transform(args->arg( 0 ) , tss);
        if (output.isEmpty()) {
            fprintf(stderr, "unable to parse %s\n", args->arg( 0 ));
            return(1);
        }

        if ( warnings_exist )
            return( 1 );

        QString cache = args->getOption( "cache" );
        if ( !cache.isEmpty() ) {
            if ( !saveToCache( output, cache ) ) {
                kdError() << i18n( "Couldn't write to cache file %1." ).arg( cache ) << endl;
            }
            goto end;
        }

        if (output.find( "<FILENAME " ) == -1 || args->isSet( "stdout" ))
        {
            QFile file;
            if (args->isSet( "stdout" ) ) {
                file.open( IO_WriteOnly, stdout );
            } else {
                file.setName( "index.html" );
                file.open(IO_WriteOnly);
            }
            replaceCharsetHeader( output );

            QCString data = output.local8Bit();
            file.writeBlock(data.data(), data.length());
            file.close();
        } else {
            int index = 0;
            while (true) {
                index = output.find("<FILENAME ", index);
                if (index == -1)
                    break;
                int filename_index = index + strlen("<FILENAME filename=\"");

                QString filename = output.mid(filename_index,
                                              output.find("\"", filename_index) -
                                              filename_index);

                QString filedata = splitOut(output, index);
                QFile file(filename);
                file.open(IO_WriteOnly);
                replaceCharsetHeader( filedata );
                QCString data = fromUnicode( filedata );
                file.writeBlock(data.data(), data.length());
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

