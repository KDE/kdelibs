/*
   This file is part of the KDE project

   Copyright © 2006-2007 Fredrik Höglund <fredrik@kde.org>

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

#include "kfileitemdelegate.h"
#include "kfileitemdelegate.moc"

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
#include <kiconloader.h>
#include <kiconeffect.h>
#include <kpixmapeffect.h>
#include <kdirmodel.h>
#include <kfileitem.h>


//#define DEBUG_RECTS


struct Margin
{
    int left, right, top, bottom;
};


class KFileItemDelegate::Private
{
    public:
        enum MarginType { ItemMargin = 0, TextMargin, IconMargin, NMargins };

        Private(KFileItemDelegate *parent) : q(parent) {}
        ~Private() {}

        QSize decorationSizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
        QSize displaySizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
        inline bool wordWrapText(const QStyleOptionViewItem &options) const;
        inline Qt::Alignment alignment(const QStyleOptionViewItem &option, const QModelIndex &index) const;
        QString replaceNewlines(const QString &string) const;
        inline KFileItem fileItem(const QModelIndex &index) const;
        inline QFont font(const QStyleOptionViewItem &option, const QModelIndex &index, const KFileItem &item) const;
        QString elidedText(QTextLayout &layout, const QStyleOptionViewItem &option, const QSize &maxSize) const;
        QSize layoutText(QTextLayout &layout, const QStyleOptionViewItem &option,
                         const QString &text, const QSize &constraints) const;
        QSize layoutText(QTextLayout &layout, const QString &text, int maxWidth) const;
        inline void setLayoutOptions(QTextLayout &layout, const QStyleOptionViewItem &options,
                                     const QModelIndex &index, const KFileItem &item) const;
        inline bool verticalLayout(const QStyleOptionViewItem &option) const;
        QPainterPath roundedRectangle(const QRectF &rect, qreal radius) const;
        QPixmap toPixmap(const QStyleOptionViewItem &option, const QColor &color) const;
        QPixmap toPixmap(const QStyleOptionViewItem &option, const QIcon &icon) const;
        inline QBrush brush(const QVariant &value) const;
        QBrush foregroundBrush(const QStyleOptionViewItem &option, const QModelIndex &index) const;
        QBrush backgroundBrush(const QStyleOptionViewItem &option, const QModelIndex &index) const;
        inline bool alternateBackground(const QStyleOptionViewItem &option, const QModelIndex &index) const;
        inline void setActiveMargins(Qt::Orientation layout);
        void setVerticalMargin(MarginType type, int left, int right, int top, int bottom);
        void setHorizontalMargin(MarginType type, int left, int right, int top, int bottom);
        inline void setVerticalMargin(MarginType type, int hor, int ver);
        inline void setHorizontalMargin(MarginType type, int hor, int ver);
        inline QRect addMargin(const QRect &rect, MarginType type) const;
        inline QRect subtractMargin(const QRect &rect, MarginType type) const;
        inline QSize addMargin(const QSize &size, MarginType type) const;
        inline QSize subtractMargin(const QSize &size, MarginType type) const;
        QString itemSize(const QModelIndex &index, const KFileItem &item) const;
        QString information(const QStyleOptionViewItem &option, const QModelIndex &index, const KFileItem &item) const;

    public:
        KFileItemDelegate::AdditionalInformation additionalInformation;

    private:
        KFileItemDelegate * const q;
        Margin verticalMargin[NMargins];
        Margin horizontalMargin[NMargins];
        Margin *activeMargins;
};


void KFileItemDelegate::Private::setActiveMargins(Qt::Orientation layout)
{
    activeMargins = (layout == Qt::Horizontal ?
            horizontalMargin : verticalMargin);
}


void KFileItemDelegate::Private::setVerticalMargin(MarginType type, int left, int top, int right, int bottom)
{
    verticalMargin[type].left   = left;
    verticalMargin[type].right  = right;
    verticalMargin[type].top    = top;
    verticalMargin[type].bottom = bottom;
}


void KFileItemDelegate::Private::setHorizontalMargin(MarginType type, int left, int top, int right, int bottom)
{
    horizontalMargin[type].left   = left;
    horizontalMargin[type].right  = right;
    horizontalMargin[type].top    = top;
    horizontalMargin[type].bottom = bottom;
}


void KFileItemDelegate::Private::setVerticalMargin(MarginType type, int horizontal, int vertical)
{
    setVerticalMargin(type, horizontal, vertical, horizontal, vertical);
}


void KFileItemDelegate::Private::setHorizontalMargin(MarginType type, int horizontal, int vertical)
{
    setHorizontalMargin(type, horizontal, vertical, horizontal, vertical);
}


QRect KFileItemDelegate::Private::addMargin(const QRect &rect, MarginType type) const
{
    const Margin &m = activeMargins[type];
    return rect.adjusted(-m.left, -m.top, m.right, m.bottom);
}


QRect KFileItemDelegate::Private::subtractMargin(const QRect &rect, MarginType type) const
{
    const Margin &m = activeMargins[type];
    return rect.adjusted(m.left, m.top, -m.right, -m.bottom);
}


QSize KFileItemDelegate::Private::addMargin(const QSize &size, MarginType type) const
{
    const Margin &m = activeMargins[type];
    return QSize(size.width() + m.left + m.right, size.height() + m.top + m.bottom);
}


QSize KFileItemDelegate::Private::subtractMargin(const QSize &size, MarginType type) const
{
    const Margin &m = activeMargins[type];
    return QSize(size.width() - m.left - m.right, size.height() - m.top - m.bottom);
}


bool KFileItemDelegate::Private::wordWrapText(const QStyleOptionViewItem &option) const
{
    const QStyleOptionViewItemV2 *option2;

    if ((option2 = qstyleoption_cast<const QStyleOptionViewItemV2*>(&option)))
        return (option2->features & QStyleOptionViewItemV2::WrapText);

    return false;
}


// Returns the size of a file, or the number of items in a directory, as a QString
QString KFileItemDelegate::Private::itemSize(const QModelIndex &index, const KFileItem &item) const
{
    // Return a formatted string containing the file size, if the item is a file
    if (item.isFile())
        return KGlobal::locale()->formatByteSize(item.size());

    // Return the number of items in the directory
    const QVariant value = index.model()->data(index, KDirModel::ChildCountRole);
    const int count = value.type() == QVariant::Int ? value.toInt() : KDirModel::ChildCountUnknown;

    if (count == KDirModel::ChildCountUnknown)
        return i18nc("Items in a folder", "? items");

    return i18ncp("Items in a folder", "1 item", "%1 items", count);
}


// Returns the additional information string, if one should be shown, or an empty string otherwise
QString KFileItemDelegate::Private::information(const QStyleOptionViewItem &option, const QModelIndex &index,
                                                const KFileItem &item) const
{
    if (additionalInformation == KFileItemDelegate::NoInformation || item.isNull() || !verticalLayout(option))
        return QString();

    switch (additionalInformation)
    {
        case KFileItemDelegate::Size:
            return itemSize(index, item);

        case KFileItemDelegate::Permissions:
            return item.permissionsString();

        case KFileItemDelegate::OctalPermissions:
            return QString('0') + QString::number(item.permissions(), 8);

        case KFileItemDelegate::Owner:
            return item.user();

        case KFileItemDelegate::OwnerAndGroup:
            return item.user() + ':' + item.group();

        case KFileItemDelegate::CreationTime:
            return item.timeString(KIO::UDS_CREATION_TIME);

        case KFileItemDelegate::ModificationTime:
            return item.timeString(KIO::UDS_MODIFICATION_TIME);

        case KFileItemDelegate::AccessTime:
            return item.timeString(KIO::UDS_ACCESS_TIME);

        case KFileItemDelegate::MimeType:
            return item.isMimeTypeKnown() ? item.mimetype() : i18n("Unknown");

        case KFileItemDelegate::FriendlyMimeType:
            return item.isMimeTypeKnown() ? item.mimeComment() : i18n("Unknown");

        default:
            return QString();
    }
}


// Returns the KFileItem for the index
KFileItem KFileItemDelegate::Private::fileItem(const QModelIndex &index) const
{
    const QVariant value = index.model()->data(index, KDirModel::FileItemRole);
    return qvariant_cast<KFileItem>(value);
}


// Returns the font that should be used to render the display role.
QFont KFileItemDelegate::Private::font(const QStyleOptionViewItem &option, const QModelIndex &index,
                                       const KFileItem &item) const
{
    QFont font = option.font;

    // Try to get the font from the model
    const QVariant value = index.model()->data(index, Qt::FontRole);
    if (value.isValid())
        font = qvariant_cast<QFont>(value).resolve(option.font);

    // Use an italic font for symlinks
    if (item.isLink())
        font.setItalic(true);

    return font;
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


// Lays the text out in a rectangle no larger than constraints, eliding it as necessary
QSize KFileItemDelegate::Private::layoutText(QTextLayout &layout, const QStyleOptionViewItem &option,
                                             const QString &text, const QSize &constraints) const
{
    const QSize size = layoutText(layout, text, constraints.width());

    if (size.width() > constraints.width() || size.height() > constraints.height())
    {
        const QString elided = elidedText(layout, option, constraints);
        return layoutText(layout, elided, constraints.width());
    }

    return size;
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


// Elides the text in the layout, by iterating over each line in the layout, eliding
// or word breaking the line if it's wider than the max width, and finally adding an
// ellipses at the end of the last line, if there are more lines than will fit within
// the vertical size constraints.
QString KFileItemDelegate::Private::elidedText(QTextLayout &layout, const QStyleOptionViewItem &option,
                                               const QSize &size) const
{
    QFontMetrics metrics(layout.font());
    const QString text = layout.text();
    int maxWidth       = size.width();
    int maxHeight      = size.height();
    qreal height       = 0;

    // If the string contains a single line of text that shouldn't be word wrapped
    if (!wordWrapText(option) && text.indexOf(QChar::LineSeparator) == -1)
        return metrics.elidedText(text, option.textElideMode, maxWidth);

    // Elide each line that has already been laid out in the layout.
    QString elided;
    elided.reserve(text.length());

    for (int i = 0; i < layout.lineCount(); i++)
    {
        QTextLine line = layout.lineAt(i);
        int start  = line.textStart();
        int length = line.textLength();

        height += metrics.leading();
        if (height + line.height() + metrics.lineSpacing() > maxHeight)
        {
            // Unfortunately, if the line ends because of a line separator, elidedText() will be too
            // clever and keep adding lines until it finds one that's too wide.
            if (line.naturalTextWidth() < maxWidth && text[start + length - 1] == QChar::LineSeparator)
                elided += text.mid(start, length - 1);
            else
                elided += metrics.elidedText(text.mid(start), option.textElideMode, maxWidth);
            break;
        }
        else if (line.naturalTextWidth() > maxWidth)
            elided += metrics.elidedText(text.mid(start, length), option.textElideMode, maxWidth);
        else
            elided += text.mid(start, length);

        height += line.height();
    }

    return elided;
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
                                                  const QModelIndex &index, const KFileItem &item) const
{
    QTextOption textoption;
    textoption.setTextDirection(option.direction);
    textoption.setAlignment(alignment(option, index));
    textoption.setWrapMode(wordWrapText(option) ? QTextOption::WordWrap : QTextOption::NoWrap);

    layout.setFont(font(option, index, item));
    layout.setTextOption(textoption);
}


QSize KFileItemDelegate::Private::displaySizeHint(const QStyleOptionViewItem &option,
                                                  const QModelIndex &index) const
{
    QString label = q->display(index);
    const int maxWidth = verticalLayout(option) && wordWrapText(option) ?
            option.decorationSize.width() + 10 : 32757;

    KFileItem item = fileItem(index);

    // To compute the nominal size for the label + info, we'll just append
    // the information string to the label
    const QString info = information(option, index, item);
    if (!info.isEmpty())
        label += QString(QChar::LineSeparator) + info;

    QTextLayout layout;
    setLayoutOptions(layout, option, index, item);

    QSize size = layoutText(layout, label, maxWidth);
    return addMargin(size, TextMargin);
}


QSize KFileItemDelegate::Private::decorationSizeHint(const QStyleOptionViewItem &option,
                                                     const QModelIndex &index) const
{
    Q_UNUSED(index)

    return addMargin(option.decorationSize, IconMargin);
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
    const QPalette::ColorGroup group = option.state & QStyle::State_Enabled ?
            QPalette::Normal : QPalette::Disabled;

    // Always use the highlight color for selected items
    if (option.state & QStyle::State_Selected)
        return option.palette.brush(group, QPalette::HighlightedText);

    // If the model provides its own foreground color/brush for this item
    const QVariant value = index.model()->data(index, Qt::ForegroundRole);
    if (value.isValid())
        return brush(value);

    return option.palette.brush(group, QPalette::Text);
}


QBrush KFileItemDelegate::Private::backgroundBrush(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const QPalette::ColorGroup group = option.state & QStyle::State_Enabled ?
            QPalette::Normal : QPalette::Disabled;

    QBrush bg(Qt::NoBrush);

    // Always use the highlight color for selected items
    if (option.state & QStyle::State_Selected)
        bg = option.palette.brush(group, QPalette::Highlight);
    else
    {
        // If the item isn't selected, check if model provides its own background
        // color/brush for this item
        const QVariant value = index.model()->data(index, Qt::BackgroundRole);
        if (value.isValid())
            bg = brush(value);
    }

    // If we don't already have a background brush, check if the background color
    // should be alternated for this item.
    if (bg.style() == Qt::NoBrush && alternateBackground(option, index))
        bg = option.palette.brush(group, QPalette::AlternateBase);

    // Lighten the background color on hover, if we have one, and use a lighter version
    // of the highlight color otherwise.
    if ((option.state & QStyle::State_MouseOver) && index.column() == KDirModel::Name)
    {
        if (bg.style() == Qt::SolidPattern)
            bg = QBrush(bg.color().light());
        else
            bg = option.palette.color(group, QPalette::Highlight).light();
    }

    return bg;
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
    : QAbstractItemDelegate(parent), d(new Private(this))
{
    int focusHMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin);
    int focusVMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameVMargin);

    // Margins for horizontal mode (list views, tree views, table views)
    d->setHorizontalMargin(Private::TextMargin, focusHMargin, focusVMargin);
    d->setHorizontalMargin(Private::IconMargin, focusHMargin, focusVMargin);
    d->setHorizontalMargin(Private::ItemMargin, 0, 0);

    // Margins for vertical mode (icon views)
    d->setVerticalMargin(Private::TextMargin, 6, 2);
    d->setVerticalMargin(Private::IconMargin, focusHMargin, focusVMargin);
    d->setVerticalMargin(Private::ItemMargin, 0, 0);

    setAdditionalInformation(NoInformation);
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

    d->setActiveMargins(d->verticalLayout(option) ? Qt::Vertical : Qt::Horizontal);

    const QSize displaySize    = d->displaySizeHint(option, index);
    const QSize decorationSize = d->decorationSizeHint(option, index);

    QSize size;

    if (d->verticalLayout(option))
    {
        size.rwidth()  = qMax(decorationSize.width(), displaySize.width());
        size.rheight() = decorationSize.height() + displaySize.height() + 1;
    }
    else
    {
        size.rwidth()  = decorationSize.width() + displaySize.width() + 1;
        size.rheight() = qMax(decorationSize.height(), displaySize.height());
    }

    return d->addMargin(size, Private::ItemMargin);
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
            return QString();
    }
}


void KFileItemDelegate::setAdditionalInformation(AdditionalInformation value)
{
    d->additionalInformation = value;
}


KFileItemDelegate::AdditionalInformation KFileItemDelegate::additionalInformation() const
{
    return d->additionalInformation;
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

    if (!pixmap.isNull())
    {
        // If the item is selected, and the selection rectangle only covers the
        // text label, blend the pixmap with the highlight color.
        if (!option.showDecorationSelected && option.state & QStyle::State_Selected)
        {
            QPalette::ColorGroup group = option.state & QStyle::State_Enabled ? 
                    QPalette::Normal : QPalette::Disabled;

            pixmap = KPixmapEffect::selectedPixmap(pixmap,
                                option.palette.color(group, QPalette::Highlight));
        }

        // Apply the configured hover effect
        if ((option.state & QStyle::State_MouseOver) && index.column() == KDirModel::Name)
        {
            KIconEffect *effect = KIconLoader::global()->iconEffect();

            // Note that in KIconLoader terminology, active = hover.
            // ### We're assuming that the icon group is desktop/filemanager, since this
            //     is KFileItemDelegate.
            if (effect->hasEffect(K3Icon::Desktop, K3Icon::ActiveState))
                pixmap = effect->apply(pixmap, K3Icon::Desktop, K3Icon::ActiveState);
        }
    }

    return pixmap;
}


QRect KFileItemDelegate::labelRectangle(const QStyleOptionViewItem &option, const QPixmap &icon,
                                        const QString &string) const
{
    Q_UNUSED(string)

    if (icon.isNull())
        return option.rect;

    const QSize decoSize = d->addMargin(option.decorationSize, Private::IconMargin);
    const QRect itemRect = d->subtractMargin(option.rect, Private::ItemMargin);
    QRect textArea(QPoint(0, 0), itemRect.size());

    switch (option.decorationPosition)
    {
        case QStyleOptionViewItem::Top:
            textArea.setTop(decoSize.height() + 1);
            break;

        case QStyleOptionViewItem::Bottom:
            textArea.setBottom(itemRect.height() - decoSize.height() - 1);
            break;

        case QStyleOptionViewItem::Left:
            textArea.setLeft(decoSize.width() + 1);
            break;

        case QStyleOptionViewItem::Right:
            textArea.setRight(itemRect.width() - decoSize.width() - 1);
            break;
    }

    textArea.translate(itemRect.topLeft());
    return QStyle::visualRect(option.direction, option.rect, textArea);
}


QPoint KFileItemDelegate::iconPosition(const QStyleOptionViewItem &option, const QPixmap &pixmap) const
{
    const QRect itemRect = d->subtractMargin(option.rect, Private::ItemMargin);
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
    const QSize size = d->addMargin(option.decorationSize, Private::IconMargin);
    const QRect rect = QStyle::alignedRect(option.direction, alignment, size, itemRect);

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

    const QString label  = display(index);
    const QPixmap pixmap = decoration(option, index);
    KFileItem item      = d->fileItem(index);
    const QString info   = d->information(option, index, item);
    bool showInformation = false;


    // Compute the metrics, and lay out the text items
    // ========================================================================
    QTextLayout labelLayout, infoLayout;
    d->setLayoutOptions(labelLayout, option, index, item);

    d->setActiveMargins(d->verticalLayout(option) ? Qt::Vertical : Qt::Horizontal);

    QFontMetrics fm      = QFontMetrics(labelLayout.font());
    const QRect textArea = labelRectangle(option, pixmap, label);
    QRect textRect       = d->subtractMargin(textArea, Private::TextMargin);

    // Sizes and constraints for the different text parts
    QSize maxLabelSize = textRect.size();
    QSize maxInfoSize  = textRect.size();
    QSize labelSize;
    QSize infoSize;

    // If we have additional info text, and there's space for at least two lines of text,
    // adjust the max label size to make room for at least one line of the info text
    if (!info.isEmpty() && textRect.height() >= fm.lineSpacing() * 2)
    {
        infoLayout.setFont(labelLayout.font());
        infoLayout.setTextOption(labelLayout.textOption());

        maxLabelSize.rheight() -= fm.lineSpacing();
        showInformation = true;
    }

    // Lay out the label text, and adjust the max info size based on the label size
    labelSize = d->layoutText(labelLayout, option, label, maxLabelSize);
    maxInfoSize.rheight() -= labelSize.height();

    // Lay out the info text
    if (showInformation)
        infoSize = d->layoutText(infoLayout, option, info, maxInfoSize);
    else
        infoSize = QSize(0, 0);

    // Compute the bounding rect of the text
    const Qt::Alignment alignment = labelLayout.textOption().alignment();
    const QSize size(qMax(labelSize.width(), infoSize.width()), labelSize.height() + infoSize.height());
    const QRect textBoundingRect = QStyle::alignedRect(option.direction, alignment, size, textRect);

    // Compute the positions where we should draw the layouts
    const QPoint labelPos(textRect.x(), textBoundingRect.y());
    const QPoint infoPos(textRect.x(), textBoundingRect.y() + labelSize.height());


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
            const QRect r = d->addMargin(textBoundingRect, Private::TextMargin);
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
    labelLayout.draw(painter, labelPos);

    if (showInformation)
    {
        QColor color;
        if (option.state & QStyle::State_Selected)
        {
            color = option.palette.color(QPalette::HighlightedText);
            color.setAlphaF(.5);
        } else
            color = option.palette.color(QPalette::Highlight);

        painter->setPen(color);
        infoLayout.draw(painter, infoPos);
    }

    painter->restore();
}


QWidget *KFileItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(parent)
    Q_UNUSED(option)
    Q_UNUSED(index)

    return NULL;
}


bool KFileItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem & option, const QModelIndex &index)
{
    Q_UNUSED(event)
    Q_UNUSED(model)
    Q_UNUSED(option)
    Q_UNUSED(index)

    return false;
}


void KFileItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    Q_UNUSED(editor)
    Q_UNUSED(index)
}


void KFileItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    Q_UNUSED(editor)
    Q_UNUSED(model)
    Q_UNUSED(index)
}


void KFileItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,const QModelIndex &index) const
{
    Q_UNUSED(editor)
    Q_UNUSED(option)
    Q_UNUSED(index)
}


bool KFileItemDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_UNUSED(event)
    Q_UNUSED(view)
    Q_UNUSED(option)
    Q_UNUSED(index)

    return false;
}


// kate: space-indent on; indent-width 4; replace-tabs on;
