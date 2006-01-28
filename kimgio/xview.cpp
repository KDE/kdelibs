// Oliver Eiden <o.eiden@pop.ruhr.de>
// 23.3.99
// changed the mapping from 3-3-2 decoded pixels to 8-8-8 decoded true-color pixels
// now it uses the same mapping as xv, this leads to better visual results
// Patch merged in HEAD by Chris Spiegel <matrix@xirtam.org>
// This library is distributed under the conditions of the GNU LGPL.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <qimage.h>

#include <kdelibs_export.h>

#include "xview.h"

#define BUFSIZE 1024

static const int b_255_3[]= {0,85,170,255},  // index*255/3
           rg_255_7[]={0,36,72,109,145,182,218,255}; // index *255/7


XVHandler::XVHandler()
{
}

bool XVHandler::canRead() const
{
    return canRead(device());
}

bool XVHandler::read(QImage *retImage)
{
    int x=-1;
    int y=-1;
    int maxval=-1;
    QIODevice *iodev = device();

    char str[ BUFSIZE ];

    // magic number must be "P7 332"
    iodev->readLine( str, BUFSIZE );
    if (strncmp(str,"P7 332",6))
        return false;

    // next line #XVVERSION
    iodev->readLine( str, BUFSIZE );
    if (strncmp(str, "#XVVERSION", 10))
        return false;

    // now it gets interesting, #BUILTIN means we are out.
    // if IMGINFO comes, we are happy!
    iodev->readLine( str, BUFSIZE );
    if (strncmp(str, "#IMGINFO:", 9))
        return false;

    // after this an #END_OF_COMMENTS signals everything to be ok!
    iodev->readLine( str, BUFSIZE );
    if (strncmp(str, "#END_OF", 7))
        return false;

    // now a last line with width, height, maxval which is
    // supposed to be 255
    iodev->readLine( str, BUFSIZE );
    sscanf(str, "%d %d %d", &x, &y, &maxval);

    if (maxval != 255)
        return false;
    int blocksize = x*y;
    if(x < 0 || y < 0 || blocksize < x || blocksize < y)
        return false;

    // now follows a binary block of x*y bytes.
    char *block = (char*) malloc(blocksize);
    if(!block)
        return false;

    if (iodev->readBlock(block, blocksize) != blocksize )
    {
        return false;
    }

    // Create the image
    QImage image( x, y, 8, maxval + 1, QImage::BigEndian );

    // how do the color handling? they are absolute 24bpp
    // or at least can be calculated as such.
    int r,g,b;

    for ( int j = 0; j < 256; j++ )
    {
        r =  rg_255_7[((j >> 5) & 0x07)];
        g =  rg_255_7[((j >> 2) & 0x07)];
        b =  b_255_3[((j >> 0) & 0x03)];
        image.setColor( j, qRgb( r, g, b ) );
    }

    for ( int py = 0; py < y; py++ )
    {
        uchar *data = image.scanLine( py );
        memcpy( data, block + py * x, x );
    }

    *retImage = image;

    free(block);
    return true;
}

bool XVHandler::write(const QImage &image)
{
    QIODevice& f = *( device() );

    // Removed "f.open(...)" and "f.close()" (tanghus)

    int w = image.width(), h = image.height();

    char str[ 1024 ];

    // magic number must be "P7 332"
    f.writeBlock( "P7 332\n", 7 );

    // next line #XVVERSION
    f.writeBlock( "#XVVERSION:\n", 12 );

    // now it gets interesting, #BUILTIN means we are out.
    // if IMGINFO comes, we are happy!
    f.writeBlock( "#IMGINFO:\n", 10 );

    // after this an #END_OF_COMMENTS signals everything to be ok!
    f.writeBlock( "#END_OF_COMMENTS:\n", 18 );

    // now a last line with width, height, maxval which is supposed to be 255
    sprintf( str, "%i %i 255\n", w, h );
    f.writeBlock( str, strlen( str ) );


    if ( image.depth() == 1 )
    {
        image.convertDepth( 8 );
    }

    uchar buffer[ 128 ];

    for ( int py = 0; py < h; py++ )
    {
        const uchar *data = image.scanLine( py );
        for ( int px = 0; px < w; px++ )
        {
            int r, g, b;
            if ( image.depth() == 32 )
            {
                const QRgb *data32 = (QRgb*) data;
                r = qRed( *data32 ) >> 5;
                g = qGreen( *data32 ) >> 5;
                b = qBlue( *data32 ) >> 6;
                data += sizeof( QRgb );
            }
            else
            {
                QRgb color = image.color( *data );
                r = qRed( color ) >> 5;
                g = qGreen( color ) >> 5;
                b = qBlue( color ) >> 6;
                data++;
            }
            buffer[ px ] = ( r << 5 ) | ( g << 2 ) | b;
        }
        f.writeBlock( (const char*)buffer, w );
    }

    return true;
}

QByteArray XVHandler::name() const
{
    return "xv";
}

bool XVHandler::canRead(QIODevice *device)
{
     if (!device) {
        qWarning("XVHandler::canRead() called with no device");
        return false;
    }

    qint64 oldPos = device->pos();

    char head[6];
    qint64 readBytes = device->read(head, sizeof(head));
    if (readBytes != sizeof(head)) {
        if (device->isSequential()) {
            while (readBytes > 0)
                device->ungetChar(head[readBytes-- - 1]);
        } else {
            device->seek(oldPos);
        }
        return false;
    }

    if (device->isSequential()) {
        while (readBytes > 0)
            device->ungetChar(head[readBytes-- - 1]);
    } else {
        device->seek(oldPos);
    }

    return qstrncmp(head, "P7 332", 6) == 0;
}


class XVPlugin : public QImageIOPlugin
{
public:
    QStringList keys() const;
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const;
};

QStringList XVPlugin::keys() const
{
    return QStringList() << "xv";
}

QImageIOPlugin::Capabilities XVPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "xv")
        return Capabilities(CanRead | CanWrite);
    if (!format.isEmpty())
        return 0;
    if (!device->isOpen())
        return 0;

    Capabilities cap;
    if (device->isReadable() && XVHandler::canRead(device))
        cap |= CanRead;
    if (device->isWritable())
        cap |= CanWrite;
    return cap;
}

QImageIOHandler *XVPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new XVHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

Q_EXPORT_STATIC_PLUGIN(XVPlugin)
Q_EXPORT_PLUGIN2(xv, XVPlugin)
