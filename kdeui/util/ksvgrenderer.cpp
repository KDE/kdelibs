/*
 * This file is part of the KDE project, module kdecore.
 * Copyright (C) 2006 Oswald Buddenhagen <ossi@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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
