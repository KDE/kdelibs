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

int closeQString(void * context) {
    QString *t = (QString*)context;
    *t += '\n';
    return 0;
}

QString transform( const QString &pat, const QString& tss,
                   const QVector<const char *> &params )
{
    QString parsed;

    INFO(i18n("Parsing stylesheet"));

    xsltStylesheetPtr style_sheet =
        xsltParseStylesheetFile((const xmlChar *)tss.toLatin1().data());

    if ( !style_sheet ) {
        return parsed;
    }
    if (style_sheet->indent == 1)
        xmlIndentTreeOutput = 1;
    else
        xmlIndentTreeOutput = 0;

    INFO(i18n("Parsing document"));

    xmlDocPtr doc = xmlParseFile( pat.toLatin1() );
    xsltTransformContextPtr ctxt;

    ctxt = xsltNewTransformContext(style_sheet, doc);
    if (ctxt == NULL)
        return parsed;

    INFO(i18n("Applying stylesheet"));
    QVector<const char *> p = params;
    p.append( NULL );
    xmlDocPtr res = xsltApplyStylesheet(style_sheet, doc, const_cast<const char **>(&p[0]));
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

    if (parsed.isEmpty())
	parsed = " "; // avoid error message
    return parsed;
}

/*
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
    if (!qstrcmp(ID, "-//OASIS//DTD DocBook XML V4.1.2//EN"))
        URL = "docbook/xml-dtd-4.1.2/docbookx.dtd";
    if (!qstrcmp(ID, "-//OASIS//DTD XML DocBook V4.1.2//EN"))
	URL = "docbook/xml-dtd-4.1.2/docbookx.dtd";

    QString file;
    if (KStandardDirs::exists( QDir::currentPath() + "/" + URL ) )
        file = QDir::currentPath() + "/" + URL;
    else
        file = locate("dtd", URL);

    ret = xmlNewInputFromFile(ctxt, file.toLatin1().constData());
    if (ret == NULL) {
        if ((ctxt->sax != NULL) && (ctxt->sax->warning != NULL))
            ctxt->sax->warning(ctxt,

                "failed to load external entity \"%s\"\n", URL);
    }
    return(ret);
}
*/

QString splitOut(const QString &parsed, int index)
{
    int start_index = index + 1;
    while (parsed.at(start_index - 1) != '>') start_index++;

    int inside = 0;

    QString filedata;

    while (true) {
        int endindex = parsed.indexOf("</FILENAME>", index);
        int startindex = parsed.indexOf("<FILENAME ", index) + 1;

//        kDebug() << "FILENAME " << startindex << " " << endindex << " " << inside << " " << parsed.mid(startindex + 18, 15)<< " " << parsed.length();

        if (startindex > 0) {
            if (startindex < endindex) {
                //              kDebug() << "finding another";
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

    index = filedata.indexOf("<FILENAME ");

    if (index > 0) {
        int endindex = filedata.lastIndexOf("</FILENAME>");
        while (filedata.at(endindex) != '>') endindex++;
        endindex++;
        filedata = filedata.left(index) + filedata.mid(endindex);
    }

    // filedata.replace(QRegExp(">"), "\n>");
    return filedata;
}

void fillInstance(KComponentData &ins, const QString &srcdir)
{
    QByteArray catalogs;

    if ( srcdir.isEmpty() ) {
        catalogs += KUrl::fromLocalFile( ins.dirs()->findResource("data", "ksgmltools2/customization/catalog.xml") ).toEncoded();
        catalogs += ' ';
        catalogs += KUrl::fromLocalFile( ins.dirs()->findResource("data", "ksgmltools2/docbook/xml-dtd-4.2/catalog.xml") ).toEncoded();
        ins.dirs()->addResourceType("dtd", "data", "ksgmltools2/");
    } else {
        catalogs += KUrl::fromLocalFile( srcdir +"/customization/catalog.xml" ).toEncoded();
        catalogs += ' ';
        catalogs += KUrl::fromLocalFile( srcdir + "/docbook/xml-dtd-4.2/catalog.xml" ).toEncoded();
        ins.dirs()->addResourceDir("dtd", srcdir);
    }

    setenv( "XML_CATALOG_FILES", catalogs.constData(), 1 );
    xmlInitializeCatalog();
}

static QIODevice *getBZip2device(const QString &fileName )
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
    assert( filename.endsWith( ".docbook" ) );
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

QByteArray fromUnicode( const QString &data )
{
#ifdef Q_WS_WIN
    return data.toUtf8();
#else
    QTextCodec *locale = QTextCodec::codecForLocale();
    QByteArray result;
    char buffer[30000];
    uint buffer_len = 0;
    uint len = 0;
    int offset = 0;
    const int part_len = 5000;

    QString part;

    while ( offset < data.length() )
    {
        part = data.mid( offset, part_len );
        QByteArray test = locale->fromUnicode( part );
        if ( locale->toUnicode( test ) == part ) {
            result += test;
            offset += part_len;
            continue;
        }
        len = part.length();
        buffer_len = 0;
        for ( uint i = 0; i < len; i++ ) {
            QByteArray test = locale->fromUnicode( part.mid( i, 1 ) );
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
        result += QByteArray( buffer, buffer_len + 1);
        offset += part_len;
    }
    return result;
#endif	
}

void replaceCharsetHeader( QString &output )
{
    QString name;
#ifdef Q_WS_WIN
    name = "utf-8"; 
    // may be required for all xml output 
    if (output.contains("<table-of-contents>"))
        output.replace( QString( "<?xml version=\"1.0\"?>" ),
                        QString( "<?xml version=\"1.0\" encoding=\"%1\"?>").arg( name ) );
#else                    
    name = QTextCodec::codecForLocale()->name();
    name.replace( QString( "ISO " ), "iso-" );
    output.replace( QString( "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">" ),
                    QString( "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=%1\">" ).arg( name ) );
#endif    
}
