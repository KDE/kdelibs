/* This file is part of the KDE libraries
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                       David Faure <faure@kde.org>
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

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

#include "jobuidelegate.h"

#include <kdebug.h>
#include <kjob.h>
#include <klocalizedstring.h>
#include <kmessagebox.h>
#include <ksharedconfig.h>
#include <QDBusInterface>

#include <QPointer>
#include <QWidget>
#include <QIcon>
#include <QUrl>

#include "kio/scheduler.h"

// Porting helpers. Qt 5: remove
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#define toDisplayString toString
#endif

class KIO::JobUiDelegate::Private
{
public:
};

KIO::JobUiDelegate::JobUiDelegate()
    : d(new Private())
{
}

KIO::JobUiDelegate::~JobUiDelegate()
{
    delete d;
}

/*
  Returns the top most window associated with widget.

  Unlike QWidget::window(), this function does its best to find and return the
  main application window associated with the given widget.

  If widget itself is a dialog or its parent is a dialog, and that dialog has a
  parent widget then this function will iterate through all those widgets to
  find the top most window, which most of the time is the main window of the
  application. By contrast, QWidget::window() would simply return the first
  file dialog it encountered since it is the "next ancestor widget that has (or
  could have) a window-system frame".
*/
static QWidget* topLevelWindow(QWidget* widget)
{
    QWidget* w = widget;
    while (w && w->parentWidget()) {
        w = w->parentWidget();
    }
    return (w ? w->window() : 0);
}

class JobUiDelegateStatic : public QObject
{
    Q_OBJECT
public:
    void registerWindow(QWidget *wid)
    {
        if (!wid)
            return;

        QWidget* window = topLevelWindow(wid);
        QObject *obj = static_cast<QObject *>(window);
        if (!m_windowList.contains(obj)) {
            // We must store the window Id because by the time
            // the destroyed signal is emitted we can no longer
            // access QWidget::winId() (already destructed)
            WId windowId = window->winId();
            m_windowList.insert(obj, windowId);
            connect(window, SIGNAL(destroyed(QObject*)),
                    this, SLOT(slotUnregisterWindow(QObject*)));
            QDBusInterface("org.kde.kded5", "/kded", "org.kde.kded5").
                call(QDBus::NoBlock, "registerWindowId", qlonglong(windowId));
        }
    }
private Q_SLOTS:
    void slotUnregisterWindow(QObject *obj)
    {
        if (!obj)
            return;

        QMap<QObject *, WId>::Iterator it = m_windowList.find(obj);
        if (it == m_windowList.end())
            return;
        WId windowId = it.value();
        disconnect(it.key(), SIGNAL(destroyed(QObject*)),
                   this, SLOT(slotUnregisterWindow(QObject*)));
        m_windowList.erase( it );
        QDBusInterface("org.kde.kded5", "/kded", "org.kde.kded5").
            call(QDBus::NoBlock, "unregisterWindowId", qlonglong(windowId));
    }
private:
    QMap<QObject *, WId> m_windowList;
};

Q_GLOBAL_STATIC(JobUiDelegateStatic, s_static);

void KIO::JobUiDelegate::setWindow(QWidget *window)
{
    KDialogJobUiDelegate::setWindow(window);
    s_static()->registerWindow(window);
}

KIO::RenameDialog_Result KIO::JobUiDelegate::askFileRename(KJob * job,
                                                           const QString & caption,
                                                           const QUrl & src,
                                                           const QUrl & dest,
                                                           KIO::RenameDialog_Mode mode,
                                                           QString& newDest,
                                                           KIO::filesize_t sizeSrc,
                                                           KIO::filesize_t sizeDest,
                                                           time_t ctimeSrc,
                                                           time_t ctimeDest,
                                                           time_t mtimeSrc,
                                                           time_t mtimeDest)
{
    Q_UNUSED(job);
    //kDebug() << "job=" << job;
    // We now do it in process, so that opening the rename dialog
    // doesn't start uiserver for nothing if progressId=0 (e.g. F2 in konq)
    KIO::RenameDialog dlg( window(), caption, src, dest, mode,
                           sizeSrc, sizeDest,
                           ctimeSrc, ctimeDest, mtimeSrc,
                           mtimeDest);
    connect(job, SIGNAL(finished(KJob*)), &dlg, SLOT(reject())); // #192976
    KIO::RenameDialog_Result res = static_cast<RenameDialog_Result>(dlg.exec());
    if (res == R_AUTO_RENAME) {
        newDest = dlg.autoDestUrl().path();
    }
    else {
        newDest = dlg.newDestUrl().path();
    }
    return res;
}

