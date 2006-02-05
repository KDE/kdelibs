// This library is distributed under the conditions of the GNU LGPL.
#include "config.h"

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_JASPER

#include "jp2.h"

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#include <QImage>
#include <QVariant>
#include <QTextStream>

// dirty, but avoids a warning because jasper.h includes jas_config.h.
#undef PACKAGE
#undef VERSION
#include <jasper/jasper.h>

// code taken in parts from JasPer's jiv.c

#define DEFAULT_RATE 0.10
#define MAXCMPTS 256


/************************* JasPer QIODevice stream ***********************/

//unfortunately this is declared as static in JasPer libraries
static jas_stream_t *jas_stream_create()
{
        jas_stream_t *stream;

        if (!(stream = (jas_stream_t*)jas_malloc(sizeof(jas_stream_t)))) {
                return 0;
        }
        stream->openmode_ = 0;
        stream->bufmode_ = 0;
        stream->flags_ = 0;
        stream->bufbase_ = 0;
        stream->bufstart_ = 0;
        stream->bufsize_ = 0;
        stream->ptr_ = 0;
        stream->cnt_ = 0;
        stream->ops_ = 0;
        stream->obj_ = 0;
        stream->rwcnt_ = 0;
        stream->rwlimit_ = -1;

        return stream;
}

//unfortunately this is declared as static in JasPer libraries
static void jas_stream_initbuf(jas_stream_t *stream, int bufmode, char *buf,
  int bufsize)
{
        /* If this function is being called, the buffer should not have been
          initialized yet. */
        assert(!stream->bufbase_);

        if (bufmode != JAS_STREAM_UNBUF) {
                /* The full- or line-buffered mode is being employed. */
                if (!buf) {
                        /* The caller has not specified a buffer to employ, so allocate
                          one. */
                        if ((stream->bufbase_ = (unsigned char*)jas_malloc(JAS_STREAM_BUFSIZE +
                          JAS_STREAM_MAXPUTBACK))) {
                                stream->bufmode_ |= JAS_STREAM_FREEBUF;
                                stream->bufsize_ = JAS_STREAM_BUFSIZE;
                        } else {
                                /* The buffer allocation has failed.  Resort to unbuffered
                                  operation. */
                                stream->bufbase_ = stream->tinybuf_;
                                stream->bufsize_ = 1;
                        }
                } else {
                        /* The caller has specified a buffer to employ. */
                        /* The buffer must be large enough to accommodate maximum
                          putback. */
                        assert(bufsize > JAS_STREAM_MAXPUTBACK);
                        stream->bufbase_ = JAS_CAST(uchar *, buf);
                        stream->bufsize_ = bufsize - JAS_STREAM_MAXPUTBACK;
                }
        } else {
                /* The unbuffered mode is being employed. */
                /* A buffer should not have been supplied by the caller. */
                assert(!buf);
                /* Use a trivial one-character buffer. */
                stream->bufbase_ = stream->tinybuf_;
                stream->bufsize_ = 1;
        }
        stream->bufstart_ = &stream->bufbase_[JAS_STREAM_MAXPUTBACK];
        stream->ptr_ = stream->bufstart_;
        stream->cnt_ = 0;
        stream->bufmode_ |= bufmode & JAS_STREAM_BUFMODEMASK;
}
                                
static int qiodevice_read(jas_stream_obj_t *obj, char *buf, int cnt)
{
        QIODevice *io = (QIODevice*) obj;
        return io->read(buf, cnt);
}

static int qiodevice_write(jas_stream_obj_t *obj, char *buf, int cnt)
{
        QIODevice *io = (QIODevice*) obj;
        return io->write(buf, cnt);
}

static long qiodevice_seek(jas_stream_obj_t *obj, long offset, int origin)
{
        QIODevice *io = (QIODevice*) obj;
        long newpos;

        switch (origin) {
        case SEEK_SET:
                newpos = offset;
                break;
        case SEEK_END:
                newpos = io->size() - offset;
                break;
        case SEEK_CUR:
                newpos = io->size() + offset;
                break;
        default:
                return -1;
        }
        if (newpos < 0) {
                return -1;
        }
        if ( io->seek(newpos) )
            return newpos;
        else
            return -1;
}

static int qiodevice_close(jas_stream_obj_t *obj)
{
        return 0;
}

static jas_stream_ops_t jas_stream_qiodeviceops = {
        qiodevice_read,
        qiodevice_write,
        qiodevice_seek,
        qiodevice_close
};

