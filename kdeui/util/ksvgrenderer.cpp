/*
 * This file is part of the KDE project, module kdecore.
 * Copyright (C) 2006 Oswald Buddenhagen <ossi@kde.org>
 *
 * This is free software; it comes under the GNU Library General
 * Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 */

#include "ksvgrenderer.h"

#include <kfilterdev.h>

#include <QtCore/QBuffer>
#include <QtCore/QFile>

KSvgRenderer::KSvgRenderer(QObject *parent) : QSvgRenderer(parent)
{
}

KSvgRenderer::KSvgRenderer(const QString &filename, QObject *parent) :
    QSvgRenderer(parent)
{
    load(filename);
}

KSvgRenderer::KSvgRenderer(const QByteArray &contents, QObject *parent) :
    QSvgRenderer(parent)
{
    load(contents);
}

bool
KSvgRenderer::load(const QString &filename)
{
    QFile fi(filename);
    if (!fi.open(QIODevice::ReadOnly))
        return false;
    return load(fi.readAll());
}

bool
KSvgRenderer::load(const QByteArray &contents)
{
    if (!contents.startsWith("<?xml")) {
        QBuffer buf(const_cast<QByteArray *>(&contents));
        QIODevice *flt = KFilterDev::device(
            &buf, QString::fromLatin1("application/x-gzip"), false);
        if (!flt)
            return false;
        if (!flt->open(QIODevice::ReadOnly)) {
            delete flt;
            return false;
        }
        QByteArray ar = flt->readAll();
        delete flt;
        return QSvgRenderer::load(ar);
    }
    return QSvgRenderer::load(contents);
}

#include "ksvgrenderer.moc"
