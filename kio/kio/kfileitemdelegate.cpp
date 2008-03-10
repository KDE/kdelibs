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

#include <config.h>

#include "kfileitemdelegate.h"

#include <QApplication>
#include <QStyle>
#include <QModelIndex>
#include <QPainter>
#include <QCache>
#include <QImage>
#include <QPainterPath>
#include <QTextLayout>
#include <QListView>
#include <QPaintEngine>

#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kiconeffect.h>
#include <kdirmodel.h>
#include <kfileitem.h>
#include <kcolorscheme.h>
#include <kglobalsettings.h>

#include "delegateanimationhandler_p.h"

#if defined(Q_WS_X11) && defined(HAVE_XRENDER)
#  include <X11/Xlib.h>
#  include <X11/extensions/Xrender.h>
#  include <QX11Info>
#endif


struct Margin
{
    int left, right, top, bottom;
};


class KFileItemDelegate::Private
{
    public:
        enum MarginType { ItemMargin = 0, TextMargin, IconMargin, NMargins };

        Private(KFileItemDelegate *parent);
        ~Private() {}

        QSize decorationSizeHint(const QStyleOptionViewItemV4 &option, const QModelIndex &index) const;
        QSize displaySizeHint(const QStyleOptionViewItemV4 &option, const QModelIndex &index) const;
        QString replaceNewlines(const QString &string) const;
        inline KFileItem fileItem(const QModelIndex &index) const;
        QString elidedText(QTextLayout &layout, const QStyleOptionViewItemV4 &option, const QSize &maxSize) const;
        QSize layoutText(QTextLayout &layout, const QStyleOptionViewItemV4 &option,
                         const QString &text, const QSize &constraints) const;
        QSize layoutText(QTextLayout &layout, const QString &text, int maxWidth) const;
        inline void setLayoutOptions(QTextLayout &layout, const QStyleOptionViewItemV4 &options,
                                     const QModelIndex &index, const KFileItem &item) const;
        inline bool verticalLayout(const QStyleOptionViewItemV4 &option) const;
        QPainterPath roundedRectangle(const QRectF &rect, qreal radius) const;
        QPixmap toPixmap(const QStyleOptionViewItemV4 &option, const QColor &color) const;
        QPixmap toPixmap(const QStyleOptionViewItemV4 &option, const QIcon &icon) const;
        inline QBrush brush(const QVariant &value, const QStyleOptionViewItemV4 &option) const;
        QBrush composite(const QColor &over, const QBrush &brush) const;
        QBrush foregroundBrush(const QStyleOptionViewItemV4 &option, const QModelIndex &index) const;
        QBrush backgroundBrush(const QStyleOptionViewItemV4 &option, const QModelIndex &index) const;
        inline qreal backgroundRadius(const QStyleOptionViewItemV4 &option) const;
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
        QString information(const QStyleOptionViewItemV4 &option, const QModelIndex &index, const KFileItem &item) const;
        bool isListView(const QStyleOptionViewItemV4 &option) const;
        QString display(const QModelIndex &index) const;
        QSize iconSizeHint(const QStyleOptionViewItemV4 &option, const QModelIndex &index) const;
        QPixmap decoration(const QStyleOptionViewItemV4 &option, const QModelIndex &index) const;
        QPoint iconPosition(const QStyleOptionViewItemV4 &option, const QPixmap &pixmap) const;
        QRect labelRectangle(const QStyleOptionViewItemV4 &option, const QPixmap &icon, const QString &string) const;
        void layoutTextItems(const QStyleOptionViewItemV4 &option, const QModelIndex &index, const QPixmap &icon,
                             QTextLayout *labelLayout, QTextLayout *infoLayout, QRect *textBoundingRect) const;
        void drawBackground(QPainter *painter, const QStyleOptionViewItemV4 &option, const QModelIndex &index,
                            const QRect &textBoundingRect) const;
        void drawTextItems(QPainter *painter, const QStyleOptionViewItemV4 &option, const QModelIndex &index,
                           const QTextLayout &labelLayout, const QTextLayout &infoLayout) const;
        KIO::AnimationState *animationState(const QStyleOptionViewItemV4 &option, const QModelIndex &index,
                                            const QAbstractItemView *view) const;
        QPixmap applyHoverEffect(const QPixmap &icon) const;
        QPixmap transition(const QPixmap &from, const QPixmap &to, qreal amount) const;
        void initStyleOption(QStyleOptionViewItemV4 *option, const QModelIndex &index) const;

