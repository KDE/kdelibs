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

#include "krl.h"

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

// KRL has a 512 bytes header, although not much of it is used 
	int rbytes = io->readBlock( buffer, 512 );

	if( rbytes < 512 ) {
		qWarning( "krl_read: wanted %d bytes, read %d", 512, rbytes );
		return;
	}

	Q_INT16 w = LOCAL_ENDIAN( buffer[ 35 ], buffer[ 34 ] );
	Q_INT16 h = LOCAL_ENDIAN( buffer[ 37 ], buffer[ 36 ] );
w=423;h=279;

	int samples = (int)(w*h);

	//debug( "kimgio_krl_read: image w: %d, h: %d samples: %d", 
	//		(int)w, (int)h, samples );

	QImage image( w, h, 32 );
	rbytes = 0;
	Q_INT16 *currptr = (Q_INT16 *)buffer, 
			*endptr = (Q_INT16 *) (buffer + BUF_SIZE);
        Q_INT16 min=32766;
        Q_INT16 max=0; 

//First, we have to guess the maximum and minimum
        rbytes = io->readBlock( (char *)buffer, BUF_SIZE );
        for ( int samp = 0; samp < samples; samp++, currptr++ ) {
                if ( currptr >= endptr ) {
                        rbytes = io->readBlock( (char *)buffer, BUF_SIZE );
                        currptr = (Q_INT16 *)buffer;
                }
                SWAP_BYTES( *(unsigned short *)currptr );
                if (*currptr<min) min=*currptr;
                if (*currptr>max) max=*currptr;
        }
 
        currptr = (Q_INT16 *)buffer;
        uint *ptr = (uint *)image.bits();
 
//Let's do a real read   
	io->at(512);     
	for ( int samp = 0; samp < samples; samp++, currptr++, ptr++ ) {
		if ( currptr >= endptr ) {
			rbytes = io->readBlock( (char *)buffer, BUF_SIZE );
			currptr = (Q_INT16 *)buffer;
		}
		SWAP_BYTES( *(unsigned short *)currptr );

// It's not enought to delete the two least significant bits, because krl
// doesn't use the whole range of colours all the time and we want a quality
// image, so we have to scale the used colour range
		*ptr = (Q_INT16)((((unsigned short)*currptr)-min)*255.0/(max-min));
//		*ptr = ((*currptr >> 2) & 0xFF);

		*ptr = qRgb( *ptr, *ptr, *ptr );
	}

	iio->setImage( image );
	iio->setStatus( 0 );

	return;
}

void kimgio_krl_write( QImageIO * imageio )
{
   QIODevice& f = *( imageio->ioDevice() );
   const QImage& image = imageio->image();
   char tmp[512];
   for (int p=0;p<512;p++) tmp[p]=0;

   int w=image.width();
   int h=image.height();
   unsigned char *cc1,*cc2,cc3;    
   cc1=(unsigned char *)&w;
   cc2=cc1+1;
   cc3=*cc1;
   *cc1=*cc2;
   *cc2=cc3;
   *(short *)&tmp[34]=w;

   cc1=(unsigned char *)&h;
   cc2=cc1+1;
   cc3=*cc1;
   *cc1=*cc2;
   *cc2=cc3;
   *(short *)&tmp[36]=h;

   w=image.width();
   h=image.height();
   f.writeBlock( tmp, 512 );
   Q_INT16 c;
   for (int j=0;j<h;j++)
     for (int i=0;i<w;i++)
     {
	 c=(Q_INT16)qGray(image.pixel(i,j));
         f.writeBlock((char *)&c,2);
     }

}

