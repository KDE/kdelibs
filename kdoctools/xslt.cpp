#include <libxslt/xsltconfig.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <libxml/xmlIO.h>
#include <libxml/parserInternals.h>
#include <kdebug.h>
#include <kstddirs.h>
#include <qregexp.h>
#include <qdir.h>
#include <xslt.h>
#include <kinstance.h>
#include "kio_help.h"
#include <klocale.h>
#include <assert.h>
#include <kfilterbase.h>
#include <kfilterdev.h>
#include <qtextcodec.h>
#include <stdlib.h>
#include <config.h>
#include <stdarg.h>
#include <klibloader.h>
#include <kcharsets.h>
#include <gzip/kgzipfilter.h>
#include <bzip2/kbzip2filter.h>

#if !defined( SIMPLE_XSLT )
extern HelpProtocol *slave;
#define INFO( x ) if (slave) slave->infoMessage(x);
#else
#define INFO( x )
#endif

extern "C" void warningsFunc(void *ctx, const char *msg, ...);

bool warnings_exist = false;

void warningsFunc(void *ctx, const char *msg, ...)
{
    xmlParserCtxtPtr ctxt = (xmlParserCtxtPtr) ctx;
    xmlParserInputPtr input;
    xmlParserInputPtr cur = NULL;

    input = ctxt->input;
    if ((input != NULL) && (input->filename == NULL) &&
        (ctxt->inputNr > 1))
    {
        cur = input;
        input = ctxt->inputTab[ctxt->inputNr - 2];
    }

    va_list args;

    char buffer[50000];
    buffer[0] = 0;

    if (input->filename) {
        sprintf(&buffer[strlen(buffer)], "%s:%d: ", input->filename,
                input->line);
    } else {
        sprintf(&buffer[strlen(buffer)], "Entity: line %d: ", input->line);
    }

    va_start(args, msg);
    vsprintf(&buffer[strlen(buffer)], msg, args);
    va_end(args);

    fprintf( stderr, "%s", buffer );
    xmlParserPrintFileContext(input);

    warnings_exist = true;
}

int writeToQString(void * context, const char * buffer, int len)
{
    QString *t = (QString*)context;
    *t += QString::fromUtf8(buffer, len);
    return len;
}

void closeQString(void * context) {
    QString *t = (QString*)context;
    *t += QString::fromLatin1("\n");
}

QString transform( const QString &pat, const QString& tss)
{
    INFO(i18n("Reading document"));
    QFile xmlFile( pat );
    xmlFile.open(IO_ReadOnly);
    QCString contents;
    contents.assign(xmlFile.readAll());
    contents.truncate(xmlFile.size());
    xmlFile.close();
    /* if (contents.left(5) != "<?xml") {
        fprintf(stderr, "xmlizer\n");
        INFO(i18n("XMLize document"));
        QString cmd = "xmlizer ";
        cmd += KProcess::quote(pat);
        FILE *p = popen(QFile::encodeName(cmd), "r");
        xmlFile.open(IO_ReadOnly, p);
        char buffer[5001];
        contents.truncate(0);
        int len;
        while ((len = xmlFile.readBlock(buffer, 5000)) != 0) {
            buffer[len] = 0;
            contents += buffer;
        }
        xmlFile.close();
        pclose(p);
    } */

    INFO(i18n("Parsing document"));
    xmlParserCtxtPtr ctxt = xmlCreateMemoryParserCtxt
                            (contents.data(),
                             contents.length());
    ctxt->sax->warning = warningsFunc;

    int directory = pat.findRev('/');
    if (directory != -1)
        ctxt->directory = (char *)xmlStrdup((const xmlChar *)pat.
                                            left(directory + 1).latin1());

    return transform( ctxt, tss );
}

