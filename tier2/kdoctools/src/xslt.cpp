#include "xslt.h"

#include <libxslt/xsltconfig.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <libxml/xmlIO.h>
#include <libxml/parserInternals.h>
#include <libxml/catalog.h>

#include <QtCore/QDate>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QRegExp>
#include <QtCore/QStandardPaths>
#include <QtCore/QTextCodec>
#include <QtCore/QUrl>
#include <QtCore/QDebug>

#ifdef Q_OS_WIN
#include <config-kdoctools.h>
#include <QtCore/QHash>
#endif

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
    *t += QLatin1Char('\n');
    return 0;
}

#if defined (SIMPLE_XSLT) && defined(Q_OS_WIN)

#define MAX_PATHS 64
xmlExternalEntityLoader defaultEntityLoader = NULL;
static xmlChar *paths[MAX_PATHS + 1];
static int nbpaths = 0;
static QHash<QString,QString> replaceURLList;

/*
* Entity loading control and customization.
* taken from xsltproc.c
*/
static xmlParserInputPtr xsltprocExternalEntityLoader(const char *_URL, const char *ID,xmlParserCtxtPtr ctxt)
{
    xmlParserInputPtr ret;
    warningSAXFunc warning = NULL;

    // use local available dtd versions instead of fetching it every time from the internet
	QString url = QLatin1String(_URL);
	QHash<QString, QString>::const_iterator i;
	for(i = replaceURLList.constBegin(); i != replaceURLList.constEnd(); i++)
	{
		if (url.startsWith(i.key()))
		{
			url.replace(i.key(),i.value());
			qDebug() << "converted" << _URL << "to" << url;
		}
	}
	char URL[1024];
	strcpy(URL,url.toLatin1().constData());

    const char *lastsegment = URL;
    const char *iter = URL;

    if (nbpaths > 0) {
        while (*iter != 0) {
            if (*iter == '/')
            lastsegment = iter + 1;
            iter++;
        }
    }

    if ((ctxt != NULL) && (ctxt->sax != NULL)) {
        warning = ctxt->sax->warning;
        ctxt->sax->warning = NULL;
    }

    if (defaultEntityLoader != NULL) {
        ret = defaultEntityLoader(URL, ID, ctxt);
        if (ret != NULL) {
            if (warning != NULL)
                ctxt->sax->warning = warning;
            qDebug() << "Loaded URL=\"" << URL << "\" ID=\"" << ID << "\"";
            return(ret);
        }
    }
    for (int i = 0;i < nbpaths;i++) {
        xmlChar *newURL;

        newURL = xmlStrdup((const xmlChar *) paths[i]);
        newURL = xmlStrcat(newURL, (const xmlChar *) "/");
        newURL = xmlStrcat(newURL, (const xmlChar *) lastsegment);
        if (newURL != NULL) {
            ret = defaultEntityLoader((const char *)newURL, ID, ctxt);
        if (ret != NULL) {
            if (warning != NULL)
                ctxt->sax->warning = warning;
                qDebug() << "Loaded URL=\"" << newURL << "\" ID=\"" << ID << "\"";
                xmlFree(newURL);
                return(ret);
            }
            xmlFree(newURL);
        }
    }
    if (warning != NULL) {
        ctxt->sax->warning = warning;
        if (URL != NULL)
            warning(ctxt, "failed to load external entity \"%s\"\n", URL);
        else if (ID != NULL)
            warning(ctxt, "failed to load external entity \"%s\"\n", ID);
    }
    return(NULL);
}
#endif

