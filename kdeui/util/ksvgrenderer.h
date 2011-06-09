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
 */

#ifndef KSVGRENDERER_H
#define KSVGRENDERER_H

#include <kdeui_export.h>

#include <QtSvg/QSvgRenderer>

/**
 * Thin wrapper around QSvgRenderer with SVGZ support.
 *
 * @deprecated At least since Qt 4.6, QSvgRenderer has built-in SVGZ support,
 * so you can use QSvgRenderer instead of KSvgRenderer in new code, and
 * replace KSvgRenderer by QSvgRenderer in old code without any regressions.
 */
class KDEUI_EXPORT_DEPRECATED KSvgRenderer : public QSvgRenderer
{
    Q_OBJECT

public:
    KSvgRenderer(QObject *parent = 0);
    explicit KSvgRenderer(const QString &filename, QObject *parent = 0);
    explicit KSvgRenderer(const QByteArray &contents, QObject *parent = 0);

public Q_SLOTS:
    bool load(const QString &filename);
    bool load(const QByteArray &contents);
};


#endif // KSVGRENDERER_H
