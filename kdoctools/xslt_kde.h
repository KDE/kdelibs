#ifndef _MEIN_XSLT_KDE_H_
#define _MEIN_XSLT_KDE_H_

#include <QtCore/QString>
#include <QtCore/QVector>

class QByteArray;
class QIODevice;
class KComponentData;

void fillInstance(KComponentData &ins, const QString &srcdir = QString() );
bool saveToCache( const QString &contents, const QString &filename );
QIODevice *getBZip2device(const QString &fileName );

#endif