static jas_stream_t *jas_stream_qiodevice(QIODevice *iodevice)
{
        jas_stream_t *stream;

        if ( !iodevice ) return 0;
        if (!(stream = jas_stream_create())) {
                return 0;
        }

        /* A stream associated with a memory buffer is always opened
        for both reading and writing in binary mode. */
        stream->openmode_ = JAS_STREAM_READ | JAS_STREAM_WRITE | JAS_STREAM_BINARY;

        jas_stream_initbuf(stream, JAS_STREAM_FULLBUF, 0, 0);

        /* Select the operations for a memory stream. */
        stream->obj_ = (void *)iodevice;
        stream->ops_ = &jas_stream_qiodeviceops;

        return stream;
}

/************************ End of JasPer QIODevice stream ****************/

typedef struct {
    jas_image_t* image;

    int cmptlut[MAXCMPTS];

    jas_image_t* altimage;
} gs_t;


static jas_image_t*
read_image( QIODevice* io )
{
    jas_stream_t* in = 0;

    in = jas_stream_qiodevice( io );

    if( !in ) return 0;

    jas_image_t* image = jas_image_decode( in, -1, 0 );
    jas_stream_close( in );

    // image may be 0, but that's Ok
    return image;
} // read_image

static bool
convert_colorspace( gs_t& gs )
{
    jas_cmprof_t *outprof = jas_cmprof_createfromclrspc( JAS_CLRSPC_SRGB );
    if( !outprof ) return false;

    gs.altimage = jas_image_chclrspc( gs.image, outprof,
                                      JAS_CMXFORM_INTENT_PER );
    if( !gs.altimage ) return false;

    return true;
} // convert_colorspace

static bool
render_view( gs_t& gs, QImage* outImage )
{
    QImage qti;
    if((gs.cmptlut[0] = jas_image_getcmptbytype(gs.altimage,
                                                JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_R))) < 0 ||
       (gs.cmptlut[1] = jas_image_getcmptbytype(gs.altimage,
                                                JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_G))) < 0 ||
       (gs.cmptlut[2] = jas_image_getcmptbytype(gs.altimage,
                                                JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_B))) < 0) {
        return false;
    } // if

    const int* cmptlut = gs.cmptlut;
    int v[3];

    // check that all components have the same size.
    const int width = jas_image_cmptwidth( gs.altimage, cmptlut[0] );
    const int height = jas_image_cmptheight( gs.altimage, cmptlut[0] );
    for( int i = 1; i < 3; ++i ) {
        if (jas_image_cmptwidth( gs.altimage, cmptlut[i] ) != width ||
            jas_image_cmptheight( gs.altimage, cmptlut[i] ) != height)
            return false;
    } // for

    qti = QImage( jas_image_width( gs.altimage ), jas_image_height( gs.altimage ),
                  QImage::Format_RGB32 );

    uint32_t* data = (uint32_t*)qti.bits();

    for( int y = 0; y < height; ++y ) {
        for( int x = 0; x < width; ++x ) {
            for( int k = 0; k < 3; ++k ) {
                v[k] = jas_image_readcmptsample( gs.altimage, cmptlut[k], x, y );
                // if the precision of the component is too small, increase
                // it to use the complete value range.
                v[k] <<= 8 - jas_image_cmptprec( gs.altimage, cmptlut[k] );

                if( v[k] < 0 ) v[k] = 0;
                else if( v[k] > 255 ) v[k] = 255;
            } // for k

            *data++ = qRgb( v[0], v[1], v[2] );
        } // for x
    } // for y
    *outImage = qti;
    return true;
} // render_view


static jas_image_t*
create_image( const QImage& qi )
{
    // prepare the component parameters
    jas_image_cmptparm_t* cmptparms = new jas_image_cmptparm_t[ 3 ];

    for ( int i = 0; i < 3; ++i ) {
        // x and y offset
        cmptparms[i].tlx = 0;
        cmptparms[i].tly = 0;

        // the resulting image will be hstep*width x vstep*height !
        cmptparms[i].hstep = 1;
        cmptparms[i].vstep = 1;
        cmptparms[i].width = qi.width();
        cmptparms[i].height = qi.height();

        // we write everything as 24bit truecolor ATM
        cmptparms[i].prec = 8;
        cmptparms[i].sgnd = false;
    }

    jas_image_t* ji = jas_image_create( 3 /* number components */, cmptparms, JAS_CLRSPC_UNKNOWN );
    delete[] cmptparms;

    // returning 0 is ok
    return ji;
} // create_image