QString transform(xmlParserCtxtPtr ctxt, const QString &tss)
{
    QString parsed;

    warnings_exist = false;

    INFO(i18n("Parsing stylesheet"));

    xsltStylesheetPtr style_sheet =
        xsltParseStylesheetFile((const xmlChar *)tss.latin1());

    if ( !style_sheet ) {
        return parsed;
    }

    if (style_sheet->indent == 1)
        xmlIndentTreeOutput = 1;
    else
        xmlIndentTreeOutput = 0;

    xmlParseDocument(ctxt);
    xmlDocPtr doc;

    if (ctxt->wellFormed)
        doc = ctxt->myDoc;
    else {
        xmlFreeDoc(ctxt->myDoc);
        xmlFreeParserCtxt(ctxt);
        return parsed;
    }

    xmlFreeParserCtxt(ctxt);

    // the params can be used to customize it more flexible
    const char *params[16 + 1];
    params[0] = NULL;
    INFO(i18n("Applying stylesheet"));
    xmlDocPtr res = xsltApplyStylesheet(style_sheet, doc, params);
    xmlFreeDoc(doc);
    if (res != NULL) {
        xmlOutputBufferPtr outp = xmlOutputBufferCreateIO(writeToQString, (xmlOutputCloseCallback)closeQString, &parsed, 0);
        outp->written = 0;
        INFO(i18n("Writing document"));
        xsltSaveResultTo ( outp, res, style_sheet );
        xmlOutputBufferFlush(outp);
        xmlFreeDoc(res);
    }
    xsltFreeStylesheet(style_sheet);

    return parsed;
}

xmlParserInputPtr meinExternalEntityLoader(const char *URL, const char *ID,
					   xmlParserCtxtPtr ctxt) {
    xmlParserInputPtr ret = NULL;

    // fprintf(stderr, "loading %s %s %s\n", URL, ID, ctxt->directory);

    if (URL == NULL) {
        if ((ctxt->sax != NULL) && (ctxt->sax->warning != NULL))
            ctxt->sax->warning(ctxt,
                    "failed to load external entity \"%s\"\n", ID);
        return(NULL);
    }
    if (ID && !strcmp(ID, "-//OASIS//DTD DocBook XML V4.1.2//EN"))
        URL = "docbook/xml-dtd-4.1.2/docbookx.dtd";
    if (ID && !strcmp(ID, "-//OASIS//DTD XML DocBook V4.1.2//EN"))
        URL = "docbook/xml-dtd-4.1.2/docbookx.dtd";
    if (ID && !strcmp(ID, "-//KDE//DTD DocBook XML V4.1-Based Variant V1.0//EN"))
        URL = "customization/dtd/kdex.dtd";
    if (ID && !strcmp(ID, "-//KDE//DTD DocBook XML V4.1.2-Based Variant V1.0//EN"))
        URL = "customization/dtd/kdex.dtd";

    QString file;
    if (KStandardDirs::exists( QDir::currentDirPath() + "/" + URL ) )
        file = QDir::currentDirPath() + "/" + URL;
    else
        file = locate("dtd", URL);

    ret = xmlNewInputFromFile(ctxt, file.latin1());
    if (ret == NULL) {
        if ((ctxt->sax != NULL) && (ctxt->sax->warning != NULL))
            ctxt->sax->warning(ctxt,

                "failed to load external entity \"%s\"\n", URL);
    }
    return(ret);
}

QString splitOut(const QString &parsed, int index)
{
    int start_index = index + 1;
    while (parsed.at(start_index - 1) != '>') start_index++;

    int inside = 0;

    QString filedata;

    while (true) {
        int endindex = parsed.find("</FILENAME>", index);
        int startindex = parsed.find("<FILENAME ", index) + 1;

//        kdDebug() << "FILENAME " << startindex << " " << endindex << " " << inside << " " << parsed.mid(startindex + 18, 15)<< " " << parsed.length() << endl;

        if (startindex > 0) {
            if (startindex < endindex) {
                //              kdDebug() << "finding another" << endl;
                index = startindex + 8;
                inside++;
            } else {
                index = endindex + 8;
                inside--;
            }
        } else {
            inside--;
            index = endindex + 1;
        }

        if (inside == 0) {
            filedata = parsed.mid(start_index, endindex - start_index);
            break;
        }

    }

    index = filedata.find("<FILENAME ");

    if (index > 0) {
        int endindex = filedata.findRev("</FILENAME>");
        while (filedata.at(endindex) != '>') endindex++;
        endindex++;
        filedata = filedata.left(index) + filedata.mid(endindex);
    }

    // filedata.replace(QRegExp(">"), "\n>");
    return filedata;
}

void fillInstance(KInstance &ins) {
    if ( !getenv( "KDELIBS_UNINSTALLED" ) ) {
        ins.dirs()->addResourceType("dtd", KStandardDirs::kde_default("data") + "ksgmltools2/");
    }
    ins.dirs()->addResourceDir( "dtd", SRCDIR );
}

