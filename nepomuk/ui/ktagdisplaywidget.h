/*
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2007 Sebastian Trueg <trueg@kde.org>
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

#ifndef _KTAG_DISPLAY_WIDGET_H_
#define _KTAG_DISPLAY_WIDGET_H_

#include <QtGui/QWidget>

#include <nepomuk/nepomuk_export.h>

/**
 * The KTagDisplayWidget shows a list of tags and allows clicking each of them.
 */
class NEPOMUK_EXPORT KTagDisplayWidget : public QWidget
{
    Q_OBJECT

 public:
    KTagDisplayWidget( QWidget* parent = 0 );
    ~KTagDisplayWidget();

 public Q_SLOTS:
    /**
     * Set \a tags to be displayed.
     */
    void setTags( const QStringList& tags );

    /**
     * Add \a tag to be displayed.
     * Existing tags with the same name will be overwritten.
     */
    void addTag( const QString& tag );

    /**
     * Add a set of tags to be displayed.
     * Existing tags with the same name will be overwritten.
     */
    void addTags( const QStringList& tags );

    /**
     * Remove all tags.
     */
    void clear();

 Q_SIGNALS:
    void tagClicked( const QString& tag );

 private:
    class Private;
    Private* const d;
};

#endif
