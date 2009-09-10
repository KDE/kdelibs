/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 by Marco Martin <notmart@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "desktoptoolbox_p.h"

#include <QGraphicsSceneHoverEvent>
#include <QPainter>
#include <QRadialGradient>
#include <QGraphicsView>
#include <QAction>

#include <kcolorscheme.h>
#include <kdebug.h>
#include <kiconloader.h>

#include <plasma/theme.h>
#include <plasma/paintutils.h>
#include <plasma/framesvg.h>

#include <plasma/applet.h>
#include <plasma/containment.h>
#include <plasma/widgets/iconwidget.h>

namespace Plasma
{

class EmptyGraphicsItem : public QGraphicsItem
{
    public:
        EmptyGraphicsItem(QGraphicsItem *parent)
            : QGraphicsItem(parent),
              m_toolbar(false)
        {
            setAcceptsHoverEvents(true);
            m_background = new Plasma::FrameSvg();
            m_toolbarBackground = new Plasma::FrameSvg();

            m_toolbarBackground->setImagePath("widgets/background");
            m_background->setImagePath("widgets/translucentbackground");

            m_toolbarBackground->setEnabledBorders(FrameSvg::LeftBorder|FrameSvg::RightBorder|FrameSvg::BottomBorder);
            m_background->setEnabledBorders(FrameSvg::AllBorders);
        }

        ~EmptyGraphicsItem()
        {
            delete m_background;
            delete m_toolbarBackground;
        }

        QRectF boundingRect() const
        {
            return QRectF(QPointF(0, 0), m_rect.size());
        }

        QRectF rect() const
        {
            return m_rect;
        }

        void setIsToolbar(bool toolbar)
        {
            m_toolbar = toolbar;
        }

        bool isToolbar() const
        {
            return m_toolbar;
        }

        void getContentsMargins(qreal &left, qreal &top, qreal &right, qreal &bottom)
        {
            if (m_toolbar) {
                m_toolbarBackground->getMargins(left, top, right, bottom);
            } else {
                m_background->getMargins(left, top, right, bottom);
            }
        }

        QRectF contentsRect() const
        {
            qreal left, top, right, bottom;

            if (m_toolbar) {
                m_toolbarBackground->getMargins(left, top, right, bottom);
            } else {
                m_background->getMargins(left, top, right, bottom);
            }
            return m_rect.adjusted(left, top, -right, -bottom);
        }

        void setRect(const QRectF &rect)
        {
            if (m_rect == rect)
                return;
            //kDebug() << "setting rect to" << rect;
            prepareGeometryChange();
            m_rect = rect;
            setPos(m_rect.topLeft());
            if (m_toolbar) {
                m_toolbarBackground->resizeFrame(m_rect.size());
            } else {
                m_background->resizeFrame(m_rect.size());
            }
        }

        void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *)
        {
            if (m_toolbar) {
                m_toolbarBackground->paintFrame(p);
            } else {
                m_background->paintFrame(p);
            }
        }

    private:
        bool m_toolbar;
        QRectF m_rect;
        Plasma::FrameSvg *m_toolbarBackground;
        Plasma::FrameSvg *m_background;
};

// used with QGrahphicsItem::setData
static const int ToolName = 7001;

class DesktopToolBoxPrivate
{
public:
    DesktopToolBoxPrivate(DesktopToolBox *toolbox)
      : q(toolbox),
        background(0),
        containment(0),
        icon("plasma"),
        toolBacker(0),
        animCircleId(0),
        animHighlightId(0),
        animCircleFrame(0),
        animHighlightFrame(0),
        hovering(0)
    {}