static KFilterBase *filter = 0;

static KFilterBase *findFilterByFileName( const QString &filename )
{
    if ( filter )
        return filter;

    if ( filename.right( 4 ) == ".bz2" ) {
#if defined( HAVE_BZIP2_SUPPORT )
        filter = new KBzip2Filter;
#endif
    }

    if ( !filter )
        filter = new KGzipFilter;

    return filter;
}

bool saveToCache( const QString &contents, const QString &filename )
{
    QFile raw(filename);
    KFilterBase *f = ::findFilterByFileName(filename);
    if ( !f )
        return false;

    QIODevice *fd= KFilterDev::createFilterDevice(f, &raw);

    if (!fd->open(IO_WriteOnly))
    {
       delete fd;
       return false;
    }

    fd->writeBlock( contents.utf8() );
    fd->close();
    delete fd;
    return true;
}

static bool readCache( const QString &filename,
                       const QString &cache, QString &output)
{
    kdDebug( 7119 ) << "verifyCache " << filename << " " << cache << endl;
    if ( !compareTimeStamps( filename, cache ) )
        return false;
    if ( !compareTimeStamps( locate( "dtd", "customization/kde-chunk.xsl"), cache ) )
        return false;

    kdDebug( 7119 ) << "create filter" << endl;
    QFile raw(cache);
    KFilterBase *f = ::findFilterByFileName(cache);
    QIODevice *fd= KFilterDev::createFilterDevice(f, &raw);

    if (!fd->open(IO_ReadOnly))
    {
       delete fd;
       ::unlink( cache.local8Bit() );
       return false;
    }

    kdDebug( 7119 ) << "reading" << endl;

    char buffer[32000];
    int n;
    QCString text;
    while ( ( n = fd->readBlock(buffer, 31900) ) )
    {
        buffer[n] = 0;
        text += buffer;
    }
    kdDebug( 7119 ) << "read " << text.length() << endl;
    fd->close();

    output = QString::fromUtf8( text );
    delete fd;
    kdDebug( 7119 ) << "finished " << endl;

    return true;
}

QString lookForCache( const QString &filename )
{
    kdDebug() << "lookForCache " << filename << endl;
    assert( filename.right( 8 ) == ".docbook" );
    assert( filename.at( 0 ) == '/' );

    QString cache = filename.left( filename.length() - 7 );
    QString output;
    if ( readCache( filename, cache + "cache.bz2", output) )
        return output;
    if ( readCache( filename,
                    locateLocal( "data",
                                 "kio_help/cache" + cache +
                                 "cache.bz2" ), output ) )
        return output;

    return QString::null;
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

QCString fromUnicode( const QString &data )
{
    QTextCodec *locale = QTextCodec::codecForLocale();
    QCString result;
    char buffer[30000];
    uint buffer_len = 0;
    uint len = 0;
    uint offset = 0;
    const int part_len = 5000;

    QString part;

    while ( offset < data.length() )
    {
        part = data.mid( offset, part_len );
        QCString test = locale->fromUnicode( part );
        if ( locale->toUnicode( test ) == part ) {
            result += test;
            offset += part_len;
            continue;
        }
        len = part.length();
        buffer_len = 0;
        for ( uint i = 0; i < len; i++ ) {
            QCString test = locale->fromUnicode( part.mid( i, 1 ) );
            if ( locale->toUnicode( test ) == part.mid( i, 1 ) ) {
                if (buffer_len + test.length() + 1 > sizeof(buffer))
                   break;
                strcpy( buffer + buffer_len, test.data() );
                buffer_len += test.length();
            } else {
                QString res;
                res.sprintf( "&#%d;", part.at( i ).unicode() );
                test = locale->fromUnicode( res );
                if (buffer_len + test.length() + 1 > sizeof(buffer))
                   break;
                strcpy( buffer + buffer_len, test.data() );
                buffer_len += test.length();
            }
        }
        result += QCString( buffer, buffer_len + 1);
        offset += part_len;
    }
    return result;
}

void replaceCharsetHeader( QString &output )
{
    QString name = QTextCodec::codecForLocale()->name();
    name.replace( QRegExp( "ISO " ), "iso-" );
    output.replace( QRegExp( "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">" ),
                    QString( "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=%1\">" ).arg( name  ) );
}
