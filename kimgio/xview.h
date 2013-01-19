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
#ifndef KIMG_XVIEW_H
#define KIMG_XVIEW_H

#include <QImageIOPlugin>

class XVHandler : public QImageIOHandler
{
public:
    XVHandler();

    virtual bool canRead() const;
    virtual bool read(QImage *image);
    virtual bool write(const QImage &image);

    static bool canRead(QIODevice *device);
};

class XVPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "xview.json")

public:
    virtual Capabilities capabilities(QIODevice *device, const QByteArray &format) const;
    virtual QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const;
};

#endif // KIMG_XVIEW_H
