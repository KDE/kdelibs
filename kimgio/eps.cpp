// This library is distributed under the conditions of the GNU LGPL.
#include <unistd.h>
#include <stdio.h>
#include <qimage.h>
#include <qiodevice.h>
#include <qdatastream.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qpainter.h>
#include <qprinter.h>
#include <qpaintdevicemetrics.h>
#include <qstring.h>
#include <kapp.h>
#include <ktempfile.h>

#include "eps.h"

#define BUFLEN 200

#define BBOX "%%BoundingBox:"
#define BBOX_LEN strlen(BBOX)

int bbox ( QImageIO *imageio, int *x1, int *y1, int *x2, int *y2)
{
	int ret = FALSE;
        char buf[BUFLEN+1];

	while (imageio->ioDevice()->readLine(buf, BUFLEN) != -1)
        {
		if (strncmp (buf, BBOX, BBOX_LEN) == 0)
		{
			// Some EPS files have non-integer values for the bbox
			// We don't support that currently, but at least we parse it
			float _x1, _y1, _x2, _y2;
                        if ( sscanf (buf, "%*s %f %f %f %f", 
				&_x1, &_y1, &_x2, &_y2) == 5) {
				*x1=(int)_x1; *y1=(int)_y1; *x2=(int)_x2; *y2=(int)_y2;
				ret = TRUE;
				break;
			}
		}
	}

	return ret;
}

void kimgio_eps_read (QImageIO *image)
{
	FILE * ghostfd;
	int x1, y1, x2, y2;

	QString cmdBuf;
	QString tmp;

	// find bounding box
	if ( bbox (image, &x1, &y1, &x2, &y2) == 0 ) {
	    return;
	}

        KTempFile tmpFile;
        tmpFile.setAutoDelete(true);

	if( tmpFile.status() != 0 ) {
	    return;
	}
        tmpFile.close(); // Close the file, we just want the filename

	// x1, y1 -> translation
	// x2, y2 -> new size

	x2 -= x1;
	y2 -= y1;

	// create GS command line

	cmdBuf = "gs -sOutputFile=";
	cmdBuf += tmpFile.name();
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

	ghostfd = popen (QFile::encodeName(cmdBuf), "w");

	if ( ghostfd == 0 ) {
		return;
	}

	fprintf (ghostfd, "\n%d %d translate\n", -x1, -y1);

	// write image to gs

        QByteArray buffer = image->ioDevice()->readAll();
        fwrite(buffer.data(), sizeof(char), buffer.size(), ghostfd);
        buffer.resize(0);

	pclose ( ghostfd );

	// load image
	QImage myimage;
	bool loadstat = myimage.load (tmpFile.name());

	if( loadstat ) {
		myimage.createHeuristicMask();

		image->setImage (myimage);
		image->setStatus (0);
	}

	return;
}

// Sven Wiegand <SWiegand@tfh-berlin.de> -- eps output filter (from KSnapshot)
void kimgio_eps_write( QImageIO *imageio )
{
  QPrinter	 psOut;
  QPainter	 p;

  // making some definitions (papersize, output to file, filename):
  psOut.setCreator( "KDE " KDE_VERSION_STRING  );
  psOut.setOutputToFile( TRUE );

  KTempFile tmpFile;
  tmpFile.setAutoDelete(true);
  if ( tmpFile.status() != 0)
     return;
  tmpFile.close(); // Close the file, we just want the filename

  psOut.setOutputFileName(tmpFile.name());

  // painting the pixmap to the "printer" which is a file
  p.begin( &psOut );

  p.translate( -36, 820 - imageio->image().height() );

  p.drawImage( QPoint( 0, 0 ), imageio->image() );
  p.end();

  // write BoundingBox to File
  QFile	inFile(tmpFile.name());
  QString szBoxInfo;

  szBoxInfo.sprintf("%%%%BoundingBox: 0 0 %d %d\n", 
                    imageio->image().width(),
                    imageio->image().height());

  inFile.open( IO_ReadOnly );

  QTextStream in( &inFile );
  in.setEncoding( QTextStream::Latin1 );
  QTextStream out( imageio->ioDevice() );
  out.setEncoding( QTextStream::Latin1 );

  QString szInLine = in.readLine();
  out << szInLine << '\n';
  out << szBoxInfo;

  while( !in.atEnd() ){
    szInLine = in.readLine();
    out << szInLine << '\n';
  }

  inFile.close();

  imageio->setStatus(0);
}

