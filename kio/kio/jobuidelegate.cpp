/* This file is part of the KDE libraries
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                       David Faure <faure@kde.org>
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>
    Copyright (C) 2013 Dawit Alemayehu <adawit@kde.org>

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
#include <klocale.h>
#include <kmessagebox.h>
#include <ksharedconfig.h>
#include <ksslinfodialog.h>
#include <kmessage.h>

#include <QPointer>
#include <QWidget>

#include "kio/scheduler.h"

#if defined Q_WS_X11
#include <QX11Info>
#include <netwm.h>
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

void KIO::JobUiDelegate::setWindow(QWidget *window)
{
    KDialogJobUiDelegate::setWindow(window);
    KIO::Scheduler::registerWindow(window);
}

KIO::RenameDialog_Result KIO::JobUiDelegate::askFileRename(KJob * job,
                                                           const QString & caption,
                                                           const QString& src,
                                                           const QString & dest,
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
    dlg.setWindowModality(Qt::WindowModal);
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
    dlg.setWindowModality(Qt::WindowModal);
    connect(job, SIGNAL(finished(KJob*)), &dlg, SLOT(reject())); // #192976
    return static_cast<KIO::SkipDialog_Result>(dlg.exec());
}

bool KIO::JobUiDelegate::askDeleteConfirmation(const KUrl::List& urls,
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
        Q_FOREACH(const KUrl& url, urls) {
            if ( url.protocol() == "trash" ) {
                QString path = url.path();
                // HACK (#98983): remove "0-foo". Note that it works better than
                // displaying KFileItem::name(), for files under a subdir.
                path.remove(QRegExp("^/[0-9]*-"));
                prettyList.append(path);
            } else {
                prettyList.append(url.pathOrUrl());
            }
        }

        int result;
        QWidget* widget = window();
        const KMessageBox::Options options = KMessageBox::Notify | KMessageBox::WindowModal;
        switch(deletionType) {
        case Delete:
            result = KMessageBox::warningContinueCancelList(
                widget,
             	i18np("Do you really want to delete this item?", "Do you really want to delete these %1 items?", prettyList.count()),
             	prettyList,
		i18n("Delete Files"),
		KStandardGuiItem::del(),
		KStandardGuiItem::cancel(),
		keyName, options);
            break;
        case EmptyTrash:
	    result = KMessageBox::warningContinueCancel(
	        widget,
		i18nc("@info", "Do you want to permanently delete all items from Trash? This action cannot be undone."),
		QString(),
		KGuiItem(i18nc("@action:button", "Empty Trash"),
		KIcon("user-trash")),
		KStandardGuiItem::cancel(),
		keyName, options);
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
		keyName, options);
        }
        if (!keyName.isEmpty()) {
            // Check kmessagebox setting... erase & copy to konquerorrc.
            KSharedConfig::Ptr config = KGlobal::config();
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

int KIO::JobUiDelegate::requestMessageBox(KIO::JobUiDelegate::MessageBoxType type,
                                          const QString& text, const QString& caption,
                                          const QString& buttonYes, const QString& buttonNo,
                                          const QString& iconYes, const QString& iconNo,
                                          const QString& dontAskAgainName,
                                          const KIO::MetaData& sslMetaData)
{
    int result = -1;

    //kDebug() << type << text << "caption=" << caption;

    KConfig config("kioslaverc");
    KMessageBox::setDontShowAskAgainConfig(&config);

    const KGuiItem buttonYesGui (buttonYes, iconYes);
    const KGuiItem buttonNoGui (buttonNo, iconNo);
    KMessageBox::Options options = (KMessageBox::Notify | KMessageBox::WindowModal);

    switch (type) {
    case QuestionYesNo:
        result = KMessageBox::questionYesNo(
                    window(), text, caption, buttonYesGui,
                    buttonNoGui, dontAskAgainName, options);
        break;
    case WarningYesNo:
        result = KMessageBox::warningYesNo(
                    window(), text, caption, buttonYesGui,
                    buttonNoGui, dontAskAgainName,
                    options | KMessageBox::Dangerous);
        break;
    case WarningYesNoCancel:
        result = KMessageBox::warningYesNoCancel(
                    window(), text, caption, buttonYesGui, buttonNoGui,
                    KStandardGuiItem::cancel(), dontAskAgainName, options);
        break;
    case WarningContinueCancel:
        result = KMessageBox::warningContinueCancel(
                    window(), text, caption, buttonYesGui,
                    KStandardGuiItem::cancel(), dontAskAgainName, options);
        break;
    case Information:
        KMessageBox::information(window(), text, caption, dontAskAgainName, options);
        result = 1; // whatever
        break;
    case SSLMessageBox:
    {
        QPointer<KSslInfoDialog> kid (new KSslInfoDialog(window()));
        //### this is boilerplate code and appears in khtml_part.cpp almost unchanged!
        const QStringList sl = sslMetaData.value(QLatin1String("ssl_peer_chain")).split('\x01', QString::SkipEmptyParts);
        QList<QSslCertificate> certChain;
        bool decodedOk = true;
        foreach (const QString &s, sl) {
            certChain.append(QSslCertificate(s.toLatin1())); //or is it toLocal8Bit or whatever?
            if (certChain.last().isNull()) {
                decodedOk = false;
                break;
            }
        }

        if (decodedOk) {
            result = 1; // whatever
            kid->setSslInfo(certChain,
                            sslMetaData.value(QLatin1String("ssl_peer_ip")),
                            text, // the URL
                            sslMetaData.value(QLatin1String("ssl_protocol_version")),
                            sslMetaData.value(QLatin1String("ssl_cipher")),
                            sslMetaData.value(QLatin1String("ssl_cipher_used_bits")).toInt(),
                            sslMetaData.value(QLatin1String("ssl_cipher_bits")).toInt(),
                            KSslInfoDialog::errorsFromString(sslMetaData.value(QLatin1String("ssl_cert_errors"))));
            kid->exec();
        } else {
            result = -1;
            KMessageBox::information(window(),
                                     i18n("The peer SSL certificate chain appears to be corrupt."),
                                     i18n("SSL"), QString(), options);
        }
        // KSslInfoDialog deletes itself (Qt::WA_DeleteOnClose).
        delete kid;
        break;
    }
    default:
        kWarning() << "Unknown type" << type;
        result = 0;
        break;
    }
    KMessageBox::setDontShowAskAgainConfig(0);
    return result;
}

#include "jobuidelegate.moc"
