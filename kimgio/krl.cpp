/*
* KRL.CPP -- QImageIO read/write handlers for
*       the krl graphic format.
*
*       Copyright (c) May 1999, Antonio Larrosa Jimenez.  Distributed under
*       the LGPL.
*
*/

#ifdef HAVE_CONFIG_H
#include"config.h"
#endif

#include<qimage.h>
#include<qfile.h>
#include<qglobal.h>
#include<qcolor.h>
#include<assert.h>

#define BUF_SIZE 1024

#define SWAP_BYTES( v )		v = ((v) >> 8) + ( ( (v) & 0xFF ) << 8 )

#define LOCAL_ENDIAN( a, b )	(((int)(b)) << 8) + (int)(b);

#if 0
void kimgio_krl_read_old( QImageIO *io )
{
	char tmp[ BUF_SIZE ];
	short *col;
	// open krl file


	QIODevice *dev = io->ioDevice();
	assert( dev != 0 );

        int bytes = dev->readBlock( &tmp, BUF_SIZE );

// Will this bring problems for big-endian people ? (Antonio)
// Does this mean that KRL has 16-bit little-endian samples? (Taj)

	int w = *(short *)&tmp[34];
	int h = *(short *)&tmp[36];
 
	SWAP_BYTES( w );
	SWAP_BYTES( h );

	//printf("Cool, a %d x %d krl image\n", w, h);
                                                 
	QImage image( w, h, 32 );
	size_t imgsize = w*h*sizeof(col);
	unsigned char *ptr=new char[ imgsize ];
	if( ptr == 0 ){
		warning( "Out of memory" );
		return;
	}

	int bytes = dev->readBlock( &tmp, imgsize );

// krl images have 10 bits per pixel, so we should get the maximum
// and minimum to convert the output to 8 bits

	col=(short *)ptr;
	for (i=0;i<h;i++)
   	{
		for (j=0;j<w;j++)
		{
			SWAP_BYTES( *col );
			if (*col>max) max=*col;
			if (*col<min) min=*col;
			col++;
		};
	};  

// And finally, the real read

	col=(short *)ptr;
	for (i=0;i<h;i++)
  	{
		for (j=0;j<w;j++)
		{
			SWAP_BYTES( *col );
			t=(short)((*col-min)*255.0/(max-min));
			//      printf("%d , %d , %d \n",t,min,max);
			img->setPixel(j,i,QColor(t,t,t).rgb());
			col++;
		};
	};
 
	io->setImage( img );
	io->setStatus( 0 );
	
	// clean up 
	delete ptr;

	return;
}
#endif

void kimgio_krl_read( QImageIO *iio )
{
	QIODevice *io = iio->ioDevice();
	assert( io != 0 );

	char buffer[ BUF_SIZE ];

	int rbytes = io->readBlock( buffer, 37 );

	if( rbytes < 37 ) {
		warning( "krl_read: wanted %d bytes, read %d", 37, rbytes );
		return;
	}

	Q_INT16 w = LOCAL_ENDIAN( buffer[ 34 ], buffer[ 35 ] );
	Q_INT16 h = LOCAL_ENDIAN( buffer[ 36 ], buffer[ 37 ] );
	int samples = (int)(w*h);

	debug( "kimgio_krl_read: image w: %d, h: %d samples: %d", 
			(int)w, (int)h, samples );

	QImage image( w, h, 32 );
	rbytes = 0;
	Q_INT16 *currptr = (Q_INT16 *)buffer, 
			*endptr = (Q_INT16 *) (buffer + BUF_SIZE);
	uint *ptr = (uint *)image.bits();
	
	for ( int samp = 0; samp < samples; samp++, currptr++, ptr++ ) {
		if ( currptr >= endptr ) {
			rbytes = io->readBlock( buffer, BUF_SIZE );
			currptr = (Q_INT16 *)buffer;
		}
		SWAP_BYTES( *currptr );
		*ptr = ((*currptr >> 2) & 0xFF);
		*ptr = qGray( *ptr, *ptr, *ptr );
	}

	iio->setImage( image );
	iio->setStatus( 0 );

	return;
}

void kimgio_krl_write( QImageIO * )
{
        // TODO: implement this
        warning("kimgio_krl_write: not (yet?) implemented");
}

extern "C" void kimgio_init_krl() {
    QImageIO::defineIOHandler( "KRL", "^DUMMYKRLREGEXP", 0,
			       kimgio_krl_read, kimgio_krl_write );     
}