    void adjustBackgroundBorders()
    {
        switch (q->corner()) {
          case ToolBox::TopRight:
            background->setEnabledBorders(FrameSvg::BottomBorder|FrameSvg::LeftBorder);
            break;
        case ToolBox::Top:
            background->setEnabledBorders(FrameSvg::BottomBorder|FrameSvg::LeftBorder|FrameSvg::RightBorder);
            break;
        case ToolBox::TopLeft:
            background->setEnabledBorders(FrameSvg::BottomBorder|FrameSvg::RightBorder);
            break;
        case ToolBox::Left:
            background->setEnabledBorders(FrameSvg::BottomBorder|FrameSvg::TopBorder|FrameSvg::RightBorder);
            break;
        case ToolBox::Right:
            background->setEnabledBorders(FrameSvg::BottomBorder|FrameSvg::TopBorder|FrameSvg::LeftBorder);
            break;
        case ToolBox::BottomLeft:
            background->setEnabledBorders(FrameSvg::TopBorder|FrameSvg::RightBorder);
            break;
        case ToolBox::Bottom:
            background->setEnabledBorders(FrameSvg::TopBorder|FrameSvg::LeftBorder|FrameSvg::RightBorder);
            break;
        case ToolBox::BottomRight:
        default:
            background->setEnabledBorders(FrameSvg::TopBorder|FrameSvg::LeftBorder);
            break;
        }
    }

    DesktopToolBox *q;
    Plasma::FrameSvg *background;
    Containment *containment;
    KIcon icon;
    EmptyGraphicsItem *toolBacker;
    int animCircleId;
    int animHighlightId;
    qreal animCircleFrame;
    qreal animHighlightFrame;
    QRect shapeRect;
    QColor fgColor;
    QColor bgColor;
    bool hovering : 1;
};

DesktopToolBox::DesktopToolBox(Containment *parent)
    : ToolBox(parent),
      d(new DesktopToolBoxPrivate(this))
{
    d->background = new Plasma::FrameSvg(this);
    d->background->setImagePath("widgets/toolbox");

    d->containment = parent;
    setZValue(10000000);

    setIsMovable(true);
    updateTheming();

    connect(Plasma::Animator::self(), SIGNAL(movementFinished(QGraphicsItem*)),
            this, SLOT(toolMoved(QGraphicsItem*)));
    connect(this, SIGNAL(toggled()), this, SLOT(toggle()));
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
            this, SLOT(updateTheming()));
}

DesktopToolBox::~DesktopToolBox()
{
    delete d;
}

QSize DesktopToolBox::cornerSize() const
{
    d->background->setEnabledBorders(FrameSvg::AllBorders);
    qreal left, top, right, bottom;
    d->background->getMargins(left, top, right, bottom);
    d->adjustBackgroundBorders();

    return QSize(size() + left, size() + bottom);
}

QSize DesktopToolBox::fullWidth() const
{
    d->background->setEnabledBorders(FrameSvg::AllBorders);
    qreal left, top, right, bottom;
    d->background->getMargins(left, top, right, bottom);
    d->adjustBackgroundBorders();

    int extraSpace = 0;
    if (!d->containment->activity().isNull()) {
        extraSpace = Plasma::Theme::defaultTheme()->fontMetrics().width(d->containment->activity()+"x");
    }

    return QSize(size() + left + right + extraSpace, size() + bottom);
}

QSize DesktopToolBox::fullHeight() const
{
    d->background->setEnabledBorders(FrameSvg::AllBorders);
    qreal left, top, right, bottom;
    d->background->getMargins(left, top, right, bottom);
    d->adjustBackgroundBorders();

    int extraSpace = 0;
    if (!d->containment->activity().isNull()) {
        extraSpace = Plasma::Theme::defaultTheme()->fontMetrics().width(d->containment->activity()+"x");
    }

    return QSize(size() + left, size() + top + bottom + extraSpace);
}

QRectF DesktopToolBox::boundingRect() const
{
    int extraSpace = size();

    d->adjustBackgroundBorders();

    //keep space for the label and a character more
    if (!d->containment->activity().isNull()) {
        extraSpace = Plasma::Theme::defaultTheme()->fontMetrics().width(d->containment->activity()+"x");
    }

    qreal left, top, right, bottom;
    d->background->getMargins(left, top, right, bottom);

    QRectF rect;

    //disable text at corners
    if (corner() == TopLeft || corner() == TopRight || corner() == BottomLeft || corner() == BottomRight) {
        rect = QRectF(0, 0, size()+left+right, size()+top+bottom);
    } else if (corner() == Left || corner() == Right) {
        rect = QRectF(0, 0, size()+left+right, size()+extraSpace+top+bottom);
    //top or bottom
    } else {
        rect = QRectF(0, 0, size()+extraSpace+left+right, size()+top+bottom);
    }

    return rect;
}

