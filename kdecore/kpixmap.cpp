/*
 * $Id$
 *
 * $Log$
 * Revision 1.1.1.1  1997/04/13 14:42:41  cvsuser
 * Source imported
 *
 * Revision 1.1.1.1  1997/04/09 00:28:07  cvsuser
 * Sources imported
 *
 * Revision 1.6  1997/03/09 17:43:13  kalle
 * load() returns bool
 *
 * Revision 1.5  1997/03/09 17:38:35  kalle
 * Read other formats than xpm again (bug report by Martin Jones)
 *
 * Revision 1.4  1997/01/15 20:24:48  kalle
 * merged changes from Matthias
 *
 * Revision 1.3  1996/12/07 22:14:53  kalle
 * formatting
 *
 * Revision 1.2  1996/12/07 22:13:44  kalle
 * RCS header
 *
 *
 * KPixmap implementation. Taken from kfm
 */

#include "kpixmap.h"
#include <ctype.h>

KPixmap::KPixmap() : QPixmap()
{
}

KPixmap::KPixmap( const char *_file ) : QPixmap()
{
  load( _file );
}

bool KPixmap::loadXVPICS( const char *_file ) {
QFile f( _file );
  
    if ( !f.open( IO_ReadOnly ) )
      return false;
  
    int x=-1;
    int y=-1;
    int maxval=-1;
  
    QPainter painter;
    QPainter maskpaint;

    char str[ 1024 ];

    // magic number must be "P7 332"
    f.readLine( str, 1024);
    if (strncmp(str,"P7 332",6)) return false;

    // next line #XVVERSION
    f.readLine( str, 1024 );
    if (strncmp(str, "#XVVERSION", 10)) 
      return false;

    // now it gets interesting, #BUILTIN means we are out.
    // if IMGINFO comes, we are happy!
    f.readLine( str, 1024 );
    if (strncmp(str, "#IMGINFO:", 9))
      return false;
    
    // after this an #END_OF_COMMENTS signals everything to be ok!
    f.readLine( str, 1024 );
    if (strncmp(str, "#END_OF", 7))
      return false;

    // now a last line with width, height, maxval which is supposed to be 255
    f.readLine( str, 1024 );
    sscanf(str, "%d %d %d", &x, &y, &maxval);

    if (maxval != 255) return false;

    // now follows a binary block of x*y bytes. 

    char block[x*y];

    if (f.readBlock(block, x*y) != x*y) 
      printf("kpixmap::readXVPICS could not read datablock of %d bytes\n", 
	     x*y);

    resize( x,y );
    bitmap.resize( x,y );
			  
    painter.begin( this );
    maskpaint.begin( &bitmap );
			  
    fill( white );
    bitmap.fill( color0 );
    
    // how do the color handling? they are absolute 24bpp
    // or at least can be calculated as such.

    int i, maxi, r,g,b, posx, posy;
    QColor color;

    maskpaint.setPen( color1);

    char *p;

    maxi = x*y;

    for (i=0, p=block; i<maxi; i++, p++) {

      r =  ( ((int) ((*p >> 5) & 0x07)) * 255) / 7;      
      g =  ( ((int) ((*p >> 2) & 0x07)) * 255) / 7;   
      b =  ( ((int) ((*p >> 0) & 0x03)) * 255) / 3;

      posx =  (int) i%x;
      posy =  (int) i/x;

      color.setRgb(r,g,b);

      painter.setPen(color);
      painter.drawPoint(posx, posy);
      maskpaint.drawPoint(posx, posy);
    }

    painter.end();
    maskpaint.end();
  
    setMask( bitmap );
  
    f.close();

    return true;
}

bool KPixmap::load( const char *_file )
{
	if (_file) {

  // let QT find out if it can handle this, if so, call QPixmap
  if (QPixmap::imageFormat(_file) != NULL)
    return QPixmap::load( _file );

  // otherwise call ours 

  return loadXVPICS(_file);
  } else // if filename is NULL
	return false;
}
