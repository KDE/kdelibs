// This library is distributed under the conditions of the GNU LGPL.
#include "config.h"

#ifdef HAVE_JASPER

#include "jp2.h"

#include <stdint.h>
#include <qcolor.h>
#include <qfile.h>
#include <qimage.h>

#include <jasper/jasper.h>

// code taken in parts from JasPer's jiv.c

namespace {
	const unsigned MAXCMPTS = 256;

	typedef struct {
		jas_image_t*	image;

		jas_matrix_t*	cmpts[MAXCMPTS];

		int				cmptlut[MAXCMPTS];
	} gs_t;


	jas_image_t*
	read_image( const QImageIO* io )
	{
		// TODO: is there a way to read the image in chunks? Reading an
		// unlimited amount of data at once doesn't sound too good...
		QByteArray ba = io->ioDevice()->readAll();
		jas_stream_t* in = jas_stream_memopen( ba.data(), ba.size() );
		if( !in ) return 0;

		jas_image_t* image;
		if( !(image = jas_image_decode( in, -1, 0 ) ) ) {
			jas_stream_close( in );
			return 0;
		} // if
		jas_stream_close( in );

		return image;
	} // read_image

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
	jcbcr_to_rgb( int jcbcr[3], int* const ret )
	{
		int dummy[3] = {
			(int)((1/1.772) * (jcbcr[0] + 1.402 * jcbcr[2])),
			(int)((1/1.772) * (jcbcr[0] - 0.34413 * jcbcr[1] - 0.71414 * jcbcr[2])),
			(int)((1/1.772) * (jcbcr[0] + 1.772 * jcbcr[1])) };
		for( int k = 0; k < 3; ++k ) ret[k] = dummy[k];
	} // jcbcr_to_rgb


	void draw_view( gs_t& gs, QImage& qti )
	{
		int numcmpts = 3;
		bool color = true;

		switch( jas_image_colorspace( gs.image ) ) {
			case JAS_IMAGE_CS_RGB:
			case JAS_IMAGE_CS_YCBCR:
				color = true;
				numcmpts = 3;
				break;
			case JAS_IMAGE_CS_GRAY:
				color = false;
				numcmpts = 1;
				break;
		} // switch


		int* cmptlut = gs.cmptlut;
		int v[3];

		uint32_t* data = (uint32_t*)qti.bits();

		uint width = jas_image_width( gs.image );
		uint height = jas_image_height( gs.image );
		for( uint y = 0; y < height; ++y ) {
			for( uint x = 0; x < width; ++x ) {
				if( color ) {
					for( int k = 0; k < numcmpts; ++k ) {
						v[k] = ( x < jas_image_cmptwidth(gs.image, cmptlut[k])
								&& y < jas_image_cmptheight(gs.image, cmptlut[k]))
							? jas_matrix_get(gs.cmpts[cmptlut[k]], y, x) : 0;
						// if the precision of the component is too small, increase
						// it to use the complete value range.
						v[k] <<= 8 - jas_image_cmptprec(gs.image, cmptlut[k]);
					} // for k
					switch( jas_image_colorspace( gs.image ) ) {
						case JAS_IMAGE_CS_RGB:
							break;
						case JAS_IMAGE_CS_YCBCR:
							// the first argument is passed by value
							jcbcr_to_rgb( v, v );
							break;
					} // switch
				} else { // !color
					v[0] = jas_matrix_get( gs.cmpts[cmptlut[0]], y, x );
					v[0] <<= 8 - jas_image_cmptprec(gs.image, 0);
					v[1] = v[2] = v[0];
				} // else

				for( int k = 0; k < 3; ++k ) {
					if( v[k] < 0 ) v[k] = 0;
					else if( v[k] > 255 ) v[k] = 255;
				} // for k

				*data++ = qRgb( v[0], v[1], v[2] );
			} // for x
		} // for y
	} // draw_view
} // namespace


void
kimgio_jp2_read( QImageIO* io )
{
	if( jas_init() ) return;

	gs_t gs;
	if( !(gs.image = read_image( io )) ) return;

	if( !init_components( gs ) ) return;
	init_cmptlut( gs );

	QImage image( jas_image_width( gs.image ), jas_image_height( gs.image ),
			32 );
	draw_view( gs, image );

	if( gs.image ) jas_image_destroy( gs.image );
	for( uint i = 0; i < MAXCMPTS; ++i )
		if( gs.cmpts[i] ) jas_matrix_destroy( gs.cmpts[i] );

	io->setImage( image );
	io->setStatus( 0 );
} // kimgio_jp2_read


void
kimgio_jp2_write( QImageIO* )
{
	// TODO
} // kimgio_jp2_write

#endif // HAVE_JASPER

