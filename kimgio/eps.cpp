// This library is distributed under the conditions of the GNU LGPL.
#include <unistd.h>
#include <stdio.h>
#include <qimage.h>
#include <qfile.h>
#include <qpainter.h>
#include <qprinter.h>
#include <kapplication.h>
#include <ktempfile.h>
#include "eps.h"

#define BUFLEN 200

#define BBOX "%%BoundingBox:"
#define BBOX_LEN strlen(BBOX)

bool bbox ( QImageIO *imageio, int *x1, int *y1, int *x2, int *y2)
{
        int ret = false;
        char buf[BUFLEN+1];

        while (imageio->ioDevice()->readLine(buf, BUFLEN) > 0)
        {
                if (strncmp (buf, BBOX, BBOX_LEN) == 0)
                {
                        // Some EPS files have non-integer values for the bbox
                        // We don't support that currently, but at least we parse it
                        float _x1, _y1, _x2, _y2;
                        if ( sscanf (buf, "%*s %f %f %f %f", 
                                &_x1, &_y1, &_x2, &_y2) == 5) {
                                *x1=(int)_x1; *y1=(int)_y1; *x2=(int)_x2; *y2=(int)_y2;
                                ret = true;
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
        //QTime dt;
        //dt.start();

        QString cmdBuf;
        QString tmp;

        // find bounding box
        if ( !bbox (image, &x1, &y1, &x2, &y2)) {
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
        //kdDebug() << "origin point: " << x1 << "," << y1 << "  size:" << x2 << "," << y2 << endl;
        double xScale = 1.0;
        double yScale = 1.0;
	bool needsScaling = false;
        int wantedWidth = x2;
        int wantedHeight = y2;

        if (image->parameters())
        {
            // Size forced by the caller
            QStringList params = QStringList::split(':', image->parameters());
            if (params.count() >= 2 && x2 != 0.0 && y2 != 0.0)
            {
                wantedWidth = params[0].toInt();
                xScale = (double)wantedWidth / (double)x2;
                wantedHeight = params[1].toInt();
                yScale = (double)wantedHeight / (double)y2;
                //kdDebug() << "wanted size:" << wantedWidth << "x" << wantedHeight << endl;
                //kdDebug() << "scaling:" << xScale << "," << yScale << endl;
		needsScaling = true;
            }
        }

        // create GS command line

        cmdBuf = "gs -sOutputFile=";
        cmdBuf += tmpFile.name();
        cmdBuf += " -q -g";
        tmp.setNum( wantedWidth );
        cmdBuf += tmp;
        tmp.setNum( wantedHeight );
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

        fprintf (ghostfd, "\n%d %d translate\n", -qRound(x1*xScale), -qRound(y1*yScale));
        if ( needsScaling )
            fprintf (ghostfd, "%g %g scale\n", xScale, yScale);

        // write image to gs

        QByteArray buffer = image->ioDevice()->readAll();
        fwrite(buffer.data(), sizeof(char), buffer.size(), ghostfd);
        buffer.resize(0);

        pclose ( ghostfd );

        // load image
        QImage myimage;
        if( myimage.load (tmpFile.name()) ) {
                image->setImage (myimage);
                image->setStatus (0);
        }

        //kdDebug() << "Loading EPS took " << (float)(dt.elapsed()) / 1000 << " seconds" << endl;
        return;
}

// Sven Wiegand <SWiegand@tfh-berlin.de> -- eps output filter (from KSnapshot)
void kimgio_eps_write( QImageIO *imageio )
{
  QPrinter psOut;
  QPainter p;

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
  QFile inFile(tmpFile.name());
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

