#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <libxml/xmlIO.h>
#include <libxml/parserInternals.h>
#include <kdebug.h>
#include <kstddirs.h>
#include <qdir.h>
#include <xslt.h>
#include <kinstance.h>
#include "kio_help.h"
#include <klocale.h>
#include <assert.h>
#include <kfilterbase.h>
#include <kfilterdev.h>
#include <qtextcodec.h>

#if !defined( SIMPLE_XSLT )
extern HelpProtocol *slave;
#define INFO( x ) if (slave) slave->infoMessage(x);
#else
#define INFO( x )
#endif

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
    QString tmp;
    if (contents.left(5) != "<?xml") {
        fprintf(stderr, "xmlizer\n");
        INFO(i18n("XMLize document"));
        FILE *p = popen(QString::fromLatin1("xmlizer %1").arg(pat).latin1(), "r");
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
    }

    INFO(i18n("Parsing document"));
    xmlParserCtxtPtr ctxt = xmlCreateMemoryParserCtxt
                            (contents.data(),
                                                      contents.length());
    int directory = pat.findRev('/');
    if (directory != -1)
        ctxt->directory = (char *)xmlStrdup((const xmlChar *)pat.
                                            left(directory + 1).latin1());

    return transform( ctxt, tss );
}

QString transform(xmlParserCtxtPtr ctxt, const QString &tss)
{
    QString parsed;

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
        xmlOutputBufferPtr outp = xmlOutputBufferCreateIO(writeToQString, closeQString, &parsed, 0);
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
    if (!strcmp(ID, "-//OASIS//DTD DocBook XML V4.1.2//EN"))
        URL = "docbook/xml-dtd-4.1.2/docbookx.dtd";
    if (!strcmp(ID, "-//OASIS//DTD XML DocBook V4.1.2//EN"))
	URL = "docbook/xml-dtd-4.1.2/docbookx.dtd";
    if (!strcmp(ID, "-//KDE//DTD DocBook XML V4.1-Based Variant V1.0//EN"))
        URL = "customization/dtd/kdex.dtd";
    if (!strcmp(ID, "-//KDE//DTD DocBook XML V4.1.2-Based Variant V1.0//EN"))
        URL = "customization/dtd/kdex.dtd";

    QString file = locate("dtd", URL);
    if (!file.isEmpty())
        ret = xmlNewInputFromFile(ctxt, file.latin1());
    else {
        if (KStandardDirs::exists( QDir::currentDirPath() + "/" + URL ) )
            file = QDir::currentDirPath() + "/" + URL;
        ret = xmlNewInputFromFile(ctxt, file.latin1());
    }
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
    ins.dirs()->addResourceType("dtd", KStandardDirs::kde_default("data") + "ksgmltools2/");
}

bool saveToCache( const QString &contents, const QString &filename )
{
    QFile raw(filename);
    KFilterBase *f = KFilterBase::findFilterByFileName(filename);
    QIODevice *fd= KFilterDev::createFilterDevice(f, &raw);

    if (!fd->open(IO_WriteOnly))
    {
       delete f;
       delete fd;
       return false;
    }

    fd->writeBlock( contents.utf8() );
    fd->close();
    delete fd;
    delete f;
    return true;
}

static bool readCache( const QString &filename,
                       const QString &cache, QString &output)
{
    kdDebug() << "verifyCache " << filename << " " << cache << endl;
    if ( !KStandardDirs::exists( cache ) )
        return false;
    // TODO check time stamps

    QFile raw(cache);
    KFilterBase *f = KFilterBase::findFilterByFileName(cache);
    QIODevice *fd= KFilterDev::createFilterDevice(f, &raw);

    if (!fd->open(IO_ReadOnly))
    {
       delete f;
       delete fd;
       ::unlink( cache.local8Bit() );
       return false;
    }

    char buffer[1025];
    int n;
    QCString text;
    while ( ( n = fd->readBlock(buffer, 1024) ) )
    {
        buffer[n] = 0;
        text += buffer;
    }
    kdDebug() << "read " << text.length() << endl;
    fd->close();

    output = QString::fromUtf8( text );
    delete fd;
    delete f;

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
    if ( readCache( filename, cache + "cache.gz", output ) )
        return output;
    if ( readCache( filename,
                    locateLocal( "data",
                                 "kio_help/cache" + cache +
                                 "cache.bz2" ), output ) )
        return output;
    if ( readCache( filename,
                    locateLocal( "data",
                                 "kio_help/cache" + cache +
                                 "cache.gz" ), output ) )
        return output;

    return QString::null;
}
