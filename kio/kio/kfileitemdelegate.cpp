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

#include <QApplication>
#include <QStyle>
#include <QModelIndex>
#include <QPainter>
#include <QCache>
#include <QImage>
#include <QPainterPath>
#include <QTextLayout>

#include <kglobal.h>
#include <klocale.h>
#include <kpixmapeffect.h>

#include "kfileitemdelegate.h"


//#define DEBUG_RECTS


namespace
{
    const int selectionHMargin = 6;
    const int selectionVMargin = 2;
    const int textHMargin = 5;
}


class KFileItemDelegate::Private
{
    public:
        Private(KFileItemDelegate *parent) : q(parent) {}
        ~Private() {}

        QSize decorationSizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
        QSize displaySizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
        bool inline wordWrapText(const QStyleOptionViewItem &options) const;
        Qt::Alignment inline alignment(const QStyleOptionViewItem &option, const QModelIndex &index) const;
        QString replaceNewlines(const QString &string) const;
        inline QFont font(const QStyleOptionViewItem &options, const QModelIndex &index) const;
        QString elideText(QTextLayout &layout, const QStyleOptionViewItem &option,
                          const QString &text, const QSize maxSize) const;
        QString elidedWordWrappedText(QTextLayout &layout, const QString &text, const QSize &maxSize) const;
        QSize layoutText(QTextLayout &layout, const QString &text, int maxWidth) const;
        void inline setLayoutOptions(QTextLayout &layout, const QStyleOptionViewItem &options,
                                     const QModelIndex &index) const;
        bool inline verticalLayout(const QStyleOptionViewItem &option) const;
        QPainterPath roundedRectangle(const QRectF &rect, qreal radius) const;
        QPixmap inline selected(const QStyleOptionViewItem &option, const QPixmap &pixmap) const;
        QPixmap toPixmap(const QStyleOptionViewItem &option, const QColor &color) const;
        QPixmap toPixmap(const QStyleOptionViewItem &option, const QIcon &icon) const;
        QBrush inline brush(const QVariant &value) const;
        QBrush foregroundBrush(const QStyleOptionViewItem &option, const QModelIndex &index) const;
        QBrush backgroundBrush(const QStyleOptionViewItem &option, const QModelIndex &index) const;
        bool inline alternateBackground(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    private:
        KFileItemDelegate * const q;
};


bool KFileItemDelegate::Private::wordWrapText(const QStyleOptionViewItem &option) const
{
    const QStyleOptionViewItemV2 *option2;

    if ((option2 = qstyleoption_cast<const QStyleOptionViewItemV2*>(&option)))
        return (option2->features & QStyleOptionViewItemV2::WrapText);

    return false;
}


QString KFileItemDelegate::Private::elideText(QTextLayout &layout, const QStyleOptionViewItem &option,
                                               const QString &text, const QSize maxSize) const
{
    if (wordWrapText(option))
        return elidedWordWrappedText(layout, text, maxSize);

    // If the string contains a single line
    if (text.indexOf(QChar::LineSeparator) == -1)
        return QFontMetrics(layout.font()).elidedText(text, option.textElideMode, maxSize.width());

    // ### Handle text with line separators

    return QString::null;
}


// Returns the font that should be used to render the display role.
QFont KFileItemDelegate::Private::font(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // Try to get the font from the model first
    const QVariant value = index.model()->data(index, Qt::FontRole);
    if (value.isValid())
        return qvariant_cast<QFont>(value).resolve(option.font);

    return option.font;
}


// Replaces any newline characters in the provided string, with QChar::LineSeparator
QString KFileItemDelegate::Private::replaceNewlines(const QString &text) const
{
    QString string = text;
    const QChar newline = QLatin1Char('\n');

    for (int i = 0; i < string.length(); i++)
        if (string[i] == newline)
            string[i] = QChar::LineSeparator;

    return string;
}


// Lays the text out in a rectangle no wider than maxWidth
QSize KFileItemDelegate::Private::layoutText(QTextLayout &layout, const QString &text, int maxWidth) const
{
    QFontMetrics metrics(layout.font());
    int leading     = metrics.leading();
    int height      = 0;
    qreal widthUsed = 0;
    QTextLine line;

    layout.setText(text);

    layout.beginLayout();
    while ((line = layout.createLine()).isValid())
    {
        line.setLineWidth(int(maxWidth));
        height += leading;
        line.setPosition(QPoint(0, height));
        height += int(line.height());
        widthUsed = qMax(widthUsed, line.naturalTextWidth());
    }
    layout.endLayout();

    return QSize(int(widthUsed), height);
}


// Elides word wrapped text, by laying out as many lines as will fit in the size constraints,
// and adding an ellipses at the end of the last line. The only elide mode supported is currently
// Qt::ElideRight.
QString KFileItemDelegate::Private::elidedWordWrappedText(QTextLayout &layout, const QString &text,
                                                          const QSize &size) const
{
    QFontMetrics metrics(layout.font());
    int elideStart = 0;
    int elideAfter = 0;
    int maxWidth   = size.width();
    int maxHeight  = size.height();
    int height     = 0;
    QTextLine line;

    layout.setText(text);

    // Keep laying out lines until we run out of vertical space, and mark
    // the position in the string where the last line begins.
    // We'll elide all the text from that position, using size.width().
    layout.beginLayout();
    while ((line = layout.createLine()).isValid())
    {
        line.setLineWidth(maxWidth);
        height += metrics.leading() + int(line.height());

        if (height + metrics.lineSpacing() > maxHeight)
        {
            elideStart = line.textStart();
            break;
        }

        elideAfter = line.textStart() + line.textLength();
    }
    layout.endLayout();

    return text.left(elideAfter) +
            metrics.elidedText(text.mid(elideStart), Qt::ElideRight, maxWidth);
}


// Returns the alignment that should be used for the display role
Qt::Alignment KFileItemDelegate::Private::alignment(const QStyleOptionViewItem &option,
                                                    const QModelIndex &index) const
{
    // Try to get the alignment for the item from the model, and fallback to the
    // alignment specified in option.
    const QVariant value = index.model()->data(index, Qt::TextAlignmentRole);
    if (value.isValid())
        return Qt::Alignment(value.toInt());

    return option.displayAlignment;
}


void KFileItemDelegate::Private::setLayoutOptions(QTextLayout &layout, const QStyleOptionViewItem &option,
                                                  const QModelIndex &index) const
{
    QTextOption textoption;
    textoption.setTextDirection(option.direction);
    textoption.setAlignment(alignment(option, index));
    textoption.setWrapMode(wordWrapText(option) ? QTextOption::WordWrap : QTextOption::NoWrap);

    layout.setFont(font(option, index));
    layout.setTextOption(textoption);
}


QSize KFileItemDelegate::Private::displaySizeHint(const QStyleOptionViewItem &option,
                                                  const QModelIndex &index) const
{
    const QString label = q->display(index);
    const int maxWidth = verticalLayout(option) && wordWrapText(option) ?
            option.decorationSize.width() + 10 : 32757;

    QTextLayout layout;
    setLayoutOptions(layout, option, index);

    QSize size = layoutText(layout, label, maxWidth);

    size.rwidth()  += selectionHMargin * 2;
    size.rheight() += selectionVMargin * 2;

    return size;
}


QSize KFileItemDelegate::Private::decorationSizeHint(const QStyleOptionViewItem &option,
                                                     const QModelIndex &index) const
{
    Q_UNUSED(index)

    int focusHMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin);
    int focusVMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameVMargin);

    return option.decorationSize + QSize(focusHMargin * 2, focusVMargin * 2);
}


