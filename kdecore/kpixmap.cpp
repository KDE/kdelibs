/*
 * $Id$
 *
 * $Log$
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

bool KPixmap::loadXPM( const char *_file ) {
QFile f( _file );
  
    if ( !f.open( IO_ReadOnly ) )
      return false;
  
    bool head_passed = false;
    bool colors_passed = false;
  
    int x=-1;
    int y=-1;
    int ncolors=-1;
    int chars_per_pixel=-1;
    int colors_done = 0;
  
    char *color_chars = 0L;
    QColor *colors = 0L;
    int transparent = -1;
  
    int col = 0;
  
    QPainter painter;
    QPainter maskpaint;
    
    while ( !f.atEnd() )
      {
	char str[ 1024 ];
	f.readLine( str, 1024 );
	if ( str[0] == '\"' )
	  {
	    if ( !head_passed )
	      {
		char *p = str + 1;
		while (isspace(*p))p++;
		x = atoi( p );
		while (isdigit(*p))p++;
		while (isspace(*p))p++;
		y = atoi( p );
		while (isdigit(*p))p++;
		while (isspace(*p))p++;
		ncolors = atoi( p );
		while (isdigit(*p))p++;
		while (isspace(*p))p++;
		chars_per_pixel = atoi( p );
			  
		head_passed = true;
			  
		colors = new QColor[ ncolors ];
		color_chars = new char[ ncolors ];
			  
		resize( x, y );
		bitmap.resize( x, y );
			  
		painter.begin( this );
		maskpaint.begin( &bitmap );
			  
		fill( white );
		bitmap.fill( color0 );
	      }
	    else if ( !colors_passed )
	      {
		char *p = str + 1;
		color_chars[ colors_done ] = *p;
		do 
		  p = strchr( p+1, 'c' );
		while (p != 0L && 
		       !isspace(*(p-1)) &&
		       !isspace(*p+1));
		if ( p == 0L )
		  {
		    painter.end();
		    maskpaint.end();
		    resize( 0, 0 );
		    bitmap.resize( 0, 0 );
		    delete colors;
		    delete color_chars;
		    return false;
		  }
		p+=2;
			  
		char *p2 = strchr( p, '\"' );
		if ( p2 == 0L )
		  {
		    painter.end();
		    maskpaint.end();
		    resize( 0, 0 );
		    bitmap.resize( 0, 0 );
		    delete colors;
		    delete color_chars;
		    return false;
		  }
		*p2 = 0;
			  
		/* if ( *p == '#' && strlen( p ) == 6 )
		   {
		   }
		   else if ( *p == '#' && strlen( p ) == 12 )
		   {
		   }
		   else */
			  
		if ( strcasecmp( p, "None" ) == 0 )
		  {
		    transparent = colors_done;
		    colors[ colors_done ].setRgb( 0, 0, 0 );
		  }
		else
		  colors[ colors_done ].setNamedColor( p );
			  
		colors_done++;
		if ( colors_done == ncolors )
		  colors_passed = true;
	      }
	    else
	      {
		if ( col < y )
		  {
		    char *p = str + 1;
				  
		    for ( int i = 0; i < x; i++ )
		      {
			for ( int j = 0; j < ncolors; j++ )
			  if ( *p == color_chars[ j ] )
			    {
			      painter.setPen( colors[ j ] );
			      painter.drawPoint( i, col );
			      if ( j == transparent )
				maskpaint.setPen( color0 );
			      else
				maskpaint.setPen( color1 );
			      maskpaint.drawPoint( i, col );
			    }
			p++;
		      }
		    col++;
		  }
	      }
	  }
      }

    painter.end();
    maskpaint.end();
  
    setMask( bitmap );
  
    if ( colors != 0 )
      delete colors;
    if ( color_chars != 0 )
      delete color_chars;
  
    f.close();

    return true;
}

#define M_PIXMAP_TYPE_UNKNOWN 0
#define M_PIXMAP_TYPE_XPM     1
#define M_PIXMAP_TYPE_XVPICS  2


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
  //  int pixmap_type = M_PIXMAP_TYPE_UNKNOWN;

  // Old strategie: _file does not end with .xpm, call QPixmap-loader.
  // New strategie: use QPixmap::imageformat(const char *filename)!

	if (_file) {
//#ifdef DEBUG	
//  printf("KPixmap::load(\"%s\"): type %s\n", 
//	 _file, QPixmap::imageFormat(_file));
//#endif

  // let QT find out if it can handle this, if so, call QPixmap
  if (QPixmap::imageFormat(_file) != NULL)
    return QPixmap::load( _file );

  // otherwise call our readingfunctions.

  // FIXME! This should of course not be decided on .xpm filename!
  if (!strstr( _file, ".xpm" ) == 0 ) 
    return loadXPM(_file);

  return loadXVPICS(_file);
  } else // if filename is NULL
	return false;
}
