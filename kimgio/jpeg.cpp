////////////////////////////////////////////////////
//
// Transparent support for JPEG files in Qt Pixmaps,
// using IJG JPEG library.
//
// Sirtaj Kang, Oct 1996.
// JPEG-write bindings contributed by Troll Tech
//
// $Id$

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_LIBJPEG

#include <sys/types.h>

#include <assert.h>
#include <setjmp.h>
#include <stdio.h>

// need to define this to prevent clash with jpeglib
#define QT_CLEAN_NAMESPACE

#include <qglobal.h>
#include <qimage.h>
#include <qdatastream.h>
#include <qcolor.h>
#include <qpixmap.h>

#include "jpeg.h"

// this is just stupid. jpeg should handle this itself.
#define HAVE_PROTOTYPES 

extern "C" {
	#include <jpeglib.h>
}

//////
// Plug-in source manager for IJG JPEG compression/decompression library
//


/////////////////////
//
// Plug-in to write QImage into JPEG files
// (contributed from qimgio)
//


struct kimgio_error_mgr : public jpeg_error_mgr {
    jmp_buf setjmp_buffer;
};

static
void kimgio_error_exit (j_common_ptr cinfo)
{
    kimgio_error_mgr* myerr = (kimgio_error_mgr*) cinfo->err;
    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, buffer);
    qWarning(buffer);
    longjmp(myerr->setjmp_buffer, 1);
}


// Buffer for file writes (max bytes)

#define OUTPUT_BUFFER_SIZE 4096

struct kimgio_jpeg_destination_mgr : public jpeg_destination_mgr {
    // Nothing dynamic - cannot rely on destruction over longjump
    QImageIO* iio;
    JOCTET buffer[OUTPUT_BUFFER_SIZE];

public:
    kimgio_jpeg_destination_mgr(QImageIO* iio)
    {
	jpeg_destination_mgr::init_destination = init_destination;
	jpeg_destination_mgr::empty_output_buffer = empty_output_buffer;
	jpeg_destination_mgr::term_destination = term_destination;
	this->iio = iio;
	next_output_byte = buffer;
	free_in_buffer = OUTPUT_BUFFER_SIZE;
    }

    static void init_destination(j_compress_ptr)
    {
    }

    static void exit_on_error(j_compress_ptr cinfo, QIODevice* dev)
    {
	if (dev->status() == IO_Ok) {
	    return;
        } else {
	    // cinfo->err->msg_code = JERR_FILE_WRITE; 
	    (*cinfo->err->error_exit)((j_common_ptr)cinfo);
	}
    }

    static boolean empty_output_buffer(j_compress_ptr cinfo)
    {
	kimgio_jpeg_destination_mgr* dest = (kimgio_jpeg_destination_mgr*)cinfo->dest;
	QIODevice* dev = dest->iio->ioDevice();

	if ( dev->writeBlock( (char*)dest->buffer, OUTPUT_BUFFER_SIZE ) != OUTPUT_BUFFER_SIZE )
	    exit_on_error(cinfo, dev);

	dest->next_output_byte = dest->buffer;
	dest->free_in_buffer = OUTPUT_BUFFER_SIZE;

	return TRUE;
    }

    static void term_destination(j_compress_ptr cinfo)
    {
	kimgio_jpeg_destination_mgr* dest = (kimgio_jpeg_destination_mgr*)cinfo->dest;
	QIODevice* dev = dest->iio->ioDevice();
	int n = OUTPUT_BUFFER_SIZE - dest->free_in_buffer; 

	if ( dev->writeBlock( (char*)dest->buffer, n ) != n )
	    exit_on_error(cinfo, dev);

	dev->flush();

	exit_on_error(cinfo, dev);
    }
};



int kimgio_jpeg_quality = 75; // Global (no provision for parameters)

