/**
* QImageIO Routines to read/write EPS images.
* copyright (c) 1998 Dirk Schoenberger <dirk.schoenberger@freenet.de>
*
* This library is distributed under the conditions of the GNU LGPL.
*/
#ifndef KIMG_EPS_H
#define KIMG_EPS_H

#include <QImageIOPlugin>

class EPSHandler : public QImageIOHandler
{
public:
    EPSHandler();

    virtual bool canRead() const;
    virtual bool read(QImage *image);
    virtual bool write(const QImage &image);

    static bool canRead(QIODevice *device);
};

class EPSPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "eps.json")

public:
    virtual Capabilities capabilities(QIODevice *device, const QByteArray &format) const;
    virtual QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const;
};

#endif // KIMG_EPS_H