void DesktopToolBox::updateTheming()
{
    d->bgColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
    d->fgColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
}

void DesktopToolBox::toolTriggered(bool)
{
    QAction *action = qobject_cast<QAction *>(sender());

    if (showing() && (!action || !action->autoRepeat())) {
        emit toggled();
    }
}

void DesktopToolBox::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (isToolbar()){
        return;
    }

    QPainterPath p = shape();

    QPoint iconPos;
    QRect backgroundRect;
    const QRectF rect = boundingRect();
    const QSize icons = iconSize();

    QString cornerElement;

    switch (corner()) {
    case TopLeft:
        cornerElement = "desktop-northwest";
        break;
    case TopRight:
        cornerElement = "desktop-northeast";
        break;
    case BottomRight:
        cornerElement = "desktop-southeast";
        break;
    case BottomLeft:
        cornerElement = "desktop-southwest";
        break;
    default:
        break;
    }

    QString activityName;

    QSize textSize;
    if (cornerElement.isNull()) {
        activityName = d->containment->activity();
        textSize =  Plasma::Theme::defaultTheme()->fontMetrics().size(Qt::TextSingleLine, activityName+"x");
    }

    d->adjustBackgroundBorders();

    d->background->resizeFrame(rect.size());

    if (!cornerElement.isNull()) {
        d->background->paint(painter, rect, cornerElement);
    } else {
        d->background->paintFrame(painter, rect.topLeft());
    }


    QRect iconRect;
    QRect textRect;

    if (corner() == Left || corner() == Right) {
        Qt::Alignment alignment;

        if (activityName.isNull()) {
            alignment = Qt::Alignment(Qt::AlignCenter);
        } else {
            alignment = Qt::Alignment(Qt::AlignHCenter|Qt::AlignTop);
        }

        iconRect = QStyle::alignedRect(QApplication::layoutDirection(), alignment, iconSize(), d->background->contentsRect().toRect());

        QRect boundRect(QPoint(d->background->contentsRect().top(),
                               d->background->contentsRect().left()),
                        QSize(d->background->contentsRect().height(),
                              d->background->contentsRect().width()));

        textRect = QStyle::alignedRect(QApplication::layoutDirection(), Qt::AlignRight|Qt::AlignVCenter, textSize, boundRect);
        textRect.moveTopLeft(textRect.topLeft() + QPoint(rect.top(), rect.left()));
    } else {
        Qt::Alignment alignment;

        if (activityName.isNull()) {
            alignment = Qt::Alignment(Qt::AlignCenter);
        } else {
            alignment = Qt::Alignment(Qt::AlignLeft|Qt::AlignVCenter);
        }

        iconRect = QStyle::alignedRect(QApplication::layoutDirection(), alignment, iconSize(), d->background->contentsRect().toRect());

        textRect = QStyle::alignedRect(QApplication::layoutDirection(), Qt::AlignRight|Qt::AlignVCenter, textSize, d->background->contentsRect().toRect());
        textRect.moveTopLeft(textRect.topLeft() + rect.topLeft().toPoint());
    }

    iconRect.moveTopLeft(iconRect.topLeft() + rect.topLeft().toPoint());


    iconPos = iconRect.topLeft();

    const qreal progress = d->animHighlightFrame;

    if (qFuzzyCompare(qreal(1.0), progress)) {
        d->icon.paint(painter, QRect(iconPos, iconSize()));
    } else if (qFuzzyCompare(qreal(1.0), 1 + progress)) {
        d->icon.paint(painter, QRect(iconPos, iconSize()),
                      Qt::AlignCenter, QIcon::Disabled, QIcon::Off);
    } else {
        QPixmap disabled = d->icon.pixmap(iconSize(), QIcon::Disabled, QIcon::Off);
        QPixmap enabled = d->icon.pixmap(iconSize());
        QPixmap result = PaintUtils::transition(
            d->icon.pixmap(iconSize(), QIcon::Disabled, QIcon::Off),
            d->icon.pixmap(iconSize()), progress);
        painter->drawPixmap(QRect(iconPos, iconSize()), result);
    }

    if (!cornerElement.isNull() || activityName.isNull()) {
        return;
    }

    QColor textColor = Plasma::Theme::defaultTheme()->color(Theme::TextColor);
    QColor shadowColor;
    QPoint shadowOffset;

    if (qGray(textColor.rgb()) > 192) {
        shadowColor = Qt::black;
        shadowOffset = QPoint(1,1);
    } else {
        shadowColor = Qt::white;
        shadowOffset = QPoint(0,0);
    }

    QPixmap shadowText = Plasma::PaintUtils::shadowText(activityName, textColor, shadowColor, shadowOffset);

    painter->save();
    if (corner() == Left || corner() == Right) {
        painter->rotate(90);
        painter->translate(textRect.left(), -textRect.top()-textRect.height());
        painter->drawPixmap(QPoint(0,0), shadowText);
    } else {
        painter->drawPixmap(textRect.topLeft(), shadowText);
    }

    painter->restore();
}

