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

#include "toolbox_p.h"

#include <QAction>
#include <QApplication>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsView>
#include <QPainter>
#include <QRadialGradient>

#include <kcolorscheme.h>
#include <kconfiggroup.h>
#include <kiconloader.h>
#include <kdebug.h>

#include "corona.h"
#include "theme.h"
#include "widgets/iconwidget.h"

namespace Plasma
{

class ToolBoxPrivate
{
public:
    ToolBoxPrivate(Containment *c)
      : containment(c),
        size(50),
        iconSize(KIconLoader::SizeMedium, KIconLoader::SizeMedium),
        corner(ToolBox::TopRight),
        hidden(false),
        showing(false),
        movable(false),
        toolbar(false),
        dragging(false),
        userMoved(false)
    {}

    Containment *containment;
    int size;
    QSize iconSize;
    ToolBox::Corner corner;
    QPoint dragStartRelative;
    QTransform viewTransform;
    bool hidden : 1;
    bool showing : 1;
    bool movable : 1;
    bool toolbar : 1;
    bool dragging : 1;
    bool userMoved : 1;
};

ToolBox::ToolBox(Containment *parent)
    : QGraphicsItem(parent),
      d(new ToolBoxPrivate(parent))
{
    d->userMoved = false;
    setAcceptsHoverEvents(true);
}

ToolBox::~ToolBox()
{
    delete d;
}

QPoint ToolBox::toolPosition(int toolHeight)
{
    switch (d->corner) {
    case TopRight:
        return QPoint(boundingRect().width(), -toolHeight);
    case Top:
        return QPoint((int)boundingRect().center().x() - boundingRect().width(), -toolHeight);
    case TopLeft:
        return QPoint(-boundingRect().width(), -toolHeight);
    case Left:
        return QPoint(-boundingRect().width(), (int)boundingRect().center().y() - boundingRect().height());
    case Right:
        return QPoint(boundingRect().width(), (int)boundingRect().center().y() - boundingRect().height());
    case BottomLeft:
        return QPoint(-boundingRect().width(), toolHeight);
    case Bottom:
        return QPoint((int)boundingRect().center().x() - d->iconSize.width(), toolHeight);
    case BottomRight:
    default:
        return QPoint(boundingRect().width(), toolHeight);
    }
}

void ToolBox::addTool(QAction *action)
{
    if (!action) {
        return;
    }

    foreach (QGraphicsItem *child, QGraphicsItem::children()) {
        //kDebug() << "checking tool" << child << child->data(ToolName);
        Plasma::IconWidget *tool = dynamic_cast<Plasma::IconWidget*>(child);
        if (tool && tool->action() == action) {
            return;
        }
    }

    Plasma::IconWidget *tool = new Plasma::IconWidget(this);

    tool->setTextBackgroundColor(QColor());
    tool->setAction(action);
    tool->setDrawBackground(true);
    tool->setOrientation(Qt::Horizontal);
    tool->resize(tool->sizeFromIconSize(KIconLoader::SizeSmallMedium));

    tool->hide();
    const int height = static_cast<int>(tool->boundingRect().height());
    tool->setPos(toolPosition(height));
    tool->setZValue(zValue() + 10);
    tool->setToolTip(action->text());

    //make enabled/disabled tools appear/disappear instantly
    connect(tool, SIGNAL(changed()), this, SLOT(updateToolBox()));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(toolTriggered(bool)));
    //kDebug() << "added tool" << action->text() << (QGraphicsItem*)tool;
}

void ToolBox::updateToolBox()
{
    Plasma::IconWidget *tool = qobject_cast<Plasma::IconWidget *>(sender());
    if (tool && tool->action() == 0) {
        tool->deleteLater();
        tool = 0;
    }

    if (d->showing) {
        d->showing = false;
        showToolBox();
    } else if (tool && !tool->isEnabled()) {
        tool->hide();
    }
}

void ToolBox::toolTriggered(bool)
{
}

void ToolBox::removeTool(QAction *action)
{
    foreach (QGraphicsItem *child, QGraphicsItem::children()) {
        //kDebug() << "checking tool" << child << child->data(ToolName);
        Plasma::IconWidget *tool = dynamic_cast<Plasma::IconWidget*>(child);
        if (tool && tool->action() == action) {
            //kDebug() << "tool found!";
            tool->deleteLater();
            break;
        }
    }
}

int ToolBox::size() const
{
    return  d->size;
}

void ToolBox::setSize(const int newSize)
{
    d->size = newSize;
}

QSize ToolBox::iconSize() const
{
    return d->iconSize;
}

void ToolBox::setIconSize(const QSize newSize)
{
    d->iconSize = newSize;
}

bool ToolBox::showing() const
{
    return d->showing;
}

void ToolBox::setShowing(const bool show)
{
    d->showing = show;
}

void ToolBox::setCorner(const Corner corner)
{
    d->corner = corner;
}

ToolBox::Corner ToolBox::corner() const
{
    return d->corner;
}

void ToolBox::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
    // set grab position relative to toolbox
    d->dragStartRelative = mapToParent(event->pos()).toPoint() - pos().toPoint();
}

