/*
 *   Copyright 2008 Marco Martin <notmart@gmail.com>
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

#include "tabbar.h"

#include <QGraphicsLinearLayout>
#include <QGraphicsLayoutItem>
#include <QString>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneWheelEvent>
#include <QIcon>
#include <QStyleOption>
#include <QPainter>

#include <kdebug.h>

#include <plasma/animator.h>

#include "private/nativetabbar_p.h"

namespace Plasma
{

class TabBarProxy : public QGraphicsProxyWidget
{
public:
    TabBarProxy(QGraphicsWidget *parent)
    : QGraphicsProxyWidget(parent)
    {
        native = new NativeTabBar();
        native->setAttribute(Qt::WA_NoSystemBackground);
        setWidget(native);
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    }

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget)
    {
        Q_UNUSED(option);
        Q_UNUSED(widget);
        //Don't paint the child widgets
        static_cast<NativeTabBar *>(QGraphicsProxyWidget::widget())->render(
            painter, QPoint(0, 0), QRegion(), 0);
    }

    NativeTabBar *native;
};

class TabBarPrivate
{
public:
    TabBarPrivate(TabBar *parent)
        : q(parent),
          tabProxy(0),
          currentIndex(0),
          tabWidgetMode(true),
          oldPage(0),
          newPage(0),
          oldPageAnimId(-1),
          newPageAnimId(-1)
    {
    }

    ~TabBarPrivate()
    {
    }

    void updateTabWidgetMode();
    void slidingCompleted(QGraphicsItem *item);
    void shapeChanged(const KTabBar::Shape shape);

    TabBar *q;
    TabBarProxy *tabProxy;
    QList<QGraphicsWidget *> pages;
    QGraphicsLinearLayout *mainLayout;
    QGraphicsLinearLayout *tabWidgetLayout;
    QGraphicsLinearLayout *tabBarLayout;
    int currentIndex;
    bool tabWidgetMode;

    QGraphicsWidget *oldPage;
    QGraphicsWidget *newPage;
    int oldPageAnimId;
    int newPageAnimId;
};

void TabBarPrivate::updateTabWidgetMode()
{
    bool tabWidget = false;

    foreach (QGraphicsWidget *page, pages) {
        if (page->preferredSize() != QSize(0, 0)) {
            tabWidget = true;
            break;
        }
    }

    if (tabWidget != tabWidgetMode) {
        if (tabWidget) {
            mainLayout->removeAt(0);
            tabBarLayout->insertItem(1, tabProxy);
            mainLayout->addItem(tabWidgetLayout);
        } else {
            mainLayout->removeAt(0);
            tabBarLayout->removeAt(1);
            mainLayout->addItem(tabProxy);
        }
    }

    //always show the tabbar
    //FIXME: Qt BUG: calling show on a child of an hidden item it shows it anyways
    //so we avoid to call it if the parent is hidden
    if (!tabWidget && q->isVisible()) {
        q->setTabBarShown(true);
    }

    tabWidgetMode = tabWidget;
}

void TabBarPrivate::slidingCompleted(QGraphicsItem *item)
{
    if (item == oldPage || item == newPage) {
        if (item == newPage) {
            tabWidgetLayout->addItem(newPage);
            newPageAnimId = -1;
        } else {
            oldPageAnimId = -1;
            item->hide();
        }
        q->setFlags(0);
    }
}

void TabBarPrivate::shapeChanged(const QTabBar::Shape shape)
{
    //FIXME: QGraphicsLinearLayout doesn't have setDirection, so for now
    // North is equal to south and East is equal to West
    switch (shape) {
    case QTabBar::RoundedWest:
    case QTabBar::TriangularWest:

    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast:
        tabBarLayout->setOrientation(Qt::Vertical);
        tabWidgetLayout->setOrientation(Qt::Horizontal);
        tabWidgetLayout->itemAt(0)->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        if (tabWidgetLayout->count() > 1) {
            tabWidgetLayout->itemAt(1)->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        }
        tabProxy->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        break;

    case QTabBar::RoundedSouth:
    case QTabBar::TriangularSouth:

    case QTabBar::RoundedNorth:
    case QTabBar::TriangularNorth:
    default:
        tabBarLayout->setOrientation(Qt::Horizontal);
        tabWidgetLayout->setOrientation(Qt::Vertical);
        tabWidgetLayout->itemAt(0)->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        if (tabWidgetLayout->count() > 1) {
            tabWidgetLayout->itemAt(1)->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        }
        tabProxy->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    }
    tabProxy->setPreferredSize(tabProxy->native->sizeHint());
}

TabBar::TabBar(QGraphicsWidget *parent)
    : QGraphicsWidget(parent),
      d(new TabBarPrivate(this))
{
    d->tabProxy = new TabBarProxy(this);
    d->tabWidgetLayout = new QGraphicsLinearLayout(Qt::Vertical);
    d->tabBarLayout = new QGraphicsLinearLayout(Qt::Horizontal);

    d->mainLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    d->mainLayout->addItem(d->tabWidgetLayout);

    setLayout(d->mainLayout);
    d->mainLayout->setContentsMargins(0,0,0,0);

    d->tabWidgetLayout->addItem(d->tabBarLayout);

    //tabBar is centered, so a stretch at begin one at the end
    d->tabBarLayout->addStretch();
    d->tabBarLayout->addItem(d->tabProxy);
    d->tabBarLayout->addStretch();
    //d->tabBarLayout->setStretchFactor(d->tabProxy, 2);

    connect(d->tabProxy->native, SIGNAL(currentChanged(int)),
            this, SLOT(setCurrentIndex(int)));
    connect(d->tabProxy->native, SIGNAL(shapeChanged(QTabBar::Shape)),
            this, SLOT(shapeChanged(QTabBar::Shape)));
    connect(Plasma::Animator::self(), SIGNAL(movementFinished(QGraphicsItem*)),
            this, SLOT(slidingCompleted(QGraphicsItem*)));
}

TabBar::~TabBar()
{
    delete d;
}


int TabBar::insertTab(int index, const QIcon &icon, const QString &label,
                      QGraphicsLayoutItem *content)
{
    QGraphicsWidget *page = new QGraphicsWidget(this);
    page->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    if (content) {
        if (content->isLayout()) {
            page->setLayout(static_cast<QGraphicsLayout *>(content));
        } else {
            QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Vertical, page);
            layout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            layout->addItem(content);
            page->setLayout(layout);
        }
    } else {
        page->setPreferredSize(0, 0);
    }

    d->pages.insert(qBound(0, index, d->pages.count()), page);

    if (d->pages.count() == 1) {
        d->tabWidgetLayout->addItem(page);
        page->setVisible(true);
        page->setEnabled(true);
    } else {
        page->setVisible(false);
        page->setEnabled(false);
    }

    d->tabProxy->setPreferredSize(d->tabProxy->native->sizeHint());
    d->updateTabWidgetMode();

    int actualIndex = d->tabProxy->native->insertTab(index, icon, label);
    d->currentIndex = d->tabProxy->native->currentIndex();
    d->tabProxy->setPreferredSize(d->tabProxy->native->sizeHint());
    d->updateTabWidgetMode();
    return actualIndex;
}

int TabBar::insertTab(int index, const QString &label, QGraphicsLayoutItem *content)
{
    return insertTab(index, QIcon(), label, content);
}

int TabBar::addTab(const QIcon &icon, const QString &label, QGraphicsLayoutItem *content)
{
    return insertTab(d->pages.count(), icon, label, content);
}

int TabBar::addTab(const QString &label, QGraphicsLayoutItem *content)
{
    return insertTab(d->pages.count(), QIcon(), label, content);
}

int TabBar::currentIndex() const
{
    return d->tabProxy->native->currentIndex();
}

void TabBar::resizeEvent(QGraphicsSceneResizeEvent * event)
{
    if (!d->tabWidgetMode) {
        d->tabProxy->setMinimumSize(event->newSize().toSize());
        setMinimumSize(QSize(0, 0));
        setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    } else {
        d->tabProxy->native->setMinimumSize(QSize(0,0));
    }
}

void TabBar::setCurrentIndex(int index)
{
    if (index >= d->tabProxy->native->count() ||
        d->tabProxy->native->count() <= 1 ||
        d->currentIndex == index) {
        return;
    }

    d->tabWidgetLayout->removeAt(1);

    if (d->currentIndex >= 0) {
        d->oldPage = d->pages[d->currentIndex];
    } else {
        d->oldPage = 0;
    }

    if (index >= 0) {
        d->newPage = d->pages[index];
    } else {
        d->newPage = 0;
    }

    if (d->newPage) {
        d->newPage->resize(d->oldPage->size());
    }

    setFlags(QGraphicsItem::ItemClipsChildrenToShape);

    //if an animation was in rogress hide everything to avoid an inconsistent state
    if (d->newPageAnimId != -1 || d->oldPageAnimId != -1) {
        foreach (QGraphicsWidget *page, d->pages) {
            page->hide();
        }
        if (d->newPageAnimId != -1) {
            Animator::self()->stopItemMovement(d->newPageAnimId);
        }
        if (d->oldPageAnimId != -1) {
            Animator::self()->stopItemMovement(d->oldPageAnimId);
        }
    }

    if (d->newPage) {
        d->newPage->show();
        d->newPage->setEnabled(true);
    }

    if (d->oldPage) {
        d->oldPage->show();
        d->oldPage->setEnabled(false);
    }

    if (d->newPage && d->oldPage) {
        QRect beforeCurrentGeom(d->oldPage->geometry().toRect());
        beforeCurrentGeom.moveTopRight(beforeCurrentGeom.topLeft());

        d->newPageAnimId = Animator::self()->moveItem(
            d->newPage, Plasma::Animator::SlideOutMovement,
            d->oldPage->pos().toPoint());
        if (index > d->currentIndex) {
            d->newPage->setPos(d->oldPage->geometry().topRight());
            d->oldPageAnimId = Animator::self()->moveItem(
                d->oldPage, Plasma::Animator::SlideOutMovement,
                beforeCurrentGeom.topLeft());
        } else {
            d->newPage->setPos(beforeCurrentGeom.topLeft());
            d->oldPageAnimId = Animator::self()->moveItem(
                d->oldPage, Plasma::Animator::SlideOutMovement,
                d->oldPage->geometry().topRight().toPoint());
        }
    } else {
        d->tabWidgetLayout->addItem(d->newPage);
    }

    d->currentIndex = index;
    emit currentChanged(index);
    d->tabProxy->native->setCurrentIndex(index);
}

int TabBar::count() const
{
    return d->pages.count();
}

void TabBar::removeTab(int index)
{
    if (index > d->pages.count()) {
        return;
    }

    int oldCurrentIndex = d->tabProxy->native->currentIndex();
    d->tabProxy->native->removeTab(index);
    QGraphicsWidget *page = d->pages.takeAt(index);

    int currentIndex = d->tabProxy->native->currentIndex();

    if (oldCurrentIndex == index) {
        d->tabWidgetLayout->removeAt(1);
    }

    scene()->removeItem(page);
    page->deleteLater();

    if (oldCurrentIndex != currentIndex) {
        setCurrentIndex(currentIndex);
    }

    d->updateTabWidgetMode();
    d->tabProxy->setPreferredSize(d->tabProxy->native->sizeHint());
}

void TabBar::setTabText(int index, const QString &label)
{
    if (index > d->pages.count()) {
        return;
    }

    d->tabProxy->native->setTabText(index, label);
}

QString TabBar::tabText(int index) const
{
    return d->tabProxy->native->tabText(index);
}

void TabBar::setTabIcon(int index, const QIcon &icon)
{
    d->tabProxy->native->setTabIcon(index, icon);
}

QIcon TabBar::tabIcon(int index) const
{
    return d->tabProxy->native->tabIcon(index);
}

void TabBar::setTabBarShown(bool show)
{
    if (!show && !d->tabWidgetMode) {
        return;
    }

    if (!show && d->tabProxy->isVisible()) {
        d->tabProxy->hide();
        d->tabBarLayout->removeItem(d->tabProxy);
    } else if (show && !d->tabProxy->isVisible()) {
        d->tabProxy->show();
        d->tabBarLayout->insertItem(0, d->tabProxy);
    }
}

bool TabBar::isTabBarShown() const
{
    return d->tabProxy->isVisible();
}

void TabBar::setStyleSheet(const QString &stylesheet)
{
    d->tabProxy->native->setStyleSheet(stylesheet);
}

QString TabBar::styleSheet() const
{
    return d->tabProxy->native->styleSheet();
}

KTabBar *TabBar::nativeWidget() const
{
    return d->tabProxy->native;
}

void TabBar::wheelEvent(QGraphicsSceneWheelEvent * event)
{
    Q_UNUSED(event)
    //Still here for binary compatibility
}

} // namespace Plasma

#include <tabbar.moc>

