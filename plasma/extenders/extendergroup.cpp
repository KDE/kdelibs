/*
 * Copyright 2009 by Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#include "extendergroup.h"

#include <QAction>
#include <QApplication>
#include <QString>
#include <QList>
#include <QTimer>
#include <QtGui/QGraphicsWidget>
#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QGraphicsSceneDragDropEvent>

#include "applet.h"
#include "extender.h"
#include "extenderitem.h"
#include "theme.h"
#include "widgets/scrollwidget.h"

#include "private/extender_p.h"
#include "private/extendergroup_p.h"
#include "private/extenderitemmimedata_p.h"

namespace Plasma
{

ExtenderGroup::ExtenderGroup(Extender *parent, uint groupId)
             : ExtenderItem(parent, groupId),
               d(new ExtenderGroupPrivate(this))
{
    connect(extender(), SIGNAL(itemAttached(Plasma::ExtenderItem*)),
            this, SLOT(addItemToGroup(Plasma::ExtenderItem*)));
    connect(extender(), SIGNAL(itemDetached(Plasma::ExtenderItem*)),
            this, SLOT(removeItemFromGroup(Plasma::ExtenderItem*)));

    //this isn't actually connected to anything, we will just check if it's running or not
    d->resizeTimer = new QTimer(this);
    d->resizeTimer->setSingleShot(true);

    config().writeEntry("isGroup", true);

    setAcceptDrops(true);

    QGraphicsLinearLayout *lay = static_cast<QGraphicsLinearLayout *>(layout());
    d->childsWidget = new QGraphicsWidget(this);
    d->childsWidget->installEventFilter(this);
    d->layout = new QGraphicsLinearLayout(Qt::Vertical, d->childsWidget);
    d->childsWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    lay->addItem(d->childsWidget);

    QAction *expand = new QAction(this);
    expand->setVisible(false);
    expand->setToolTip(i18n("Show this group."));
    connect(expand, SIGNAL(triggered()), this, SLOT(expandGroup()));
    addAction("expand", expand);

    QAction *collapse = new QAction(this);
    collapse->setVisible(true);
    collapse->setToolTip(i18n("Hide this group."));
    connect(collapse, SIGNAL(triggered()), this, SLOT(collapseGroup()));
    addAction("collapse", collapse);

    d->themeChanged();

    QString groupName;
    foreach (ExtenderItem *item, extender()->attachedItems()) {
        groupName = item->config().readEntry("group", "");
        if (!groupName.isEmpty() && groupName == name()) {
            item->setGroup(this);
        }
    }

    if (items().isEmpty() && d->autoHide && !isDetached()) {
        extender()->itemRemovedEvent(this);
        hide();
    }

    if (!config().readEntry("groupCollapsed", true)) {
        expandGroup();
    }

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
            this, SLOT(themeChanged()));

}

ExtenderGroup::~ExtenderGroup()
{
    delete d;
}

QList<ExtenderItem*> ExtenderGroup::items() const
{
    QList<ExtenderItem*> result;
    foreach (ExtenderItem *item, extender()->attachedItems()) {
        if (item->group() == this) {
            result.append(item);
        }
    }

    return result;
}

bool ExtenderGroup::autoHide() const
{
    return d->autoHide;
}

void ExtenderGroup::setAutoHide(bool autoHide)
{
    d->autoHide = autoHide;
    if (autoHide && items().isEmpty()) {
        extender()->itemRemovedEvent(this);
        hide();
    } else if (!autoHide && !isVisible()) {
        extender()->itemAddedEvent(this);
        show();
    }
}

bool ExtenderGroup::isAutoCollapse() const
{
    return d->autoCollapse;
}

void ExtenderGroup::setAutoCollapse(bool collapse)
{
    d->autoCollapse = collapse;
}

bool ExtenderGroup::isGroupCollapsed() const
{
    return d->collapsed;
}

void ExtenderGroup::setGroupCollapsed(bool collapsed)
{
    if (collapsed) {
        collapseGroup();
    } else {
        expandGroup();
    }
}

void ExtenderGroup::expandGroup()
{
    if (d->autoCollapse) {
        setCollapsed(false);
    }
    if (d->collapsed == false) {
        return;
    }

    d->collapsed = false;
    config().writeEntry("groupCollapsed", d->collapsed);
    action("collapse")->setVisible(true);
    action("expand")->setVisible(false);

    d->childsWidget->show();
    static_cast<QGraphicsLinearLayout *>(layout())->addItem(d->childsWidget);
    updateGeometry();

    foreach (ExtenderItem *item, extender()->attachedItems()) {
        if (item->group() == this) {
            item->show();
            extender()->itemAddedEvent(item);
        }
    }

    //extender()->resize(extender()->effectiveSizeHint(Qt::PreferredSize));
}

void ExtenderGroup::collapseGroup()
{
    if (d->autoCollapse) {
        setCollapsed(true);
    }
    if (d->collapsed == true) {
        return;
    }

    d->collapsed = true;
    config().writeEntry("groupCollapsed", d->collapsed);
    action("collapse")->setVisible(false);
    action("expand")->setVisible(true);

    d->childsWidget->hide();
    static_cast<QGraphicsLinearLayout *>(layout())->removeItem(d->childsWidget);
    updateGeometry();

    foreach (ExtenderItem *item, extender()->attachedItems()) {
        if (item != this && item->group() == this) {
            item->hide();
            extender()->itemRemovedEvent(item);
        }
    }
}

void ExtenderGroup::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    ExtenderItem::resizeEvent(event);
}

bool ExtenderGroup::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == d->childsWidget && event->type() == QEvent::GraphicsSceneResize) {
        static_cast<QGraphicsLayoutItem *>(extender()->d->scrollWidget)->updateGeometry();
        if (!d->resizeTimer->isActive()) {
            static_cast<QGraphicsLayoutItem *>(extender())->updateGeometry();
            extender()->d->adjustSize();
            d->resizeTimer->start(0);
        }
    }

    return ExtenderItem::eventFilter(watched, event);
}

void ExtenderGroup::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    if (event->mimeData()->hasFormat(ExtenderItemMimeData::mimeType())) {
        event->accept();

        dragMoveEvent(event);
    }
}

void ExtenderGroup::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    if (event->mimeData()->hasFormat(ExtenderItemMimeData::mimeType())) {
        const ExtenderItemMimeData *mimeData =
            qobject_cast<const ExtenderItemMimeData*>(event->mimeData());

        if (mimeData) {
            QPointF pos(event->pos());
            if (d->spacerWidget && d->spacerWidget->geometry().contains(pos)) {
                return;
            }

            //Make sure we remove any spacer that might already be in the layout.
            if (d->spacerWidget) {
                d->layout->removeItem(d->spacerWidget);
            }

            int insertIndex = d->insertIndexFromPos(pos);

            //Create a widget that functions as spacer, and add that to the layout.
            if (!d->spacerWidget) {
                Spacer *widget = new Spacer(this);
                ExtenderItem *item = mimeData->extenderItem();
                qreal left, top, right, bottom;
                extender()->d->background->getMargins(left, top, right, bottom);
                widget->setMargins(left, 4, right, 4);

                widget->setMinimumSize(item->minimumSize());
                widget->setPreferredSize(item->preferredSize());
                widget->setMaximumSize(item->maximumSize());
                widget->setSizePolicy(item->sizePolicy());
                d->spacerWidget = widget;
            }
            d->layout->insertItem(insertIndex, d->spacerWidget);

            extender()->d->setPositionFromDragPosition(event->scenePos());
        }
    }
}

void ExtenderGroup::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    if (event->mimeData()->hasFormat(ExtenderItemMimeData::mimeType())) {
        const ExtenderItemMimeData *mimeData =
            qobject_cast<const ExtenderItemMimeData*>(event->mimeData());

        if (mimeData) {
            mimeData->extenderItem()->setGroup(this, event->pos());
            QApplication::restoreOverrideCursor();
            d->layout->removeItem(d->spacerWidget);
            d->spacerWidget->deleteLater();
            d->spacerWidget = 0;
        }
    }
}

void ExtenderGroup::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    if (event->mimeData()->hasFormat(ExtenderItemMimeData::mimeType())) {
        if (d->spacerWidget) {
            d->layout->removeItem(d->spacerWidget);
            d->spacerWidget->deleteLater();
            d->spacerWidget = 0;
        }
    }
}

ExtenderGroupPrivate::ExtenderGroupPrivate(ExtenderGroup *group)
    : q(group),
      spacerWidget(0),
      svg(new Svg(group)),
      collapsed(false),
      autoHide(true),
      autoCollapse(false)
{
}

ExtenderGroupPrivate::~ExtenderGroupPrivate()
{
}

void ExtenderGroupPrivate::addItemToGroup(Plasma::ExtenderItem *item, const QPointF &pos)
{
    if (item->group() == q) {
        item->setParentItem(childsWidget);
        item->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        layout->insertItem(insertIndexFromPos(pos), item);
        layout->activate();

        if (!q->isVisible() && !q->items().isEmpty()) {
            q->extender()->itemAddedEvent(q);
            q->show();
        }
        childsWidget->setVisible(!q->isGroupCollapsed());

        if (!q->isGroupCollapsed()) {
            layout->updateGeometry();
            static_cast<QGraphicsLayoutItem *>(q)->updateGeometry();
            static_cast<QGraphicsLayoutItem *>(childsWidget)->updateGeometry();
            static_cast<QGraphicsLayoutItem *>(q->extender()->d->scrollWidget)->updateGeometry();
        }

        q->extender()->d->adjustSize();
    }
}

void ExtenderGroupPrivate::removeItemFromGroup(Plasma::ExtenderItem *item)
{
    if (item->group() == q) {
        if (q->items().isEmpty() && autoHide && !q->isDetached()) {
            q->extender()->itemRemovedEvent(q);
            q->hide();
        }
        layout->removeItem(item);
        item->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        layout->updateGeometry();
        static_cast<QGraphicsLayoutItem *>(q)->updateGeometry();
        static_cast<QGraphicsLayoutItem *>(childsWidget)->updateGeometry();
        static_cast<QGraphicsLayoutItem *>(q->extender()->d->scrollWidget)->updateGeometry();

        q->extender()->d->adjustSize();
    }
}

void ExtenderGroupPrivate::themeChanged()
{
    svg->setImagePath("widgets/configuration-icons");
    svg->resize();

    q->action("expand")->setIcon(QIcon(svg->pixmap("restore")));
    q->action("collapse")->setIcon(QIcon(svg->pixmap("collapse")));
}

int ExtenderGroupPrivate::insertIndexFromPos(const QPointF &pos) const
{
    int insertIndex = -1;

    //XXX: duplicated from panel
    if (pos != QPointF(-1, -1)) {
        for (int i = 0; i < layout->count(); ++i) {
            QRectF siblingGeometry = layout->itemAt(i)->geometry();
            qreal middle = (siblingGeometry.top() + siblingGeometry.bottom()) / 2.0;
            if (pos.y() < middle) {
                insertIndex = i;
                break;
            } else if (pos.y() <= siblingGeometry.bottom()) {
                insertIndex = i + 1;
                break;
            }
        }
    }

    return insertIndex;
}

} // Plasma namespace

#include "extendergroup.moc"
