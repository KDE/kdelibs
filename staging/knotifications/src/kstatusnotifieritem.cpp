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

#include "kstatusnotifieritem.h"
#include "kstatusnotifieritemprivate_p.h"
#include "kstatusnotifieritemdbus_p.h"

#include <QDBusConnection>
#include <QtEndian>
#include <QMessageBox>
#include <QDebug>
#include <QPixmap>
#include <QImage>
#include <QApplication>
#include <QMenu>
#include <QMovie>
#include <QPainter>
#include <qstandardpaths.h>

#include <kiconloader.h>
#include <kwindowinfo.h>
#include <kwindowsystem.h>
#include <kactioncollection.h>

#include <cstdlib>

#include <config-knotifications.h>

static const char s_statusNotifierWatcherServiceName[] = "org.kde.StatusNotifierWatcher";

#if HAVE_DBUSMENUQT
#include <dbusmenuexporter.h>

#include "statusnotifieritemadaptor.h"

/**
 * Specialization to provide access to KDE icon names
 */
class KDBusMenuExporter : public DBusMenuExporter
{
public:
    KDBusMenuExporter(const QString &dbusObjectPath, QMenu *menu, const QDBusConnection &dbusConnection)
    : DBusMenuExporter(dbusObjectPath, menu, dbusConnection)
    {}

protected:
    virtual QString iconNameForAction(QAction *action)
    {
	QIcon icon(action->icon());
#if QT_VERSION >= 0x040701
        // QIcon::name() is in the 4.7 git branch, but it is not in 4.7 TP.
        // If you get a build error here, you need to update your pre-release
        // of Qt 4.7.
        return icon.isNull() ? QString() : icon.name();
#else
        // Qt 4.6: If the icon was created by us, via our engine, serializing it
        // will let us get to the name.
        if (!icon.isNull()) {
            QBuffer encBuf;
            encBuf.open(QIODevice::WriteOnly);
            QDataStream encode(&encBuf);
            encode.setVersion(QDataStream::Qt_4_6);
            encode << icon;
            encBuf.close();

            if (!encBuf.data().isEmpty()) {
                QDataStream decode(encBuf.data());
                QString key;
                decode >> key;
                if (key == QLatin1String("KIconEngine")) {
                    QString name;
                    decode >> name;
                    return name;
                }
            }
        }

        return QString();
#endif
    }
};
#endif //HAVE_DBUSMENUQT

KStatusNotifierItem::KStatusNotifierItem(QObject *parent)
      : QObject(parent),
        d(new KStatusNotifierItemPrivate(this))
{
    d->init(QString());
}


KStatusNotifierItem::KStatusNotifierItem(const QString &id, QObject *parent)
      : QObject(parent),
        d(new KStatusNotifierItemPrivate(this))
{
    d->init(id);
}

KStatusNotifierItem::~KStatusNotifierItem()
{
    delete d->statusNotifierWatcher;
    delete d->notificationsClient;
    delete d->systemTrayIcon;
    if (!qApp->closingDown()) {
        delete d->menu;
    }
    delete d;
}

QString KStatusNotifierItem::id() const
{
    //qDebug() << "id requested" << d->id;
    return d->id;
}

void KStatusNotifierItem::setCategory(const ItemCategory category)
{
    d->category = category;
}

KStatusNotifierItem::ItemStatus KStatusNotifierItem::status() const
{
    return d->status;
}

KStatusNotifierItem::ItemCategory KStatusNotifierItem::category() const
{
    return d->category;
}

void KStatusNotifierItem::setTitle(const QString &title)
{
    d->title = title;
}

void KStatusNotifierItem::setStatus(const ItemStatus status)
{
    if (d->status == status) {
        return;
    }

    d->status = status;
    emit d->statusNotifierItemDBus->NewStatus(metaObject()->enumerator(metaObject()->indexOfEnumerator("ItemStatus")).valueToKey(d->status));

    if (d->systemTrayIcon) {
        d->syncLegacySystemTrayIcon();
    }
}



//normal icon

void KStatusNotifierItem::setIconByName(const QString &name)
{
    if (d->iconName == name) {
        return;
    }

    d->serializedIcon = KDbusImageVector();
    d->iconName = name;
    emit d->statusNotifierItemDBus->NewIcon();
    if (d->systemTrayIcon) {
        d->systemTrayIcon->setIcon(QIcon::fromTheme(name));
    }
}

