/*
 * Copyright 2008 by Montel Laurent <montel@kde.org>
 * Copyright 2008 by Marco Martin <notmart@gmail.com>
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

#include "popupapplet.h"
#include "private/popupapplet_p.h"
#include "private/dialog_p.h"

#include <QApplication>
#include <QGraphicsProxyWidget>
#include <QGraphicsLinearLayout>
#include <QTimer>
#include <QVBoxLayout>

#ifdef Q_WS_X11
#include <QX11Info>
#endif

#include <kicon.h>
#include <kiconloader.h>
#include <kwindowsystem.h>
#include <kglobalsettings.h>
#include <netwm.h>

#include "plasma/private/applet_p.h"
#include "plasma/corona.h"
#include "plasma/containment.h"
#include "plasma/private/containment_p.h"
#include "plasma/dialog.h"
#include "plasma/package.h"
#include "plasma/theme.h"
#include "plasma/scripting/appletscript.h"
#include "plasma/tooltipmanager.h"
#include "plasma/widgets/iconwidget.h"

namespace Plasma
{

PopupApplet::PopupApplet(QObject *parent, const QVariantList &args)
    : Applet(parent, args),
      d(new PopupAppletPrivate(this))
{
}

PopupApplet::PopupApplet(const QString &packagePath, uint appletId, const QVariantList &args)
    : Applet(packagePath, appletId, args),
      d(new PopupAppletPrivate(this))
{
}

PopupApplet::~PopupApplet()
{
    delete widget();
    delete d;
}

void PopupApplet::setPopupIcon(const QIcon &icon)
{
    if (icon.isNull()) {
        if (d->icon) {
            delete d->icon;
            d->icon = 0;
            setLayout(0);
            setAspectRatioMode(d->savedAspectRatio);
        }

        return;
    }

    d->createIconWidget();
    d->icon->setIcon(icon);
}

void PopupApplet::setPopupIcon(const QString &iconName)
{
    // Attempt 1: is it in the plasmoid package?
    if (package().isValid()) {
        const QString file = package().filePath("images", iconName);
        if (!file.isEmpty()) {
            setPopupIcon(KIcon(file));
            return;
        }
    }

    // Attempt 2: is it a svg in the icons directory?
    QString name = QString("icons/") + iconName.split("-").first();
    if (!Plasma::Theme::defaultTheme()->imagePath(name).isEmpty()) {
        d->createIconWidget();
        d->icon->setSvg(name, iconName);
        if (d->icon->svg().isEmpty()) {
            setPopupIcon(KIcon(iconName));
        }

        return;
    }

    // Final Attempt: use KIcon
    setPopupIcon(KIcon(iconName));
}

QIcon PopupApplet::popupIcon() const
{
    return d->icon ? d->icon->icon() : QIcon();
}

QWidget *PopupApplet::widget()
{
    return d->widget;
}

void PopupApplet::setWidget(QWidget *widget)
{
    if (d->widget) {
        Plasma::Dialog *dialog = d->dialogPtr.data();
        if (dialog) {
            dialog->setGraphicsWidget(0);
            QVBoxLayout *lay = 0;

            QLayout *existingLayout = dialog->layout();
            if (existingLayout) {
                lay = dynamic_cast<QVBoxLayout *>(existingLayout);
                if (!lay) {
                    delete existingLayout;
                }
            }

            if (!lay) {
                lay = new QVBoxLayout;
                dialog->setLayout(lay);
            }

            lay->removeWidget(d->widget);
            lay->addWidget(widget);
        } else if (d->proxy) {
            d->proxy.data()->setWidget(widget);
        }
    }

    d->widget = widget;
}

QGraphicsWidget *PopupApplet::graphicsWidget()
{
    return d->graphicsWidget.data();
}

void PopupApplet::setGraphicsWidget(QGraphicsWidget *graphicsWidget)
{
    if (d->graphicsWidget) {
        if (d->dialogPtr) {
            d->dialogPtr.data()->setGraphicsWidget(graphicsWidget);
        } else if (layout())  {
            QGraphicsLinearLayout *lay = static_cast<QGraphicsLinearLayout *>(layout());
            lay->removeAt(0);
            if (graphicsWidget) {
                lay->addItem(graphicsWidget);
            }
        }
    }

    d->graphicsWidget = graphicsWidget;
}

void PopupAppletPrivate::popupConstraintsEvent(Plasma::Constraints constraints)
{
    Plasma::FormFactor f = q->formFactor();

    if (constraints & Plasma::FormFactorConstraint ||
        constraints & Plasma::StartupCompletedConstraint ||
        (constraints & Plasma::SizeConstraint &&
         (f == Plasma::Vertical || f == Plasma::Horizontal))) {
        QGraphicsLinearLayout *lay = dynamic_cast<QGraphicsLinearLayout *>(q->layout());

        if (icon && lay && lay->count() > 0) {
            lay->removeAt(0);
        }

        QSizeF minimum;
        QSizeF parentSize;

        QGraphicsWidget *gWidget = q->graphicsWidget();
        //kDebug() << "graphics widget is" << (QObject*)gWidget;
        QWidget *qWidget = q->widget();

        if (gWidget) {
            minimum = gWidget->minimumSize();
            // our layout may have been replaced on us in the call to graphicsWidget!
            lay = dynamic_cast<QGraphicsLinearLayout *>(q->layout());
        } else if (qWidget) {
            minimum = qWidget->minimumSizeHint();
        }

        //99% of the times q->parentWidget() is the containment, but using it  we can also manage the applet-in-applet case (i.e. systray)
        //there are also cases where the parentlayoutitem is bigger than the containment (e.g. newspaper)
        if (q->parentLayoutItem()) {
            parentSize = q->parentLayoutItem()->geometry().size();
        } else if (q->parentWidget()) {
            parentSize = q->parentWidget()->size();
        }

        //check if someone did the nasty trick of applets in applets, in this case we always want to be collapsed
        QGraphicsWidget *candidateParentApplet = q;
        Plasma::Applet *parentApplet = 0;
        //this loop should be executed normally a single time, at most 2-3 times for quite complex containments
        while (candidateParentApplet) {
            candidateParentApplet = candidateParentApplet->parentWidget();
            parentApplet = qobject_cast<Plasma::Applet *>(candidateParentApplet);
            if (parentApplet) {
                break;
            }
        }

        //Applet on desktop
        if ((!parentApplet || parentApplet->isContainment() ) && icon && (!icon->svg().isEmpty() || !icon->icon().isNull()) && ((f != Plasma::Vertical && f != Plasma::Horizontal) ||
            ((f == Plasma::Vertical && parentSize.width() >= minimum.width()) ||
             (f == Plasma::Horizontal && parentSize.height() >= minimum.height())))) {
            //kDebug() << "we are expanding the popupapplet";


            // we only switch to expanded if we aren't horiz/vert constrained and
            // this applet has an icon.
            // otherwise, we leave it up to the applet itself to figure it out
            if (icon) {
                icon->hide();
            }

            if (savedAspectRatio != Plasma::InvalidAspectRatioMode) {
                q->setAspectRatioMode(savedAspectRatio);
            }

            Dialog *dialog = dialogPtr.data();
            if (dialog) {
                if (dialog->layout() && qWidget) {
                    //we don't want to delete Widget inside the dialog layout
                    dialog->layout()->removeWidget(qWidget);
                }

                if (qWidget) {
                    qWidget->setParent(0);
                }

                delete dialog;
            }

            if (!lay) {
                lay = new QGraphicsLinearLayout();
                lay->setContentsMargins(0, 0, 0, 0);
                lay->setSpacing(0);
                lay->setOrientation(Qt::Horizontal);
                q->setLayout(lay);
            }

            QSize prefSize;

            if (gWidget) {
                if (proxy) {
                    proxy.data()->setWidget(0);
                    delete proxy.data();
                }

                Corona *corona = qobject_cast<Corona *>(gWidget->scene());

                if (corona) {
                    corona->removeOffscreenWidget(gWidget);
                }

                lay->addItem(gWidget);
                prefSize = gWidget->preferredSize().toSize();
            } else if (qWidget) {
                if (!proxy) {
                    proxy = new QGraphicsProxyWidget(q);
                    proxy.data()->setWidget(qWidget);
                    proxy.data()->show();
                }

                lay->addItem(proxy.data());
                prefSize = qWidget->sizeHint();
            }

            //we could be on a big panel, but in that case we will be able to resize
            //more than the natural minimum size, because we'll transform into an icon
            if (f == Plasma::Horizontal) {
                minimum.setHeight(0);
            } else if (f == Plasma::Vertical) {
                minimum.setWidth(0);
            }

            qreal left, top, right, bottom;
            q->getContentsMargins(&left, &top, &right, &bottom);
            QSizeF oldSize(q->size());

            //size not saved/invalid size saved
            if (oldSize.width() < q->minimumSize().width() || oldSize.height() < q->minimumSize().height()) {
                q->resize(prefSize);
                emit q->appletTransformedItself();
            }
        } else {
            //Applet on popup
            if (icon && lay) {
                lay->addItem(icon);
            }

            //kDebug() << "about to switch to a popup";
            if (!qWidget && !gWidget) {
                delete dialogPtr.data();
                return;
            }

            //there was already a dialog? don't make the switch again
            if (dialogPtr) {
                return;
            }

            if (proxy) {
                proxy.data()->setWidget(0); // prevent it from deleting our widget!
                delete proxy.data();
            }

            //save the aspect ratio mode in case we drag'n drop in the Desktop later
            savedAspectRatio = q->aspectRatioMode();

            if (icon) {
                icon->show();
                q->setAspectRatioMode(Plasma::ConstrainedSquare);
            }

            Dialog *dialog = new Dialog();
            dialog->d->appletPtr = q;
            dialogPtr = dialog;

            if (icon) {
                dialog->setAspectRatioMode(savedAspectRatio);
            }

            //no longer use Qt::Popup since that seems to cause a lot of problem when you drag
            //stuff out of your Dialog (extenders). Monitor WindowDeactivate events so we can
            //emulate the same kind of behavior as Qt::Popup (close when you click somewhere
            //else.

            if (gWidget) {
                Corona *corona = qobject_cast<Corona *>(gWidget->scene());
                if (!corona) {
                    corona = qobject_cast<Corona *>(q->scene());
                }

                if (corona) {
                    corona->addOffscreenWidget(gWidget);
                }

                gWidget->show();
                dialog->setGraphicsWidget(gWidget);
                dialog->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | (gWidget->windowFlags() & Qt::X11BypassWindowManagerHint));
            } else if (qWidget) {
                QVBoxLayout *l_layout = new QVBoxLayout(dialog);
                l_layout->setSpacing(0);
                l_layout->setMargin(0);
                l_layout->addWidget(qWidget);
                dialog->adjustSize();
                dialog->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | (qWidget->windowFlags() & Qt::X11BypassWindowManagerHint));
            } else {
                dialog->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
            }

            restoreDialogSize();
            KWindowSystem::setState(dialog->winId(), NET::SkipTaskbar | NET::SkipPager);
            dialog->installEventFilter(q);

            QObject::connect(dialog, SIGNAL(dialogResized()), q, SLOT(dialogSizeChanged()));
            QObject::connect(dialog, SIGNAL(dialogVisible(bool)), q, SLOT(dialogStatusChanged(bool)));
        }
    }

    if (constraints & Plasma::PopupConstraint) {
        updateDialogPosition();
    }

    if (icon) {
        // emit the size hint changing stuff for our applet as we are handling
        // the size changings
        emit q->sizeHintChanged(Qt::PreferredSize);
    }
}

void PopupAppletPrivate::appletActivated()
{
    internalTogglePopup(true);
}

QSizeF PopupApplet::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
{
    if (!d->dialogPtr || which != Qt::PreferredSize) {
        return Applet::sizeHint(which, constraint);
    }

    switch (formFactor()) {
        case Vertical:
        case Horizontal: {
            const int size = IconSize(KIconLoader::Panel);
            return QSizeF(size, size);
        }
        default:
            break;
    }

    const int size = IconSize(KIconLoader::Desktop);
    return QSizeF(size, size);
}

void PopupApplet::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!d->icon && !d->popupLostFocus && event->buttons() == Qt::LeftButton) {
        d->clicked = scenePos().toPoint();
        event->setAccepted(true);
        return;
    } else {
        d->popupLostFocus = false;
        Applet::mousePressEvent(event);
    }
}

void PopupApplet::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (!d->icon &&
        (d->clicked - scenePos().toPoint()).manhattanLength() < KGlobalSettings::dndEventDelay()) {
        d->internalTogglePopup();
    } else {
        Applet::mouseReleaseEvent(event);
    }
}

bool PopupApplet::eventFilter(QObject *watched, QEvent *event)
{
    if (!d->passive && watched == d->dialogPtr.data() && (event->type() == QEvent::WindowDeactivate)) {
        d->popupLostFocus = true;
        QTimer::singleShot(100, this, SLOT(clearPopupLostFocus()));
    }

    if (watched == d->dialogPtr.data() && event->type() == QEvent::ContextMenu) {
        //pass it up to the applet
        //well, actually we have to pass it to the *containment*
        //because all the code for showing an applet's contextmenu is actually in Containment.
        Containment *c = containment();
        if (c) {
            Applet *applet = this;
            Dialog *dialog = d->dialogPtr.data();
            if (dialog && dialog->graphicsWidget()) {
                int left, top, right, bottom;
                dialog->getContentsMargins(&left, &top, &right, &bottom);
                const QPoint eventPos = static_cast<QContextMenuEvent*>(event)->pos() - QPoint(left, top);
                QPointF pos = dialog->graphicsWidget()->mapToScene(eventPos);

                if (Applet *actual = c->d->appletAt(pos)) {
                    applet = actual;
                }
            }

            KMenu desktopMenu;
            c->d->addAppletActions(desktopMenu, applet, event);

            if (!desktopMenu.isEmpty()) {
                desktopMenu.exec(static_cast<QContextMenuEvent*>(event)->globalPos());
                return true;
            }

            return false;
        }
    }

    return Applet::eventFilter(watched, event);
}

void PopupApplet::showPopup(uint popupDuration)
{
    // use autohideTimer to store when the next show should be
    if (popupDuration > 0 || d->autohideTimer) {
        if (!d->autohideTimer) {
            d->autohideTimer = new QTimer(this);
            d->autohideTimer->setSingleShot(true);
            connect(d->autohideTimer, SIGNAL(timeout()), this, SLOT(hideTimedPopup()));
        }

        d->autohideTimer->stop();
        d->autohideTimer->setInterval(popupDuration);
    }

    //kDebug() << "starting delayed show, duration for popup is" << popupDuration;
    d->delayedShowTimer.start(0, this);
}

void PopupApplet::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == d->delayedShowTimer.timerId()) {
        d->delayedShowTimer.stop();
        Dialog *dialog = d->dialogPtr.data();
        if (dialog) {
            // move the popup before its fist show, even if the show isn't triggered by
            // a click, this should fix the first random position seen in some widgets
            if (!dialog->isVisible()) {
                d->internalTogglePopup();
            }

            const int popupDuration = d->autohideTimer ? d->autohideTimer->interval() : 0;
            //kDebug() << "popupDuration is:" << (d->autohideTimer ? d->autohideTimer->interval() : 0);
            if (popupDuration > 0) {
                d->autohideTimer->start();
            } else if (d->autohideTimer) {
                d->autohideTimer->stop();
            }
        }
    } else if (event->timerId() == d->showDialogTimer.timerId()) {
        d->showDialogTimer.stop();
        d->showDialog();
    } else {
        Applet::timerEvent(event);
    }
}

void PopupApplet::hidePopup()
{
    d->showDialogTimer.stop();
    d->delayedShowTimer.stop();

    Dialog *dialog = d->dialogPtr.data();
    if (dialog && dialog->isVisible()) {
        if (location() != Floating) {
            dialog->animatedHide(locationToInverseDirection(location()));
        } else {
            dialog->hide();
        }
    }
}

void PopupApplet::togglePopup()
{
    d->internalTogglePopup();
}

Plasma::PopupPlacement PopupApplet::popupPlacement() const
{
    return d->popupPlacement;
}

void PopupApplet::setPopupAlignment(Qt::AlignmentFlag alignment)
{
    d->popupAlignment = alignment;
}

Qt::AlignmentFlag PopupApplet::popupAlignment() const
{
    return d->popupAlignment;
}

void PopupApplet::popupEvent(bool popped)
{
    if (Applet::d->script) {
        emit Applet::d->script->popupEvent(popped);
    }
}

void PopupApplet::setPassivePopup(bool passive)
{
    d->passive = passive;
}

bool PopupApplet::isPassivePopup() const
{
    return d->passive;
}

bool PopupApplet::isPopupShowing() const
{
    return d->dialogPtr && d->dialogPtr.data()->isVisible();
}

bool PopupApplet::isIconified() const
{
    return d->dialogPtr;
}

PopupAppletPrivate::PopupAppletPrivate(PopupApplet *applet)
        : q(applet),
          icon(0),
          widget(0),
          popupPlacement(Plasma::FloatingPopup),
          popupAlignment(Qt::AlignLeft),
          savedAspectRatio(Plasma::InvalidAspectRatioMode),
          autohideTimer(0),
          preShowStatus(UnknownStatus),
          popupLostFocus(false),
          passive(false)
{
    int iconSize = IconSize(KIconLoader::Desktop);
    q->resize(iconSize, iconSize);
    q->setAcceptDrops(true);
    QObject::disconnect(q, SIGNAL(activate()), static_cast<Applet*>(q), SLOT(setFocus()));
    QObject::connect(q, SIGNAL(activate()), q, SLOT(appletActivated()));
    QObject::connect(KGlobalSettings::self(), SIGNAL(iconChanged(int)), q, SLOT(iconSizeChanged(int)));
}

PopupAppletPrivate::~PopupAppletPrivate()
{
    if (proxy) {
        proxy.data()->setWidget(0);
    }

    delete dialogPtr.data();
    delete icon;
}

void PopupAppletPrivate::iconSizeChanged(int group)
{
    if (icon && (group == KIconLoader::Desktop || group == KIconLoader::Panel)) {
        q->updateGeometry();
    }
}

void PopupAppletPrivate::internalTogglePopup(bool fromActivatedSignal)
{
    if (autohideTimer) {
        autohideTimer->stop();
    }

    delayedShowTimer.stop();

    Plasma::Dialog *dialog = dialogPtr.data();
    if (!dialog) {
        q->setFocus(Qt::ShortcutFocusReason);
        if (!fromActivatedSignal) {
            QObject::disconnect(q, SIGNAL(activate()), q, SLOT(appletActivated()));
            emit q->activate();
            QObject::connect(q, SIGNAL(activate()), q, SLOT(appletActivated()));
        }
        return;
    }

    if (!q->view()) {
        return;
    }

    if (dialog->isVisible()) {
        if (q->location() != Floating) {
            dialog->animatedHide(locationToInverseDirection(q->location()));
        } else {
            dialog->hide();
        }

        dialog->clearFocus();
    } else {
        if (!graphicsWidget) {
            // we have nothing to show, so let's not.
            if (!fromActivatedSignal) {
                QObject::disconnect(q, SIGNAL(activate()), q, SLOT(appletActivated()));
                emit q->activate();
                QObject::connect(q, SIGNAL(activate()), q, SLOT(appletActivated()));
            }
            return;
        }

        ToolTipManager::self()->hide(q);
        showDialogTimer.start(0, q);
    }
}

void PopupAppletPrivate::showDialog()
{
    Plasma::Dialog *dialog = dialogPtr.data();
    if (!dialog) {
        return;
    }

    updateDialogPosition();

    KWindowSystem::setOnAllDesktops(dialog->winId(), true);
    KWindowSystem::setState(dialog->winId(), NET::SkipTaskbar | NET::SkipPager);

    if (icon) {
        dialog->setAspectRatioMode(savedAspectRatio);
    }

    if (q->location() != Floating) {
        dialog->animatedShow(locationToDirection(q->location()));
    } else {
        dialog->show();
    }

    if (!(dialog->windowFlags() & Qt::X11BypassWindowManagerHint)) {
        KWindowSystem::activateWindow(dialog->winId());
    }
}

void PopupAppletPrivate::hideTimedPopup()
{
    autohideTimer->stop();
    q->hidePopup();
}

void PopupAppletPrivate::clearPopupLostFocus()
{
    if (!icon || !icon->isDown()) {
        q->hidePopup();
    }

    popupLostFocus = false;
}

KConfigGroup PopupAppletPrivate::popupConfigGroup()
{
    KConfigGroup *mainGroup = static_cast<Applet*>(q)->d->mainConfigGroup();
    return KConfigGroup(mainGroup, "PopupApplet");
}

void PopupAppletPrivate::dialogSizeChanged()
{
    //Reposition the dialog
    Plasma::Dialog *dialog = dialogPtr.data();
    if (dialog) {
        KConfigGroup sizeGroup = popupConfigGroup();
        sizeGroup.writeEntry("DialogHeight", dialog->height());
        sizeGroup.writeEntry("DialogWidth", dialog->width());

        updateDialogPosition(!dialog->isUserResizing());

        emit q->configNeedsSaving();
        emit q->appletTransformedByUser();
    }
}

void PopupAppletPrivate::dialogStatusChanged(bool shown)
{
    if (shown) {
        preShowStatus = q->status();
        q->setStatus(NeedsAttentionStatus);
        QObject::connect(q, SIGNAL(newStatus(Plasma::ItemStatus)),
                         q, SLOT(statusChangeWhileShown(Plasma::ItemStatus)),
                         Qt::UniqueConnection);
    } else {
        QObject::disconnect(q, SIGNAL(newStatus(Plasma::ItemStatus)),
                            q, SLOT(statusChangeWhileShown(Plasma::ItemStatus)));
        q->setStatus(preShowStatus);
    }

    q->popupEvent(shown);
}

void PopupAppletPrivate::statusChangeWhileShown(Plasma::ItemStatus status)
{
    preShowStatus = status;
}

void PopupAppletPrivate::createIconWidget()
{
    if (icon) {
        return;
    }

    icon = new Plasma::IconWidget(q);
    QObject::connect(icon, SIGNAL(clicked()), q, SLOT(internalTogglePopup()));

    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->setOrientation(Qt::Horizontal);
    layout->addItem(icon);
    layout->setAlignment(icon, Qt::AlignCenter);
    q->setLayout(layout);
}

void PopupAppletPrivate::restoreDialogSize()
{
    Plasma::Dialog *dialog = dialogPtr.data();
    if (!dialog) {
        return;
    }

    Corona *corona = qobject_cast<Corona *>(q->scene());
    if (!corona) {
        return;
    }

    KConfigGroup sizeGroup = popupConfigGroup();

    int preferredWidth = 0;
    int preferredHeight = 0;
    QGraphicsWidget *gWidget = dialog->graphicsWidget();
    if (gWidget) {
        preferredWidth = gWidget->preferredSize().width();
        preferredHeight = gWidget->preferredSize().height();
    }

    const int width = qMin(sizeGroup.readEntry("DialogWidth", preferredWidth),
                           corona->screenGeometry(-1).width() - 50);
    const int height = qMin(sizeGroup.readEntry("DialogHeight", preferredHeight),
                            corona->screenGeometry(-1).height() - 50);

    QSize saved(width, height);

    if (saved.isNull()) {
        saved = dialog->sizeHint();
    } else {
        saved = saved.expandedTo(dialog->minimumSizeHint());
    }

    if (saved.width() != dialog->width() || saved.height() != dialog->height()) {
        dialog->resize(saved);
       /*if (gWidget) {
         gWidget->resize(saved);
       }*/
    }
}