bool KFileItemDelegate::Private::verticalLayout(const QStyleOptionViewItem &option) const
{
    return (option.decorationPosition == QStyleOptionViewItem::Top ||
            option.decorationPosition == QStyleOptionViewItem::Bottom);
}


// Move to kdefx/kdrawutil.cpp?
QPainterPath KFileItemDelegate::Private::roundedRectangle(const QRectF &rect, qreal radius) const
{
    QPainterPath path(QPointF(rect.left(), rect.top() + radius));
    path.quadTo(rect.left(), rect.top(), rect.left() + radius, rect.top());         // Top left corner
    path.lineTo(rect.right() - radius, rect.top());                                 // Top side
    path.quadTo(rect.right(), rect.top(), rect.right(), rect.top() + radius);       // Top right corner
    path.lineTo(rect.right(), rect.bottom() - radius);                              // Right side
    path.quadTo(rect.right(), rect.bottom(), rect.right() - radius, rect.bottom()); // Bottom right corner
    path.lineTo(rect.left() + radius, rect.bottom());                               // Bottom side
    path.quadTo(rect.left(), rect.bottom(), rect.left(), rect.bottom() - radius);   // Bottom left corner
    path.closeSubpath();

    return path;
}


QPixmap KFileItemDelegate::Private::selected(const QStyleOptionViewItem &option, const QPixmap &pixmap) const
{
    QPalette::ColorGroup group = option.state & QStyle::State_Enabled ?
            QPalette::Normal : QPalette::Disabled;

    return KPixmapEffect::selectedPixmap(pixmap, option.palette.color(group, QPalette::Highlight));
}


// Extracts the correct pixmap from a QIcon with respect to option
QPixmap KFileItemDelegate::Private::toPixmap(const QStyleOptionViewItem &option, const QIcon &icon) const
{
    QIcon::Mode mode   = option.state & QStyle::State_Enabled ? QIcon::Normal : QIcon::Disabled;
    QIcon::State state = option.state & QStyle::State_Open ? QIcon::On : QIcon::Off;

    return icon.pixmap(option.decorationSize, mode, state);
}


