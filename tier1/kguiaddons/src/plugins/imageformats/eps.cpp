/**
* QImageIO Routines to read/write EPS images.
* copyright (c) 1998 Dirk Schoenberger <dirk.schoenberger@freenet.de>
* Copyright (c) 2013 Alex Merry <alex.merry@kdemail.net>
* Includes code by Sven Wiegand <SWiegand@tfh-berlin.de> from KSnapshot
*
* This library is distributed under the conditions of the GNU LGPL.
*/
#include "eps.h"
#include <unistd.h>
#include <stdio.h>
#include <QDebug>
#include <QImage>
#include <QFile>
#include <QLoggingCategory>
#include <QPainter>
#include <QPrinter>
#include <QProcess>
#include <QtCore/QTextStream>
#include <QtCore/QTemporaryFile>

Q_LOGGING_CATEGORY(EPSPLUGIN, "epsplugin")
//#define EPS_PERFORMANCE_DEBUG 1

#define BUFLEN 200

#define BBOX "%%BoundingBox:"
#define BBOX_LEN strlen(BBOX)

static bool seekToCodeStart(QIODevice * io, quint32 & ps_offset, quint32 & ps_size)
{
    char buf[4]; // We at most need to read 4 bytes at a time
    ps_offset = 0L;
    ps_size = 0L;

    if (io->read(buf, 2) != 2) { // Read first two bytes
        qCDebug(EPSPLUGIN) << "EPS file has less than 2 bytes.";
        return false;
    }

    if (buf[0] == '%' && buf[1] == '!') { // Check %! magic
        qCDebug(EPSPLUGIN) << "normal EPS file";
    } else if (buf[0] == char(0xc5) && buf[1] == char(0xd0)) { // Check start of MS-DOS EPS magic
        // May be a MS-DOS EPS file
        if (io->read(buf + 2, 2) != 2) { // Read further bytes of MS-DOS EPS magic
            qCDebug(EPSPLUGIN) << "potential MS-DOS EPS file has less than 4 bytes.";
            return false;
        }
        if (buf[2] == char(0xd3) && buf[3] == char(0xc6)) { // Check last bytes of MS-DOS EPS magic
            if (io->read(buf, 4) != 4) { // Get offset of PostScript code in the MS-DOS EPS file.
                qCDebug(EPSPLUGIN) << "cannot read offset of MS-DOS EPS file";
                return false;
            }
            ps_offset // Offset is in little endian
                = ((unsigned char) buf[0])
                  + ((unsigned char) buf[1] << 8)
                  + ((unsigned char) buf[2] << 16)
                  + ((unsigned char) buf[3] << 24);
            if (io->read(buf, 4) != 4) { // Get size of PostScript code in the MS-DOS EPS file.
                qCDebug(EPSPLUGIN) << "cannot read size of MS-DOS EPS file";
                return false;
            }
            ps_size // Size is in little endian
                = ((unsigned char) buf[0])
                  + ((unsigned char) buf[1] << 8)
                  + ((unsigned char) buf[2] << 16)
                  + ((unsigned char) buf[3] << 24);
            qCDebug(EPSPLUGIN) << "Offset: " << ps_offset <<" Size: " << ps_size;
            if (!io->seek(ps_offset)) { // Get offset of PostScript code in the MS-DOS EPS file.
                qCDebug(EPSPLUGIN) << "cannot seek in MS-DOS EPS file";
                return false;
            }
            if (io->read(buf, 2) != 2) { // Read first two bytes of what should be the Postscript code
                qCDebug(EPSPLUGIN) << "PostScript code has less than 2 bytes.";
                return false;
            }
            if (buf[0] == '%' && buf[1] == '!') { // Check %! magic
                qCDebug(EPSPLUGIN) << "MS-DOS EPS file";
            } else {
                qCDebug(EPSPLUGIN) << "supposed Postscript code of a MS-DOS EPS file doe not start with %!.";
                return false;
            }
        } else {
            qCDebug(EPSPLUGIN) << "wrong magic for potential MS-DOS EPS file!";
            return false;
        }
    } else {
        qCDebug(EPSPLUGIN) << "not an EPS file!";
        return false;
    }
    return true;
}

