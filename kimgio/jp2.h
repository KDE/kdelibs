// This library is distributed under the conditions of the GNU LGPL.
#ifndef KIMG_JP2_H
#define KIMG_JP2_H

#include <QImageIOHandler>

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