QPainterPath DesktopToolBox::shape() const
{
    const QRectF rect = boundingRect();
    const int w = rect.width();
    const int h = rect.height();

    QPainterPath path;
    switch (corner()) {
    case BottomLeft:
        path.moveTo(rect.bottomLeft());
        path.arcTo(QRectF(rect.left() - w, rect.top(), w * 2, h * 2), 0, 90);
        break;
    case BottomRight:
        path.moveTo(rect.bottomRight());
        path.arcTo(QRectF(rect.left(), rect.top(), w * 2, h * 2), 90, 90);
        break;
    case TopRight:
        path.moveTo(rect.topRight());
        path.arcTo(QRectF(rect.left(), rect.top() - h, w * 2, h * 2), 180, 90);
        break;
    case TopLeft:
        path.arcTo(QRectF(rect.left() - w, rect.top() - h, w * 2, h * 2), 270, 90);
        break;
    default:
        path.addRect(rect);
        break;
    }

    return path;
}

void DesktopToolBox::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (showing() || d->hovering) {
        QGraphicsItem::hoverEnterEvent(event);
        return;
    }
    Plasma::Animator *animdriver = Plasma::Animator::self();
    if (d->animHighlightId) {
        animdriver->stopCustomAnimation(d->animHighlightId);
    }
    d->hovering = true;
    d->animHighlightId =
        animdriver->customAnimation(
            10, 240, Plasma::Animator::EaseInCurve, this, "animateHighlight");

    QGraphicsItem::hoverEnterEvent(event);
}

