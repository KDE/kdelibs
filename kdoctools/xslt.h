#ifndef _MEIN_XSLT_H_
#define _MEIN_XSLT_H_

#include <libxml/parser.h>

QString transform(const QString &file, const QString& stylesheet);
QString transform(xmlParserCtxtPtr ctxt, const QString &tss);
xmlParserInputPtr meinExternalEntityLoader(const char *URL, const char *ID,
					   xmlParserCtxtPtr ctxt);
QString splitOut(const QString &parsed, int index);
void fillInstance(KInstance &ins);
bool saveToCache( const QString &contents, const QString &filename );
QString lookForCache( const QString &filename );
QCString fromUnicode( const QString &data );
void replaceCharsetHeader( QString &output );

extern bool warnings_exist;

/**
 * Compares two files and returns true if @param newer exists and is newer than
 * @param older
 **/
bool compareTimeStamps( const QString &older, const QString &newer );
#endif
