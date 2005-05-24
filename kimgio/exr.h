/**
* QImageIO Routines to read (and perhaps in the future, write) images
* in the high definition EXR format.
*
* Copyright (c) 2003, Brad Hards <bradh@frogmouth.net>
*
* This library is distributed under the conditions of the GNU LGPL.
*
*/

#ifndef KIMG_EXR_H
#define KIMG_EXR_H

#include <QImageIOHandler>

class EXRHandler : public QImageIOHandler
{
public:
    EXRHandler();

    bool canRead() const;
    bool read( QImage *outImage );
    bool write( const QImage &image );

    QByteArray name() const;

    static bool canRead( QIODevice *device );
};


class EXRPlugin : public QImageIOPlugin
{
public:
    QStringList keys() const;
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const;
};

#endif
