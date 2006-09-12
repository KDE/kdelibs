/*
 * This file is part of the KDE project, module kdecore.
 * Copyright (C) 2006 Oswald Buddenhagen <ossi@kde.org>
 *
 * This is free software; it comes under the GNU Library General
 * Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 */

#ifndef KSVGRENDERER_H
#define KSVGRENDERER_H

#include "kdelibs_export.h"

#include <QtSvg/QSvgRenderer>

/**
 * Thin wrapper around QSvgRenderer with SVGZ support.
 *
 * Please refer to the QSvgRenderer documentation for details.
 */
class KDECORE_EXPORT KSvgRenderer : public QSvgRenderer
{
    Q_OBJECT

public:
    KSvgRenderer(QObject *parent = 0);
    KSvgRenderer(const QString &filename, QObject *parent = 0);
    KSvgRenderer(const QByteArray &contents, QObject *parent = 0);

public Q_SLOTS:
    bool load(const QString &filename);
    bool load(const QByteArray &contents);
};


#endif // KSVGRENDERER_H