void PopupAppletPrivate::updateDialogPosition(bool move)
{
    Plasma::Dialog *dialog = dialogPtr.data();
    if (!dialog) {
        return;
    }

    Corona *corona = qobject_cast<Corona *>(q->scene());
    if (!corona) {
        return;
    }

    QGraphicsView *view = q->view();
    if (!view) {
        return;
    }

    const QPoint appletPos = view->mapToGlobal(view->mapFromScene(q->scenePos()));

    QPoint dialogPos = dialog->pos();
    if (move) {
        if (!q->containment() || view == q->containment()->view()) {
            dialogPos = corona->popupPosition(q, dialog->size(), popupAlignment);
        } else {
            dialogPos = corona->popupPosition(q->parentItem(), dialog->size(), popupAlignment);
        }
    }

    bool reverse = false;
    if (q->formFactor() == Plasma::Vertical) {
        reverse = (appletPos.y() + (q->size().height() / 2)) < (dialogPos.y() + (dialog->size().height() / 2));
        dialog->setMinimumResizeLimits(-1, appletPos.y(), -1, appletPos.y() + q->size().height());
    } else {
        reverse = (appletPos.x() + (q->size().width() / 2)) < (dialogPos.x() + (dialog->size().width() / 2));
        dialog->setMinimumResizeLimits(appletPos.x(), -1, appletPos.x() + q->size().width(), -1);
    }

    Dialog::ResizeCorners resizeCorners = Dialog::NoCorner;
    switch (q->location()) {
    case BottomEdge:
        resizeCorners = Dialog::NorthEast | Dialog::NorthWest;
        popupPlacement = reverse ? TopPosedLeftAlignedPopup : TopPosedRightAlignedPopup;
        break;
    case TopEdge:
        resizeCorners = Dialog::SouthEast | Dialog::SouthWest;
        popupPlacement = reverse ? Plasma::BottomPosedLeftAlignedPopup : Plasma::BottomPosedRightAlignedPopup;
        break;
    case LeftEdge:
        resizeCorners = Dialog::SouthEast | Dialog::NorthEast;
        popupPlacement = reverse ? RightPosedTopAlignedPopup : RightPosedBottomAlignedPopup;
        break;

    case RightEdge:
        resizeCorners = Dialog::SouthWest | Dialog::NorthWest;
        popupPlacement = reverse ? LeftPosedTopAlignedPopup : LeftPosedBottomAlignedPopup;
        break;

    default:
        popupPlacement = FloatingPopup;
        resizeCorners = Dialog::All;
        break;
    }

    dialog->setResizeHandleCorners(resizeCorners);
    if (move) {
        dialog->move(dialogPos);
    }
}

} // Plasma namespace

#include "popupapplet.moc"

