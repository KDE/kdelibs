////////////////////////////////////////////////////
//
// Transparent support for JPEG files in Qt Pixmaps,
// using IJG JPEG library.
//
// Sirtaj Kang, Oct 1996.
//
// $Id$

#ifdef HAVE_CONFIG_H
#include"config.h"
#endif

#ifdef HAVE_LIBJPEG

#include<stdio.h>
#include<assert.h>
#include<sys/types.h>

#include"qimage.h"
#include"qdstream.h"
#include"qcolor.h"
#include"qpixmap.h"
#include"jpeg.h"

//
// Hack to fix INT16 redefinition problem
//

#define XMD_H
typedef short INT16;
extern "C" {
#include<jpeglib.h>
}
#undef XMD_H

//////
// Plug-in source manager for IJG JPEG compression/decompression library
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

/////////////////////
//
// No JPEG save support yet
//

void kimgio_jpeg_write(QImageIO *)
{
    fprintf(stderr, "JPEG saving unimplemented.\n");
    return;
}




///////////
//
// Plug-in to read files from JPEG into QImage
//

void kimgio_jpeg_read(QImageIO * iio)
{
    QIODevice *d = iio->ioDevice();
    QImage image;
    QDataStream s(d);
    JSAMPROW buffer[1];
    unsigned int *ui_row;
    unsigned char *uc_row, *uc_row_index;
    unsigned int depth, col;
    unsigned char r,g,b;

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
	    uc_row = buffer[0];

	    jpeg_read_scanlines(&cinfo, buffer, 1);

	    for (col = 0; col < cinfo.output_width; col++)
		*uc_row_index++ = *uc_row++;
	}
	// Decompress 24-bit
    } else {
	while (cinfo.output_scanline < cinfo.output_height) {
	    ui_row = (unsigned int *)
		image.scanLine(cinfo.output_scanline);
	    uc_row = buffer[0];

	    jpeg_read_scanlines(&cinfo, buffer, 1);

	    for (col = 0; col < cinfo.output_width; col++) {
		r = *uc_row++;
		g = *uc_row++;
		b = *uc_row++;
		*ui_row++ = qRgb(r, g, b);
            }
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
	fprintf(stderr, "warning: premature EOF in file.\n");

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

#endif
