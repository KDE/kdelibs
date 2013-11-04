/**
* QImageIO Routines to read/write JPEG2000 images.
* copyright (c) 2002 Michael Ritzert <michael@ritzert.de>
*
* This library is distributed under the conditions of the GNU LGPL.
*/
#ifndef KIMG_JP2_H
#define KIMG_JP2_H

#include <QImageIOPlugin>

class JP2Handler : public QImageIOHandler
{
public:
    JP2Handler();
    virtual ~JP2Handler();

    virtual bool canRead() const;
    virtual bool read(QImage *image);
    virtual bool write(const QImage &image);

    virtual bool supportsOption(ImageOption option) const;
    virtual QVariant option(ImageOption option) const;
    virtual void setOption(ImageOption option, const QVariant &value);

    static bool canRead(QIODevice *device);

private:
    int quality;
};

class JP2Plugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "jp2.json")

public:
    virtual Capabilities capabilities(QIODevice *device, const QByteArray &format) const;
    virtual QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const;
};

#endif // KIMG_JP2_H
