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

#ifndef KRATINGWIDGET_H
#define KRATINGWIDGET_H

#include <QtGui/QFrame>

#include <nepomuk/nepomuk_export.h>

/**
 * The KRatingWidget displays a range of stars or other arbitrary
 * pixmaps and allows the user to select a certain number by mouse
 */
class NEPOMUK_EXPORT KRatingWidget : public QFrame
{
    Q_OBJECT

 public:
    /**
     * Creates a new rating widget.
     */
    KRatingWidget( QWidget* parent = 0 );

    /**
     * Destructor
     */
    ~KRatingWidget();

    unsigned int rating() const;

    /**
     * \sa setSpacing
     */
    int spacing() const;

    QSize minimumSizeHint() const;

    // FIXME: implement heightForWidth and sizeHInt and resizeEvent and allow a smaller widget

 Q_SIGNALS:
    /**
     * Emitted if the rating is changed by user interaction (ie. mouse click).
     * A call to setRating does not trigger this signal.
     */
    void ratingChanged( unsigned int rating );

 public Q_SLOTS:
    /**
     * Set the current rating. Calling this method will NOT trigger the
     * ratingChanged signal.
     */
    void setRating( unsigned int rating );

    /**
     * Set the maximum allowed rating value. The default is 10 which means
     * that a rating from 1 to 10 is selectable. If \a max is uneven steps
     * are automatically only allowed full.
     */
    void setMaxRating( unsigned int max );

    /**
     * Painting only full steps means that each step of the rating is displayed
     * using one pixmap. Otherwise two steps are displayed using one pixmap
     * by painting a smaller one or only coloring half of the pixmap.
     *
     * If the current max is uneven the call is ignored.
     */
    void setOnlyPaintFullSteps( bool );

    /**
     * Set the spacing between the pixmaps. The default is 2.
     */
    void setSpacing( int );

    Qt::Alignment aligment() const;
    void setAlignment( Qt::Alignment align );

    /**
     * Set the pixap to be used to display a rating step.
     * By default the "rating" pixmap is loaded.
     */
    void setPixmap( const QPixmap& );

    /**
     * Set the size of the pixmap. It will be resized accordingly.
     * KRatingWidget also tries to reload another version of the pixmap
     * if \a size is one of KIconLoader::StdSizes.
     */
    void setPixmapSize( int size );

 protected:
    void mousePressEvent( QMouseEvent* e );
    void mouseMoveEvent( QMouseEvent* e );
    void leaveEvent( QEvent* e );
    void paintEvent( QPaintEvent* e );
    void resizeEvent( QResizeEvent* e );

 private:
    class Private;
    Private* const d;
};

#endif
