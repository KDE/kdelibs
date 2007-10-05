/*
 *
 * This file is part of the KDE project, module kdecore.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 *                    Antonio Larrosa <larrosa@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
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

#ifndef K3ICON_P_H
#define K3ICON_P_H

#include <QtCore/QString>

#include <kdeui_export.h>
#include <kiconloader.h>

/**
 * @internal
 * One icon as found by KIconTheme. Also serves as a namespace containing
 * icon related constants.
 * @warning You should not use this class externally. This class is exported because
 *          the KCM needs it.
 */
class KDEUI_EXPORT K3Icon
{
public:
    K3Icon();
    ~K3Icon();

    bool isValid() const;
    int size;
    KIconLoader::Context context;
    KIconLoader::Type type;
    int threshold;
    QString path;

private:
    class KIconPrivate;
    KIconPrivate * d;
};

#endif
