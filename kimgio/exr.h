/**
* QImageIO Routines to read (and perhaps in the future, write) images
* in the high definition EXR format.
*
* Copyright (c) 2003, Brad Hards <bradh@frogmouth.net>
*
* This library is distributed under the conditions of the GNU LGPL.
*
*/

#ifndef KIMG_EXR_H
#define KIMG_EXR_H

#include <QtGui/QImageIOPlugin>

class EXRHandler : public QImageIOHandler
{
public:
    EXRHandler();

    /**
       Test if the file / stream can potentially read more data
    */
    bool canRead() const;

    /**
       Read contents from the file / stream into an image

       \param outImage pointer to the QImage that the file / stream will
       be read into

       \return true on a successful read, false on failure
    */
    bool read( QImage *outImage );

    /**
       Write the contents of an image into the file / stream

       \param image the image to write out

       \return true on a successful write, false on failure
    */
    bool write( const QImage &image );

    /**
       The name of this plugin

       \return always returns "exr" for this plugin
    */
    QByteArray name() const;

    /**
       Convenience routine. You should use canRead() instead.
    */
    static bool canRead( QIODevice *device );
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
class EXRPlugin : public QImageIOPlugin
{
public:
    QStringList keys() const;
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const;
};
#endif

#endif
