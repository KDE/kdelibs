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
#include <qregexp.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <stdlib.h>
#include <kdebug.h>

extern int xmlLoadExtDtdDefaultValue;

static KCmdLineOptions options[] =
{
    { "stylesheet <xsl>",  I18N_NOOP( "Stylesheet to use" ), 0 },
    { "stdout", I18N_NOOP( "output whole document to stdout" ), 0 },
    { "+xml", I18N_NOOP("The file to transform"), 0},
    { 0, 0, 0 } // End of options.
};


int main(int argc, char **argv) {

    //    xsltSetGenericDebugFunc(stderr, NULL);

    KAboutData aboutData( "xml2man", I18N_NOOP("XML2MAN Processor" ),
	"$Id$",
	I18N_NOOP("KDE Translator for XML"));

    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineArgs::addCmdLineOptions( options );

    KGlobal::locale()->setMainCatalogue("kio_help");
    KInstance ins("xml2man");
    fillInstance(ins);

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if ( args->count() != 1 ) {
        args->usage();
        return ( 1 );
    }
    LIBXML_TEST_VERSION
    xmlSubstituteEntitiesDefault(1);
    xmlLoadExtDtdDefaultValue = 1;
    xmlSetExternalEntityLoader(meinExternalEntityLoader);

    QString tss = locate( "dtd", "db2man.xsl" );

    QString pat = args->arg( 0 );
    QFile xmlFile( pat );
    xmlFile.open(IO_ReadOnly);
    QCString contents;
    contents.assign(xmlFile.readAll());
    contents.truncate(xmlFile.size());
    xmlFile.close();

    contents.replace( QRegExp( "<!--[^-]*-->" ), "" );

    int index = 0;
    while ( true ) {
        int index1 = contents.find( ".", index );
        int index2 = contents.find( "-", index );
        int index3 = contents.find( "\"", index );
        if ( index1 == -1 && index2 == -1 && index3 == -1 )
            break;

        if ( index3 == -1 )
            index3 = contents.length();
        if ( index2 == -1 )
            index2 = contents.length();
        if ( index1 == -1 )
            index1 = contents.length();

        if ( index3 < index1 && index3 < index2 ) {
            index = contents.find( "\"", index3 + 1 ) + 2;
        } else {
            if ( index1 < index2 ) {
                contents.replace( index1, 1, "\\." );
                index = index1 + 2;
            } else {
                contents.replace( index2, 1, "\\-" );
                index = index2 + 2;
            }
        }
        if ( index >= (int)contents.length() )
            break;
    }

    xmlParserCtxtPtr ctxt = xmlCreateMemoryParserCtxt(contents.data(),
                                                      contents.length());
    int directory = pat.findRev('/');
    if (directory != -1)
        ctxt->directory = (char *)xmlStrdup((const xmlChar *)pat.left(directory + 1).latin1());
    QString output = transform(ctxt , tss );
    index = output.find( ".TH " );
    if ( index == -1 ) {
        kdError() << "needing article/title\n";
        return 1;
    }

    int endIndex = output.find( "\"", index + 5);
    QString name = output.mid( index + 5, endIndex - index - 5 );
    int sIndex = name.find( "(" );
    int eIndex = name.find( ")" );
    if ( sIndex == -1 || sIndex > eIndex ) {
        kdError() << "article/title should look like \"example (sect)\" in man pages\n";
        return 1;
    }
    QString sect = name.mid( sIndex + 1, eIndex - sIndex - 1).stripWhiteSpace();
    name = name.left( sIndex ).stripWhiteSpace();
    sIndex = output.find( "\"", endIndex + 1 ) + 1;
    eIndex = output.find( "\"", sIndex );
    QString date = output.mid( sIndex, eIndex - sIndex ).stripWhiteSpace();
    output.replace( index, eIndex - index + 1, QString(".TH \"%1\" %2 \"%3\" \"KDE Manual Page\" \"KDE\"" ).arg( name.left( sIndex - 1 ).stripWhiteSpace() ).arg( sect ).arg( date ) );

    output.replace( QRegExp( " *\n\n*" ), "\n" );
    kdDebug() << output << endl;

    xmlCleanupParser();
    xmlMemoryDump();
    return(0);
}

