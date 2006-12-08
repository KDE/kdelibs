/*
   This file is part of the KDE project

   Copyright (C) 2006 Fredrik Höglund <fredrik@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <QItemDelegate>
#include <kio/global.h>


/**
 * KFileItemDelegate is intended to be used to provide a KDE file system
 * view, when using one of the standard item views in Qt with KDirModel.
 *
 * While primarily intended to be used with KDirModel, it uses
 * Qt::DecorationRole and Qt::DisplayRole for the icons and text labels,
 * just like QItemDelegate, and can thus be used with any standard model.
 *
 * To use KFileItemDelegate, instantiate an object from the delegate,
 * and call setItemDelegate() in one of the standard item views in Qt:
 *
 * @code
 * QListView *listview = new QListView(this);
 * KFileItemDelegate *delegate = new KFileItemDelegate(this);
 * listview->setItemDelegate(delegate);
 * @endcode
 */
class KIO_EXPORT KFileItemDelegate : public QItemDelegate
{
     public:
        /**
         * Constructs a new KFileItemDelegate.
         *
         * @param parent The parent object for the delegate.
         */
        KFileItemDelegate(QObject *parent = 0);


        /**
         * Destroys the item delegate.
         */
        virtual ~KFileItemDelegate();


        /**
         * Returns the nominal size for the item referred to by @p index, given the
         * provided options.
         *
         * If the model provides a valid Qt::FontRole and/or Qt::AlignmentRole for the item,
         * those will be used instead of the ones specified in the style options.
         *
         * This function is reimplemented from @ref QItemDelegate.
         *
         * @param option  The style options that should be used when painting the item.
         * @param index   The index to the item for which to return the size hint.
         */
        virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;


        /**
         * Paints the item indicated by @p index, using @p painter.
         *
         * The item will be drawn in the rectangle specified by option.rect.
         * The correct size for that rectangle can be obtained by calling
         * @ref sizeHint().
         *
         * This function will use the following data values if the model provides
         * them for the item, in place of the values in @p option:
         *
         * @li Qt::FontRole        The font that should be used for the display role.
         * @li Qt::AlignmentRole   The alignment of the display role.
         * @li Qt::ForegroundRole  The text color for the display role.
         * @li Qt::BackgroundRole  The background color for the item.
         *
         * This function is reimplemented from @ref QItemDelegate.
         *
         * @param painter The painter with which to draw the item.
         * @param option  The style options that should be used when painting the item.
         * @param index   The index to the item that should be painted.
         */
        virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;


    protected:
        /**
         * Convenience function that returns the display role as a QString.
         *
         * The supported display types are QString, double, int and unsigned int.
         *
         * If the value is a number, it will be formatted according to the KDE locale
         * with the default precision, i.e. 2 for floating point values, and 0
         * for integers. In the default locale, 123456.5 would be formatted as
         * 123,456.50.
         *
         * If the value is a QString, any newline characters will be replaced
         * with QChar::LineSeparator in the returned string.
         *
         * @param index The index to the item for which to retrieve the Qt::DisplayRole.
         */
        QString display(const QModelIndex &index) const;


        /**
         * Convenience function that returns the decoration role as a QPixmap.
         *
         * The supported decoration types are QPixmap, QIcon and QColor.
         *
         * If the value is a QIcon, the decoration size and state in @p options
         * determine which pixmap in the icon is used. In the case of a QIcon,
         * the returned pixmap may be smaller than decorationSize, but never larger.
         *
         * In the case of a QColor, a pixmap the size of decorationSize is created
         * and filled with the color.
         *
         * Regardless of the type, the pixmap will be blended with the highlight
         * color, if the state in @p options indicate that the item is selected,
         * and option.showDecorationSelected is false.
         *
         * @param option The style options that should be used when painting the item.
         * @param index  The index to the item for which to retrieve the Qt::DecorationRole.
         */
        QPixmap decoration(const QStyleOptionViewItem &option, const QModelIndex &index) const;


        /**
         * Returns the position the decoration role pixmap should be drawn at.
         *
         * @param option The style options for the item.
         * @param pixmap The decoration role pixmap.
         */
        QPoint iconPosition(const QStyleOptionViewItem &option, const QPixmap &pixmap) const;


        /**
         * Returns the rectangle that's available for the display area.
         * The returned rectangle includes the margins around the text area.
         *
         * @param option The style options for the item.
         * @param icon   The decoration role pixmap.
         * @param string The display role string.
         */
        QRect labelRectangle(const QStyleOptionViewItem &option, const QPixmap &icon, const QString &string) const;

    private:
        class Private;
        Private * const d; /// @internal
};

// kate: space-indent on; indent-width 4; replace-tabs on;
