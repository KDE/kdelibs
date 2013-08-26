/*  This file is part of the KDE libraries
 *  Copyright 2013 Alejandro Fiestas Olivares <afiestas@kde.org>
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

#ifndef KHINTS_SETTINGS_H
#define KHINTS_SETTINGS_H

#include <QObject>
#include <QVariant>

#include <qpa/qplatformtheme.h>

class KHintsSettings : public QObject
{
    Q_OBJECT
    public:
        explicit KHintsSettings();

        inline QVariant hint(QPlatformTheme::ThemeHint hint)
        {
            return m_hints[hint];
        }

    private Q_SLOTS:
        void setupIconLoader();

    private:
        void iconChanged(int group);

        QStringList xdgIconThemePaths() const;
        QHash<QPlatformTheme::ThemeHint, QVariant> m_hints;
};

#endif //KHINTS_SETTINGS_H