void kimgio_jpeg_write(QImageIO *iio)
{
    QImage image = iio->image();

    struct jpeg_compress_struct cinfo;
    JSAMPROW row_pointer[1];
    row_pointer[0] = 0;

    struct kimgio_jpeg_destination_mgr *iod_dest = new kimgio_jpeg_destination_mgr(iio);
    struct kimgio_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);

    jerr.error_exit = kimgio_error_exit;

    if (!setjmp(jerr.setjmp_buffer)) {
	jpeg_create_compress(&cinfo);

	cinfo.dest = iod_dest;

	cinfo.image_width = image.width();
	cinfo.image_height = image.height();

	QRgb* cmap=0;
	bool gray=FALSE;
	switch ( image.depth() ) {
	  case 1:
	  case 8:
	    cmap = image.colorTable();
	    gray = TRUE;
	    int i;
	    for (i=image.numColors(); gray && i--; ) {
		gray &=
		       qRed(cmap[i]) == qGreen(cmap[i])
		    && qRed(cmap[i]) == qBlue(cmap[i]);
	    }
	    cinfo.input_components = gray ? 1 : 3;
	    cinfo.in_color_space = gray ? JCS_GRAYSCALE : JCS_RGB;
	    break;
	  case 32:
	    cinfo.input_components = 3;
	    cinfo.in_color_space = JCS_RGB;
	}

	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, kimgio_jpeg_quality, TRUE /* limit to baseline-JPEG values */);

	jpeg_start_compress(&cinfo, TRUE);

	row_pointer[0] = new char[cinfo.image_width*cinfo.input_components];
	int w = cinfo.image_width;
	while (cinfo.next_scanline < cinfo.image_height) {
	    char *row = row_pointer[0];
	    switch ( image.depth() ) {
	      case 1:
		if (gray) {
		    uchar* data = image.scanLine(cinfo.next_scanline);
		    if ( image.bitOrder() == QImage::LittleEndian ) {
			for (int i=0; i<w; i++) {
			    bool bit = !!(*(data + (i >> 3)) & (1 << (i & 7)));
			    row[i] = qRed(cmap[bit]);
			}
		    } else {
			for (int i=0; i<w; i++) {
			    bool bit = !!(*(data + (i >> 3)) & (1 << (7 -(i & 7))));
			    row[i] = qRed(cmap[bit]);
			}
		    }
		} else {
		    uchar* data = image.scanLine(cinfo.next_scanline);
		    if ( image.bitOrder() == QImage::LittleEndian ) {
			for (int i=0; i<w; i++) {
			    bool bit = !!(*(data + (i >> 3)) & (1 << (i & 7)));
			    *row++ = qRed(cmap[bit]);
			    *row++ = qGreen(cmap[bit]);
			    *row++ = qBlue(cmap[bit]);
			}
		    } else {
			for (int i=0; i<w; i++) {
			    bool bit = !!(*(data + (i >> 3)) & (1 << (7 -(i & 7))));
			    *row++ = qRed(cmap[bit]);
			    *row++ = qGreen(cmap[bit]);
			    *row++ = qBlue(cmap[bit]);
			}
		    }
		}
		break;
	      case 8:
		if (gray) {
		    uchar* pix = image.scanLine(cinfo.next_scanline);
		    for (int i=0; i<w; i++) {
			*row = qRed(cmap[*pix]);
			++row; ++pix;
		    }
		} else {
		    uchar* pix = image.scanLine(cinfo.next_scanline);
		    for (int i=0; i<w; i++) {
			*row++ = qRed(cmap[*pix]);
			*row++ = qGreen(cmap[*pix]);
			*row++ = qBlue(cmap[*pix]);
			++pix;
		    }
		}
		break;
	      case 32: {
		QRgb* rgb = (QRgb*)image.scanLine(cinfo.next_scanline);
		for (int i=0; i<w; i++) {
		    *row++ = qRed(*rgb);
		    *row++ = qGreen(*rgb);
		    *row++ = qBlue(*rgb);
		    ++rgb;
		}
	      }
	    }
	    jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	iio->setStatus(0);
    }

    delete iod_dest;
    delete row_pointer[0];
}




///////////
//
// Plug-in to read files from JPEG into QImage
//

// 
// Source control structure.
// 

