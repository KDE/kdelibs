#ifndef _MEIN_XSLT_H_
#define _MEIN_XSLT_H_

#include <libxml/parser.h>
#include <qvaluevector.h>

QString transform(const QString &file, const QString& stylesheet,
                  const QValueVector<const char *> &params = QValueVector<const char *>());
QString splitOut(const QString &parsed, int index);
void fillInstance(KInstance &ins, const QString &srcdir = QString::null );
bool saveToCache( const QString &contents, const QString &filename );
QString lookForCache( const QString &filename );
QCString fromUnicode( const QString &data );
void replaceCharsetHeader( QString &output );

extern bool warnings_exist;
extern QString *SRCDIR;

/**
 * Compares two files and returns true if @param newer exists and is newer than
 * @param older
 **/
bool compareTimeStamps( const QString &older, const QString &newer );
#endif