void DesktopToolBox::showToolBox()
{
    setFlag(ItemIgnoresTransformations, isToolbar());

    if (showing() && !isToolbar())
        return;

    // put tools 5px from icon edge
    const int iconWidth = KIconLoader::SizeMedium;
    int x;
    int y;
    switch (corner()) {
    case TopRight:
        x = (int)boundingRect().right() - iconWidth - 5;
        y = (int)boundingRect().top() + 10;
        break;
    case Top:
        x = (int)boundingRect().center().x() - iconWidth;
        y = (int)boundingRect().top() + iconWidth + 10;
        break;
    case TopLeft:
        x = (int)boundingRect().left() + iconWidth + 5;
        y = (int)boundingRect().top() + 10;
        break;
    case Left:
        x = (int)boundingRect().left() + iconWidth + 5;
        y = (int)boundingRect().center().y() - iconWidth;
        break;
    case Right:
        x = (int)boundingRect().right() - iconWidth - 5;
        y = (int)boundingRect().center().y() - iconWidth;
        break;
    case BottomLeft:
        x = (int)boundingRect().left() + iconWidth + 5;
        y = (int)boundingRect().bottom() - 5;
        break;
    case Bottom:
        x = (int)boundingRect().center().x() - iconWidth;
        y = (int)boundingRect().bottom() - iconWidth - 5;
        break;
    case BottomRight:
    default:
        x = (int)boundingRect().right() - iconWidth - 5;
        y = (int)boundingRect().bottom() - iconWidth - 5;
        break;
    }

    int startY = y;

    // find our theoretical X and Y end coordinates

    int maxWidth = 0;
    int maxHeight = 0;
    int totalWidth = 0;

    foreach (QGraphicsItem *tool, QGraphicsItem::children()) {
        if (tool == d->toolBacker) {
            continue;
        }

        Plasma::IconWidget *icon = qgraphicsitem_cast<Plasma::IconWidget *>(tool);
        if (tool->isEnabled()) {
            tool->show();
            //kDebug() << tool << "is enabled";
            y += 5;
            QSize toolSize = tool->boundingRect().size().toSize();
            totalWidth += toolSize.width() + 5;

            maxWidth = qMax(toolSize.width(), maxWidth);
            maxHeight = qMax(toolSize.height(), maxHeight);
            y += static_cast<int>(tool->boundingRect().height());
        }

        if (icon) {
            if (viewTransform().m11() != Plasma::scalingFactor(Plasma::OverviewZoom) &&
                (viewTransform().m11() == Plasma::scalingFactor(Plasma::DesktopZoom) ||
                 icon->action() == d->containment->action("add sibling containment") ||
                 icon->action() == d->containment->action("add widgets"))) {
                icon->setText(icon->action()->text());
            } else {
                icon->setText(QString());
            }
        }
    }

    if (corner() == TopRight || corner() == Right || corner() == BottomRight) {
        x -= maxWidth;
    }

    //y += 5;


    if (!d->toolBacker) {
        d->toolBacker = new EmptyGraphicsItem(this);
        d->toolBacker->setZValue(zValue() + 1);
    }

    qreal left, top, right, bottom;
    d->toolBacker->getContentsMargins(left, top, right, bottom);

    // the rect the tools back should have
    QRectF backerRect = QRectF(QPointF(x, startY), QSizeF(maxWidth + left+right, y - startY + top + bottom));


    d->toolBacker->setIsToolbar(isToolbar());


    if (isToolbar()) {
        QPointF topRight;

        //could that cast ever fail?
        if (d->containment) {
            topRight = viewTransform().map(mapFromParent(d->containment->boundingRect().bottomRight()));
        } else {
            topRight = boundingRect().topRight();
        }


        backerRect.setSize(QSize(totalWidth+left+right, maxHeight+top+bottom));
        backerRect.moveTopRight(topRight);
    } else {
        //kDebug() << "starting at" <<  x << startY;

        // now check that is actually fits within the parent's boundaries
        backerRect = mapToParent(backerRect).boundingRect();
        QSizeF parentSize = parentWidget()->size();
        if (backerRect.x() < 5) {
            backerRect.moveLeft(5);
        } else if (backerRect.right() > parentSize.width() - 5) {
            backerRect.moveRight(parentSize.width() - 5);
        }

        if (backerRect.y() < 5) {
            backerRect.moveTop(5);
        } else if (backerRect.bottom() > parentSize.height() - 5) {
            backerRect.moveBottom(parentSize.height() - 5);
        }

        // re-map our starting points back to our coordinate system
        backerRect = mapFromParent(backerRect).boundingRect();
    }
    x = backerRect.x() + left;
    y = backerRect.y() + top;

    // now move the items
    Plasma::Animator *animdriver = Plasma::Animator::self();
    foreach (QGraphicsItem *tool, QGraphicsItem::children()) {
        if (tool == d->toolBacker) {
            continue;
        }

        Plasma::IconWidget *icon = qgraphicsitem_cast<Plasma::IconWidget *>(tool);
        const QSize iconSizeHint = icon->sizeFromIconSize(KIconLoader::SizeSmallMedium).toSize();

        //force max size if we aren't zooming
        if (viewTransform().m11() == 1) {
            icon->resize(maxWidth, iconSizeHint.height());
        } else {
            icon->resize(iconSizeHint);
        }

        if (tool->isEnabled()) {
            if (isToolbar()) {
                //kDebug() << tool << "is enabled";
                x += 5;
                //kDebug() << "let's show and move" << tool << tool->boundingRect();
                tool->show();
                tool->setPos(QPoint(x, y));
                x += static_cast<int>(tool->boundingRect().width());
            } else {
                //kDebug() << tool << "is enabled";
                y += 5;
                //kDebug() << "let's show and move" << tool << tool->boundingRect();
                tool->show();
                animdriver->moveItem(tool, Plasma::Animator::SlideInMovement, QPoint(x, y));
                //x += 0;
                y += static_cast<int>(tool->boundingRect().height());
            }
        } else if (tool->isVisible()) {
            // disabled, but visible, so hide it!
            const int height = static_cast<int>(tool->boundingRect().height());
            if (isToolbar()) {
                tool->hide();
            } else {
                animdriver->moveItem(tool, Plasma::Animator::SlideOutMovement, toolPosition(height));
            }
        }
    }

    d->toolBacker->setRect(backerRect);
    d->toolBacker->show();

    if (d->animCircleId) {
        animdriver->stopCustomAnimation(d->animCircleId);
    }

    setShowing(true);
}

