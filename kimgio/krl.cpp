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

#include<stdio.h>
#include<stdlib.h>
#include<qimage.h>
#include<qfile.h>

void kimgio_krl_read( QImageIO *io )
{
	char tmp[512];
	short *col;
	// open krl file

	QImage image;
	FILE *fp = fopen ( io->fileName(), "rb" );
        fread(&tmp,512,1,fp);

// Will this bring problems for big-endian people ?

	w = *(short *)&tmp[34];
	h = *(short *)&tmp[36];
 
// There may be better solutions to do this but this seems to work

	cc1=(unsigned char *)&w;
	cc2=cc1+1;
	cc3=*cc1;
	*cc1=*cc2;
	*cc2=cc3;
	cc1=(unsigned char *)&h;
	cc2=cc1+1;
	cc3=*cc1;
	*cc1=*cc2;
	*cc2=cc3;
	//printf("Cool, a %d x %d krl image\n", w, h);
                                                 

        QImage *img=new QImage(w,h,32);
        unsigned char *c1,*c2,c3;
	unsigned char *ptr=new char[w*h*sizeof(col)];
	fread(&ptr,w*h,sizeof(col),fp);

// krl images have 10 bits per pixel, so we should get the maximum
// and minimum to convert the output to 8 bits

	col=(short *)ptr;
	for (i=0;i<h;i++)
   	{
      	   for (j=0;j<w;j++)
      	   {
           c1=(unsigned char *)col;
           c2=c1+1;
           c3=*c1;
           *c1=*c2;
           *c2=c3;
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
           c1=(unsigned char *)col;
           c2=c1+1;
           c3=*c1;
           *c1=*c2;
           *c2=c3;
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
	fclose( fp );

	return;
}

void kimgio_png_write( QImageIO *iio )
{
        // TODO: implement this
        warning("kimgio_krl_write: not (yet?) implemented");
}

extern "C" void kimgio_init_png() {
    QImageIO::defineIOHandler( "KRL", "KRL", 0,
			       kimgio_krl_read, kimgio_krl_write );     
}