QSize ToolBox::cornerSize() const
{
    return boundingRect().size().toSize();
}

QSize ToolBox::fullWidth() const
{
    return boundingRect().size().toSize();
}

QSize  ToolBox::fullHeight() const
{
    return boundingRect().size().toSize();
}

void ToolBox::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!d->movable || (!d->dragging && boundingRect().contains(event->pos())) || isToolbar()) {
        return;
    }

    d->dragging = true;
    d->userMoved = true;
    const QPoint newPos = mapToParent(event->pos()).toPoint();
    const QPoint curPos = pos().toPoint();

    const QSize cSize = cornerSize();
    const QSize fHeight = fullHeight();
    const QSize fWidth = fullWidth();
    const int h = fHeight.height();
    const int w = fWidth.width();

    const int areaWidth = parentWidget()->size().width();
    const int areaHeight = parentWidget()->size().height();

    int x = curPos.x();
    int y = curPos.y();

    // jump to the nearest desktop border
    int distanceToLeft = newPos.x() - d->dragStartRelative.x();
    int distanceToRight = areaWidth - w - distanceToLeft;
    int distanceToTop = newPos.y() - d->dragStartRelative.y();
    int distanceToBottom = areaHeight - h - distanceToTop;

    int distancetoHorizontalMiddle = qAbs(newPos.x() - areaWidth/2);
    int distancetoVerticalMiddle = qAbs(newPos.y() - areaHeight/2);

    // decide which border is the nearest
    if (distanceToLeft < distanceToTop && distanceToLeft < distanceToRight &&
        distanceToLeft < distanceToBottom ) {
        x = 0;
        y = (newPos.y() - d->dragStartRelative.y());
    } else if (distanceToRight < distanceToTop && distanceToRight < distanceToLeft &&
               distanceToRight < distanceToBottom) {
        x = areaWidth - w;
        y = (newPos.y() - d->dragStartRelative.y());
    } else if (distanceToTop < distanceToLeft && distanceToTop < distanceToRight &&
               distanceToTop < distanceToBottom ) {
        y = 0;
        x = (newPos.x() - d->dragStartRelative.x());
    } else if (distanceToBottom < distanceToLeft && distanceToBottom < distanceToRight &&
               distanceToBottom < distanceToTop) {
        y = areaHeight - h;
        x = (newPos.x() - d->dragStartRelative.x());
    }

    if (distancetoHorizontalMiddle < 10) {
        x = areaWidth/2 - d->dragStartRelative.x();
    } else if (distancetoVerticalMiddle < 10) {
        y = areaHeight/2 - d->dragStartRelative.y();
    }

    x = qBound(0, x, areaWidth - w);
    y = qBound(0, y, areaHeight - h);

    Corner newCorner = d->corner;
    if (x == 0) {
        if (y == 0) {
            newCorner = TopLeft;
        } else if (areaHeight - cSize.height() < newPos.y()) {
            y = areaHeight - cSize.height();
            newCorner = BottomLeft;
        } else {
            newCorner = Left;
        }
    } else if (y == 0) {
        if (areaWidth - cSize.width() < newPos.x()) {
            x = areaWidth - cSize.width();
            newCorner = TopRight;
        } else {
            newCorner = Top;
        }
    } else if (x + w >= areaWidth) {
        if (areaHeight - cSize.height() < newPos.y()) {
            y = areaHeight - cSize.height();
            x = areaWidth - cSize.width();
            newCorner = BottomRight;
        } else {
            x = areaWidth - fHeight.width();
            newCorner = Right;
        }
    } else {
        y = areaHeight - fWidth.height();
        newCorner = Bottom;
    }

    if (newCorner != d->corner) {
        prepareGeometryChange();
        d->corner = newCorner;
    }

    setPos(x, y);
}

void ToolBox::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (!d->dragging && boundingRect().contains(event->pos())) {
        emit toggled();
    }

    d->dragging = false;
    KConfigGroup cg(d->containment->config());
    save(cg);
}

bool ToolBox::isMovable() const
{
    return d->movable;
}

void ToolBox::setIsMovable(bool movable)
{
    d->movable = movable;
}

bool ToolBox::isToolbar() const
{
    return d->toolbar;
}

void ToolBox::setIsToolbar(bool toolbar)
{
    d->toolbar = toolbar;
}

QTransform ToolBox::viewTransform() const
{
    return d->viewTransform;
}

void ToolBox::setViewTransform(const QTransform &transform)
{
    if (transform.isScaling()) {
        d->toolbar = true;
        showToolBox();
    } else {
        d->toolbar = false;
        if (d->viewTransform != transform) {
            hideToolBox();
        }
    }
    d->viewTransform = transform;
}

void ToolBox::save(KConfigGroup &cg) const
{
    if (!d->movable) {
        return;
    }

    KConfigGroup group(&cg, "ToolBox");
    if (!d->userMoved) {
        group.deleteGroup();
        return;
    }

    int offset = 0;
    if (d->corner == ToolBox::Left ||
        d->corner == ToolBox::Right) {
        offset = y();
    } else if (d->corner == ToolBox::Top ||
               d->corner == ToolBox::Bottom) {
        offset = x();
    }

    group.writeEntry("corner", int(d->corner));
    group.writeEntry("offset", offset);
}

