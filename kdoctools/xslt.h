#ifndef _MEIN_XSLT_H_
#define _MEIN_XSLT_H_

#include <libxml/parser.h>
#include <QtCore/QString>
#include <QtCore/QVector>

class QByteArray;

QString transform(const QString &file, const QString& stylesheet,
                  const QVector<const char *> &params = QVector<const char *>());
QString splitOut(const QString &parsed, int index);
QByteArray fromUnicode( const QString &data );
void replaceCharsetHeader( QString &output );

bool saveToCache(const QString& contents, const QString& filename);

void setupStandardDirs(const QString &srcdir = QString() );
QString locateFileInDtdResource(const QString& file);

#endif
