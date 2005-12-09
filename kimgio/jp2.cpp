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
#include <ktempfile.h>
#include <qcolor.h>
#include <qfile.h>
#include <qimage.h>

// dirty, but avoids a warning because jasper.h includes jas_config.h.
#undef PACKAGE
#undef VERSION
#include <jasper/jasper.h>

// code taken in parts from JasPer's jiv.c

#define DEFAULT_RATE 0.10
#define MAXCMPTS 256


typedef struct {
    jas_image_t* image;

    int	cmptlut[MAXCMPTS];

    jas_image_t* altimage;
} gs_t;


jas_image_t*
read_image( const QImageIO* io )
{
    jas_stream_t* in = 0;
    // for QIODevice's other than QFile, a temp. file is used.
    KTempFile* tempf = 0;

    QFile* qf = 0;
    if( ( qf = dynamic_cast<QFile*>( io->ioDevice() ) ) ) {
        // great, it's a QFile. Let's just take the filename.
        in = jas_stream_fopen( QFile::encodeName( qf->name() ), "rb" );
    } else {
        // not a QFile. Copy the whole data to a temp. file.
        tempf = new KTempFile();
        if( tempf->status() != 0 ) {
            delete tempf;
            return 0;
        } // if
        tempf->setAutoDelete( true );
        QFile* out = tempf->file();
        // 4096 (=4k) is a common page size.
        QByteArray b( 4096 );
        Q_LONG size;
        // 0 or -1 is EOF / error
        while( ( size = io->ioDevice()->readBlock( b.data(), 4096 ) ) > 0 ) {
            // in case of a write error, still give the decoder a try
            if( ( out->writeBlock( b.data(), size ) ) == -1 ) break;
        } // while
        // flush everything out to disk
        out->flush();

        in = jas_stream_fopen( QFile::encodeName( tempf->name() ), "rb" );
    } // else
    if( !in ) {
        delete tempf;
        return 0;
    } // if

    jas_image_t* image = jas_image_decode( in, -1, 0 );
    jas_stream_close( in );
    delete tempf;

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
render_view( gs_t& gs, QImage& qti )
{
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

    if( !qti.create( jas_image_width( gs.altimage ),
                     jas_image_height( gs.altimage ), 32 ) )
        return false;

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
    return true;
} // render_view


KDE_EXPORT void
kimgio_jp2_read( QImageIO* io )
{
	if( jas_init() ) return;

	gs_t gs;
	if( !(gs.image = read_image( io )) ) return;

	if( !convert_colorspace( gs ) ) return;

	QImage image;
	render_view( gs, image );

	if( gs.image ) jas_image_destroy( gs.image );
	if( gs.altimage ) jas_image_destroy( gs.altimage );

	io->setImage( image );
	io->setStatus( 0 );
} // kimgio_jp2_read


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

KDE_EXPORT void
kimgio_jp2_write( QImageIO* io )
{
	if( jas_init() ) return;

	// open the stream. we write directly to the file if possible, to a
	// temporary file otherwise.
	jas_stream_t* stream = 0;

	QFile* qf = 0;
	KTempFile* ktempf = 0;
	if( ( qf = dynamic_cast<QFile*>( io->ioDevice() ) ) ) {
		// jas_stream_fdopen works here, but not when reading...
		stream = jas_stream_fdopen( dup( qf->handle() ), "w" );
	} else {
		ktempf = new KTempFile;
		ktempf->setAutoDelete( true );
		stream = jas_stream_fdopen( ktempf->handle(), "w" );
	} // else


	// by here, a jas_stream_t is open
	if( !stream ) return;

	jas_image_t* ji = create_image( io->image() );
	if( !ji ) {
		delete ktempf;
		jas_stream_close( stream );
		return;
	} // if

	if( !write_components( ji, io->image() ) ) {
		delete ktempf;
		jas_stream_close( stream );
		jas_image_destroy( ji );
		return;
	} // if

	// optstr:
	// - rate=#B => the resulting file size is about # bytes
	// - rate=0.0 .. 1.0 => the resulting file size is about the factor times
	//                      the uncompressed size
	QString rate;
	QTextStream ts( &rate, QIODevice::WriteOnly );
	ts << "rate="
		<< ( (io->quality() < 0) ? DEFAULT_RATE : io->quality() / 100.0F );
	int i = jp2_encode( ji, stream, rate.toUtf8().data() );

	jas_image_destroy( ji );
	jas_stream_close( stream );

	if( i != 0 ) { delete ktempf; return; }

	if( ktempf ) {
		// We've written to a tempfile. Copy the data to the final destination.
		QFile* in = ktempf->file();

		QByteArray b( 4096 );
		Q_LONG size;

		// seek to the beginning of the file.
		if( !in->at( 0 ) ) { delete ktempf; return; }

		// 0 or -1 is EOF / error
		while( ( size = in->readBlock( b.data(), 4096 ) ) > 0 ) {
			if( ( io->ioDevice()->writeBlock( b.data(), size ) ) == -1 ) {
				delete ktempf;
				return;
			} // if
		} // while
		io->ioDevice()->flush();
		delete ktempf;

		// see if we've left the while loop due to an error.
		if( size == -1 ) return;
	} // if


	// everything went fine
	io->setStatus( IO_Ok );
} // kimgio_jp2_write

#endif // HAVE_JASPER

