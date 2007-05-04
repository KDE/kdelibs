/**
* QImageIO Routines to read/write JPEG2000 images.
* copyright (c) 2002 Michael Ritzert <michael@ritzert.de>
*
* This library is distributed under the conditions of the GNU LGPL.
*/
#ifndef KIMG_JP2_H
#define KIMG_JP2_H

#include <QtGui/QImageIOPlugin>

class JP2Handler : public QImageIOHandler
{
public:
    JP2Handler();
    virtual ~JP2Handler();
    
    bool canRead() const;
    bool read(QImage *image);
    bool write(const QImage &image);
                
    QByteArray name() const;
    bool supportsOption(ImageOption option) const;
    QVariant option(ImageOption option) const;
                    
    static bool canRead(QIODevice *device);
private:
    int quality;
};
                        

#endif