QString KStatusNotifierItem::iconName() const
{
    return d->iconName;
}

void KStatusNotifierItem::setIconByPixmap(const QIcon &icon)
{
    if (d->iconName.isEmpty() && d->icon.cacheKey() == icon.cacheKey()) {
        return;
    }

    d->iconName.clear();
    d->serializedIcon = d->iconToVector(icon);
    emit d->statusNotifierItemDBus->NewIcon();

    d->icon = icon;
    if (d->systemTrayIcon) {
        d->systemTrayIcon->setIcon(icon);
    }
}

QIcon KStatusNotifierItem::iconPixmap() const
{
    return d->icon;
}

void KStatusNotifierItem::setOverlayIconByName(const QString &name)
{
    if (d->overlayIconName == name) {
        return;
    }

    d->overlayIconName = name;
    emit d->statusNotifierItemDBus->NewOverlayIcon();
    if (d->systemTrayIcon) {
        QPixmap iconPixmap = QIcon::fromTheme(d->iconName).pixmap(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium);
        if (!name.isEmpty()) {
            QPixmap overlayPixmap = QIcon::fromTheme(d->overlayIconName).pixmap(KIconLoader::SizeSmallMedium/2, KIconLoader::SizeSmallMedium/2);
            QPainter p(&iconPixmap);
            p.drawPixmap(iconPixmap.width()-overlayPixmap.width(), iconPixmap.height()-overlayPixmap.height(), overlayPixmap);
            p.end();
        }
        d->systemTrayIcon->setIcon(iconPixmap);
    }
}

QString KStatusNotifierItem::overlayIconName() const
{
    return d->overlayIconName;
}

