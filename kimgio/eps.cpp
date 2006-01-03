// This library is distributed under the conditions of the GNU LGPL.
#include <unistd.h>
#include <stdio.h>
#include <qimage.h>
#include <qfile.h>
#include <qpainter.h>
#include <qprinter.h>
#include <QTextStream>
#include <kapplication.h>
#include <ktempfile.h>
#include <kdebug.h>
#include "eps.h"

#define BUFLEN 200

#define BBOX "%%BoundingBox:"
#define BBOX_LEN strlen(BBOX)

static bool seekToCodeStart( QIODevice * io, Q_UINT32 & ps_offset, Q_UINT32 & ps_size )
{
    char buf[4]; // We at most need to read 4 bytes at a time
    ps_offset=0L;
    ps_size=0L;

    if ( io->readBlock(buf, 2)!=2 ) // Read first two bytes
    {
        kdError(399) << "kimgio EPS: EPS file has less than 2 bytes." << endl;
        return false;
    }

    if ( buf[0]=='%' && buf[1]=='!' ) // Check %! magic
    {
        kdDebug(399) << "kimgio EPS: normal EPS file" << endl;
    }
    else if ( buf[0]==char(0xc5) && buf[1]==char(0xd0) ) // Check start of MS-DOS EPS magic
    {   // May be a MS-DOS EPS file
        if ( io->readBlock(buf+2, 2)!=2 ) // Read further bytes of MS-DOS EPS magic
        {
            kdError(399) << "kimgio EPS: potential MS-DOS EPS file has less than 4 bytes." << endl;
            return false;
        }
        if ( buf[2]==char(0xd3) && buf[3]==char(0xc6) ) // Check last bytes of MS-DOS EPS magic
        {
            if (io->readBlock(buf, 4)!=4) // Get offset of PostScript code in the MS-DOS EPS file.
            {
                kdError(399) << "kimgio EPS: cannot read offset of MS-DOS EPS file" << endl;
                return false;
            }
            ps_offset // Offset is in little endian
                = ((unsigned char) buf[0])
                + ((unsigned char) buf[1] << 8)
                + ((unsigned char) buf[2] << 16)
                + ((unsigned char) buf[3] << 24);
            if (io->readBlock(buf, 4)!=4) // Get size of PostScript code in the MS-DOS EPS file.
            {
                kdError(399) << "kimgio EPS: cannot read size of MS-DOS EPS file" << endl;
                return false;
            }
            ps_size // Size is in little endian
                = ((unsigned char) buf[0])
                + ((unsigned char) buf[1] << 8)
                + ((unsigned char) buf[2] << 16)
                + ((unsigned char) buf[3] << 24);
            kdDebug(399) << "kimgio EPS: Offset: " << ps_offset <<" Size: " << ps_size << endl;
            if ( !io->at(ps_offset) ) // Get offset of PostScript code in the MS-DOS EPS file.
            {
                kdError(399) << "kimgio EPS: cannot seek in MS-DOS EPS file" << endl;
                return false;
            }
            if ( io->readBlock(buf, 2)!=2 ) // Read first two bytes of what should be the Postscript code
            {
                kdError(399) << "kimgio EPS: PostScript code has less than 2 bytes." << endl;
                return false;
            }
            if ( buf[0]=='%' && buf[1]=='!' ) // Check %! magic
            {
                kdDebug(399) << "kimgio EPS: MS-DOS EPS file" << endl;
            }
            else
            {
                kdError(399) << "kimgio EPS: supposed Postscript code of a MS-DOS EPS file doe not start with %!." << endl;
                return false;
            }
        }
        else
        {
            kdError(399) << "kimgio EPS: wrong magic for potential MS-DOS EPS file!" << endl;
            return false;
        }
    }
    else
    {
        kdError(399) << "kimgio EPS: not an EPS file!" << endl;
        return false;
    }
    return true;
}

static bool bbox ( QIODevice *io, int *x1, int *y1, int *x2, int *y2)
{
        char buf[BUFLEN+1];

        bool ret = false;

        while (io->readLine(buf, BUFLEN) > 0)
        {
                if (strncmp (buf, BBOX, BBOX_LEN) == 0)
                {
                        // Some EPS files have non-integer values for the bbox
                        // We don't support that currently, but at least we parse it
                        float _x1, _y1, _x2, _y2;
                        if ( sscanf (buf, "%*s %f %f %f %f",
                                &_x1, &_y1, &_x2, &_y2) == 4) {
                                kdDebug(399) << "kimgio EPS BBOX: " << _x1 << " " << _y1 << " " << _x2 << " " << _y2 << endl;
                                *x1=(int)_x1; *y1=(int)_y1; *x2=(int)_x2; *y2=(int)_y2;
                                ret = true;
                                break;
                        }
                }
        }

        return ret;
}

EPSHandler::EPSHandler()
{
}

bool EPSHandler::canRead() const
{
    return canRead(device());
}