static bool bbox(QIODevice *io, int *x1, int *y1, int *x2, int *y2)
{
    char buf[BUFLEN + 1];

    bool ret = false;

    while (io->readLine(buf, BUFLEN) > 0) {
        if (strncmp(buf, BBOX, BBOX_LEN) == 0) {
            // Some EPS files have non-integer values for the bbox
            // We don't support that currently, but at least we parse it
            float _x1, _y1, _x2, _y2;
            if (sscanf(buf, "%*s %f %f %f %f",
                       &_x1, &_y1, &_x2, &_y2) == 4) {
                qCDebug(EPSPLUGIN) << "BBOX: " << _x1 << " " << _y1 << " " << _x2 << " " << _y2;
                *x1 = (int)_x1; *y1 = (int)_y1; *x2 = (int)_x2; *y2 = (int)_y2;
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
    if (canRead(device())) {
        setFormat("eps");
        return true;
    }
    return false;
}

bool EPSHandler::read(QImage *image)
{
    qCDebug(EPSPLUGIN) << "starting...";

    FILE * ghostfd;
    int x1, y1, x2, y2;
#ifdef EPS_PERFORMANCE_DEBUG
    QTime dt;
    dt.start();
#endif

    QString cmdBuf;
    QString tmp;

    QIODevice* io = device();
    quint32 ps_offset, ps_size;

    // find start of PostScript code
    if (!seekToCodeStart(io, ps_offset, ps_size))
        return false;

    // find bounding box
    if (!bbox(io, &x1, &y1, &x2, &y2)) {
        qCDebug(EPSPLUGIN) << "no bounding box found!";
        return false;
    }

    QTemporaryFile tmpFile;
    if (!tmpFile.open()) {
        qWarning() << "Could not create the temporary file" << tmpFile.fileName();
        return false;
    }

    // x1, y1 -> translation
    // x2, y2 -> new size

    x2 -= x1;
    y2 -= y1;
    qCDebug(EPSPLUGIN) << "origin point: " << x1 << "," << y1 << "  size:" << x2 << "," << y2;
    double xScale = 1.0;
    double yScale = 1.0;
    int wantedWidth = x2;
    int wantedHeight = y2;

    // create GS command line

    cmdBuf = QLatin1String("gs -sOutputFile=");
    cmdBuf += tmpFile.fileName();
    cmdBuf += QLatin1String(" -q -g");
    tmp.setNum(wantedWidth);
    cmdBuf += tmp;
    tmp.setNum(wantedHeight);
    cmdBuf += QLatin1Char('x');
    cmdBuf += tmp;
    cmdBuf += QLatin1String(" -dSAFER -dPARANOIDSAFER -dNOPAUSE -sDEVICE=ppm -c "
                            "0 0 moveto "
                            "1000 0 lineto "
                            "1000 1000 lineto "
                            "0 1000 lineto "
                            "1 1 254 255 div setrgbcolor fill "
                            "0 0 0 setrgbcolor - -c showpage quit");

    // run ghostview

    ghostfd = popen(QFile::encodeName(cmdBuf).constData(), "w");

    if (ghostfd == 0) {
        qCDebug(EPSPLUGIN) << "no GhostScript?";
        return false;
    }

    fprintf(ghostfd, "\n%d %d translate\n", -qRound(x1 * xScale), -qRound(y1 * yScale));

    // write image to gs

    io->reset(); // Go back to start of file to give all the file to GhostScript
    if (ps_offset > 0L) // We have an offset
        io->seek(ps_offset);
    QByteArray buffer(io->readAll());

    // If we have no MS-DOS EPS file or if the size seems wrong, then choose the buffer size
    if (ps_size <= 0 || ps_size > (unsigned int)buffer.size())
        ps_size = buffer.size();

    fwrite(buffer.data(), sizeof(char), ps_size, ghostfd);
    buffer.resize(0);

    pclose(ghostfd);

    // load image
    if (image->load(tmpFile.fileName())) {
        qCDebug(EPSPLUGIN) << "success!";
#ifdef EPS_PERFORMANCE_DEBUG
        qCDebug(EPSPLUGIN) << "Loading EPS took " << (float)(dt.elapsed()) / 1000 << " seconds";
#endif
        return true;
    }

    qCDebug(EPSPLUGIN) << "no image!" << endl;
    return false;
}


bool EPSHandler::write(const QImage &image)
{
    QPrinter psOut(QPrinter::PrinterResolution);
    QPainter p;

    QTemporaryFile tmpFile(QStringLiteral("XXXXXXXX.pdf"));
    if (!tmpFile.open())
        return false;

    psOut.setCreator(QStringLiteral("KDE EPS image plugin"));
    psOut.setOutputFileName(tmpFile.fileName());
    psOut.setOutputFormat(QPrinter::PdfFormat);
    psOut.setFullPage(true);
    psOut.setPaperSize(image.size(), QPrinter::DevicePixel);

    // painting the pixmap to the "printer" which is a file
    p.begin(&psOut);
    p.drawImage(QPoint(0, 0), image);
    p.end();

    QProcess converter;
    converter.setProcessChannelMode(QProcess::ForwardedErrorChannel);
    converter.setReadChannel(QProcess::StandardOutput);

    // pdftops comes with Poppler and produces much smaller EPS files than GhostScript
    QStringList pdftopsArgs;
    pdftopsArgs << QStringLiteral("-eps")
                << tmpFile.fileName()
                << QStringLiteral("-");
    qCDebug(EPSPLUGIN) << "Running pdftops with args" << pdftopsArgs;
    converter.start(QStringLiteral("pdftops"), pdftopsArgs);

    if (!converter.waitForStarted()) {
        // GhostScript produces huge files, and takes a long time doing so
        QStringList gsArgs;
        gsArgs << QStringLiteral("-q") << QStringLiteral("-P-")
               << QStringLiteral("-dNOPAUSE") << QStringLiteral("-dBATCH")
               << QStringLiteral("-dSAFER")
               << QStringLiteral("-sDEVICE=epswrite")
               << QStringLiteral("-sOutputFile=-")
               << QStringLiteral("-c")
               << QStringLiteral("save") << QStringLiteral("pop")
               << QStringLiteral("-f")
               << tmpFile.fileName();
        qCDebug(EPSPLUGIN) << "Failed to start pdftops; trying gs with args" << gsArgs;
        converter.start(QStringLiteral("gs"), gsArgs);

        if (!converter.waitForStarted(3000)) {
            qWarning() << "Creating EPS files requires pdftops (from Poppler) or gs (from GhostScript)";
            return false;
        }
    }

    while (converter.bytesAvailable() || (converter.state() == QProcess::Running && converter.waitForReadyRead(2000))) {
        device()->write(converter.readAll());
    }

    return true;
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

QImageIOPlugin::Capabilities EPSPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "eps" || format == "epsi" || format == "epsf")
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
