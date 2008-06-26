/**
  * This file is part of the KDE project
  * Copyright (C) 2007-2008 Rafael Fernández López <ereslibre@kde.org>
  * Copyright (C) 2008 Kevin Ottens <ervin@kde.org>
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

#include "kwidgetitemdelegate.h"
#include "kwidgetitemdelegate_p.h"

#include <QIcon>
#include <QSize>
#include <QStyle>
#include <QEvent>
#include <QHoverEvent>
#include <QFocusEvent>
#include <QCursor>
#include <QBitmap>
#include <QLayout>
#include <QPainter>
#include <QScrollBar>
#include <QKeyEvent>
#include <QStyleOption>
#include <QPaintEngine>
#include <QCoreApplication>
#include <QAbstractItemView>

#include "kwidgetitemdelegatepool_p.h"

Q_DECLARE_METATYPE(QList<QEvent::Type>)

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
KWidgetItemDelegatePrivate::KWidgetItemDelegatePrivate(KWidgetItemDelegate *q, QObject *parent)
    : QObject(parent)
    , itemView(0)
    , hoveredIndex(QPersistentModelIndex())
    , lastHoveredIndex(QPersistentModelIndex())
    , hoveredWidget(0)
    , focusedIndex(QPersistentModelIndex())
    , focusedWidget(0)
    , buttonPressedWidget(0)
    , currentIndex(QPersistentModelIndex())
    , selectionModel(0)
    , widgetPool(new KWidgetItemDelegatePool(q))
    , q(q)
{
}

KWidgetItemDelegatePrivate::~KWidgetItemDelegatePrivate()
{
    delete widgetPool;
}

// When receiving move events on the viewport we need to check if we should
// post events to certain widgets like "enter", "leave"...
// Note: mouseEvent can be 0
void KWidgetItemDelegatePrivate::analyzeInternalMouseEvents(const QStyleOptionViewItem &option,
                                                            QMouseEvent *mouseEvent)
{
    QPoint pos;
    if (mouseEvent) {
        pos = mouseEvent->globalPos();
    } else {
        pos = QCursor::pos();
    }

    QRect mappedRect;

    if (hoveredWidget) {
        mappedRect = QRect(itemView->viewport()->mapToGlobal(widgetRect(hoveredWidget, option, hoveredIndex).topLeft()),
                           itemView->viewport()->mapToGlobal(widgetRect(hoveredWidget, option, hoveredIndex).bottomRight()));
    }

    QRect globalViewPortRect(itemView->viewport()->mapToGlobal(itemView->viewport()->rect().topLeft()),
                             itemView->viewport()->mapToGlobal(itemView->viewport()->rect().bottomRight()));
    bool itemViewContainsMousePos = globalViewPortRect.contains(pos);

    if (hoveredWidget && (!mappedRect.contains(pos) || !itemViewContainsMousePos)) {
        QPersistentModelIndex indexList[] = { lastHoveredIndex, hoveredIndex };
        for (int i = 0; i < 2; i++) {
            if (!indexList[i].isValid()) {
                continue;
            }

            QEvent leaveEvent(QEvent::Leave);
            QCoreApplication::sendEvent(hoveredWidget, &leaveEvent);
            if (mouseEvent) {
                QHoverEvent hoverLeaveEvent(QEvent::HoverLeave,
                                            mappedPointForWidget(hoveredWidget, indexList[i], mouseEvent->pos()),
                                            mappedPointForWidget(hoveredWidget, indexList[i], mouseEvent->pos()));
                QCoreApplication::sendEvent(hoveredWidget, &hoverLeaveEvent);
            }
        }

        hoveredWidget = 0;

        return;
    }

    if (!itemViewContainsMousePos) {
        return;
    }

    QList<QWidget*> widgetList = widgetPool->findWidgets(hoveredIndex, option);

    foreach (QWidget *widget, widgetList) {
        if (!widget->isVisibleTo(widget->parentWidget())) continue;

        QWidget *childWidget = widget;
        if (mouseEvent) {
            QPoint eventPos = mappedPointForWidget(widget, hoveredIndex, mouseEvent->pos());
            childWidget = widget->childAt(eventPos);

            if (!childWidget) {
                childWidget = widget;
            }
        }

        if (widget != hoveredWidget || hoveredWidget != childWidget) {
            QRect mappedRect = QRect(itemView->viewport()->mapToGlobal(widgetRect(widget, option, hoveredIndex).topLeft()),
                                     itemView->viewport()->mapToGlobal(widgetRect(widget, option, hoveredIndex).bottomRight()));

            if (mappedRect.contains(pos)) {
                hoveredWidget = childWidget;

                QEvent enterEvent(QEvent::Enter);
                QCoreApplication::sendEvent(hoveredWidget, &enterEvent);
                if (mouseEvent) {
                    QHoverEvent hoverEnterEvent(QEvent::HoverEnter,
                                                mappedPointForWidget(hoveredWidget, hoveredIndex, mouseEvent->pos()),
                                                mappedPointForWidget(hoveredWidget, hoveredIndex, mouseEvent->pos()));
                    QCoreApplication::sendEvent(hoveredWidget, &hoverEnterEvent);
                }
                break;
            }
        } else if (mouseEvent) { // we are moving the mouse over a previously hovered widget, generate MouseMove events
            QPoint eventPos = mappedPointForWidget(childWidget, hoveredIndex, mouseEvent->pos());
            QMouseEvent genMouseEvent(QEvent::MouseMove, eventPos, pos, mouseEvent->button(),
                                      mouseEvent->buttons(), mouseEvent->modifiers());
            QCoreApplication::sendEvent(hoveredWidget, &genMouseEvent);
            QHoverEvent hoverMoveEvent(QEvent::HoverMove,
                                       mappedPointForWidget(hoveredWidget, hoveredIndex, mouseEvent->pos()),
                                       mappedPointForWidget(hoveredWidget, hoveredIndex, mouseEvent->pos()));
            QCoreApplication::sendEvent(hoveredWidget, &hoverMoveEvent);
            break;
        }
    }
}

QPoint KWidgetItemDelegatePrivate::mappedPointForWidget(QWidget *widget,
                                                        const QPersistentModelIndex &index,
                                                        const QPoint &pos) const
{
    // Map the event point relative to the widget
    QStyleOptionViewItem option;
    option.rect = itemView->visualRect(index);
    QPoint widgetPos = widget->pos();
    widgetPos.setX(widgetPos.x() + option.rect.left());
    widgetPos.setY(widgetPos.y() + option.rect.top());

    return pos - widgetPos;
}

void KWidgetItemDelegatePrivate::slotCurrentChanged(const QModelIndex &currentIndex,
                                                    const QModelIndex &previousIndex)
{
    Q_UNUSED(previousIndex);

    this->currentIndex = currentIndex;

    itemView->viewport()->update();
}

void KWidgetItemDelegatePrivate::slotSelectionModelDestroyed()
{
    selectionModel = 0;
}
//@endcond

KWidgetItemDelegate::KWidgetItemDelegate(QAbstractItemView *itemView, QObject *parent)
    : QAbstractItemDelegate(parent)
    , d(new KWidgetItemDelegatePrivate(this))
{
    Q_ASSERT(itemView);

    itemView->setMouseTracking(true);
    itemView->viewport()->setAttribute(Qt::WA_Hover);

    d->itemView = itemView;

    itemView->viewport()->installEventFilter(d); // mouse events
    itemView->installEventFilter(d);             // keyboard events

    if (itemView->selectionModel()) {
        d->selectionModel = itemView->selectionModel();

        connect(itemView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                d, SLOT(slotCurrentChanged(QModelIndex,QModelIndex)));
    }
}

KWidgetItemDelegate::~KWidgetItemDelegate()
{
    delete d;
}

QAbstractItemView *KWidgetItemDelegate::itemView() const
{
    return d->itemView;
}

QPersistentModelIndex KWidgetItemDelegate::focusedIndex() const
{
    return d->focusedIndex;
}

//@cond PRIVATE
QRect KWidgetItemDelegatePrivate::widgetRect(QWidget *widget,
                                             const QStyleOptionViewItem &option,
                                             const QPersistentModelIndex &index) const
{
    Q_UNUSED(index);
    QRect retRect = QRect(widget->pos(), widget->size());
    retRect.translate(option.rect.topLeft());
    return retRect;
}
//@endcond

void KWidgetItemDelegate::paintWidgets(QPainter *painter, const QStyleOptionViewItem &option,
                                       const QPersistentModelIndex &index) const
{
    if (!index.isValid()) {
        return;
    }

    Q_ASSERT(d->itemView);

    QList<QWidget*> widgetList = d->widgetPool->findWidgets(index, option);

    // Now that all widgets have been set up we can ask for their positions, sizes
    // and for them being rendered.
    foreach (QWidget *widget, widgetList) {
        if (!widget->isVisibleTo(widget->parentWidget())) continue;

        QPoint widgetPos = widget->pos();
        QSize widgetSize = widget->size();

        QPixmap pixmap(widgetSize.width(), widgetSize.height());
        pixmap.fill(QColor(Qt::transparent));

        widget->render(&pixmap, QPoint(0, 0),
                       QRect(QPoint(0, 0), widgetSize),
                       QWidget::DrawChildren);
        painter->drawPixmap(widgetPos + option.rect.topLeft(), pixmap);
    }
}

//@cond PRIVATE
bool KWidgetItemDelegatePrivate::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Destroy) {
        return false;
    }

    Q_UNUSED(watched);
    Q_ASSERT(itemView);

    if (selectionModel != itemView->selectionModel()) {
        if (selectionModel) {
            disconnect(selectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                       this, SLOT(slotCurrentChanged(QModelIndex,QModelIndex)));
        }

        selectionModel = itemView->selectionModel();

        if (selectionModel) {
            connect(selectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                    this, SLOT(slotCurrentChanged(QModelIndex,QModelIndex)));
            connect(selectionModel, SIGNAL(destroyed(QObject*)),
                    this, SLOT(slotSelectionModelDestroyed()));
        }
    }

    bool filterEvent = false;
    bool eventReply = false;

    switch (event->type()) {
        case QEvent::Paint:
        case QEvent::Timer:
        case QEvent::UpdateRequest:
        case QEvent::Destroy:
        case QEvent::MetaCall:
            return false;
        case QEvent::Leave:
        case QEvent::Enter:
        case QEvent::MouseMove: {
                QStyleOptionViewItem styleOptionViewItem;
                styleOptionViewItem.initFrom(itemView);
                styleOptionViewItem.rect = itemView->visualRect(hoveredIndex);

                QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent*>(event);
                lastHoveredIndex = hoveredIndex;
                hoveredIndex = itemView->indexAt(itemView->viewport()->mapFromGlobal(QCursor::pos()));

                // If a widget has been pressed and not released, mouse move events will be forwarded to the
                // initially pressed widget (until the button mouse is released).
                if (buttonPressedWidget) {
                    if (event->type() == QEvent::MouseMove) {
                        hoveredWidget = buttonPressedWidget;

                        QMouseEvent mouseEventCpy(mouseEvent->type(), mappedPointForWidget(hoveredWidget, buttonPressedIndex, mouseEvent->pos()),
                                                  mouseEvent->globalPos(), mouseEvent->button(), mouseEvent->buttons(), mouseEvent->modifiers());

                        QCoreApplication::sendEvent(hoveredWidget, &mouseEventCpy);
                    }
                } else {
                    // Consider moving this block into analyzeInternalMouseEvents
                    if (hoveredWidget && lastHoveredIndex.isValid() && (hoveredIndex != lastHoveredIndex)) {
                        QEvent leaveEvent(QEvent::Leave);
                        QCoreApplication::sendEvent(hoveredWidget, &leaveEvent);
                    }

                    if (event->type() == QEvent::Leave && !buttonPressedWidget) {
                        QStyleOptionViewItem option;
                        option.rect = itemView->visualRect(hoveredIndex);
                        analyzeInternalMouseEvents(option, 0);
                    } else {
                        QStyleOptionViewItem option;
                        option.rect = itemView->visualRect(hoveredIndex);
                        analyzeInternalMouseEvents(option, mouseEvent);
                    }
                }

                itemView->viewport()->update();

                if (hoveredWidget) {
                    itemView->setCursor(hoveredWidget->cursor());
                } else {
                    itemView->setCursor(Qt::ArrowCursor);
                }
            }
            break;
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease: {
                focusedIndex = hoveredIndex;

                QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

                if (event->type() == QEvent::MouseButtonPress) {
                    buttonPressedWidget = hoveredWidget;
                    buttonPressedIndex = hoveredIndex;
                } else {
                    buttonPressedWidget = 0;
                    buttonPressedIndex = QModelIndex();
                }

                if (focusedWidget && (focusedWidget != hoveredWidget)) {

                    QFocusEvent focusEvent(QEvent::FocusOut);
                    QCoreApplication::sendEvent(focusedWidget, &focusEvent);
                }

                if (hoveredWidget) {
                    QPoint eventPos = mappedPointForWidget(hoveredWidget,
                                                           hoveredIndex,
                                                           mouseEvent->pos());

                    QMouseEvent mouseEvt(mouseEvent->type(), eventPos, mouseEvent->button(),
                                         mouseEvent->buttons(), mouseEvent->modifiers());

                    QWidget *receiver = hoveredWidget;

                    QWidget *parent = hoveredWidget->parentWidget();
                    QPoint itemPos = itemView->visualRect(hoveredIndex).topLeft();
                    QPoint globalPos = itemView->viewport()->mapToGlobal(itemPos);
                    parent->move(globalPos);
                    parent->resize(itemView->visualRect(hoveredIndex).size());

                    QCoreApplication::sendEvent(hoveredWidget, &mouseEvt);

                    QList<QEvent::Type> blocked = q->blockedEventTypes(receiver);
                    filterEvent = blocked.contains(mouseEvent->type());

                    focusedWidget = receiver;

                    QFocusEvent focusEvent(QEvent::FocusIn);
                    QCoreApplication::sendEvent(focusedWidget, &focusEvent);
                } else {
                    focusedWidget = 0;
                }
            }
            itemView->viewport()->update();
            break;
        default: {
                if (event->type() == QEvent::FocusOut) {
                    buttonPressedWidget = 0;
                    buttonPressedIndex = QModelIndex();
                }

                // We don't need to do special stuff with this events. Just forward them the best we can
                // at this point. If it is a key event, send it to the focused widget (if any), and in other
                // case, forward it to the hovered widget (if any).
                if (dynamic_cast<QKeyEvent*>(event) && focusedWidget) {
                    QCoreApplication::sendEvent(focusedWidget, event);
                    QList<QEvent::Type> blocked = q->blockedEventTypes(focusedWidget);
                    filterEvent = blocked.contains(event->type());
                } else if (hoveredWidget) {
                    QCoreApplication::sendEvent(hoveredWidget, event);
                    QList<QEvent::Type> blocked = q->blockedEventTypes(hoveredWidget);
                    filterEvent = blocked.contains(event->type());
                }
            }
            itemView->viewport()->update();
            break;
    }

    return filterEvent || eventReply;
}
//@endcond

void KWidgetItemDelegate::setBlockedEventTypes(QWidget *widget, QList<QEvent::Type> types) const
{
    widget->setProperty("goya:blockedEventTypes", qVariantFromValue(types));
}

QList<QEvent::Type> KWidgetItemDelegate::blockedEventTypes(QWidget *widget) const
{
    return widget->property("goya:blockedEventTypes").value<QList<QEvent::Type> >();
}

#include "kwidgetitemdelegate_p.moc"
