/**
* QImageIO Routines to read/write XV images.
* copyright (c) 1998 Torben Weis <weis@kde.org>
* copyright (c) 1999 Oliver Eiden <o.eiden@pop.ruhr.de>
*
* This library is distributed under the conditions of the GNU LGPL.
*
*
* Changelog:
* 23.3.99 Oliver Eiden <o.eiden@pop.ruhr.de>
* changed the mapping from 3-3-2 decoded pixels to 8-8-8 decoded true-color pixels
* now it uses the same mapping as xv, this leads to better visual results
* Patch merged in HEAD by Chris Spiegel <matrix@xirtam.org>
*/
#ifndef XVIEW_H
#define XVIEW_H


#include <QtGui/QImageIOPlugin>

class XVHandler : public QImageIOHandler
{
public:
    XVHandler();

    bool canRead() const;
    bool read(QImage *image);
    bool write(const QImage &image);

    QByteArray name() const;

    static bool canRead(QIODevice *device);
};

#endif