KIO::SkipDialog_Result KIO::JobUiDelegate::askSkip(KJob *job,
                                              bool multi,
                                              const QString & error_text)
{
    // We now do it in process. So this method is a useless wrapper around KIO::open_RenameDialog.
    KIO::SkipDialog dlg( window(), multi, error_text );
    connect(job, SIGNAL(finished(KJob*)), &dlg, SLOT(reject())); // #192976
    return static_cast<KIO::SkipDialog_Result>(dlg.exec());
}

bool KIO::JobUiDelegate::askDeleteConfirmation(const QList<QUrl>& urls,
                                               DeletionType deletionType,
                                               ConfirmationType confirmationType)
{
    QString keyName;
    bool ask = ( confirmationType == ForceConfirmation );
    if (!ask) {
        KSharedConfigPtr kioConfig = KSharedConfig::openConfig("kiorc", KConfig::NoGlobals);

	switch (deletionType ) {
	case Delete:
	    keyName = "ConfirmDelete" ;
	    break;
	case Trash:
	    keyName = "ConfirmTrash" ;
	    break;
	case EmptyTrash:
	    keyName = "ConfirmEmptyTrash" ;
	    break;
	}

        // The default value for confirmations is true (for both delete and trash)
        // If you change this, update kdebase/apps/konqueror/settings/konq/behaviour.cpp
        const bool defaultValue = true;
        ask = kioConfig->group("Confirmations").readEntry(keyName, defaultValue);
    }
    if (ask) {
        QStringList prettyList;
        Q_FOREACH(const QUrl& url, urls) {
            if ( url.scheme() == "trash" ) {
                QString path = url.path();
                // HACK (#98983): remove "0-foo". Note that it works better than
                // displaying KFileItem::name(), for files under a subdir.
                path.remove(QRegExp("^/[0-9]*-"));
                prettyList.append(path);
            } else {
                prettyList.append(url.toDisplayString());
            }
        }

        QWidget* widget = window();
        int result;
        switch(deletionType) {
        case Delete:
            result = KMessageBox::warningContinueCancelList(
                widget,
             	i18np("Do you really want to delete this item?", "Do you really want to delete these %1 items?", prettyList.count()),
             	prettyList,
		i18n("Delete Files"),
		KStandardGuiItem::del(),
		KStandardGuiItem::cancel(),
		keyName, KMessageBox::Notify);
            break;
        case EmptyTrash:
	    result = KMessageBox::warningContinueCancel(
	        widget,
		i18nc("@info", "Do you want to permanently delete all items from Trash? This action cannot be undone."),
		QString(),
		KGuiItem(i18nc("@action:button", "Empty Trash"),
		KDE::icon("user-trash")),
		KStandardGuiItem::cancel(),
		keyName, KMessageBox::Notify);
	    break;
        case Trash:
        default:
            result = KMessageBox::warningContinueCancelList(
                widget,
                i18np("Do you really want to move this item to the trash?", "Do you really want to move these %1 items to the trash?", prettyList.count()),
                prettyList,
		i18n("Move to Trash"),
		KGuiItem(i18nc("Verb", "&Trash"), "user-trash"),
		KStandardGuiItem::cancel(),
		keyName, KMessageBox::Notify);
        }
        if (!keyName.isEmpty()) {
            // Check kmessagebox setting... erase & copy to konquerorrc.
            KSharedConfig::Ptr config = KSharedConfig::openConfig();
            KConfigGroup notificationGroup(config, "Notification Messages");
            if (!notificationGroup.readEntry(keyName, true)) {
                notificationGroup.writeEntry(keyName, true);
                notificationGroup.sync();

                KSharedConfigPtr kioConfig = KSharedConfig::openConfig("kiorc", KConfig::NoGlobals);
                kioConfig->group("Confirmations").writeEntry(keyName, false);
            }
        }
        return (result == KMessageBox::Continue);
    }
    return true;
}

#include "jobuidelegate.moc"