    public:
        KFileItemDelegate::InformationList informationList;

    private:
        KFileItemDelegate * const q;
        KIO::DelegateAnimationHandler *animationHandler;
        Margin verticalMargin[NMargins];
        Margin horizontalMargin[NMargins];
        Margin *activeMargins;
};


KFileItemDelegate::Private::Private(KFileItemDelegate *parent)
    : q(parent), animationHandler(new KIO::DelegateAnimationHandler(parent))
{
}


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


// Returns the size of a file, or the number of items in a directory, as a QString
QString KFileItemDelegate::Private::itemSize(const QModelIndex &index, const KFileItem &item) const
{
    // Return a formatted string containing the file size, if the item is a file
    if (item.isFile())
        return KGlobal::locale()->formatByteSize(item.size());

    // Return the number of items in the directory
    const QVariant value = index.data(KDirModel::ChildCountRole);
    const int count = value.type() == QVariant::Int ? value.toInt() : KDirModel::ChildCountUnknown;

    if (count == KDirModel::ChildCountUnknown)
        return i18nc("Items in a folder", "? items");

    return i18ncp("Items in a folder", "1 item", "%1 items", count);
}


// Returns the additional information string, if one should be shown, or an empty string otherwise
QString KFileItemDelegate::Private::information(const QStyleOptionViewItemV4 &option, const QModelIndex &index,
                                                const KFileItem &item) const
{
    QString string;

    if (informationList.isEmpty() || item.isNull() || !isListView(option))
        return string;

    foreach (KFileItemDelegate::Information info, informationList)
    {
        if (info == KFileItemDelegate::NoInformation)
            continue;

        if (!string.isEmpty())
            string += QChar::LineSeparator;

        switch (info)
        {
            case KFileItemDelegate::Size:
                string += itemSize(index, item);
                break;

            case KFileItemDelegate::Permissions:
                string += item.permissionsString();
                break;

            case KFileItemDelegate::OctalPermissions:
                string += QString('0') + QString::number(item.permissions(), 8);
                break;

            case KFileItemDelegate::Owner:
                string += item.user();
                break;

            case KFileItemDelegate::OwnerAndGroup:
                string += item.user() + ':' + item.group();
                break;

            case KFileItemDelegate::CreationTime:
                string += item.timeString(KFileItem::CreationTime);
                break;

            case KFileItemDelegate::ModificationTime:
                string += item.timeString(KFileItem::ModificationTime);
                break;

            case KFileItemDelegate::AccessTime:
                string += item.timeString(KFileItem::AccessTime);
                break;

            case KFileItemDelegate::MimeType:
                string += item.isMimeTypeKnown() ? item.mimetype() : i18nc("@info mimetype","Unknown");
                break;

            case KFileItemDelegate::FriendlyMimeType:
                string += item.isMimeTypeKnown() ? item.mimeComment() : i18nc("@info mimetype","Unknown");
                break;

            default:
                break;
        } // switch (info)
    } // foreach (info, list)

    return string;
}


