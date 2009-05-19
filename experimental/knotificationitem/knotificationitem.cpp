/* This file is part of the KDE libraries
   Copyright 2009 by Marco Martin <notmart@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later
   version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "knotificationitem.h"
#include "knotificationitemprivate_p.h"
#include "knotificationitemdbus_p.h"

#include <QDBusConnection>
#include <QPixmap>
#include <QImage>
#include <QApplication>
#include <QMovie>
#include <QPainter>


#include <kdebug.h>
#include <ksystemtrayicon.h>
#include <kaboutdata.h>
#include <kicon.h>
#include <kmenu.h>
#include <kaction.h>
#include <kwindowinfo.h>
#include <kwindowsystem.h>
#include <kmessagebox.h>
#include <kactioncollection.h>



#include "notificationitemadaptor.h"

using namespace Experimental;

KNotificationItem::KNotificationItem(QObject *parent)
      : QObject(parent),
        d(new KNotificationItemPrivate(this))
{
    d->init(QString());
}


KNotificationItem::KNotificationItem(const QString &id, QObject *parent)
      : QObject(parent),
        d(new KNotificationItemPrivate(this))
{
    d->init(id);
}

KNotificationItem::~KNotificationItem()
{
    delete d->notificationItemWatcher;
    delete d->visualNotifications;
    delete d->systemTrayIcon;
    delete d->menu;
    delete d;
    KGlobal::deref();
}

QString KNotificationItem::id() const
{
    kDebug() << "id requested" << d->id;
    return d->id;
}

void KNotificationItem::setCategory(const ItemCategory category)
{
    d->category = category;
}

KNotificationItem::ItemStatus KNotificationItem::status() const
{
    return d->status;
}

KNotificationItem::ItemCategory KNotificationItem::category() const
{
    return d->category;
}

void KNotificationItem::setTitle(const QString &title)
{
    d->title = title;
}

void KNotificationItem::setStatus(const ItemStatus status)
{
    d->status = status;
    emit d->notificationItemDbus->NewStatus(metaObject()->enumerator(metaObject()->indexOfEnumerator("ItemStatus")).valueToKey(d->status));

    if (d->systemTrayIcon) {
        d->syncLegacySystemTrayIcon();
    }
}



//normal icon

void KNotificationItem::setIcon(const QString &name)
{
    d->iconName = name;
    emit d->notificationItemDbus->NewIcon();
    if (d->systemTrayIcon) {
        d->systemTrayIcon->setIcon(KIcon(name));
    }
}

QString KNotificationItem::icon() const
{
    return d->iconName;
}

void KNotificationItem::setImage(const QIcon &icon)
{
    d->serializedIcon = d->iconToVector(icon);
    emit d->notificationItemDbus->NewIcon();

    d->icon = icon;
    if (d->systemTrayIcon) {
        d->systemTrayIcon->setIcon(icon);
    }
}

QIcon KNotificationItem::image() const
{
    return d->icon;
}

void KNotificationItem::setOverlayIcon(const QString &name)
{
    d->overlayIconName = name;
    emit d->notificationItemDbus->NewOverlayIcon();
    if (d->systemTrayIcon) {
        QPixmap iconPixmap = KIcon(d->iconName).pixmap(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium);
        if (!name.isEmpty()) {
            QPixmap overlayPixmap = KIcon(d->overlayIconName).pixmap(KIconLoader::SizeSmallMedium/2, KIconLoader::SizeSmallMedium/2);
            QPainter p(&iconPixmap);
            p.drawPixmap(iconPixmap.width()-overlayPixmap.width(), iconPixmap.height()-overlayPixmap.height(), overlayPixmap);
            p.end();
        }
        d->systemTrayIcon->setIcon(iconPixmap);
    }
}

QString KNotificationItem::overlayIcon() const
{
    return d->overlayIconName;
}

void KNotificationItem::setOverlayImage(const QIcon &icon)
{
    d->serializedOverlayIcon = d->iconToVector(icon);
    emit d->notificationItemDbus->NewOverlayIcon();

    d->overlayIcon = icon;
    if (d->systemTrayIcon) {
        QPixmap iconPixmap = d->icon.pixmap(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium);
        QPixmap overlayPixmap = d->overlayIcon.pixmap(KIconLoader::SizeSmallMedium/2, KIconLoader::SizeSmallMedium/2);

        QPainter p(&iconPixmap);
        p.drawPixmap(iconPixmap.width()-overlayPixmap.width(), iconPixmap.height()-overlayPixmap.height(), overlayPixmap);
        p.end();
        d->systemTrayIcon->setIcon(iconPixmap);
    }
}

QIcon KNotificationItem::overlayImage() const
{
    return d->overlayIcon;
}

//Icons and movie for requesting attention state

void KNotificationItem::setAttentionIcon(const QString &name)
{
    d->attentionIconName = name;
    emit d->notificationItemDbus->NewAttentionIcon();
}

QString KNotificationItem::attentionIcon() const
{
    return d->attentionIconName;
}

void KNotificationItem::setAttentionImage(const QIcon &icon)
{
    d->serializedAttentionIcon = d->iconToVector(icon);
    d->attentionIcon = icon;
    emit d->notificationItemDbus->NewAttentionIcon();
}

QIcon KNotificationItem::attentionImage() const
{
    return d->attentionIcon;
}

void KNotificationItem::setAttentionMovie(QMovie *movie)
{
    //really ugly, but frameCount just returns 0 usually...
    for (int i=0; true; ++i) {

        if (!movie->jumpToFrame(i)) {
            break;
        }

        QImage frame = movie->currentImage();
        d->movieVector.append(d->imageToStruct(frame));
    }

    d->movie = movie;

    emit d->notificationItemDbus->NewAttentionIcon();

    if (d->systemTrayIcon) {
        d->systemTrayIcon->setMovie(movie);
    }
}

void KNotificationItem::setAttentionMovie(const QVector<QPixmap> &movie)
{
    //really ugly, but frameCount just returns 0 usually...
    for (int i=0; movie.size(); ++i) {
        d->movieVector.append(d->imageToStruct(movie[i].toImage()));
    }

    emit d->notificationItemDbus->NewAttentionIcon();
    //FIXME?: movie on the legacy systemtray icon is not supported here
}

void KNotificationItem::setAttentionMovie(const QVector<QImage> &movie)
{
    //really ugly, but frameCount just returns 0 usually...
    for (int i=0; movie.size(); ++i) {
        d->movieVector.append(d->imageToStruct(movie[i]));
    }

    emit d->notificationItemDbus->NewAttentionIcon();
}


QMovie *KNotificationItem::attentionMovie() const
{
    return d->movie;
}

//ToolTip

void KNotificationItem::setToolTip(const QString &iconName, const QString &title, const QString &subTitle)
{
    setToolTipIcon(iconName);
    setToolTipTitle(title);
    setToolTipSubTitle(subTitle);
    emit d->notificationItemDbus->NewToolTip();
}

void KNotificationItem::setToolTip(const QIcon &icon, const QString &title, const QString &subTitle)
{
    setToolTipImage(icon);
    setToolTipTitle(title);
    setToolTipSubTitle(subTitle);
    emit d->notificationItemDbus->NewToolTip();
}

void KNotificationItem::setToolTipIcon(const QString &name)
{
    d->toolTipIconName = name;
    emit d->notificationItemDbus->NewToolTip();
}

QString KNotificationItem::toolTipIcon() const
{
    return d->toolTipIconName;
}

void KNotificationItem::setToolTipImage(const QIcon &icon)
{
    d->serializedToolTipIcon = d->iconToVector(icon);
    d->toolTipIcon = icon;
    emit d->notificationItemDbus->NewToolTip();
}

QIcon KNotificationItem::toolTipImage() const
{
    return d->toolTipIcon;
}

void KNotificationItem::setToolTipTitle(const QString &title)
{
    d->toolTipTitle = title;
    emit d->notificationItemDbus->NewToolTip();
    if (d->systemTrayIcon) {
        d->systemTrayIcon->setToolTip(title);
    }
}

QString KNotificationItem::toolTipTitle() const
{
    return d->toolTipTitle;
}

void KNotificationItem::setToolTipSubTitle(const QString &subTitle)
{
    d->toolTipSubTitle = subTitle;
    emit d->notificationItemDbus->NewToolTip();
}

QString KNotificationItem::toolTipSubTitle() const
{
    return d->toolTipSubTitle;
}


void KNotificationItem::setContextMenu(KMenu *menu)
{
    if (d->menu != menu) {
        d->menu->removeEventFilter(this);
        delete d->menu;
    }

    d->menu = menu;

    if (d->menu) {
        d->menu->installEventFilter(this);
        connect(d->menu, SIGNAL(aboutToShow()), this, SLOT(contextMenuAboutToShow()));
    }

    if (d->systemTrayIcon) {
        d->systemTrayIcon->setContextMenu(menu);
    }
}

KMenu *KNotificationItem::contextMenu() const
{
    return d->menu;
}

void KNotificationItem::setAssociatedWidget(QWidget *associatedWidget)
{
    d->associatedWidget = associatedWidget;
    if (d->systemTrayIcon) {
        delete d->systemTrayIcon;
        d->systemTrayIcon = 0;
        d->setLegacySystemTrayEnabled(true);
    }

    if (associatedWidget) {
        QAction *action = d->actionCollection->action("minimizeRestore");

        if (!action) {
            action = d->actionCollection->addAction("minimizeRestore");
            action->setText(i18n("Minimize"));
            connect(action, SIGNAL(triggered(bool)), this, SLOT(minimizeRestore()));
        }

#ifdef Q_WS_X11
        KWindowInfo info = KWindowSystem::windowInfo(associatedWidget->winId(), NET::WMDesktop);
        d->onAllDesktops = info.onAllDesktops();
#else
        d->onAllDesktops = false;
#endif
    } else {
        d->onAllDesktops = false;
    }
}

QWidget *KNotificationItem::associatedWidget() const
{
    return d->associatedWidget;
}

KActionCollection *KNotificationItem::actionCollection() const
{
    return d->actionCollection;
}

void KNotificationItem::showMessage(const QString & title, const QString & message, const QString &icon, int timeout)
{
    if (!d->visualNotifications) {
        d->visualNotifications = new org::kde::VisualNotifications("org.kde.VisualNotifications", "/VisualNotifications",
                                                QDBusConnection::sessionBus());
    }

    d->visualNotifications->Notify(d->title, ++d->notificationId, 0, icon, title, message, QStringList(), QVariantMap(), timeout);
}

QString KNotificationItem::title() const
{
    return d->title;
}








void KNotificationItem::activate(const QPoint &pos)
{
    //if the user activated the icon the NeedsAttention state is no longer necessary
    //FIXME: always true?
    if (d->status == NeedsAttention) {
        d->status = Active;
        emit d->notificationItemDbus->NewStatus(metaObject()->enumerator(metaObject()->indexOfEnumerator("ItemStatus")).valueToKey(d->status));
    }

    if (!d->associatedWidget) {
        emit activateRequested(true, pos);
        return;
    }

#ifdef Q_WS_WIN
    // the problem is that we lose focus when the systray icon is activated
    // and we don't know the former active window
    // therefore we watch for activation event and use our stopwatch :)
    if(GetTickCount() - d->dwTickCount < 300) {
        // we were active in the last 300ms -> hide it
        d->minimizeRestore(false);
        emit activateRequested(false, pos);
    } else {
        d->minimizeRestore(true);
        emit activateRequested(true, pos);
    }
#elif defined(Q_WS_X11)
    KWindowInfo info1 = KWindowSystem::windowInfo(d->associatedWidget->winId(), NET::XAWMState | NET::WMState | NET::WMDesktop);
    // mapped = visible (but possibly obscured)
    bool mapped = (info1.mappingState() == NET::Visible) && !info1.isMinimized();

//    - not mapped -> show, raise, focus
//    - mapped
//        - obscured -> raise, focus
//        - not obscured -> hide
    //info1.mappingState() != NET::Visible -> window on another desktop?
    if (!mapped) {
        emit activateRequested(true, pos);
        d->minimizeRestore(true);
    } else {
        QListIterator< WId > it (KWindowSystem::stackingOrder());
        it.toBack();
        while (it.hasPrevious()) {
            WId id = it.previous();
            if (id == d->associatedWidget->winId()) {
                break;
            }

            KWindowInfo info2 = KWindowSystem::windowInfo(id,
                NET::WMDesktop | NET::WMGeometry | NET::XAWMState | NET::WMState | NET::WMWindowType);

            if (info2.mappingState() != NET::Visible) {
                continue; // not visible on current desktop -> ignore
            }

            if (!info2.geometry().intersects(d->associatedWidget->geometry())) {
                continue; // not obscuring the window -> ignore
            }

            if (!info1.hasState(NET::KeepAbove) && info2.hasState(NET::KeepAbove)) {
                continue; // obscured by window kept above -> ignore
            }

            NET::WindowType type = info2.windowType(NET::NormalMask | NET::DesktopMask
                | NET::DockMask | NET::ToolbarMask | NET::MenuMask | NET::DialogMask
                | NET::OverrideMask | NET::TopMenuMask | NET::UtilityMask | NET::SplashMask);

            if (type == NET::Dock || type == NET::TopMenu) {
                continue; // obscured by dock or topmenu -> ignore
            }

            emit activateRequested(true, pos);
            KWindowSystem::raiseWindow(d->associatedWidget->winId());
            KWindowSystem::activateWindow(d->associatedWidget->winId());
            return;
        }

        //not on current desktop?
        if (!info1.isOnCurrentDesktop()) {
            emit activateRequested(true, pos);
            KWindowSystem::activateWindow(d->associatedWidget->winId());
            return;
        }

        d->minimizeRestore(false); // hide
        emit activateRequested(false, pos);
    }
#endif
}

bool KNotificationItem::eventFilter(QObject *watched, QEvent *event)
{
    if (d->systemTrayIcon == 0) {
        //FIXME: ugly ugly workaround to weird QMenu's focus problems
        if (watched == d->menu &&
            (event->type() == QEvent::WindowDeactivate || event->type() == QEvent::MouseButtonRelease)) {
            //put at the back of even queue to let the action activate anyways
            QTimer::singleShot(0, this, SLOT(hideMenu()));
        }
    }
    return false;
}


//KNotificationItemPrivate

const int KNotificationItemPrivate::s_protocolVersion = 0;

KNotificationItemPrivate::KNotificationItemPrivate(KNotificationItem *item)
    : q(item),
      dbus(QDBusConnection::sessionBus()),
      category(KNotificationItem::ApplicationStatus),
      status(KNotificationItem::Passive),
      movie(0),
      menu(0),
      hasQuit(false),
      onAllDesktops(false),
      titleAction(0),
      notificationItemWatcher(0),
      visualNotifications(0),
      notificationId(0),
      systemTrayIcon(0)
{
}

void KNotificationItemPrivate::init(const QString &extraId)
{
    // Ensure that closing the last KMainWindow doesn't exit the application
    // if a system tray icon is still present.
    KGlobal::ref();
    KGlobal::locale()->insertCatalog("knotificationitem-1");
    qDBusRegisterMetaType<ExperimentalKDbusImageStruct>();
    qDBusRegisterMetaType<ExperimentalKDbusImageVector>();
    qDBusRegisterMetaType<ExperimentalKDbusToolTipStruct>();

    actionCollection = new KActionCollection(q);
    notificationItemDbus = new KNotificationItemDBus(q);
    q->setAssociatedWidget(qobject_cast<QWidget*>(q->parent()));

    QObject::connect(dbus.interface(), SIGNAL(serviceOwnerChanged(QString,QString,QString)),
                     q, SLOT(serviceChange(QString,QString,QString)));

    //create a default menu, just like in KSystemtrayIcon
    menu = new KMenu(associatedWidget);
    titleAction = menu->addTitle(qApp->windowIcon(), KGlobal::caption());
    menu->setTitle(KGlobal::mainComponent().aboutData()->programName());
    q->setContextMenu(menu);

    KStandardAction::quit(q, SLOT(maybeQuit()), actionCollection);

    id = title = KGlobal::mainComponent().aboutData()->programName();

    if (!extraId.isEmpty()) {
        id.append('_').append(extraId);
    }

    registerToDaemon();
}

void KNotificationItemPrivate::registerToDaemon()
{
    kDebug() << "Registering a client interface to the system tray daemon";
    if (!notificationItemWatcher) {
        QString interface("org.kde.NotificationItemWatcher");
        notificationItemWatcher = new org::kde::NotificationItemWatcher(interface, "/NotificationItemWatcher",
                                                                        QDBusConnection::sessionBus());
    }

    if (notificationItemWatcher->isValid() &&
        notificationItemWatcher->ProtocolVersion() == s_protocolVersion) {
        QObject::disconnect(notificationItemWatcher, SIGNAL(NotificationHostRegistered()), q, SLOT(registerToDaemon()));

        if (notificationItemWatcher->IsNotificationHostRegistered()) {
            kDebug() << "service is" << notificationItemDbus->service();
            notificationItemWatcher->RegisterService(notificationItemDbus->service());
            setLegacySystemTrayEnabled(false);
        } else {
            QObject::connect(notificationItemWatcher, SIGNAL(NotificationHostRegistered()), q, SLOT(registerToDaemon()));
        }
    } else {
        kDebug()<<"System tray daemon not reachable or no registered system trays";
        setLegacySystemTrayEnabled(true);
    }
}

void KNotificationItemPrivate::serviceChange(const QString& name, const QString& oldOwner, const QString& newOwner)
{
    bool legacy = false;
    if (name == "org.kde.NotificationItemWatcher") {
        if (newOwner.isEmpty()) {
            //unregistered
            kDebug() << "Connection to the systemtray daemon lost";
            legacy = true;
        } else if (oldOwner.isEmpty()) {
            //registered
            legacy = false;
        }
    } else if (name.startsWith("org.kde.Notification-")) {
        if (newOwner.isEmpty() && (!notificationItemWatcher ||
                                   !notificationItemWatcher->IsNotificationHostRegistered())) {
            //unregistered
            legacy = true;
        } else if (oldOwner.isEmpty()) {
            //registered
            legacy = false;
        }
    } else {
        return;
    }

    kDebug() << "Service " << name << "status change, old owner:" << oldOwner << "new:" << newOwner;

    if (legacy == (systemTrayIcon != 0)) {
        return;
    }

    if (legacy) {
        //unregistered
        kDebug()<<"Connection to the systemtray daemon lost";
        setLegacySystemTrayEnabled(true);
    } else {
        //registered
        registerToDaemon();
        setLegacySystemTrayEnabled(false);
    }
}

void KNotificationItemPrivate::legacyWheelEvent(int delta)
{
    notificationItemDbus->Scroll(delta, "vertical");
}

void KNotificationItemPrivate::legacyActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::MiddleClick) {
        q->emit secondaryActivateRequested(systemTrayIcon->geometry().topLeft());
    }
}

void KNotificationItemPrivate::setLegacySystemTrayEnabled(bool enabled)
{
    if (enabled) {
        delete notificationItemWatcher;
        notificationItemWatcher = 0;

        if (!systemTrayIcon) {
            systemTrayIcon = new KNotificationLegacyIcon(associatedWidget);
            systemTrayIcon->setContextMenu(menu);
            syncLegacySystemTrayIcon();
            systemTrayIcon->setToolTip(toolTipTitle);
            systemTrayIcon->show();
            QObject::connect(systemTrayIcon, SIGNAL(wheel(int)), q, SLOT(legacyWheelEvent(int)));
            QObject::connect(systemTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), q, SLOT(legacyActivated(QSystemTrayIcon::ActivationReason)));
        }

        if (menu) {
            menu->setWindowFlags(Qt::Popup);
        }
    } else {
        delete systemTrayIcon;
        systemTrayIcon = 0;

        if (menu) {
            menu->setWindowFlags(Qt::Window);
        }
    }
}

void KNotificationItemPrivate::syncLegacySystemTrayIcon()
{
    if (status == KNotificationItem::NeedsAttention) {
        if (movie) {
            systemTrayIcon->setMovie(movie);
        } else if (!attentionIconName.isNull()) {
            systemTrayIcon->setIcon(KIcon(attentionIconName));
        } else {
            systemTrayIcon->setIcon(attentionIcon);
        }
    } else {
        if (!iconName.isNull()) {
            systemTrayIcon->setIcon(KIcon(iconName));
        } else {
            systemTrayIcon->setIcon(icon);
        }
    }

    systemTrayIcon->setToolTip(toolTipTitle);
}

void KNotificationItemPrivate::contextMenuAboutToShow()
{
    if (!hasQuit) {
        // we need to add the actions to the menu afterwards so that these items
        // appear at the _END_ of the menu
        menu->addSeparator();
        if (associatedWidget) {
            QAction *action = actionCollection->action("minimizeRestore");

            if (action) {
                menu->addAction(action);
            }
        }

        QAction *action = actionCollection->action(KStandardAction::name(KStandardAction::Quit));

        if (action) {
            menu->addAction(action);
        }

        hasQuit = true;
    }

    if (associatedWidget) {
        QAction* action = actionCollection->action("minimizeRestore");
        if (associatedWidget->isVisible())  {
            action->setText(i18n("&Minimize"));
        } else {
            action->setText(i18n("&Restore"));
        }
    }
}

void KNotificationItemPrivate::maybeQuit()
{
    QString caption = KGlobal::caption();
    QString query = i18n("<qt>Are you sure you want to quit <b>%1</b>?</qt>", caption);

    if (KMessageBox::warningContinueCancel(associatedWidget, query,
                                     i18n("Confirm Quit From System Tray"),
                                     KStandardGuiItem::quit(),
                                     KStandardGuiItem::cancel(),
                                     QString("systemtrayquit%1")
                                            .arg(caption)) == KMessageBox::Continue) {
        qApp->quit();
    }

}

void KNotificationItemPrivate::minimizeRestore()
{
    minimizeRestore(!associatedWidget->isVisible());
}

void KNotificationItemPrivate::hideMenu()
{
    menu->hide();
}

void KNotificationItemPrivate::minimizeRestore(bool show)
{
#ifdef Q_WS_X11
    KWindowInfo info = KWindowSystem::windowInfo(associatedWidget->winId(), NET::WMGeometry | NET::WMDesktop);
    if (show) {
        if (onAllDesktops) {
            KWindowSystem::setOnAllDesktops(associatedWidget->winId(), true);
        } else {
            KWindowSystem::setCurrentDesktop(info.desktop());
        }

        associatedWidget->move(info.geometry().topLeft()); // avoid placement policies
        associatedWidget->show();
        associatedWidget->raise();
        KWindowSystem::activateWindow(associatedWidget->winId());
    } else {
        onAllDesktops = info.onAllDesktops();
        associatedWidget->hide();
    }
#else
    if (show) {
        associatedWidget->show();
        associatedWidget->raise();
        KWindowSystem::forceActiveWindow(associatedWidget->winId());
    } else {
        associatedWidget->hide();
    }
#endif
}

ExperimentalKDbusImageStruct KNotificationItemPrivate::imageToStruct(const QImage &image)
{
    ExperimentalKDbusImageStruct icon;
    icon.width = image.size().width();
    icon.height = image.size().height();

    icon.data = QByteArray((char*)image.bits(), image.numBytes());

    return icon;
}

ExperimentalKDbusImageVector KNotificationItemPrivate::iconToVector(const QIcon &icon)
{
    ExperimentalKDbusImageVector iconVector;

    QPixmap iconPixmap = icon.pixmap(KIconLoader::SizeSmall, KIconLoader::SizeSmall);

    iconVector.append(imageToStruct(iconPixmap.toImage()));

    //if an icon exactly that size wasn't found don't add it to the vector
    iconPixmap = icon.pixmap(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium);
    if (iconPixmap.width() == KIconLoader::SizeSmallMedium) {
        iconVector.append(imageToStruct(iconPixmap.toImage()));
    }

    iconPixmap = icon.pixmap(KIconLoader::SizeMedium, KIconLoader::SizeMedium);
    if (iconPixmap.width() == KIconLoader::SizeMedium) {
        iconVector.append(imageToStruct(iconPixmap.toImage()));
    }

    iconPixmap = icon.pixmap(KIconLoader::SizeLarge, KIconLoader::SizeLarge);
    if (iconPixmap.width() == KIconLoader::SizeLarge) {
        iconVector.append(imageToStruct(iconPixmap.toImage()));
    }

    //hopefully huge and enormous not necessary right now, since it's quite costly
    return iconVector;
}

#include "knotificationitem.moc"
#include "knotificationitemprivate_p.moc"