static bool
write_components( jas_image_t* ji, const QImage& qi )
{
    const unsigned height = qi.height();
    const unsigned width = qi.width();

    jas_matrix_t* m = jas_matrix_create( height, width );
    if( !m ) return false;

    jas_image_setclrspc( ji, JAS_CLRSPC_SRGB );

    jas_image_setcmpttype( ji, 0, JAS_IMAGE_CT_RGB_R );
    for( uint y = 0; y < height; ++y )
        for( uint x = 0; x < width; ++x )
            jas_matrix_set( m, y, x, qRed( qi.pixel( x, y ) ) );
    jas_image_writecmpt( ji, 0, 0, 0, width, height, m );

    jas_image_setcmpttype( ji, 1, JAS_IMAGE_CT_RGB_G );
    for( uint y = 0; y < height; ++y )
        for( uint x = 0; x < width; ++x )
            jas_matrix_set( m, y, x, qGreen( qi.pixel( x, y ) ) );
    jas_image_writecmpt( ji, 1, 0, 0, width, height, m );

    jas_image_setcmpttype( ji, 2, JAS_IMAGE_CT_RGB_B );
    for( uint y = 0; y < height; ++y )
        for( uint x = 0; x < width; ++x )
            jas_matrix_set( m, y, x, qBlue( qi.pixel( x, y ) ) );
    jas_image_writecmpt( ji, 2, 0, 0, width, height, m );
    jas_matrix_destroy( m );

    return true;
} // write_components

static bool
write_image( const QImage &image, QIODevice* io, int quality )
{
        jas_stream_t* stream = 0;
        stream = jas_stream_qiodevice( io );

        // by here, a jas_stream_t is open
        if( !stream ) return false;

        jas_image_t* ji = create_image( image );
        if( !ji ) {
                jas_stream_close( stream );
                return false;
        } // if

        if( !write_components( ji, image ) ) {
                jas_stream_close( stream );
                jas_image_destroy( ji );
                return false;
        } // if

        // optstr:
        // - rate=#B => the resulting file size is about # bytes
        // - rate=0.0 .. 1.0 => the resulting file size is about the factor times
        //                      the uncompressed size
        QString rate;
        QTextStream ts( &rate, QIODevice::WriteOnly );
        ts << "rate="
                << ( (quality < 0) ? DEFAULT_RATE : quality / 100.0F );
        int i = jp2_encode( ji, stream, rate.toUtf8().data() );

        jas_image_destroy( ji );
        jas_stream_close( stream );

        if( i != 0 ) return false;

        return true;
}

JP2Handler::JP2Handler()
{
    quality = 75;
    jas_init();
}

JP2Handler::~JP2Handler()
{
    jas_cleanup();
}

bool JP2Handler::canRead() const
{
    if (canRead(device())) {
        setFormat("jp2");
        return true;
    }
    return false;
}

bool JP2Handler::canRead(QIODevice *device)
{
    if (!device) {
        return false;
    }
    return device->peek(6) == "\x00\x00\x00\x0C\x6A\x50";
}

bool JP2Handler::read(QImage *image)
{
        if (!canRead()) return false;

        gs_t gs;
        if( !(gs.image = read_image( device() )) ) return false;

        if( !convert_colorspace( gs ) ) return false;

        render_view( gs, image );

        if( gs.image ) jas_image_destroy( gs.image );
        if( gs.altimage ) jas_image_destroy( gs.altimage );
    return true;

}

bool JP2Handler::write(const QImage &image)
{
    return write_image(image, device(),quality);
}

bool JP2Handler::supportsOption(ImageOption option) const
{
    return option == Quality;
}

QVariant JP2Handler::option(ImageOption option) const
{
    if (option == Quality)
        return quality;
    return QVariant();
}

QByteArray JP2Handler::name() const
{
    return "jp2";
}

class JP2Plugin : public QImageIOPlugin
{
public:
    QStringList keys() const;
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const;
};
            
QStringList JP2Plugin::keys() const
{
    return QStringList() << "jp2";
}
                
QImageIOPlugin::Capabilities JP2Plugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "jp2")
        return Capabilities(CanRead | CanWrite);
    if (!format.isEmpty())
        return 0;
    if (!device->isOpen())
        return 0;
                                                    
    Capabilities cap;
    if (device->isReadable() && JP2Handler::canRead(device))
    cap |= CanRead;
    if (device->isWritable())
    cap |= CanWrite;
    return cap;
}

QImageIOHandler *JP2Plugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new JP2Handler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}
                                                                                                    
Q_EXPORT_STATIC_PLUGIN(JP2Plugin)
Q_EXPORT_PLUGIN2(jp2, JP2Plugin)
                                                                                                    
#endif // HAVE_JASPER