bool EPSHandler::read(QImage *image)
{
    kdDebug(399) << "kimgio EPS: starting..." << endl;

    FILE * ghostfd;
    int x1, y1, x2, y2;
    //QTime dt;
    //dt.start();

    QString cmdBuf;
    QString tmp;

    QIODevice* io = device();
    Q_UINT32 ps_offset, ps_size;

    // find start of PostScript code
    if ( !seekToCodeStart(io, ps_offset, ps_size) )
        return false;

    // find bounding box
    if ( !bbox (io, &x1, &y1, &x2, &y2)) {
        kdError(399) << "kimgio EPS: no bounding box found!" << endl;
        return false;
    }

    KTempFile tmpFile;
    tmpFile.setAutoDelete(true);

    if( tmpFile.status() != 0 ) {
        kdError(399) << "kimgio EPS: no temp file!" << endl;
        return false;
    }
    tmpFile.close(); // Close the file, we just want the filename

    // x1, y1 -> translation
    // x2, y2 -> new size

    x2 -= x1;
    y2 -= y1;
    //kdDebug(399) << "origin point: " << x1 << "," << y1 << "  size:" << x2 << "," << y2 << endl;
    double xScale = 1.0;
    double yScale = 1.0;
    bool needsScaling = false;
    int wantedWidth = x2;
    int wantedHeight = y2;

    // create GS command line

    cmdBuf = "gs -sOutputFile=";
    cmdBuf += tmpFile.name();
    cmdBuf += " -q -g";
    tmp.setNum( wantedWidth );
    cmdBuf += tmp;
    tmp.setNum( wantedHeight );
    cmdBuf += "x";
    cmdBuf += tmp;
    cmdBuf += " -dSAFER -dPARANOIDSAFER -dNOPAUSE -sDEVICE=ppm -c "
              "0 0 moveto "
              "1000 0 lineto "
              "1000 1000 lineto "
              "0 1000 lineto "
              "1 1 254 255 div setrgbcolor fill "
              "0 0 0 setrgbcolor - -c showpage quit";

    // run ghostview

    ghostfd = popen (QFile::encodeName(cmdBuf), "w");

    if ( ghostfd == 0 ) {
        kdError(399) << "kimgio EPS: no GhostScript?" << endl;
        return false;
    }

    fprintf (ghostfd, "\n%d %d translate\n", -qRound(x1*xScale), -qRound(y1*yScale));
    if ( needsScaling )
        fprintf (ghostfd, "%g %g scale\n", xScale, yScale);

    // write image to gs

    io->reset(); // Go back to start of file to give all the file to GhostScript
    if (ps_offset>0L) // We have an offset
        io->at(ps_offset);
    QByteArray buffer ( io->readAll() );

    // If we have no MS-DOS EPS file or if the size seems wrong, then choose the buffer size
    if (ps_size<=0 || ps_size>(unsigned int)buffer.size())
        ps_size=buffer.size();

    fwrite(buffer.data(), sizeof(char), ps_size, ghostfd);
    buffer.resize(0);

    pclose ( ghostfd );

    // load image
    if( image->load (tmpFile.name()) ) {
        kdDebug(399) << "kimgio EPS: success!" << endl;
        //kdDebug(399) << "Loading EPS took " << (float)(dt.elapsed()) / 1000 << " seconds" << endl;
        return true;
    }

    kdError(399) << "kimgio EPS: no image!" << endl;
    return false;
}


// Sven Wiegand <SWiegand@tfh-berlin.de> -- eps output filter (from KSnapshot)
bool EPSHandler::write(const QImage &image)
{
    QPrinter psOut(QPrinter::PrinterResolution);
    QPainter p;

    // making some definitions (papersize, output to file, filename):
    psOut.setCreator( "KDE " KDE_VERSION_STRING  );
    psOut.setOutputToFile( true );

    // Extension must be .eps so that Qt generates EPS file
    KTempFile tmpFile(QString(), ".eps");
    tmpFile.setAutoDelete(true);
    if ( tmpFile.status() != 0)
        return false;
    tmpFile.close(); // Close the file, we just want the filename

    psOut.setOutputFileName(tmpFile.name());
    psOut.setFullPage(true);

    // painting the pixmap to the "printer" which is a file
    p.begin( &psOut );
    // Qt uses the clip rect for the bounding box
    p.setClipRect( 0, 0, image.width(), image.height());
    p.drawImage( QPoint( 0, 0 ), image );
    p.end();

    // Copy file to imageio struct
    QFile inFile(tmpFile.name());
    inFile.open( QIODevice::ReadOnly );

    QTextStream in( &inFile );
    in.setEncoding( QTextStream::Latin1 );
    QTextStream out( device() );
    out.setEncoding( QTextStream::Latin1 );

    QString szInLine = in.readLine();
    out << szInLine << '\n';

    while( !in.atEnd() ){
        szInLine = in.readLine();
        out << szInLine << '\n';
    }

    inFile.close();

    return true;
}

QByteArray EPSHandler::name() const
{
    return "eps";
}

bool EPSHandler::canRead(QIODevice *device)
{
    if (!device) {
        qWarning("EPSHandler::canRead() called with no device");
        return false;
    }

    qint64 oldPos = device->pos();

    QByteArray head = device->readLine(64);
    int readBytes = head.size();
    if (device->isSequential()) {
        while (readBytes > 0)
            device->ungetChar(head[readBytes-- - 1]);
    } else {
        device->seek(oldPos);
    }

    return head.contains("%!PS-Adobe");
}

class EPSPlugin : public QImageIOPlugin
{
public:
    QStringList keys() const;
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const;
};

QStringList EPSPlugin::keys() const
{
    return QStringList() << "eps" << "EPS" << "epsi" << "EPSI" << "epsf" << "EPSF";
}

QImageIOPlugin::Capabilities EPSPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "eps" || format == "epsi" || format == "EPS" || format == "EPSI" ||
        format == "epsf" || format == "EPSF")
        return Capabilities(CanRead | CanWrite);
    if (!format.isEmpty())
        return 0;
    if (!device->isOpen())
        return 0;

    Capabilities cap;
    if (device->isReadable() && EPSHandler::canRead(device))
        cap |= CanRead;
    if (device->isWritable())
        cap |= CanWrite;
    return cap;
}

QImageIOHandler *EPSPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new EPSHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

Q_EXPORT_PLUGIN(EPSPlugin)