// Returns the KFileItem for the index
KFileItem KFileItemDelegate::Private::fileItem(const QModelIndex &index) const
{
    const QVariant value = index.data(KDirModel::FileItemRole);
    return qvariant_cast<KFileItem>(value);
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
QSize KFileItemDelegate::Private::layoutText(QTextLayout &layout, const QStyleOptionViewItemV4 &option,
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
QString KFileItemDelegate::Private::elidedText(QTextLayout &layout, const QStyleOptionViewItemV4 &option,
                                               const QSize &size) const
{
    const QString text = layout.text();
    int maxWidth       = size.width();
    int maxHeight      = size.height();
    qreal height       = 0;
    bool wrapText      = (option.features & QStyleOptionViewItemV2::WrapText);

    // If the string contains a single line of text that shouldn't be word wrapped
    if (!wrapText && text.indexOf(QChar::LineSeparator) == -1)
        return option.fontMetrics.elidedText(text, option.textElideMode, maxWidth);

    // Elide each line that has already been laid out in the layout.
    QString elided;
    elided.reserve(text.length());

    for (int i = 0; i < layout.lineCount(); i++)
    {
        QTextLine line = layout.lineAt(i);
        int start  = line.textStart();
        int length = line.textLength();

        height += option.fontMetrics.leading();
        if (height + line.height() + option.fontMetrics.lineSpacing() > maxHeight)
        {
            // Unfortunately, if the line ends because of a line separator, elidedText() will be too
            // clever and keep adding lines until it finds one that's too wide.
            if (line.naturalTextWidth() < maxWidth && text[start + length - 1] == QChar::LineSeparator)
                elided += text.mid(start, length - 1);
            else
                elided += option.fontMetrics.elidedText(text.mid(start), option.textElideMode, maxWidth);
            break;
        }
        else if (line.naturalTextWidth() > maxWidth)
        {
            elided += option.fontMetrics.elidedText(text.mid(start, length), option.textElideMode, maxWidth);
            if (!elided.endsWith(QChar::LineSeparator))
                elided += QChar::LineSeparator;
        }
        else
            elided += text.mid(start, length);

        height += line.height();
    }

    return elided;
}


void KFileItemDelegate::Private::setLayoutOptions(QTextLayout &layout, const QStyleOptionViewItemV4 &option,
                                                  const QModelIndex &index, const KFileItem &item) const
{
    QTextOption textoption;
    textoption.setTextDirection(option.direction);
    textoption.setAlignment(QStyle::visualAlignment(option.direction, option.displayAlignment));
    textoption.setWrapMode((option.features & QStyleOptionViewItemV2::WrapText) ?
                           QTextOption::WrapAtWordBoundaryOrAnywhere : QTextOption::NoWrap);

    layout.setFont(option.font);
    layout.setTextOption(textoption);
}


QSize KFileItemDelegate::Private::displaySizeHint(const QStyleOptionViewItemV4 &option,
                                                  const QModelIndex &index) const
{
    QString label = display(index);
    const int maxWidth = verticalLayout(option) && (option.features & QStyleOptionViewItemV2::WrapText)
            ? option.decorationSize.width() + 10 : 32757;

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


QSize KFileItemDelegate::Private::decorationSizeHint(const QStyleOptionViewItemV4 &option,
                                                     const QModelIndex &index) const
{
    Q_UNUSED(index)

    return addMargin(option.decorationSize, IconMargin);
}


bool KFileItemDelegate::Private::verticalLayout(const QStyleOptionViewItemV4 &option) const
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
QPixmap KFileItemDelegate::Private::toPixmap(const QStyleOptionViewItemV4 &option, const QIcon &icon) const
{
    QIcon::Mode mode   = option.state & QStyle::State_Enabled ? QIcon::Normal : QIcon::Disabled;
    QIcon::State state = option.state & QStyle::State_Open ? QIcon::On : QIcon::Off;
    const QSize size = icon.actualSize(option.decorationSize, mode, state);
    return icon.pixmap(size, mode, state);
}


// Converts a QColor to a pixmap
QPixmap KFileItemDelegate::Private::toPixmap(const QStyleOptionViewItemV4 &option, const QColor &color) const
{
    QPixmap pixmap(option.decorationSize);
    pixmap.fill(color);

    return pixmap;
}

// Converts a QVariant of type Brush or Color to a QBrush
QBrush KFileItemDelegate::Private::brush(const QVariant &value, const QStyleOptionViewItemV4 &option) const
{
    if (value.userType() == qMetaTypeId<KStatefulBrush>())
        return qvariant_cast<KStatefulBrush>(value).brush(option.palette);
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


// Composites over over brush, using the Porter/Duff over operator
// TODO move to KColorUtils?
QBrush KFileItemDelegate::Private::composite(const QColor &over, const QBrush &brush) const
{
    switch (brush.style())
    {
        case Qt::SolidPattern:
        {
            QColor under = brush.color();

            int red   = under.red()   + (over.red()   - under.red())   * over.alpha() / 255;
            int green = under.green() + (over.green() - under.green()) * over.alpha() / 255;
            int blue  = under.blue()  + (over.blue()  - under.blue())  * over.alpha() / 255;
            int alpha = over.alpha()  + under.alpha() * (255 - over.alpha()) / 255;

            return QColor(red, green, blue, alpha);
        }

        case Qt::TexturePattern:
        {
            QPixmap texture = brush.texture();

            // CompositionMode_SourceOver is the default composition mode
            QPainter painter(&texture);
            painter.fillRect(texture.rect(), over);

            return texture;
        }

        // TODO Handle gradients

        default:
            return over;
    }
}


QBrush KFileItemDelegate::Private::foregroundBrush(const QStyleOptionViewItemV4 &option, const QModelIndex &index) const
{
    // Always use the highlight color for selected items
    if (option.state & QStyle::State_Selected)
        return option.palette.brush(QPalette::HighlightedText);

    // If the model provides its own foreground color/brush for this item
    const QVariant value = index.data(Qt::ForegroundRole);
    if (value.isValid())
        return brush(value, option);

    return option.palette.brush(QPalette::Text);
}


QBrush KFileItemDelegate::Private::backgroundBrush(const QStyleOptionViewItemV4 &option, const QModelIndex &index) const
{
    QBrush background(Qt::NoBrush);

    // Always use the highlight color for selected items
    if (option.state & QStyle::State_Selected)
        background = option.palette.brush(QPalette::Highlight);
    else
    {
        // If the item isn't selected, check if model provides its own background
        // color/brush for this item
        const QVariant value = index.data(Qt::BackgroundRole);
        if (value.isValid())
            background = brush(value, option);
    }

    // If we don't already have a background brush, check if the background color
    // should be alternated for this item.
    if (background.style() == Qt::NoBrush && (option.features & QStyleOptionViewItemV2::Alternate))
        background = option.palette.brush(QPalette::AlternateBase);

    // Composite the hover color over the background brush
    if ((option.state & QStyle::State_MouseOver) && index.column() == KDirModel::Name)
    {
        // Use a lighter version of the highlight color with 1/3 opacity
        QColor hover = option.palette.color(QPalette::Highlight);
        hover.setAlpha(88);

        background = composite(hover, background);
    }

    return background;
}


qreal KFileItemDelegate::Private::backgroundRadius(const QStyleOptionViewItemV4 &option) const
{
    return (option.showDecorationSelected && option.decorationSize.width() > 24) ? 10 : 5;
}


bool KFileItemDelegate::Private::isListView(const QStyleOptionViewItemV4 &option) const
{
    if (qobject_cast<const QListView*>(option.widget) || verticalLayout(option))
        return true;

    return false;
}


QPixmap KFileItemDelegate::Private::applyHoverEffect(const QPixmap &icon) const
{
    KIconEffect *effect = KIconLoader::global()->iconEffect();

    // Note that in KIconLoader terminology, active = hover.
    // ### We're assuming that the icon group is desktop/filemanager, since this
    //     is KFileItemDelegate.
    if (effect->hasEffect(KIconLoader::Desktop, KIconLoader::ActiveState))
        return effect->apply(icon, KIconLoader::Desktop, KIconLoader::ActiveState);

    return icon;
}


KIO::AnimationState *KFileItemDelegate::Private::animationState(const QStyleOptionViewItemV4 &option,
                                                                const QModelIndex &index,
                                                                const QAbstractItemView *view) const
{
    if (!(KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects)) {
        return NULL;
    }

    if (index.column() == KDirModel::Name)
        return animationHandler->animationState(option, index, view);

    return NULL;
}


QPixmap KFileItemDelegate::Private::transition(const QPixmap &from, const QPixmap &to, qreal amount) const
{
    int value = int(0xff * amount);

    if (value == 0)
        return from;

    if (value == 1)
        return to;

    QColor color;
    color.setAlphaF(amount);

    // If the native paint engine supports Porter/Duff compositing and CompositionMode_Plus
    if (from.paintEngine()->hasFeature(QPaintEngine::PorterDuff) &&
        from.paintEngine()->hasFeature(QPaintEngine::BlendModes))
    {
        QPixmap under = from;
        QPixmap over  = to;

        QPainter p;
        p.begin(&over);
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.fillRect(over.rect(), color);
        p.end();

        p.begin(&under);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        p.fillRect(under.rect(), color);
        p.setCompositionMode(QPainter::CompositionMode_Plus);
        p.drawPixmap(0, 0, over);
        p.end();

        return under;
    }
#if defined(Q_WS_X11) && defined(HAVE_XRENDER)
    else if (from.paintEngine()->hasFeature(QPaintEngine::PorterDuff)) // We have Xrender support
    {
        // QX11PaintEngine doesn't implement CompositionMode_Plus in Qt 4.3,
        // which we need to be able to do a transition from one pixmap to
        // another.
        //
        // In order to avoid the overhead of converting the pixmaps to images
        // and doing the operation entirely in software, this function has a
        // specialized path for X11 that uses Xrender directly to do the
        // transition. This operation can be fully accelerated in HW.
        //
        // This specialization can be removed when QX11PaintEngine supports
        // CompositionMode_Plus.
        QPixmap source(to), destination(from);

        source.detach();
        destination.detach();

        Display *dpy = QX11Info::display();

        XRenderPictFormat *format = XRenderFindStandardFormat(dpy, PictStandardA8);
        XRenderPictureAttributes pa;
        pa.repeat = 1; // RepeatNormal

        // Create a 1x1 8 bit repeating alpha picture
        Pixmap pixmap = XCreatePixmap(dpy, destination.handle(), 1, 1, 8);
        Picture alpha = XRenderCreatePicture(dpy, pixmap, format, CPRepeat, &pa);
        XFreePixmap(dpy, pixmap);

        // Fill the alpha picture with the opacity value
        XRenderColor xcolor;
        xcolor.alpha = quint16(0xffff * amount);
        XRenderFillRectangle(dpy, PictOpSrc, alpha, &xcolor, 0, 0, 1, 1);

        // Reduce the alpha of the destination with 1 - opacity
        XRenderComposite(dpy, PictOpOutReverse, alpha, None, destination.x11PictureHandle(),
                         0, 0, 0, 0, 0, 0, destination.width(), destination.height());

        // Add source * opacity to the destination
        XRenderComposite(dpy, PictOpAdd, source.x11PictureHandle(), alpha,
                         destination.x11PictureHandle(),
                         0, 0, 0, 0, 0, 0, destination.width(), destination.height());

        XRenderFreePicture(dpy, alpha);
        return destination;
    }
#endif
    else
    {
        // Fall back to using QRasterPaintEngine to do the transition.
        QImage under = from.toImage();
        QImage over  = to.toImage();

        QPainter p;
        p.begin(&over);
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.fillRect(over.rect(), color);
        p.end();

        p.begin(&under);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        p.fillRect(under.rect(), color);
        p.setCompositionMode(QPainter::CompositionMode_Plus);
        p.drawImage(0, 0, over);
        p.end();

        return QPixmap::fromImage(under);
    }
}


void KFileItemDelegate::Private::layoutTextItems(const QStyleOptionViewItemV4 &option, const QModelIndex &index,
                                                 const QPixmap &icon, QTextLayout *labelLayout,
                                                 QTextLayout *infoLayout, QRect *textBoundingRect) const
{
    KFileItem item       = fileItem(index);
    const QString label  = display(index);
    const QString info   = information(option, index, item);
    bool showInformation = false;

    setLayoutOptions(*labelLayout, option, index, item);

    const QRect textArea = labelRectangle(option, icon, label);
    QRect textRect       = subtractMargin(textArea, Private::TextMargin);

    // Sizes and constraints for the different text parts
    QSize maxLabelSize = textRect.size();
    QSize maxInfoSize  = textRect.size();
    QSize labelSize;
    QSize infoSize;

    // If we have additional info text, and there's space for at least two lines of text,
    // adjust the max label size to make room for at least one line of the info text
    if (!info.isEmpty() && textRect.height() >= option.fontMetrics.lineSpacing() * 2)
    {
        infoLayout->setFont(labelLayout->font());
        infoLayout->setTextOption(labelLayout->textOption());

        maxLabelSize.rheight() -= option.fontMetrics.lineSpacing();
        showInformation = true;
    }

    // Lay out the label text, and adjust the max info size based on the label size
    labelSize = layoutText(*labelLayout, option, label, maxLabelSize);
    maxInfoSize.rheight() -= labelSize.height();

    // Lay out the info text
    if (showInformation)
        infoSize = layoutText(*infoLayout, option, info, maxInfoSize);
    else
        infoSize = QSize(0, 0);

    // Compute the bounding rect of the text
    const QSize size(qMax(labelSize.width(), infoSize.width()), labelSize.height() + infoSize.height());
    *textBoundingRect = QStyle::alignedRect(option.direction, option.displayAlignment, size, textRect);

    // Compute the positions where we should draw the layouts
    labelLayout->setPosition(QPointF(textRect.x(), textBoundingRect->y()));
    infoLayout->setPosition(QPointF(textRect.x(), textBoundingRect->y() + labelSize.height()));
}


void KFileItemDelegate::Private::drawTextItems(QPainter *painter, const QStyleOptionViewItemV4 &option,
                                               const QModelIndex &index, const QTextLayout &labelLayout,
                                               const QTextLayout &infoLayout) const
{
    QPen pen(foregroundBrush(option, index), 0);
    painter->setPen(pen);
    labelLayout.draw(painter, QPoint());

    if (!infoLayout.text().isEmpty())
    {
        // TODO - for apps not doing funny things with the color palette,
        // KColorScheme::InactiveText would be a much more correct choice. We
        // should provide an API to specify what color to use for information.
        QColor color = pen.color();
        color.setAlphaF(0.6);

        painter->setPen(color);
        infoLayout.draw(painter, QPoint());
    }
}


void KFileItemDelegate::Private::drawBackground(QPainter *painter, const QStyleOptionViewItemV4 &option,
                                                const QModelIndex &index, const QRect &textBoundingRect) const
{
    const QBrush brush = backgroundBrush(option, index);

    if (brush.style() != Qt::NoBrush)
    {
        const qreal radius = backgroundRadius(option);

        QRect rect;
        if (option.showDecorationSelected)
            rect = option.rect;
        else
            rect = addMargin(textBoundingRect, Private::TextMargin);

        // Always draw rounded selection rectangles in list views
        QPainterPath path;
        if (isListView(option))
            path = roundedRectangle(rect, radius);
        else
            path.addRect(rect);

        painter->fillPath(path, brush);
    }
}


void KFileItemDelegate::Private::initStyleOption(QStyleOptionViewItemV4 *option,
                                                 const QModelIndex &index) const
{
    const KFileItem item = fileItem(index);
    bool updateFontMetrics = false;

    // Try to get the font from the model
    QVariant value = index.data(Qt::FontRole);
    if (value.isValid()) {
        option->font = qvariant_cast<QFont>(value).resolve(option->font);
        updateFontMetrics = true;
    }

    // Use an italic font for symlinks
    if (!item.isNull() && item.isLink()) {
        option->font.setItalic(true);
        updateFontMetrics = true;
    }

    if (updateFontMetrics)
        option->fontMetrics = QFontMetrics(option->font);

    // Try to get the alignment for the item from the model
    value = index.data(Qt::TextAlignmentRole);
    if (value.isValid())
        option->displayAlignment = Qt::Alignment(value.toInt());

    value = index.data(Qt::BackgroundRole);
    if (value.isValid())
        option->backgroundBrush = brush(value, *option);

    // ### Make sure this value is always true for now
    option->showDecorationSelected = true;
}




// ---------------------------------------------------------------------------




KFileItemDelegate::KFileItemDelegate(QObject *parent)
    : QAbstractItemDelegate(parent), d(new Private(this))
{
    int focusHMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin);
    int focusVMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameVMargin);

    // Margins for horizontal mode (list views, tree views, table views)
    const int textMargin = focusHMargin * 4;
    if (QApplication::isRightToLeft())
        d->setHorizontalMargin(Private::TextMargin, textMargin, focusVMargin, focusHMargin, focusVMargin);
    else
        d->setHorizontalMargin(Private::TextMargin, focusHMargin, focusVMargin, textMargin, focusVMargin);

    d->setHorizontalMargin(Private::IconMargin, focusHMargin, focusVMargin);
    d->setHorizontalMargin(Private::ItemMargin, 0, 0);

    // Margins for vertical mode (icon views)
    d->setVerticalMargin(Private::TextMargin, 6, 2);
    d->setVerticalMargin(Private::IconMargin, focusHMargin, focusVMargin);
    d->setVerticalMargin(Private::ItemMargin, 0, 0);

    setShowInformation(NoInformation);
}


KFileItemDelegate::~KFileItemDelegate()
{
    delete d;
}


QSize KFileItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // If the model wants to provide its own size hint for the item
    const QVariant value = index.data(Qt::SizeHintRole);
    if (value.isValid())
        return qvariant_cast<QSize>(value);

    QStyleOptionViewItemV4 opt(option);
    d->initStyleOption(&opt, index);
    d->setActiveMargins(d->verticalLayout(option) ? Qt::Vertical : Qt::Horizontal);

    const QSize displaySize    = d->displaySizeHint(opt, index);
    const QSize decorationSize = d->decorationSizeHint(opt, index);

    QSize size;

    if (d->verticalLayout(opt))
    {
        const QSize iconSize = d->iconSizeHint(opt, index);
        const int iconWidth = qMin(decorationSize.width(), iconSize.width());
        size.rwidth()  = qMax(displaySize.width(), iconWidth);
        size.rheight() = decorationSize.height() + displaySize.height() + 1;
    }
    else
    {
        size.rwidth()  = decorationSize.width() + displaySize.width() + 1;
        size.rheight() = qMax(decorationSize.height(), displaySize.height());
    }

    return d->addMargin(size, Private::ItemMargin);
}


QString KFileItemDelegate::Private::display(const QModelIndex &index) const
{
    const QVariant value = index.data(Qt::DisplayRole);

    switch (value.type())
    {
        case QVariant::String:
        {
            if (index.column() == KDirModel::Size)
                return itemSize(index, fileItem(index));
            else
                return replaceNewlines(value.toString());
        }

        case QVariant::Double:
            return KGlobal::locale()->formatNumber(value.toDouble());

        case QVariant::Int:
        case QVariant::UInt:
            return KGlobal::locale()->formatLong(value.toInt());

        default:
            return QString();
    }
}


void KFileItemDelegate::setShowInformation(const InformationList &list)
{
    d->informationList = list;
}


void KFileItemDelegate::setShowInformation(Information value)
{
    if (value != NoInformation)
        d->informationList = InformationList() << value;
    else
        d->informationList = InformationList();
}


KFileItemDelegate::InformationList KFileItemDelegate::showInformation() const
{
    return d->informationList;
}


QSize KFileItemDelegate::Private::iconSizeHint(const QStyleOptionViewItemV4 &option, const QModelIndex &index) const
{
    QSize size;

    const QVariant value = index.data(Qt::DecorationRole);
    switch (value.type())
    {
        case QVariant::Icon:
            size = toPixmap(option, qvariant_cast<QIcon>(value)).size();
            break;

        case QVariant::Pixmap:
            size = qvariant_cast<QPixmap>(value).size();
            break;

        case QVariant::Color:
        default:
            size = option.decorationSize;
            break;
    }

    const int radius = backgroundRadius(option);
    size.rwidth() += radius;
    size.rheight() += radius;
    return size;
}


QPixmap KFileItemDelegate::Private::decoration(const QStyleOptionViewItemV4 &option, const QModelIndex &index) const
{
    const QVariant value = index.data(Qt::DecorationRole);
    QPixmap pixmap;

    switch (value.type())
    {
        case QVariant::Icon:
            pixmap = toPixmap(option, qvariant_cast<QIcon>(value));
            break;

        case QVariant::Pixmap:
            pixmap = qvariant_cast<QPixmap>(value);
            break;

        case QVariant::Color:
            pixmap = toPixmap(option, qvariant_cast<QColor>(value));
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
            QPainter p(&pixmap);
            QColor color = option.palette.color(QPalette::Highlight);
            color.setAlphaF(0.5);
            p.setCompositionMode(QPainter::CompositionMode_SourceAtop);
            p.fillRect(pixmap.rect(), color);
        }

        // Apply the configured hover effect
        if ((option.state & QStyle::State_MouseOver) && index.column() == KDirModel::Name)
            pixmap = applyHoverEffect(pixmap);
    }

    return pixmap;
}


