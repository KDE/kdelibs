#ifndef _XSLT_H_
#define _XSLT_H_

#include <libxml/parser.h>

QString transform(const QString &file, const QString& stylesheet);
QString transform(xmlParserCtxtPtr ctxt, const QString &tss);
xmlParserInputPtr meinExternalEntityLoader(const char *URL, const char *ID,
					   xmlParserCtxtPtr ctxt);
QString splitOut(const QString &parsed, int index);
void fillInstance(KInstance &ins);
bool saveToCache( const QString &contents, const QString &filename );
QString lookForCache( const QString &filename );

#endif