void KStatusNotifierItem::setOverlayIconByPixmap(const QIcon &icon)
{
    if (d->overlayIconName.isEmpty() && d->overlayIcon.cacheKey() == icon.cacheKey()) {
        return;
    }

    d->overlayIconName.clear();
    d->serializedOverlayIcon = d->iconToVector(icon);
    emit d->statusNotifierItemDBus->NewOverlayIcon();

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

QIcon KStatusNotifierItem::overlayIconPixmap() const
{
    return d->overlayIcon;
}

//Icons and movie for requesting attention state

void KStatusNotifierItem::setAttentionIconByName(const QString &name)
{
    if (d->attentionIconName == name) {
        return;
    }

    d->serializedAttentionIcon = KDbusImageVector();
    d->attentionIconName = name;
    emit d->statusNotifierItemDBus->NewAttentionIcon();
}

QString KStatusNotifierItem::attentionIconName() const
{
    return d->attentionIconName;
}

void KStatusNotifierItem::setAttentionIconByPixmap(const QIcon &icon)
{
    if (d->attentionIconName.isEmpty() && d->attentionIcon.cacheKey() == icon.cacheKey()) {
        return;
    }

    d->attentionIconName.clear();
    d->serializedAttentionIcon = d->iconToVector(icon);
    d->attentionIcon = icon;
    emit d->statusNotifierItemDBus->NewAttentionIcon();
}

QIcon KStatusNotifierItem::attentionIconPixmap() const
{
    return d->attentionIcon;
}

void KStatusNotifierItem::setAttentionMovieByName(const QString &name)
{
    if (d->movieName == name) {
        return;
    }

    d->movieName = name;

    delete d->movie;
    d->movie = 0;

    emit d->statusNotifierItemDBus->NewAttentionIcon();

    if (d->systemTrayIcon) {
        d->movie = new QMovie(d->movieName);
        d->systemTrayIcon->setMovie(d->movie);
    }
}

QString KStatusNotifierItem::attentionMovieName() const
{
    return d->movieName;
}

//ToolTip

void KStatusNotifierItem::setToolTip(const QString &iconName, const QString &title, const QString &subTitle)
{
    if (d->toolTipIconName == iconName &&
        d->toolTipTitle == title &&
        d->toolTipSubTitle == subTitle) {
        return;
    }

    d->serializedToolTipIcon = KDbusImageVector();
    d->toolTipIconName = iconName;

    d->toolTipTitle = title;
    if (d->systemTrayIcon) {
        d->systemTrayIcon->setToolTip(title);
    }

    d->toolTipSubTitle = subTitle;
    emit d->statusNotifierItemDBus->NewToolTip();
}

void KStatusNotifierItem::setToolTip(const QIcon &icon, const QString &title, const QString &subTitle)
{
    if (d->toolTipIconName.isEmpty() && d->toolTipIcon.cacheKey() == icon.cacheKey() &&
        d->toolTipTitle == title &&
        d->toolTipSubTitle == subTitle) {
        return;
    }

    d->toolTipIconName.clear();
    d->serializedToolTipIcon = d->iconToVector(icon);
    d->toolTipIcon = icon;

    d->toolTipTitle = title;
    if (d->systemTrayIcon) {
        d->systemTrayIcon->setToolTip(title);
    }

    d->toolTipSubTitle = subTitle;
    emit d->statusNotifierItemDBus->NewToolTip();
}

void KStatusNotifierItem::setToolTipIconByName(const QString &name)
{
    if (d->toolTipIconName == name) {
        return;
    }

    d->serializedToolTipIcon = KDbusImageVector();
    d->toolTipIconName = name;
    emit d->statusNotifierItemDBus->NewToolTip();
}

QString KStatusNotifierItem::toolTipIconName() const
{
    return d->toolTipIconName;
}

void KStatusNotifierItem::setToolTipIconByPixmap(const QIcon &icon)
{
    if (d->toolTipIconName.isEmpty() && d->toolTipIcon.cacheKey() == icon.cacheKey()) {
        return;
    }

    d->toolTipIconName.clear();
    d->serializedToolTipIcon = d->iconToVector(icon);
    d->toolTipIcon = icon;
    emit d->statusNotifierItemDBus->NewToolTip();
}

QIcon KStatusNotifierItem::toolTipIconPixmap() const
{
    return d->toolTipIcon;
}

void KStatusNotifierItem::setToolTipTitle(const QString &title)
{
    if (d->toolTipTitle == title) {
        return;
    }

    d->toolTipTitle = title;
    emit d->statusNotifierItemDBus->NewToolTip();
    if (d->systemTrayIcon) {
        d->systemTrayIcon->setToolTip(title);
    }
}

QString KStatusNotifierItem::toolTipTitle() const
{
    return d->toolTipTitle;
}

void KStatusNotifierItem::setToolTipSubTitle(const QString &subTitle)
{
    if (d->toolTipSubTitle == subTitle) {
        return;
    }

    d->toolTipSubTitle = subTitle;
    emit d->statusNotifierItemDBus->NewToolTip();
}

QString KStatusNotifierItem::toolTipSubTitle() const
{
    return d->toolTipSubTitle;
}

void KStatusNotifierItem::setContextMenu(QMenu *menu)
{
    if (d->menu && d->menu != menu) {
        d->menu->removeEventFilter(this);
        delete d->menu;
    }

    if (!menu) {
        d->menu = 0;
        return;
    }

    if (d->systemTrayIcon) {
        d->systemTrayIcon->setContextMenu(menu);
    } else if (d->menu != menu) {
        if (getenv("KSNI_NO_DBUSMENU")) {
            // This is a hack to make it possible to disable DBusMenu in an
            // application. The string "/NO_DBUSMENU" must be the same as in
            // DBusSystemTrayWidget::findDBusMenuInterface() in the Plasma
            // systemtray applet.
            d->menuObjectPath = "/NO_DBUSMENU";
            menu->installEventFilter(this);
        } else {
            d->menuObjectPath = "/MenuBar";
#if HAVE_DBUSMENUQT
            new KDBusMenuExporter(d->menuObjectPath, menu, d->statusNotifierItemDBus->dbusConnection());
#endif
        }

        connect(menu, SIGNAL(aboutToShow()), this, SLOT(contextMenuAboutToShow()));
    }

    d->menu = menu;
    d->menu->setParent(0);
}

QMenu *KStatusNotifierItem::contextMenu() const
{
    return d->menu;
}

void KStatusNotifierItem::setAssociatedWidget(QWidget *associatedWidget)
{
    if (associatedWidget) {
        d->associatedWidget = associatedWidget->window();
    } else {
        d->associatedWidget = 0;
    }

    if (d->systemTrayIcon) {
        delete d->systemTrayIcon;
        d->systemTrayIcon = 0;
        d->setLegacySystemTrayEnabled(true);
    }

    if (d->associatedWidget && d->associatedWidget != d->menu) {
        QAction *action = d->actionCollection.value("minimizeRestore");

        if (!action) {
            action = new QAction(this);
            d->actionCollection.insert("minimizeRestore", action);
            action->setText(QObject::tr("&Minimize"));
            connect(action, SIGNAL(triggered(bool)), this, SLOT(minimizeRestore()));
        }

#if HAVE_X11
        KWindowInfo info = KWindowSystem::windowInfo(d->associatedWidget->winId(), NET::WMDesktop);
        d->onAllDesktops = info.onAllDesktops();
#else
        d->onAllDesktops = false;
#endif
    } else {
        if (d->menu && d->hasQuit) {
            QAction *action = d->actionCollection.value("minimizeRestore");
            if (action) {
                d->menu->removeAction(action);
            }
        }

        d->onAllDesktops = false;
    }
}

QWidget *KStatusNotifierItem::associatedWidget() const
{
    return d->associatedWidget;
}

QList<QAction *> KStatusNotifierItem::actionCollection() const
{
    return d->actionCollection.values();
}

void KStatusNotifierItem::addAction(const QString &name, QAction *action)
{
    d->actionCollection.insert(name, action);
}

void KStatusNotifierItem::removeAction(const QString &name)
{
    d->actionCollection.remove(name);
}

void KStatusNotifierItem::setStandardActionsEnabled(bool enabled)
{
    if (d->standardActionsEnabled == enabled) {
        return;
    }

    d->standardActionsEnabled = enabled;

    if (d->menu && !enabled && d->hasQuit) {
        QAction *action = d->actionCollection.value("minimizeRestore");
        if (action) {
            d->menu->removeAction(action);
        }

        action = d->actionCollection.value("quit");
        if (action) {
            d->menu->removeAction(action);
        }


        d->hasQuit = false;
    }
}

bool KStatusNotifierItem::standardActionsEnabled() const
{
    return d->standardActionsEnabled;
}

void KStatusNotifierItem::showMessage(const QString & title, const QString & message, const QString &icon, int timeout)
{
    if (!d->notificationsClient) {
        d->notificationsClient = new org::freedesktop::Notifications("org.freedesktop.Notifications", "/org/freedesktop/Notifications",
                                                QDBusConnection::sessionBus());
    }

    uint id = 0;
    d->notificationsClient->Notify(d->title, id, icon, title, message, QStringList(), QVariantMap(), timeout);
}

QString KStatusNotifierItem::title() const
{
    return d->title;
}



void KStatusNotifierItem::activate(const QPoint &pos)
{
    //if the user activated the icon the NeedsAttention state is no longer necessary
    //FIXME: always true?
    if (d->status == NeedsAttention) {
        d->status = Active;
        emit d->statusNotifierItemDBus->NewStatus(metaObject()->enumerator(metaObject()->indexOfEnumerator("ItemStatus")).valueToKey(d->status));
    }

    if (d->associatedWidget == d->menu) {
        d->statusNotifierItemDBus->ContextMenu(pos.x(), pos.y());
        return;
    }

    if (d->menu->isVisible()) {
        d->menu->hide();
    }

    if (!d->associatedWidget) {
        emit activateRequested(true, pos);
        return;
    }

    d->checkVisibility(pos);
}

bool KStatusNotifierItemPrivate::checkVisibility(QPoint pos, bool perform)
{
#ifdef Q_OS_WIN
#if 0
    // the problem is that we lose focus when the systray icon is activated
    // and we don't know the former active window
    // therefore we watch for activation event and use our stopwatch :)
    if(GetTickCount() - dwTickCount < 300) {
        // we were active in the last 300ms -> hide it
        minimizeRestore(false);
        emit activateRequested(false, pos);
    } else {
        minimizeRestore(true);
        emit activateRequested(true, pos);
    }
#endif
#elif HAVE_X11
    KWindowInfo info1 = KWindowSystem::windowInfo(associatedWidget->winId(), NET::XAWMState | NET::WMState | NET::WMDesktop);
    // mapped = visible (but possibly obscured)
    bool mapped = (info1.mappingState() == NET::Visible) && !info1.isMinimized();

//    - not mapped -> show, raise, focus
//    - mapped
//        - obscured -> raise, focus
//        - not obscured -> hide
    //info1.mappingState() != NET::Visible -> window on another desktop?
    if (!mapped) {
        if (perform) {
            minimizeRestore(true);
            emit q->activateRequested(true, pos);
        }

        return true;
    } else {
        QListIterator< WId > it (KWindowSystem::stackingOrder());
        it.toBack();
        while (it.hasPrevious()) {
            WId id = it.previous();
            if (id == associatedWidget->winId()) {
                break;
            }

            KWindowInfo info2 = KWindowSystem::windowInfo(id,
                NET::WMDesktop | NET::WMGeometry | NET::XAWMState | NET::WMState | NET::WMWindowType);

            if (info2.mappingState() != NET::Visible) {
                continue; // not visible on current desktop -> ignore
            }

            if (!info2.geometry().intersects(associatedWidget->geometry())) {
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

            if (perform) {
                KWindowSystem::raiseWindow(associatedWidget->winId());
                KWindowSystem::forceActiveWindow(associatedWidget->winId());
                emit q->activateRequested(true, pos);
            }

            return true;
        }

        //not on current desktop?
        if (!info1.isOnCurrentDesktop()) {
            if (perform) {
                KWindowSystem::activateWindow(associatedWidget->winId());
                emit q->activateRequested(true, pos);
            }

            return true;
        }

        if (perform) {
            minimizeRestore(false); // hide
            emit q->activateRequested(false, pos);
        }

        return false;
    }
#endif

    return true;
}

bool KStatusNotifierItem::eventFilter(QObject *watched, QEvent *event)
{
    if (d->systemTrayIcon == 0) {
        //FIXME: ugly ugly workaround to weird QMenu's focus problems
        if (watched == d->menu &&
            (event->type() == QEvent::WindowDeactivate || (event->type() == QEvent::MouseButtonRelease && static_cast<QMouseEvent*>(event)->button() == Qt::LeftButton))) {
            //put at the back of even queue to let the action activate anyways
            QTimer::singleShot(0, this, SLOT(hideMenu()));
        }
    }
    return false;
}


//KStatusNotifierItemPrivate

const int KStatusNotifierItemPrivate::s_protocolVersion = 0;

KStatusNotifierItemPrivate::KStatusNotifierItemPrivate(KStatusNotifierItem *item)
    : q(item),
      category(KStatusNotifierItem::ApplicationStatus),
      status(KStatusNotifierItem::Passive),
      movie(0),
      menu(0),
      titleAction(0),
      statusNotifierWatcher(0),
      notificationsClient(0),
      systemTrayIcon(0),
      hasQuit(false),
      onAllDesktops(false),
      standardActionsEnabled(true)
{
}

void KStatusNotifierItemPrivate::init(const QString &extraId)
{
    qDBusRegisterMetaType<KDbusImageStruct>();
    qDBusRegisterMetaType<KDbusImageVector>();
    qDBusRegisterMetaType<KDbusToolTipStruct>();

    statusNotifierItemDBus = new KStatusNotifierItemDBus(q);
    q->setAssociatedWidget(qobject_cast<QWidget*>(q->parent()));

    QDBusServiceWatcher *watcher = new QDBusServiceWatcher(s_statusNotifierWatcherServiceName,
                                                           QDBusConnection::sessionBus(),
                                                           QDBusServiceWatcher::WatchForOwnerChange,
                                                           q);
    QObject::connect(watcher, SIGNAL(serviceOwnerChanged(QString,QString,QString)),
                     q, SLOT(serviceChange(QString,QString,QString)));

    //create a default menu, just like in KSystemtrayIcon
    QMenu *m = new QMenu(associatedWidget);

    title = QGuiApplication::applicationDisplayName();
    if (title.isEmpty())
        title = QCoreApplication::applicationName();
    titleAction = m->addSection(qApp->windowIcon(), title);
    m->setTitle(title);
    q->setContextMenu(m);

    QAction *action = new QAction(q);
    action->setText(QObject::tr("Quit"));
    action->setIcon(QIcon::fromTheme("application-exit"));
    QObject::connect(action, SIGNAL(triggered()), q, SLOT(maybeQuit()));
    actionCollection.insert("quit", action);

    id = title;
    if (!extraId.isEmpty()) {
        id.append('_').append(extraId);
    }

    // Init iconThemePath to the app folder for now
    iconThemePath = QStandardPaths::locate(QStandardPaths::DataLocation, "icons", QStandardPaths::LocateDirectory);

    registerToDaemon();
}

void KStatusNotifierItemPrivate::registerToDaemon()
{
    qDebug() << "Registering a client interface to the KStatusNotifierWatcher";
    if (!statusNotifierWatcher) {
        statusNotifierWatcher = new org::kde::StatusNotifierWatcher(s_statusNotifierWatcherServiceName, "/StatusNotifierWatcher",
                                                                    QDBusConnection::sessionBus());
        QObject::connect(statusNotifierWatcher, SIGNAL(StatusNotifierHostRegistered()),
                         q, SLOT(checkForRegisteredHosts()));
        QObject::connect(statusNotifierWatcher, SIGNAL(StatusNotifierHostUnregistered()),
                         q, SLOT(checkForRegisteredHosts()));
    }

    if (statusNotifierWatcher->isValid() &&
        statusNotifierWatcher->property("ProtocolVersion").toInt() == s_protocolVersion) {

        statusNotifierWatcher->RegisterStatusNotifierItem(statusNotifierItemDBus->service());
        setLegacySystemTrayEnabled(false);
    } else {
        qDebug()<<"KStatusNotifierWatcher not reachable";
        setLegacySystemTrayEnabled(true);
    }
}

void KStatusNotifierItemPrivate::serviceChange(const QString &name, const QString &oldOwner, const QString &newOwner)
{
    Q_UNUSED(name)
    if (newOwner.isEmpty()) {
        //unregistered
        qDebug() << "Connection to the KStatusNotifierWatcher lost";
        setLegacyMode(true);
        delete statusNotifierWatcher;
        statusNotifierWatcher = 0;
    } else if (oldOwner.isEmpty()) {
        //registered
       setLegacyMode(false);
    }
}

void KStatusNotifierItemPrivate::checkForRegisteredHosts()
{
    setLegacyMode(!statusNotifierWatcher ||
                  !statusNotifierWatcher->property("IsStatusNotifierHostRegistered").toBool());
}

void KStatusNotifierItemPrivate::setLegacyMode(bool legacy)
{
    if (legacy == (systemTrayIcon != 0)) {
        return;
    }

    if (legacy) {
        //unregistered
        setLegacySystemTrayEnabled(true);
    } else {
        //registered
        registerToDaemon();
    }
}

void KStatusNotifierItemPrivate::legacyWheelEvent(int delta)
{
    statusNotifierItemDBus->Scroll(delta, "vertical");
}

void KStatusNotifierItemPrivate::legacyActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::MiddleClick) {
        emit q->secondaryActivateRequested(systemTrayIcon->geometry().topLeft());
    }
}