// Converts a QColor to a pixmap
QPixmap KFileItemDelegate::Private::toPixmap(const QStyleOptionViewItem &option, const QColor &color) const
{
    QPixmap pixmap(option.decorationSize);
    pixmap.fill(color);

    return pixmap;
}


// Converts a QVariant of type Brush or Color to a QBrush
QBrush KFileItemDelegate::Private::brush(const QVariant &value) const
{
    switch (value.type())
    {
        case QVariant::Color:
            return QBrush(qvariant_cast<QColor>(value));

        case QVariant::Brush:
            return qvariant_cast<QBrush>(value);

        default:
            return QBrush(Qt::NoBrush);
    }
}


QBrush KFileItemDelegate::Private::foregroundBrush(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // If the model provides its own foreground color/brush for this item,
    // return it instead of the one in option.
    const QVariant value = index.model()->data(index, Qt::ForegroundRole);
    if (value.isValid())
        return brush(value);

    QPalette::ColorGroup group = option.state & QStyle::State_Enabled ?
            QPalette::Normal : QPalette::Disabled;

    QPalette::ColorRole role = (option.state & QStyle::State_Selected) ?
            QPalette::HighlightedText : QPalette::Text;

    return option.palette.brush(group, role);
}


QBrush KFileItemDelegate::Private::backgroundBrush(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // If the model provides its own background color/brush for this item,
    // return it instead of the one in option.
    const QVariant value = index.model()->data(index, Qt::BackgroundRole);
    if (value.isValid())
        return brush(value);

    if (!(option.state & QStyle::State_Selected) && !alternateBackground(option, index))
        return QBrush(Qt::NoBrush);

    // If we get to this point, the item is either selected, or has its background alternated
    QPalette::ColorGroup group = option.state & QStyle::State_Enabled ?
            QPalette::Normal : QPalette::Disabled;

    QPalette::ColorRole role = option.state & QStyle::State_Selected ?
            QPalette::Highlight : QPalette::AlternateBase;

    return option.palette.brush(group, role);
}


bool KFileItemDelegate::Private::alternateBackground(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const QStyleOptionViewItemV2 *option2;

    if ((option2 = qstyleoption_cast<const QStyleOptionViewItemV2*>(&option)))
        return (option2->features & QStyleOptionViewItemV2::Alternate);

    return (option.showDecorationSelected && (index.row() % 2));
}




// ---------------------------------------------------------------------------




KFileItemDelegate::KFileItemDelegate(QObject *parent)
    : QItemDelegate(parent), d(new Private(this))
{
}


KFileItemDelegate::~KFileItemDelegate()
{
    delete d;
}


QSize KFileItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // If the model wants to provide its own size hint for the item
    const QVariant value = index.model()->data(index, Qt::SizeHintRole);
    if (value.isValid())
        return qvariant_cast<QSize>(value);

    const QSize displaySize    = d->displaySizeHint(option, index);
    const QSize decorationSize = d->decorationSizeHint(option, index);

    if (d->verticalLayout(option))
        return QSize(qMax(decorationSize.width(), displaySize.width()),
                     decorationSize.height() + displaySize.height() + 1);
    else
        return QSize(decorationSize.width() + displaySize.width() + 1,
                     qMax(decorationSize.height(), displaySize.height()));
}


QString KFileItemDelegate::display(const QModelIndex &index) const
{
    const QVariant value = index.model()->data(index, Qt::DisplayRole);

    switch (value.type())
    {
        case QVariant::String:
            return d->replaceNewlines(value.toString());

        case QVariant::Double:
            return KGlobal::locale()->formatNumber(value.toDouble());

        case QVariant::Int:
        case QVariant::UInt:
            return KGlobal::locale()->formatLong(value.toInt());

        default:
            return QString::null;
    }
}


QPixmap KFileItemDelegate::decoration(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const QVariant value = index.model()->data(index, Qt::DecorationRole);
    QPixmap pixmap;

    switch (value.type())
    {
        case QVariant::Icon:
            pixmap = d->toPixmap(option, qvariant_cast<QIcon>(value));
            break;

        case QVariant::Pixmap:
            pixmap = qvariant_cast<QPixmap>(value);
            break;

        case QVariant::Color:
            pixmap = d->toPixmap(option, qvariant_cast<QColor>(value));
            break;

        default:
            pixmap = QPixmap();
    }

    // If the item is selected, and the selection rectangle only covers the
    // text label, blend the pixmap with the the highlight color.
    if (!pixmap.isNull() && (option.state & QStyle::State_Selected) && !option.showDecorationSelected)
        return d->selected(option, pixmap);

    return pixmap;
}