QRect KFileItemDelegate::Private::labelRectangle(const QStyleOptionViewItemV4 &option, const QPixmap &icon,
                                                 const QString &string) const
{
    Q_UNUSED(string)

    if (icon.isNull())
        return option.rect;

    const QSize decoSize = addMargin(option.decorationSize, Private::IconMargin);
    const QRect itemRect = subtractMargin(option.rect, Private::ItemMargin);
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


QPoint KFileItemDelegate::Private::iconPosition(const QStyleOptionViewItemV4 &option, const QPixmap &pixmap) const
{
    const QRect itemRect = subtractMargin(option.rect, Private::ItemMargin);
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
    const QSize size = addMargin(option.decorationSize, Private::IconMargin);
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

    QStyleOptionViewItemV4 opt(option);
    d->initStyleOption(&opt, index);

    const QAbstractItemView *view = qobject_cast<const QAbstractItemView*>(opt.widget);


    // Check if the item is being animated
    // ========================================================================
    KIO::AnimationState *state = d->animationState(opt, index, view);
    KIO::CachedRendering *cache = 0;
    qreal progress = ((opt.state & QStyle::State_MouseOver) &&
                index.column() == KDirModel::Name) ? 1.0 : 0.0;

    if (state)
    {
        cache    = state->cachedRendering();
        progress = state->hoverProgress();

        // Clear the mouse over bit temporarily
        opt.state &= ~QStyle::State_MouseOver;

        // If we have a cached rendering, draw the item from the cache
        if (cache)
        {
            if (cache->checkValidity(opt.state) && cache->regular.size() == opt.rect.size())
            {
                const QPixmap pixmap = d->transition(cache->regular, cache->hover, progress);
                painter->drawPixmap(option.rect.topLeft(), pixmap);
                return;
            }

            // If it wasn't valid, delete it
            state->setCachedRendering(0);
            delete cache;
            cache = 0;
        }
    }

    d->setActiveMargins(d->verticalLayout(opt) ? Qt::Vertical : Qt::Horizontal);


    // Compute the metrics, and lay out the text items
    // ========================================================================
    QPixmap icon         = d->decoration(opt, index);
    const QPoint iconPos = d->iconPosition(opt, icon);

    QTextLayout labelLayout, infoLayout;
    QRect textBoundingRect;

    d->layoutTextItems(option, index, icon, &labelLayout, &infoLayout, &textBoundingRect);


    // Create a new cached rendering of a hovered and an unhovered item.
    // We don't create a new cache for a fully hovered item, since we don't
    // know yet if a hover out animation will be run.
    // ========================================================================
    if (state && progress < 1)
    {
        cache = new KIO::CachedRendering(opt.state, option.rect.size());

        QPainter p;
        p.begin(&cache->regular);
        p.translate(-option.rect.topLeft());
        p.setRenderHint(QPainter::Antialiasing);
        d->drawBackground(&p, opt, index, textBoundingRect);
        p.drawPixmap(iconPos, icon);
        d->drawTextItems(&p, opt, index, labelLayout, infoLayout);
        p.end();

        opt.state |= QStyle::State_MouseOver;
        icon = d->applyHoverEffect(icon);

        p.begin(&cache->hover);
        p.translate(-option.rect.topLeft());
        p.setRenderHint(QPainter::Antialiasing);
        d->drawBackground(&p, opt, index, textBoundingRect);
        p.drawPixmap(iconPos, icon);
        d->drawTextItems(&p, opt, index, labelLayout, infoLayout);
        p.end();

        state->setCachedRendering(cache);

        const QPixmap pixmap = d->transition(cache->regular, cache->hover, progress);
        painter->drawPixmap(option.rect.topLeft(), pixmap);
        return;
    }


    // Render the item directly if we're not using a cached rendering
    // ========================================================================
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    if (progress > 0 && !(opt.state & QStyle::State_MouseOver))
    {
        opt.state |= QStyle::State_MouseOver;
        icon = d->applyHoverEffect(icon);
    }

    d->drawBackground(painter, opt, index, textBoundingRect);
    painter->drawPixmap(iconPos, icon);
    d->drawTextItems(painter, opt, index, labelLayout, infoLayout);

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


#include "kfileitemdelegate.moc"


// kate: space-indent on; indent-width 4; replace-tabs on;