void KStatusNotifierItemPrivate::setLegacySystemTrayEnabled(bool enabled)
{
    if (enabled == (systemTrayIcon != 0)) {
        // already in the correct state
        return;
    }

    if (enabled) {
        if (!systemTrayIcon) {
            systemTrayIcon = new KStatusNotifierLegacyIcon(associatedWidget);
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

    if (menu) {
        QMenu *m = menu;
        menu = 0;
        q->setContextMenu(m);
    }
}

void KStatusNotifierItemPrivate::syncLegacySystemTrayIcon()
{
    if (status == KStatusNotifierItem::NeedsAttention) {
        if (!movieName.isNull()) {
            if (!movie) {
                movie = new QMovie(movieName);
            }
            systemTrayIcon->setMovie(movie);
        } else if (!attentionIconName.isNull()) {
            systemTrayIcon->setIcon(QIcon::fromTheme(attentionIconName));
        } else {
            systemTrayIcon->setIcon(attentionIcon);
        }
    } else {
        if (!iconName.isNull()) {
            systemTrayIcon->setIcon(QIcon::fromTheme(iconName));
        } else {
            systemTrayIcon->setIcon(icon);
        }
    }

    systemTrayIcon->setToolTip(toolTipTitle);
}

void KStatusNotifierItemPrivate::contextMenuAboutToShow()
{
    if (!hasQuit && standardActionsEnabled) {
        // we need to add the actions to the menu afterwards so that these items
        // appear at the _END_ of the menu
        menu->addSeparator();
        if (associatedWidget && associatedWidget != menu) {
            QAction *action = actionCollection.value("minimizeRestore");

            if (action) {
                menu->addAction(action);
            }
        }

        QAction *action = actionCollection.value("quit");

        if (action) {
            menu->addAction(action);
        }

        hasQuit = true;
    }

    if (associatedWidget && associatedWidget != menu) {
        QAction* action = actionCollection.value("minimizeRestore");
        if (checkVisibility(QPoint(0, 0), false)) {
            action->setText(QObject::tr("&Restore"));
        } else {
            action->setText(QObject::tr("&Minimize"));
        }
    }
}

void KStatusNotifierItemPrivate::maybeQuit()
{
    QString caption = QGuiApplication::applicationDisplayName();
    if (caption.isEmpty())
        caption = QCoreApplication::applicationName();

    QString query = QObject::tr("<qt>Are you sure you want to quit <b>%1</b>?</qt>", caption.toUtf8());

    if (QMessageBox::question(associatedWidget, 
                              QObject::tr("Confirm Quit From System Tray"), query)) {
        qApp->quit();
    }

}

void KStatusNotifierItemPrivate::minimizeRestore()
{
    q->activate(QPoint(0, 0));
}

void KStatusNotifierItemPrivate::hideMenu()
{
    menu->hide();
}

void KStatusNotifierItemPrivate::minimizeRestore(bool show)
{
#if HAVE_X11
    KWindowInfo info = KWindowSystem::windowInfo(associatedWidget->winId(), NET::WMDesktop | NET::WMFrameExtents);
    if (show) {
        if (onAllDesktops) {
            KWindowSystem::setOnAllDesktops(associatedWidget->winId(), true);
        } else {
            KWindowSystem::setCurrentDesktop(info.desktop());
        }

        associatedWidget->move(info.frameGeometry().topLeft()); // avoid placement policies
        associatedWidget->show();
        associatedWidget->raise();
        KWindowSystem::raiseWindow(associatedWidget->winId());
        KWindowSystem::forceActiveWindow(associatedWidget->winId());
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

KDbusImageStruct KStatusNotifierItemPrivate::imageToStruct(const QImage &image)
{
    KDbusImageStruct icon;
    icon.width = image.size().width();
    icon.height = image.size().height();
    if (image.format() == QImage::Format_ARGB32) {
        icon.data = QByteArray((char*)image.bits(), image.byteCount());
    } else {
        QImage image32 = image.convertToFormat(QImage::Format_ARGB32);
        icon.data = QByteArray((char*)image32.bits(), image32.byteCount());
    }

    //swap to network byte order if we are little endian
    if (QSysInfo::ByteOrder == QSysInfo::LittleEndian) {
        quint32 *uintBuf = (quint32 *) icon.data.data();
        for (uint i = 0; i < icon.data.size()/sizeof(quint32); ++i) {
            *uintBuf = qToBigEndian(*uintBuf);
            ++uintBuf;
        }
    }

    return icon;
}

KDbusImageVector KStatusNotifierItemPrivate::iconToVector(const QIcon &icon)
{
    KDbusImageVector iconVector;

    QPixmap iconPixmap;

    //availableSizes() won't work on KDE::icon
    QList<QSize> allSizes;
    allSizes << QSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall)
             << QSize(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium)
             << QSize(KIconLoader::SizeMedium, KIconLoader::SizeMedium)
             << QSize(KIconLoader::SizeLarge, KIconLoader::SizeLarge);

    //if an icon exactly that size wasn't found don't add it to the vector
    foreach (const QSize &size, allSizes) {
        //hopefully huge and enormous not necessary right now, since it's quite costly
        if (size.width() <= KIconLoader::SizeLarge) {
            iconPixmap = icon.pixmap(size);
            iconVector.append(imageToStruct(iconPixmap.toImage()));
        }
    }

    return iconVector;
}

#include "moc_kstatusnotifieritem.cpp"
#include "moc_kstatusnotifieritemprivate_p.cpp"