QRect KFileItemDelegate::labelRectangle(const QStyleOptionViewItem &option, const QPixmap &icon,
                                        const QString &string) const
{
    Q_UNUSED(string)

    int focusHMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin);
    int focusVMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameVMargin);
    int decoHeight   = option.decorationSize.height() + focusHMargin * 2 + 1;
    int decoWidth    = option.decorationSize.width()  + focusVMargin * 2 + 1;

    if (icon.isNull())
        return option.rect;

    QRect textArea(QPoint(0, 0), option.rect.size());

    switch (option.decorationPosition)
    {
        case QStyleOptionViewItem::Top:
            textArea.setTop(decoHeight);
            break;

        case QStyleOptionViewItem::Bottom:
            textArea.setBottom(option.rect.height() - decoHeight);
            break;

        case QStyleOptionViewItem::Left:
            textArea.setLeft(decoWidth);
            break;

        case QStyleOptionViewItem::Right:
            textArea.setRight(option.rect.width() - decoWidth);
            break;
    }

    textArea.translate(option.rect.topLeft());
    return QStyle::visualRect(option.direction, option.rect, textArea);
}


QPoint KFileItemDelegate::iconPosition(const QStyleOptionViewItem &option, const QPixmap &pixmap) const
{
    int focusHMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin);
    int focusVMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameVMargin);

    Qt::Alignment alignment;

    // Convert decorationPosition to the alignment the decoration will have in option.rect
    switch (option.decorationPosition)
    {
        case QStyleOptionViewItem::Top:
            alignment = Qt::AlignHCenter | Qt::AlignTop;
            break;

        case QStyleOptionViewItem::Bottom:
            alignment = Qt::AlignHCenter | Qt::AlignBottom;
            break;

        case QStyleOptionViewItem::Left:
            alignment = Qt::AlignVCenter | Qt::AlignLeft;
            break;

        case QStyleOptionViewItem::Right:
            alignment = Qt::AlignVCenter | Qt::AlignRight;
            break;
    }

    // Compute the nominal decoration rectangle
    const QSize size = option.decorationSize + QSize(focusHMargin * 2, focusVMargin * 2);
    const QRect rect = QStyle::alignedRect(option.direction, alignment, size, option.rect);

    // Position the pixmap in the center of the rectangle
    QRect pixRect = pixmap.rect();
    pixRect.moveCenter(rect.center());

    return pixRect.topLeft();
}


void KFileItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    if (!index.isValid())
        return;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    QString label  = display(index);
    QPixmap pixmap = decoration(option, index);

    // Compute the metrics
    // ========================================================================
    QTextLayout layout;
    d->setLayoutOptions(layout, option, index);

    QRect textArea = labelRectangle(option, pixmap, label);
    QRect labelRect = textArea.adjusted(+selectionHMargin, +selectionVMargin,
                                        -selectionHMargin, -selectionVMargin);

    // Layout the text in labelRect
    QSize size = d->layoutText(layout, label, labelRect.width());
    if (size.width() > labelRect.width() || size.height() > labelRect.height())
    {
        // Elide the text if it won't fit in the rectangle, and redo the layout
        label = d->elideText(layout, option, label, labelRect.size());
        size  = d->layoutText(layout, label, labelRect.width());
    }

    // Compute the bounding rect of the text, and the position where we should draw the layout
    const Qt::Alignment alignment = layout.textOption().alignment();
    QRect selectionRect = QStyle::alignedRect(option.direction, alignment, size, labelRect);
    QPoint labelPos(labelRect.x(), selectionRect.y());


#ifdef DEBUG_RECTS
    painter->drawRect(option.rect);

    painter->setPen(Qt::blue);
    painter->drawRect(textArea);

    painter->setPen(Qt::red);
    painter->drawRect(selectionRect);
#endif


    // Draw the background
    // ========================================================================
    const QBrush brush = d->backgroundBrush(option, index);

    if (brush.style() != Qt::NoBrush)
    {
        // If the selection rectangle should only cover the text label
        if (!option.showDecorationSelected)
        {
            const QRect r = selectionRect.adjusted(-selectionHMargin, -selectionVMargin,
                                                   +selectionHMargin, +selectionVMargin);
            const QPainterPath path = d->roundedRectangle(r, 5);
            painter->fillPath(path, brush);
        }
        else
            painter->fillRect(option.rect, brush);
    }


    // Draw the decoration
    // ========================================================================
    if (!pixmap.isNull())
    {
        const QPoint pt = iconPosition(option, pixmap);
        painter->drawPixmap(pt, pixmap);
    }


    // Draw the label
    // ========================================================================
    painter->setPen(QPen(d->foregroundBrush(option, index), 0));
    layout.draw(painter, labelPos);

    painter->restore();
}

// kate: space-indent on; indent-width 4; replace-tabs on;
