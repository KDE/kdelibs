/*  This file is part of the KDE libraries
 *  Copyright 2013 Alex Merry <alex.merry@kdemail.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License or ( at
 *  your option ) version 3 or, at the discretion of KDE e.V. ( which shall
 *  act as a proxy as in section 14 of the GPLv3 ), any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include <stdio.h>

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QImageReader>
#include <QImageWriter>
#include <QTextStream>

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    QCoreApplication::addLibraryPath(QLatin1String(PLUGIN_DIR));
    QCoreApplication::setApplicationName(QLatin1String("imageconverter"));
    QCoreApplication::setApplicationVersion(QLatin1String("1.01.01.0"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QLatin1String("Converts images from one format to another"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QLatin1String("in"), QLatin1String("input image file"));
    parser.addPositionalArgument(QLatin1String("out"), QLatin1String("output image file"));
    QCommandLineOption informat(
            QStringList() << QLatin1String("i") << QLatin1String("informat"),
            QLatin1String("Image format for input file"),
            QLatin1String("format"));
    parser.addOption(informat);
    QCommandLineOption outformat(
            QStringList() << QLatin1String("o") << QLatin1String("outformat"),
            QLatin1String("Image format for output file"),
            QLatin1String("format"));
    parser.addOption(outformat);
    QCommandLineOption listformats(
            QStringList() << QLatin1String("l") << QLatin1String("list"),
            QLatin1String("List supported image formats"));
    parser.addOption(listformats);

    parser.process(app);

    const QStringList files = parser.positionalArguments();

    if (parser.isSet(listformats)) {
        QTextStream out(stdout);
        out << "Input formats:\n";
        foreach (const QByteArray &fmt, QImageReader::supportedImageFormats()) {
            out << "  " << fmt << '\n';
        }
        out << "Output formats:\n";
        foreach (const QByteArray &fmt, QImageWriter::supportedImageFormats()) {
            out << "  " << fmt << '\n';
        }
        return 0;
    }

    if (files.count() != 2) {
        QTextStream(stdout) << "Must provide exactly two files\n";
        parser.showHelp(1);
    }
    QImageReader reader(files.at(0), parser.value(informat).toLatin1());
    QImage img = reader.read();
    if (img.isNull()) {
        QTextStream(stdout) << "Could not read image: " << reader.errorString() << '\n';
        return 2;
    }

    QImageWriter writer(files.at(1), parser.value(outformat).toLatin1());
    if (!writer.write(img)) {
        QTextStream(stdout) << "Could not write image: " << writer.errorString() << '\n';
        return 3;
    }

    return 0;
}
