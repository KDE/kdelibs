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
#include <qcstring.h>
#include <qfile.h>
#include <qimage.h>

// dirty, but avoids a warning because jasper.h includes jas_config.h.
#undef PACKAGE
#undef VERSION
#include <jasper/jasper.h>

// code taken in parts from JasPer's jiv.c

namespace {
	const float DEFAULT_RATE = 0.10;
	const unsigned MAXCMPTS = 256;

	typedef struct {
		jas_image_t*	image;

		int				cmptlut[MAXCMPTS];

#ifdef JAS_IMAGE_CS_RGB
		jas_matrix_t*	cmpts[MAXCMPTS];
#else
		jas_image_t*	altimage;
#endif
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

#ifdef JAS_IMAGE_CS_RGB
	void
	init_cmptlut( gs_t& gs )
	{
		switch( jas_image_colorspace(gs.image) ) {
		case JAS_IMAGE_CS_RGB:
			if((gs.cmptlut[0] = jas_image_getcmptbytype(gs.image,
				JAS_IMAGE_CT_COLOR(JAS_IMAGE_CT_RGB_R))) < 0 ||
				(gs.cmptlut[1] = jas_image_getcmptbytype(gs.image,
				JAS_IMAGE_CT_COLOR(JAS_IMAGE_CT_RGB_G))) < 0 ||
				(gs.cmptlut[2] = jas_image_getcmptbytype(gs.image,
				JAS_IMAGE_CT_COLOR(JAS_IMAGE_CT_RGB_B))) < 0) {
				return;
			}
			break;
		case JAS_IMAGE_CS_YCBCR:
			if ((gs.cmptlut[0] = jas_image_getcmptbytype(gs.image,
				JAS_IMAGE_CT_COLOR(JAS_IMAGE_CT_YCBCR_Y))) < 0 ||
				(gs.cmptlut[1] = jas_image_getcmptbytype(gs.image,
				JAS_IMAGE_CT_COLOR(JAS_IMAGE_CT_YCBCR_CB))) < 0 ||
				(gs.cmptlut[2] = jas_image_getcmptbytype(gs.image,
				JAS_IMAGE_CT_COLOR(JAS_IMAGE_CT_YCBCR_CR))) < 0) {
				return;
			}
			break;
		case JAS_IMAGE_CS_GRAY:
			if ((gs.cmptlut[0] = jas_image_getcmptbytype(gs.image,
				JAS_IMAGE_CT_COLOR(JAS_IMAGE_CT_GRAY_Y))) < 0) {
				return;
			}
			break;
		default:
			return;
			break;
		}
	} // init_cmptlut


	bool
	init_components( gs_t& gs )
	{
		for( uint i = 0; i < MAXCMPTS; ++i ) gs.cmpts[i] = 0;
		for( uint i = 0; i < jas_image_numcmpts(gs.image); ++i)
		{
			if (!(gs.cmpts[i] = jas_matrix_create(jas_image_cmptheight(gs.image,
							i), jas_image_cmptwidth(gs.image, i))))
				return false;
			if( jas_image_readcmpt(gs.image, i, 0, 0, jas_image_cmptwidth(gs.image,
							i), jas_image_cmptheight(gs.image, i), gs.cmpts[i]))
				return false;
		} // for
		return true;
	} // init_components


	void
	ycbcr_to_rgb( int ycbcr[3], int* const ret )
	{
		int dummy[3] = {
			(int)((1/1.772) * (ycbcr[0] + 1.402 * ycbcr[2])),
			(int)((1/1.772) * (ycbcr[0] - 0.34413 * ycbcr[1] - 0.71414 * ycbcr[2])),
			(int)((1/1.772) * (ycbcr[0] + 1.772 * ycbcr[1])) };
		for( int k = 0; k < 3; ++k ) ret[k] = dummy[k];
	} // ycbcr_to_rgb


	void
	draw_view_gray( gs_t& gs, QImage& qti )
	{
		qti.create( jas_image_width( gs.image ), jas_image_height( gs.image ),
			8, 256 );
		for( int i = 0; i < 256; ++i )
			qti.setColor( i, qRgb( i, i, i ) );

		const int* cmptlut = gs.cmptlut;

		const uint width = jas_image_width( gs.image );
		const uint height = jas_image_height( gs.image );
		for( uint y = 0; y < height; ++y ) {
			uchar* sl = qti.scanLine( y );
			for( uint x = 0; x < width; ++x ) {
				*sl = jas_matrix_get( gs.cmpts[cmptlut[0]], y, x );
				*sl <<= 8 - jas_image_cmptprec(gs.image, 0);
				++sl;
			} // for x
		} // for y
	} // draw_view_gray

	void
	draw_view_color( gs_t& gs, QImage& qti )
	{
		const int numcmpts = 3;
		const bool ycbcr
			= (jas_image_colorspace( gs.image ) == JAS_IMAGE_CS_YCBCR);
		if( !qti.create( jas_image_width( gs.image ),
					jas_image_height( gs.image ), 32 ) )
				return;

		const int* cmptlut = gs.cmptlut;
		int v[3];

		uint32_t* data = (uint32_t*)qti.bits();

		const uint width = jas_image_width( gs.image );
		const uint height = jas_image_height( gs.image );

		for( uint y = 0; y < height; ++y ) {
			for( uint x = 0; x < width; ++x ) {
				for( int k = 0; k < numcmpts; ++k ) {
					v[k] = ( x < jas_image_cmptwidth(gs.image, cmptlut[k])
							&& y < jas_image_cmptheight(gs.image, cmptlut[k]))
						? jas_matrix_get(gs.cmpts[cmptlut[k]], y, x) : 0;
					// if the precision of the component is too small, increase
					// it to use the complete value range.
					v[k] <<= 8 - jas_image_cmptprec(gs.image, cmptlut[k]);
				} // for k

				if( ycbcr ) ycbcr_to_rgb( v, v );

				for( int k = 0; k < 3; ++k ) {
					if( v[k] < 0 ) v[k] = 0;
					else if( v[k] > 255 ) v[k] = 255;
				} // for k

				*data++ = qRgb( v[0], v[1], v[2] );
			} // for x
		} // for y
	} // draw_view_color

	void
	draw_view( gs_t& gs, QImage& qti )
	{
		switch( jas_image_colorspace( gs.image ) ) {
			case JAS_IMAGE_CS_RGB:
			case JAS_IMAGE_CS_YCBCR:
				draw_view_color( gs, qti );
				break;
			case JAS_IMAGE_CS_GRAY:
				draw_view_gray( gs, qti );
				break;
		} // switch

		return;
	} // draw_view

#else

	bool
	convert_colorspace( gs_t& gs )
	{
		jas_cmprof_t *outprof = jas_cmprof_createfromclrspc( JAS_CLRSPC_SRGB );
		if( !outprof ) return false;

		gs.altimage = jas_image_chclrspc( gs.image, outprof,
				JAS_CMXFORM_INTENT_PER );
		if( !gs.altimage ) return false;

		return true;
	} // convert_colorspace

	bool
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
#endif

} // namespace


KDE_EXPORT void
kimgio_jp2_read( QImageIO* io )
{
	if( jas_init() ) return;

	gs_t gs;
	if( !(gs.image = read_image( io )) ) return;

#ifdef JAS_IMAGE_CS_RGB
	if( !init_components( gs ) ) return; // TODO: free resources!
	init_cmptlut( gs );

	QImage image;
	draw_view( gs, image );
#else
	if( !convert_colorspace( gs ) ) return;

	QImage image;
	render_view( gs, image );
#endif

	if( gs.image ) jas_image_destroy( gs.image );
#ifdef JAS_IMAGE_CS_RGB
	for( uint i = 0; i < MAXCMPTS; ++i )
		if( gs.cmpts[i] ) jas_matrix_destroy( gs.cmpts[i] );
#else
	if( gs.altimage ) jas_image_destroy( gs.altimage );
#endif

	io->setImage( image );
	io->setStatus( 0 );
} // kimgio_jp2_read


namespace { // _write helpers
#ifdef JAS_IMAGE_CS_RGB
	jas_image_t*
	create_image( const QImage& qi )
	{
		// prepare the component parameters
		jas_image_cmptparm_t* cmptparms = new jas_image_cmptparm_t[ 3 ];

		// x and y offset
		cmptparms[0].tlx = 0;
		cmptparms[0].tly = 0;

		// the resulting image will be hstep*width x vstep*height !
		cmptparms[0].hstep = 1;
		cmptparms[0].vstep = 1;
		cmptparms[0].width = qi.width();
		cmptparms[0].height = qi.height();

		// we write everything as 24bit truecolor ATM
		cmptparms[0].prec = 8;
		cmptparms[0].sgnd = false;

		cmptparms[1] = cmptparms[2] = cmptparms[0];
		
		jas_image_t* ji = jas_image_create( 3, cmptparms, JAS_IMAGE_CS_RGB );
		delete[] cmptparms;

		// returning 0 is ok
		return ji;
	} // create_image


	bool
	write_components( jas_image_t* ji, const QImage& qi )
	{
		const unsigned height = qi.height();
		const unsigned width = qi.width();

		jas_matrix_t* m = jas_matrix_create( height, width );
		if( !m ) return false;

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
#endif
} // namespace

KDE_EXPORT void
kimgio_jp2_write( QImageIO* io )
{
#ifdef JAS_IMAGE_CS_RGB
	if( jas_init() ) return;

	// open the stream. we write directly to the file if possible, to a
	// temporary file otherwise.
	jas_stream_t* stream = 0;

	QFile* qf = 0;
	KTempFile* ktempf = 0;
	if( ( qf = dynamic_cast<QFile*>( io->ioDevice() ) ) ) {
		// jas_stream_fdopen works here, but not when reading...
		stream = jas_stream_fdopen( qf->handle(), "w" );
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
	QTextStream ts( &rate, IO_WriteOnly );
	ts << "rate="
		<< ( (io->quality() < 0) ? DEFAULT_RATE : io->quality() / 100.0F );
	int i = jp2_encode( ji, stream, rate.utf8().data() );

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
	io->setStatus( 0 );
#endif
} // kimgio_jp2_write

#endif // HAVE_JASPER

