/**
* QImageIO Routines to read/write XV images.
* copyright (c) 1998 Torben Weis <weis@kde.org>
* copyright (c) 1999 Oliver Eiden <o.eiden@pop.ruhr.de>
*
* This library is distributed under the conditions of the GNU LGPL.
*/

#include "xview.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <QImage>

#define BUFSIZE 1024

static const int b_255_3[]= {0,85,170,255},  // index*255/3
           rg_255_7[]={0,36,72,109,145,182,218,255}; // index *255/7


XVHandler::XVHandler()
{
}

bool XVHandler::canRead() const
{
    if (canRead(device())) {
        setFormat("xv");
        return true;
    }
    return false;
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

    if (iodev->read(block, blocksize) != blocksize )
    {
	free(block);
        return false;
    }

    // Create the image
    QImage image( x, y, QImage::Format_Indexed8 );
    int numColors;
    numColors = qMin( maxval + 1, 0 );
    numColors = qMax( 0, maxval + 1 );
    image.setNumColors( numColors );

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
    f.write( "P7 332\n", 7 );

    // next line #XVVERSION
    f.write( "#XVVERSION:\n", 12 );

    // now it gets interesting, #BUILTIN means we are out.
    // if IMGINFO comes, we are happy!
    f.write( "#IMGINFO:\n", 10 );

    // after this an #END_OF_COMMENTS signals everything to be ok!
    f.write( "#END_OF_COMMENTS:\n", 18 );

    // now a last line with width, height, maxval which is supposed to be 255
    sprintf( str, "%i %i 255\n", w, h );
    f.write( str, strlen( str ) );


    QImage tmpImage( image );
    if ( image.depth() == 1 )
        tmpImage = image.convertToFormat( QImage::Format_Indexed8, Qt::AutoColor );

    uchar* buffer = new uchar[ w ];

    for ( int py = 0; py < h; py++ )
    {
        const uchar *data = tmpImage.scanLine( py );
        for ( int px = 0; px < w; px++ )
        {
            int r, g, b;
            if ( tmpImage.depth() == 32 )
            {
                const QRgb *data32 = (QRgb*) data;
                r = qRed( *data32 ) >> 5;
                g = qGreen( *data32 ) >> 5;
                b = qBlue( *data32 ) >> 6;
                data += sizeof( QRgb );
            }
            else
            {
                QRgb color = tmpImage.color( *data );
                r = qRed( color ) >> 5;
                g = qGreen( color ) >> 5;
                b = qBlue( color ) >> 6;
                data++;
            }
            buffer[ px ] = ( r << 5 ) | ( g << 2 ) | b;
        }
        f.write( (const char*)buffer, w );
    }
    delete[] buffer;

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
