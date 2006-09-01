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

#ifndef KICONENGINE_H
#define KICONENGINE_H

#include <QtGui/QIconEngine>

#include <kdelibs_export.h>

class KIconLoader;

/**
 * \short A class to provide rendering of KDE icons.
 *
 * Currently, this class is not much more than a wrapper around QIconEngine.
 * However, it should not be difficult to extend with features such as SVG
 * rendered icons.
 *
 * @author Hamish Rodda <rodda@kde.org>
 */
class KDEUI_EXPORT KIconEngine : public QIconEngine
{
  public:
    /**
     * Constructs an icon engine for a KDE named icon.
     *
     * @param iconName the name of the icon to load
     * @param iconLoader The KDE icon loader that this engine is to use.
     *
     * @sa KIconLoader
     */
    KIconEngine(const QString& iconName, KIconLoader* iconLoader);

    /**
     * Destructor.
     */
    virtual ~KIconEngine();

    /**
     * Allows the KDE icon name assigned to this icon engine to be retrieved.
     */
    const QString& iconName() const;

    /**
     * Returns the icon loader used by this engine for rendering purposes.
     */
    KIconLoader* iconLoader() const;

    /// Reimplementation
    virtual QSize actualSize ( const QSize & size, QIcon::Mode mode, QIcon::State state );
    /// Reimplementation
    virtual void paint ( QPainter * painter, const QRect & rect, QIcon::Mode mode, QIcon::State state );
    /// Reimplementation
    virtual QPixmap pixmap ( const QSize & size, QIcon::Mode mode, QIcon::State state );

  private:
    class KIconEnginePrivate* const d;
};

#endif
