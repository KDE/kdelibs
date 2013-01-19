/* This file is part of the KDE project
   Copyright (C) 2002-2003 Nadeem Hasan <nhasan@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the Lesser GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef KIMG_PCX_H
#define KIMG_PCX_H

#include <QImageIOPlugin>

class PCXHandler : public QImageIOHandler
{
public:
    PCXHandler();

    virtual bool canRead() const;
    virtual bool read(QImage *image);
    virtual bool write(const QImage &image);

    static bool canRead(QIODevice *device);
};

class PCXPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "pcx.json")

public:
    virtual Capabilities capabilities(QIODevice *device, const QByteArray &format) const;
    virtual QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const;
};

#endif // KIMG_PCX_H
