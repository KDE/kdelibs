/**
* QImageIO Routines to read/write EPS images.
* copyright (c) 1998 Dirk Schoenberger <dirk.schoenberger@freenet.de>
*
* This library is distributed under the conditions of the GNU LGPL.
*/
#ifndef _EPS_H
#define _EPS_H


#include <QtGui/QImageIOPlugin>

class EPSHandler : public QImageIOHandler
{
public:
    EPSHandler();

    bool canRead() const;
    bool read(QImage *image);
    bool write(const QImage &image);

    QByteArray name() const;

    static bool canRead(QIODevice *device);
};

#endif