void DesktopToolBox::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    //kDebug() << event->pos() << event->scenePos()
    //         << d->toolBacker->rect().contains(event->scenePos().toPoint());
    if (!d->hovering || showing() || isToolbar()) {
        QGraphicsItem::hoverLeaveEvent(event);
        return;
    }

    Plasma::Animator *animdriver = Plasma::Animator::self();
    if (d->animHighlightId) {
        animdriver->stopCustomAnimation(d->animHighlightId);
    }
    d->hovering = false;
    d->animHighlightId =
        animdriver->customAnimation(
            10, 240, Plasma::Animator::EaseOutCurve, this, "animateHighlight");

    QGraphicsItem::hoverLeaveEvent(event);
}

void DesktopToolBox::hideToolBox()
{
    if (!showing()) {
        return;
    }

    Plasma::Animator *animdriver = Plasma::Animator::self();
    foreach (QGraphicsItem *tool, QGraphicsItem::children()) {
        if (tool == d->toolBacker) {
            continue;
        }

        const int height = static_cast<int>(tool->boundingRect().height());
        if (isToolbar()) {
            tool->setPos(toolPosition(height));
            tool->hide();
        } else {
            animdriver->moveItem(tool, Plasma::Animator::SlideOutMovement, toolPosition(height));
        }
    }

    if (d->animCircleId) {
        animdriver->stopCustomAnimation(d->animCircleId);
    }

    setShowing(false);

    if (d->toolBacker) {
        d->toolBacker->hide();
    }
}

void DesktopToolBox::animateHighlight(qreal progress)
{
    if (d->hovering) {
        d->animHighlightFrame = progress;
    } else {
        d->animHighlightFrame = 1.0 - progress;
    }

    if (progress >= 1) {
        d->animHighlightId = 0;
    }

    update();
}

void DesktopToolBox::toolMoved(QGraphicsItem *item)
{
    //kDebug() << "geometry is now " << static_cast<Plasma::Widget*>(item)->geometry();
    if (!showing() &&
        QGraphicsItem::children().indexOf(static_cast<Plasma::Applet*>(item)) != -1) {
        item->hide();
    }
}

void DesktopToolBox::toggle()
{
    if (isToolbar()) {
        return;
    }

    if (showing()) {
        hideToolBox();
    } else {
        showToolBox();
    }
}

} // plasma namespace

#include "desktoptoolbox_p.moc"