QString transform( const QString &pat, const QString& tss,
                   const QVector<const char *> &params )
{
    QString parsed;

    INFO(i18n("Parsing stylesheet"));
#if defined (SIMPLE_XSLT) && defined(Q_OS_WIN)
    // prepare use of local available dtd versions instead of fetching every time from the internet
	// this approach is url based
    if (!defaultEntityLoader) {
        defaultEntityLoader = xmlGetExternalEntityLoader();
        xmlSetExternalEntityLoader(xsltprocExternalEntityLoader);

        replaceURLList[QLatin1String("http://www.oasis-open.org/docbook/xml/4.2")] = QString("file:///%1").arg(DOCBOOK_XML_CURRDTD);
    }
#endif

    xsltStylesheetPtr style_sheet =
        xsltParseStylesheetFile((const xmlChar *)QFile::encodeName(tss).constData());

    if ( !style_sheet ) {
        return parsed;
    }
    if (style_sheet->indent == 1)
        xmlIndentTreeOutput = 1;
    else
        xmlIndentTreeOutput = 0;

    INFO(i18n("Parsing document"));

    xmlDocPtr doc = xmlParseFile(QFile::encodeName(pat).constData());
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
        parsed = QLatin1Char(' '); // avoid error message
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
    if (QFile::exists( QDir::currentPath() + "/" + URL ) )
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
    while (parsed.at(start_index - 1) != QLatin1Char('>')) start_index++;

    int inside = 0;

    QString filedata;

    while (true) {
        int endindex = parsed.indexOf(QStringLiteral("</FILENAME>"), index);
        int startindex = parsed.indexOf(QStringLiteral("<FILENAME "), index) + 1;

//        //qDebug() << "FILENAME " << startindex << " " << endindex << " " << inside << " " << parsed.mid(startindex + 18, 15)<< " " << parsed.length();

        if (startindex > 0) {
            if (startindex < endindex) {
                //              //qDebug() << "finding another";
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

    index = filedata.indexOf(QStringLiteral("<FILENAME "));

    if (index > 0) {
        int endindex = filedata.lastIndexOf(QStringLiteral("</FILENAME>"));
        while (filedata.at(endindex) != QLatin1Char('>')) endindex++;
        endindex++;
        filedata = filedata.left(index) + filedata.mid(endindex);
    }

    // filedata.replace(QRegExp(">"), "\n>");
    return filedata;
}

QByteArray fromUnicode( const QString &data )
{
#ifdef Q_OS_WIN
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
#ifdef Q_OS_WIN
    name = "utf-8";
    // may be required for all xml output
    if (output.contains("<table-of-contents>"))
        output.replace( QString( "<?xml version=\"1.0\"?>" ),
                        QString( "<?xml version=\"1.0\" encoding=\"%1\"?>").arg( name ) );
#else
    name = QLatin1String(QTextCodec::codecForLocale()->name());
    name.replace( QStringLiteral( "ISO " ), QStringLiteral("iso-") );
    output.replace( QStringLiteral( "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">" ),
                    QStringLiteral( "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=%1\">" ).arg( name ) );
#endif
}

class DtdStandardDirs
{
public:
    QString srcdir;
};

Q_GLOBAL_STATIC(DtdStandardDirs, s_dtdDirs);

void setupStandardDirs(const QString &srcdir)
{
    QByteArray catalogs;

    if ( srcdir.isEmpty() ) {
        catalogs += QUrl::fromLocalFile(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("ksgmltools2/customization/catalog.xml"))).toEncoded();
    } else {
        catalogs += QUrl::fromLocalFile( srcdir + QStringLiteral("/customization/catalog.xml") ).toEncoded();
        s_dtdDirs()->srcdir = srcdir;
    }

    qputenv( "XML_CATALOG_FILES", catalogs);
    xmlInitializeCatalog();
}

QString locateFileInDtdResource(const QString& file)
{
    QFileInfo info(file);
    if (info.exists() && info.isAbsolute())
        return file;

    const QString srcdir = s_dtdDirs()->srcdir;
    if (!srcdir.isEmpty()) {
        const QString test = srcdir + QLatin1Char('/') + file;
        if (QFile::exists(test))
            return test;
        return QString();
    }
    return QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("ksgmltools2/") + file);
}
