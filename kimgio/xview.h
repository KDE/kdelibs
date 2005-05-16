// This library is distributed under the conditions of the GNU LGPL.
#ifndef XVIEW_H
#define XVIEW_H


#include <QImageIOHandler>

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
