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

/*
  KBlockLayout is based on the FlowLayout example from QT4.
  Copyright (C) 2004-2006 Trolltech ASA. All rights reserved.
*/

#ifndef KBLOCKLAYOUT_H
#define KBLOCKLAYOUT_H

#include <QtGui/QLayout>
#include <QtGui/QLayoutItem>

/**
 * The KBlockLayout arranges widget in rows and columns like a text
 * editor does.
 */
class KBlockLayout : public QLayout
{
 public:
    explicit KBlockLayout( QWidget *parent, int margin = 0, int hSpacing = -1, int vSpacing = -1 );
    KBlockLayout( int margin = 0, int hSpacing = -1, int vSpacing = -1 );
    ~KBlockLayout();

    /**
     * Set the alignment to use. It can be a combination of a horizontal and
     * a vertical alignment flag. The vertical flag is used to arrange widgets
     * that do not fill the complete height of a row.
     *
     * The default alignment is Qt::AlignLeft|Qt::AlignTop
     */
    void setAlignment( Qt::Alignment );
    Qt::Alignment alignment() const;

    int horizontalSpacing() const;
    int verticalSpacing() const;

    void setSpacing( int h,  int v );

    void addItem( QLayoutItem* item );
    Qt::Orientations expandingDirections() const;
    bool hasHeightForWidth() const;
    int heightForWidth(int) const;
    int count() const;
    QLayoutItem* itemAt( int index ) const;
    QSize minimumSize() const;
    void setGeometry( const QRect& rect );
    QSize sizeHint() const;
    QLayoutItem* takeAt( int index );

 private:
    int doLayout( const QRect& rect, bool testOnly ) const;

    class Private;
    Private* const d;
};

#endif
