/* This file is part of the KDE libraries
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KICON_H
#define KICON_H

#include <kdeui_export.h>

#include <QtGui/QIcon>

class KIconLoader;
class QStringList;

/**
 * \short A wrapper around QIcon that provides KDE icon features
 *
 * KIcon is a convenience class for creating a QIcon with an appropriate
 * KIconEngine to perform loading and rendering.  KIcons thus adhere to
 * KDE style and effect standards.
 *
 * \sa KIconEngine, KIconLoader, KIconTheme
 *
 * \author Hamish Rodda <rodda@kde.org>
 */
class KDEUI_EXPORT KIcon : public QIcon
{
public:
    /**
     * Constructor which takes a kde style icon name, and optionally
     * a custom icon loader.
     *
     * \param iconName The name of the kde icon to load
     * \param iconLoader The icon loader to use in loading this icon, or
     *                   null to use the default global icon loader.
     * @param overlays A list of overlays to apply to this icon. They are
     *                 loaded from the emblems icons and up to four (one per
     *                 corner) is currently supported
     */
    explicit KIcon(const QString& iconName, KIconLoader* iconLoader,
                   const QStringList& overlays);

    /**
     * \overload
     */
    explicit KIcon(const QString& iconName, KIconLoader* iconLoader);

    /**
     * \overload
     */
    explicit KIcon(const QString& iconName);

    /**
     * Copy constructor which takes any QIcon.
     *
     * \param copy the icon to copy.  This should have once been a KIcon,
     *             if you want to preserve KDE icon effects.
     */
    explicit KIcon(const QIcon& copy);

    /**
     * Constructor for a null icon.
     */
    KIcon();

    /**
     * Destroys the icon.
     */
    ~KIcon();

    KIcon& operator=( const KIcon &other );

private:
    class Private;
    Private* const d;
};

#endif