void ToolBox::load(const KConfigGroup &containmentGroup)
{
    if (!d->movable) {
        return;
    }

    KConfigGroup group;
    if (containmentGroup.isValid()) {
        group = containmentGroup;
    } else {
        group = d->containment->config();
    }

    group = KConfigGroup(&group, "ToolBox");

    if (!group.hasKey("corner")) {
        return;
    }

    d->userMoved = true;
    d->corner = Corner(group.readEntry("corner", int(d->corner)));

    int offset = group.readEntry("offset", 0);
    switch (d->corner) {
        case ToolBox::TopLeft:
            setPos(0, 0);
            break;
        case ToolBox::Top:
            setPos(offset, 0);
            break;
        case ToolBox::TopRight:
            setPos(d->containment->size().width() - boundingRect().width(), 0);
            break;
        case ToolBox::Right:
            setPos(d->containment->size().width() - boundingRect().width(), offset);
            break;
        case ToolBox::BottomRight:
            setPos(d->containment->size().width() - boundingRect().width(), d->containment->size().height() - boundingRect().height());
            break;
        case ToolBox::Bottom:
            setPos(offset, d->containment->size().height() - boundingRect().height());
            break;
        case ToolBox::BottomLeft:
            setPos(0, d->containment->size().height() - boundingRect().height());
            break;
        case ToolBox::Left:
            setPos(0, offset);
            break;
    }
    //kDebug() << "marked as user moved" << pos()
    //         << (d->containment->containmentType() == Containment::PanelContainment);
}

void ToolBox::reposition()
{
    if (d->userMoved) {
        //FIXME: adjust for situations like changing of the available space
        load();
        return;
    }

    if (d->containment->containmentType() == Containment::PanelContainment ||
        d->containment->containmentType() == Containment::CustomPanelContainment) {
        QRectF rect = boundingRect();
        if (d->containment->formFactor() == Vertical) {
            setCorner(ToolBox::Bottom);
            setPos(d->containment->geometry().width() / 2 - rect.width() / 2,
                   d->containment->geometry().height() - rect.height());
        } else {
            //defaulting to Horizontal right now
            if (QApplication::layoutDirection() == Qt::RightToLeft) {
                setPos(d->containment->geometry().left(),
                       d->containment->geometry().height() / 2 - rect.height() / 2);
                setCorner(ToolBox::Left);
            } else {
                setPos(d->containment->geometry().width() - rect.width(),
                       d->containment->geometry().height() / 2 - rect.height() / 2);
                setCorner(ToolBox::Right);
            }
        }

        //kDebug() << "got ourselves a panel containment, moving to" << pos();
    } else if (d->containment->corona()) {
        //kDebug() << "desktop";

        int screen = d->containment->screen();
        QRectF avail = d->containment->geometry();
        QRectF screenGeom = avail;

        if (screen > -1 && screen < d->containment->corona()->numScreens()) {
            avail = d->containment->corona()->availableScreenRegion(screen).boundingRect();
            screenGeom = d->containment->corona()->screenGeometry(screen);
            avail.translate(-screenGeom.topLeft());
        }

        // Transform to the containment's coordinate system.
        screenGeom.moveTo(0, 0);

        if (!d->containment->view() || !d->containment->view()->transform().isScaling()) {
            if (QApplication::layoutDirection() == Qt::RightToLeft) {
                if (avail.top() > screenGeom.top()) {
                    setPos(avail.topLeft() - QPoint(0, avail.top()));
                    setCorner(ToolBox::Left);
                } else if (avail.left() > screenGeom.left()) {
                    setPos(avail.topLeft() - QPoint(boundingRect().width(), 0));
                    setCorner(ToolBox::Top);
                } else {
                    setPos(avail.topLeft());
                    setCorner(ToolBox::TopLeft);
                }
            } else {
                if (avail.top() > screenGeom.top()) {
                    setPos(avail.topRight() - QPoint(boundingRect().width(), -avail.top()));
                    setCorner(ToolBox::Right);
                } else if (avail.right() < screenGeom.right()) {
                    setPos(avail.topRight() - QPoint(boundingRect().width(), 0));
                    setCorner(ToolBox::Top);
                } else {
                    setPos(avail.topRight() - QPoint(boundingRect().width(), 0));
                    setCorner(ToolBox::TopRight);
                }
            }
        } else {
            if (QApplication::layoutDirection() == Qt::RightToLeft) {
                setPos(d->containment->mapFromScene(QPointF(d->containment->geometry().topLeft())));
                setCorner(ToolBox::TopLeft);
            } else {
                setPos(d->containment->mapFromScene(QPointF(d->containment->geometry().topRight())));
                setCorner(ToolBox::TopRight);
            }
        }
    }
}

} // plasma namespace

#include "toolbox_p.moc"