typedef struct {
  struct jpeg_source_mgr pub;   /* public fields */

    QDataStream *infile;                /* Pointer to QIODevice object */
      JOCTET * buffer;              /* start of buffer */
        boolean start_of_file;        /* have we gotten any data yet? */
} qimageio_jpeg_source_mgr;

void qimageio_jpeg_src(j_decompress_ptr cinfo, QDataStream *image);

//
// Source manager file request plug-in methods.
//

void qimageio_init_source(j_decompress_ptr cinfo);
boolean qimageio_fill_input_buffer(j_decompress_ptr cinfo);
void qimageio_skip_input_data(j_decompress_ptr cinfo, long num_bytes);
void qimageio_term_source(j_decompress_ptr cinfo);

// Buffer for file reads (max bytes)
// ?? Is this good enough? Bigger, perhaps?

#define INPUT_BUFFER_SIZE 4096

void kimgio_jpeg_read(QImageIO * iio)
{
    QIODevice *d = iio->ioDevice();
    QImage image;
    QDataStream s(d);
    JSAMPROW buffer[1];
    unsigned int *ui_row;
    unsigned char *uc_row, *uc_row_index;
    unsigned depth;
    unsigned col;

    // We need to know if the display can handle 32-bit images

  depth = QPixmap::defaultDepth();

    // Init jpeg decompression structures

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    qimageio_jpeg_src(&cinfo, &s);
    if( jpeg_read_header(&cinfo, (boolean) FALSE ) 
		!= JPEG_HEADER_OK ) {
	return; // Header error
    }




    // If we're in an 8bit display, we want a colourmap.

    if ((depth < 32) && (cinfo.out_color_space == JCS_RGB)) {
	cinfo.quantize_colors = (boolean) TRUE;
	cinfo.dither_mode = JDITHER_ORDERED;
    }
    jpeg_start_decompress(&cinfo);




    if (cinfo.quantize_colors == TRUE) {

	image.create(cinfo.output_width, cinfo.output_height,
		     8, cinfo.actual_number_of_colors,
		      QImage::LittleEndian);

	// Read colourmap

	for ( col = 0; col < (unsigned)cinfo.actual_number_of_colors; 
			col++ ) {
	    image.setColor(col, qRgb(cinfo.colormap[0][col],
				     cinfo.colormap[1][col],
				     cinfo.colormap[2][col]));
	}

    } else if (cinfo.out_color_space == JCS_GRAYSCALE) {

	image.create(cinfo.output_width, cinfo.output_height,
		      8, 256, QImage::LittleEndian);

	// Read colourmap

	for (col = 0; col < 256; col++) {
	    image.setColor(col, qRgb(col, col, col));
	}
    } else {
	image.create(cinfo.output_width, cinfo.output_height, 32);
    }




    // Alloc one-row buffer for scanline 
    buffer[0] = new JSAMPLE[cinfo.output_width * cinfo.output_components];




    //
    // Perform decompression
    //

    // Decompress with colormap
    if (cinfo.quantize_colors == TRUE || cinfo.out_color_space != JCS_RGB) {
	while (cinfo.output_scanline < cinfo.output_height) {
	    uc_row_index = image.scanLine(cinfo.output_scanline);
	    uc_row = (unsigned char *)buffer[0];

	    jpeg_read_scanlines(&cinfo, buffer, 1);

	    for (col = 0; col < cinfo.output_width; col++)
		*uc_row_index++ = *uc_row++;
	}
	// Decompress 24-bit
    } else {
	while (cinfo.output_scanline < cinfo.output_height) {
	    ui_row = (unsigned int *)
		image.scanLine(cinfo.output_scanline);
	    uc_row = (unsigned char *)buffer[0];

	    jpeg_read_scanlines(&cinfo, buffer, 1);

	    for (col = 0; col < cinfo.output_width; col++)
		*ui_row++ = qRgb(*uc_row++, *uc_row++, *uc_row++);
	}
    }

    // Clean up JPEG structs

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);


    // Bind new image to screen

    iio->setImage(image);
    iio->setStatus(0);

}


