#include <unistd.h>
#include <stdio.h>
#include <qimage.h>
#include <qiodevice.h>
#include <qstring.h>
#include <qdatastream.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qpainter.h>
#include <qprinter.h>
#include <qpaintdevicemetrics.h>
#include <qstring.h>
#include <kapp.h>

#include "eps.h"

#define BUFLEN 200
char buf[BUFLEN+1];

#define BBOX "%%BoundingBox:"
#define BBOX_LEN strlen(BBOX)

int bbox ( const char *fileName, int *x1, int *y1, int *x2, int *y2)
{
	FILE * file;
	int a, b, c, d;
	int ret = FALSE;

	file = fopen (fileName, "r");

	do {
		if (! fgets (buf, BUFLEN, file)) break;

		if (strncmp (buf, BBOX, BBOX_LEN) == 0)
		{
			ret = sscanf (buf, "%s %d %d %d %d", buf,
				&a, &b, &c, &d);    
			if (ret == 5) {
				ret = TRUE;
				break;
			} 
		}  
	} while ( true );

	*x1 = a;
	*y1 = b;
	*x2 = c;
	*y2 = d;

	fclose (file);
	return ret;
}  

void kimgio_epsf_read (QImageIO *image)
{
	FILE * ghostfd, * imagefd;
	char * tmpFileName;
	int x1, y1, x2, y2;

	QString cmdBuf;
	QString tmp;

	// find bounding box
	if ( bbox (image->fileName().ascii(), &x1, &y1, &x2, &y2) == 0 ) {
		return;
	}

	// temporary file name
	tmpFileName = tmpnam(NULL); 

	if( tmpFileName == 0 ) {
		return;
	}

	// x1, y1 -> translation
	// x2, y2 -> new size

	x2 -= x1;
	y2 -= y1;

	// create GS command line

	cmdBuf = "gs -sOutputFile=";
	cmdBuf += tmpFileName;
	cmdBuf += " -q -g";
	tmp.setNum( x2 );
	cmdBuf += tmp;
	tmp.setNum( y2 );
	cmdBuf += "x";
	cmdBuf += tmp;
	cmdBuf += " -dNOPAUSE -sDEVICE=ppm -c "
		"0 0 moveto "
		"1000 0 lineto "
		"1000 1000 lineto "
		"0 1000 lineto "
		"1 1 254 255 div setrgbcolor fill "
		"0 0 0 setrgbcolor - -c showpage quit";

	// run ghostview

	ghostfd = popen (cmdBuf.ascii(), "w");

	if ( ghostfd == 0 ) {
		return;
	}

	fprintf (ghostfd, "\n%d %d translate\n", -x1, -y1);

	// write image to gs

	imagefd = fopen( image->fileName().ascii(), "r" );

	if ( imagefd != 0 )
	{
		while( fgets (buf, BUFLEN, imagefd) != 0 ) {

			fputs (buf, ghostfd);
		}

		fclose (imagefd);  
	}
	pclose ( ghostfd );

	// load image
	QImage myimage;
	bool loadstat = myimage.load (tmpFileName);

	if( loadstat ) {
		myimage.createHeuristicMask();

		image->setImage (myimage);
		image->setStatus (0);
	}

	unlink (tmpFileName);

	return;
}

// Sven Wiegand <SWiegand@tfh-berlin.de> -- eps output filter (from KSnapshot)
void kimgio_epsf_write( QImageIO *imageio )
{
  QPrinter	 psOut;
  QPainter	 p;

  // making some definitions (papersize, output to file, filename):
  psOut.setCreator( "KDE " KDE_VERSION_STRING  );
  psOut.setOutputToFile( TRUE );

  // Make up a name for the temporary file
  QString tempname;
  tempname = tmpnam(NULL); 

  psOut.setOutputFileName(tempname);

  // painting the pixmap to the "printer" which is a file
  p.begin( &psOut );

  //	 p.translate( 0, 0 );
  QPixmap img;
  img.convertFromImage(imageio->image());

  p.drawPixmap( QPoint( 0, 0 ), img );
  p.end();

  // write BoundingBox to File
  QFile		 inFile(tempname);
  QFile		 outFile(imageio->fileName());
  QString		 szBoxInfo;

  szBoxInfo.sprintf("%sBoundingBox: 0 0 %d %d\n%sEndComments\n",
		    "%%", img.width(),
		    img.height(), "%%" );

  inFile.open( IO_ReadOnly );
  outFile.open( IO_WriteOnly );

  QTextStream in( &inFile );
  QTextStream out( &outFile );
  QString		 szInLine;
  int			 nFilePos;

  do{
    nFilePos = inFile.at();
    szInLine = in.readLine();
    out << szInLine << '\n';
  }
  while( szInLine != "%%EndComments" );

  if( outFile.at( nFilePos ) ){
    out << szBoxInfo;
  }

  while( !in.eof() ){
    szInLine = in.readLine();
    out << szInLine << '\n';
  }

  inFile.close();
  outFile.close();

  unlink( tempname );
  imageio->setStatus(0);
}

extern "C" void kimgio_init_eps() {
    QImageIO::defineIOHandler("EPS", "^%!PS-Adobe-[^\n]+\n"
			      "%%BoundingBox", "T", kimgio_epsf_read, 
			      kimgio_epsf_write );
}
