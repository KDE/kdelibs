/**
* QImageIO Routines to read/write TIFF images.
* Sirtaj Singh Kang, Oct 1998.
*
* This library is distributed under the conditions of the GNU LGPL.
*
* $Id$
*/

#ifndef KIMG_TIFFR_H
#define KIMG_TIFFR_H

#include <QImageIOHandler>

class TIFFRHandler : public QImageIOHandler
{
public:
    TIFFRHandler();

    bool canRead() const;
    bool read(QImage *image);
    bool write(const QImage &image);

    QByteArray name() const;

    static bool canRead(QIODevice *device);
};

#endif