/////////
    //
    // JPEG plug in routines for QDataStream as input.
    //

// Registers stream routines for use by IJG JPEG library

void qimageio_jpeg_src(j_decompress_ptr cinfo, QDataStream * image)
{
    qimageio_jpeg_source_mgr *src;

    // Set up buffer for the first time

    if (cinfo->src == NULL) {

	cinfo->src = (struct jpeg_source_mgr *)
	    (*cinfo->mem->alloc_small)
	    ((j_common_ptr) cinfo,
	     JPOOL_PERMANENT,
	     sizeof(qimageio_jpeg_source_mgr));

	src = (qimageio_jpeg_source_mgr *) cinfo->src;

	src->buffer = (JOCTET *)
	    (*cinfo->mem->alloc_small)
	    ((j_common_ptr) cinfo,
	     JPOOL_PERMANENT,
	     INPUT_BUFFER_SIZE * sizeof(JOCTET));
    }
    // Register methods
    src = (qimageio_jpeg_source_mgr *) cinfo->src;

    src->pub.init_source = qimageio_init_source;
    src->pub.fill_input_buffer = qimageio_fill_input_buffer;
    src->pub.skip_input_data = qimageio_skip_input_data;
    src->pub.resync_to_restart = jpeg_resync_to_restart;
    src->pub.term_source = qimageio_term_source;

    // This is potentially dangerous, as it has chance of being
    // misinterpreted. Saves effort, tho!

    src->infile = image;

    src->pub.bytes_in_buffer = 0;	/* forces fill_input_buffer on first read */
    src->pub.next_input_byte = NULL;	/* until buffer loaded */

}


/////////
    //
    // Intializes data source
    //

void qimageio_init_source(j_decompress_ptr cinfo)
{
    qimageio_jpeg_source_mgr *ptr = (qimageio_jpeg_source_mgr *) cinfo->src;

    ptr->start_of_file = (boolean) TRUE;
}				/////////

    //
    // Reads data from stream into JPEG working buffer
    //
boolean qimageio_fill_input_buffer(j_decompress_ptr cinfo)
{
    qimageio_jpeg_source_mgr *ptr = (qimageio_jpeg_source_mgr *) cinfo->src;
    size_t nbytes;

    nbytes = ptr->infile->device()->readBlock((char *) (ptr->buffer),
					      INPUT_BUFFER_SIZE);

    if (nbytes <= 0) {

	if (ptr->start_of_file) {
	    fprintf(stderr, "error: file empty.\n");

	    return (boolean) FALSE;
	}
	qWarning("warning: premature EOF in file.\n");

	/* Insert a fake EOI marker */
	ptr->buffer[0] = (JOCTET) 0xFF;
	ptr->buffer[1] = (JOCTET) JPEG_EOI;
	nbytes = 2;

    }
    ptr->pub.next_input_byte = ptr->buffer;
    ptr->pub.bytes_in_buffer = nbytes;
    ptr->start_of_file = (boolean) FALSE;

    return (boolean) TRUE;
}



//////////
    //
    // Jumps over large chunks of data. Can probably be done quicker. (fseek()
    // style).



void qimageio_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
    qimageio_jpeg_source_mgr *ptr = (qimageio_jpeg_source_mgr *) cinfo->src;


    if (num_bytes > 0) {

	while (num_bytes > (long) ptr->pub.bytes_in_buffer) {
	    num_bytes -= (long) ptr->pub.bytes_in_buffer;
	    (void) qimageio_fill_input_buffer(cinfo);
	} ptr->pub.next_input_byte += (size_t) num_bytes;
	ptr->pub.bytes_in_buffer -= (size_t) num_bytes;
    }
}


/////////
    //
    // Clean up. Doesn't really do anything, but required for compat.
    //


void qimageio_term_source(j_decompress_ptr)
{
    return;
}

extern "C" void kimgio_init_jpeg() {
    QImageIO::defineIOHandler( "JPEG", "^\377\330\377\340..JFIF", 0,
			       kimgio_jpeg_read, kimgio_jpeg_write );
}

#endif
