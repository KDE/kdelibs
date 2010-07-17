/*
 * This file is part of the KDE project.
 *
 * Copyright (C) 2008 Dirk Mueller <mueller@kde.org>
 * Copyright (C) 2008 Urs Wolfer <uwolfer @ kde.org>
 * Copyright (C) 2008 Michael Howell <mhowell123@gmail.com>
 * Copyright (C) 2009,2010 Dawit Alemayehu <adawit@kde.org>
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
 *
 */

// Own
#include "kwebpage.h"
#include "kwebwallet.h"

// Local
#include "kwebpluginfactory.h"

// KDE
#include <kaction.h>
#include <kfiledialog.h>
#include <kprotocolmanager.h>
#include <kjobuidelegate.h>
#include <krun.h>
#include <kstandarddirs.h>
#include <kstandardshortcut.h>
#include <kurl.h>
#include <kdebug.h>
#include <klocalizedstring.h>
#include <kio/accessmanager.h>
#include <kio/job.h>
#include <kio/renamedialog.h>

// Qt
#include <QtCore/QPointer>
#include <QtCore/QFileInfo>
#include <QtWebKit/QWebFrame>
#include <QtNetwork/QNetworkReply>


#define QL1S(x)  QLatin1String(x)
#define QL1C(x)  QLatin1Char(x)

static KUrl promptUser (QWidget *parent, const KUrl& url, const QString& suggestedName)
{
    KUrl destUrl;
    int result = KIO::R_OVERWRITE;
    const QUrl fileName ((suggestedName.isEmpty() ? url.fileName() : suggestedName));

    do {
        destUrl = KFileDialog::getSaveFileName(fileName, QString(), parent);

        if (destUrl.isLocalFile()) {
            QFileInfo finfo (destUrl.toLocalFile());
            if (finfo.exists()) {
                QDateTime now = QDateTime::currentDateTime();
                KIO::RenameDialog dlg (parent, i18n("Overwrite File?"), url, destUrl,
                                       KIO::RenameDialog_Mode(KIO::M_OVERWRITE | KIO::M_SKIP),
                                       -1, finfo.size(),
                                       now.toTime_t(), finfo.created().toTime_t(),
                                       now.toTime_t(), finfo.lastModified().toTime_t());
                result = dlg.exec();
            }
        }
    } while (result == KIO::R_CANCEL && destUrl.isValid());

    return destUrl;
}

static bool downloadResource (const KUrl& srcUrl, const KIO::MetaData& metaData = KIO::MetaData(),
                              QWidget* parent = 0, const QString& suggestedName = QString())
{
    const KUrl& destUrl = promptUser(parent, srcUrl, suggestedName);

    if (destUrl.isValid()) {
        KIO::Job *job = KIO::file_copy(srcUrl, destUrl, -1, KIO::Overwrite);

        if (!metaData.isEmpty())
            job->setMetaData(metaData);

        job->addMetaData(QL1S("MaxCacheSize"), QL1S("0")); // Don't store in http cache.
        job->addMetaData(QL1S("cache"), QL1S("cache")); // Use entry from cache if available.
        job->uiDelegate()->setAutoErrorHandlingEnabled(true);
        return true;
    }
    return false;
}

class KWebPage::KWebPagePrivate
{
public:
    QPointer<KWebWallet> wallet;
};

KWebPage::KWebPage(QObject *parent, Integration flags)
         :QWebPage(parent), d(new KWebPagePrivate)
{ 
    // KDE KParts integration for <embed> tag...
    if (!flags || (flags & KPartsIntegration))
        setPluginFactory(new KWebPluginFactory(this));

    WId windowId = 0;
    QWidget *widget = qobject_cast<QWidget*>(parent);
    if (widget && widget->window())
        windowId = widget->window()->winId();

    // KDE IO (KIO) integration...
    if (!flags || (flags & KIOIntegration)) {
        KIO::Integration::AccessManager *manager = new KIO::Integration::AccessManager(this);
        // Disable QtWebKit's internal cache to avoid duplication with the one in KIO...
        manager->setCache(0);
        manager->setCookieJarWindowId(windowId);
        setNetworkAccessManager(manager);
    }

    // KWallet integration...
    if (!flags || (flags & KWalletIntegration))
        setWallet(new KWebWallet(0, windowId));

    action(Back)->setIcon(KIcon("go-previous"));
    action(Forward)->setIcon(KIcon("go-next"));
    action(Reload)->setIcon(KIcon("view-refresh"));
    action(Stop)->setIcon(KIcon("process-stop"));
    action(Cut)->setIcon(KIcon("edit-cut"));
    action(Copy)->setIcon(KIcon("edit-copy"));
    action(Paste)->setIcon(KIcon("edit-paste"));
    action(Undo)->setIcon(KIcon("edit-undo"));
    action(Redo)->setIcon(KIcon("edit-redo"));
    action(InspectElement)->setIcon(KIcon("view-process-all"));
    action(OpenLinkInNewWindow)->setIcon(KIcon("window-new"));
    action(OpenFrameInNewWindow)->setIcon(KIcon("window-new"));
    action(OpenImageInNewWindow)->setIcon(KIcon("window-new"));
    action(CopyLinkToClipboard)->setIcon(KIcon("edit-copy"));
    action(CopyImageToClipboard)->setIcon(KIcon("edit-copy"));
    action(ToggleBold)->setIcon(KIcon("format-text-bold"));
    action(ToggleItalic)->setIcon(KIcon("format-text-italic"));
    action(ToggleUnderline)->setIcon(KIcon("format-text-underline"));
    action(DownloadLinkToDisk)->setIcon(KIcon("document-save"));
    action(DownloadImageToDisk)->setIcon(KIcon("document-save"));

    settings()->setWebGraphic(QWebSettings::MissingPluginGraphic, KIcon("preferences-plugin").pixmap(32, 32));
    settings()->setWebGraphic(QWebSettings::MissingImageGraphic, KIcon("image-missing").pixmap(32, 32));
    settings()->setWebGraphic(QWebSettings::DefaultFrameIconGraphic, KIcon("applications-internet").pixmap(32, 32));

    action(Back)->setShortcut(KStandardShortcut::back().primary());
    action(Forward)->setShortcut(KStandardShortcut::forward().primary());
    action(Reload)->setShortcut(KStandardShortcut::reload().primary());
    action(Stop)->setShortcut(QKeySequence(Qt::Key_Escape));
    action(Cut)->setShortcut(KStandardShortcut::cut().primary());
    action(Copy)->setShortcut(KStandardShortcut::copy().primary());
    action(Paste)->setShortcut(KStandardShortcut::paste().primary());
    action(Undo)->setShortcut(KStandardShortcut::undo().primary());
    action(Redo)->setShortcut(KStandardShortcut::redo().primary());
    action(SelectAll)->setShortcut(KStandardShortcut::selectAll().primary());
}

KWebPage::~KWebPage()
{
    delete d;
}

bool KWebPage::isExternalContentAllowed() const
{
    KIO::AccessManager *manager = qobject_cast<KIO::AccessManager*>(networkAccessManager());
    if (manager)
        return manager->isExternalContentAllowed();
    return true;
}

KWebWallet *KWebPage::wallet() const
{
    return d->wallet;
}

void KWebPage::setAllowExternalContent(bool allow)
{
    KIO::AccessManager *manager = qobject_cast<KIO::AccessManager*>(networkAccessManager());
    if (manager)
        manager->setExternalContentAllowed(allow);
}

void KWebPage::setWallet(KWebWallet* wallet)
{
    // Delete the current wallet if this object is its parent...
    if (d->wallet && this == d->wallet->parent())
        delete d->wallet;

    d->wallet = wallet;

    if (d->wallet)
        d->wallet->setParent(this);
}

void KWebPage::downloadRequest(const QNetworkRequest &request)
{
    downloadResource(request.url(),
                     request.attribute(static_cast<QNetworkRequest::Attribute>(KIO::AccessManager::MetaData)).toMap(),
                     view());
}

void KWebPage::downloadUrl(const KUrl &url)
{
    downloadResource(url, KIO::MetaData(), view());
}

void KWebPage::downloadResponse(QNetworkReply *reply)
{
    // FIXME: Remove the next line of code once support for putting an ioslave is
    // implemented in KIO::AccessManager which is waiting for an upstream fix.
    // See https://bugs.webkit.org/show_bug.cgi?id=37880.
    reply->abort();

    QString suggestedFileName;
    if (reply && reply->hasRawHeader("Content-Disposition")) {
        KIO::MetaData metaData = reply->attribute(static_cast<QNetworkRequest::Attribute>(KIO::AccessManager::MetaData)).toMap();
        if (metaData.value(QL1S("content-disposition-type")).compare(QL1S("attachment"), Qt::CaseInsensitive) == 0) {
            suggestedFileName = metaData.value(QL1S("content-disposition-filename"));
        } else {
            const QString value = QL1S(reply->rawHeader("Content-Disposition").simplified());
            if (value.startsWith(QL1S("attachment"), Qt::CaseInsensitive)) {
                const int length = value.size();
                int pos = value.indexOf(QL1S("filename"), 0, Qt::CaseInsensitive);
                if (pos > -1) {
                    pos += 9;
                    while (pos < length && (value.at(pos) == QL1C(' ') || value.at(pos) == QL1C('=') || value.at(pos) == QL1C('"')))
                        pos++;

                    int endPos = pos;
                    while (endPos < length && value.at(endPos) != QL1C('"') && value.at(endPos) != QL1C(';'))
                        endPos++;

                    if (endPos > pos) {
                        suggestedFileName = value.mid(pos, (endPos-pos)).trimmed();
                    }
                }
            }
        }
    }

    downloadResource(reply->url(), KIO::MetaData(), this->view(), suggestedFileName);
}

QString KWebPage::sessionMetaData(const QString &key) const
{
    QString value;

    KIO::Integration::AccessManager *manager = qobject_cast<KIO::Integration::AccessManager *>(networkAccessManager());
    if (manager)
        value = manager->sessionMetaData().value(key);

    return value;
}

QString KWebPage::requestMetaData(const QString &key) const
{
    QString value;

    KIO::Integration::AccessManager *manager = qobject_cast<KIO::Integration::AccessManager *>(networkAccessManager());
    if (manager)
        value = manager->requestMetaData().value(key);

    return value;
}

void KWebPage::setSessionMetaData(const QString &key, const QString &value)
{
    KIO::Integration::AccessManager *manager = qobject_cast<KIO::Integration::AccessManager *>(networkAccessManager());
    if (manager)
        manager->sessionMetaData()[key] = value;
}

void KWebPage::setRequestMetaData(const QString &key, const QString &value)
{
    KIO::Integration::AccessManager *manager = qobject_cast<KIO::Integration::AccessManager *>(networkAccessManager());
    if (manager)
        manager->requestMetaData()[key] = value;
}

void KWebPage::removeSessionMetaData(const QString &key)
{
    KIO::Integration::AccessManager *manager = qobject_cast<KIO::Integration::AccessManager *>(networkAccessManager());
    if (manager)
        manager->sessionMetaData().remove(key);
}

void KWebPage::removeRequestMetaData(const QString &key)
{
    KIO::Integration::AccessManager *manager = qobject_cast<KIO::Integration::AccessManager *>(networkAccessManager());
    if (manager)
        manager->requestMetaData().remove(key);
}

QString KWebPage::userAgentForUrl(const QUrl& _url) const
{
    const KUrl url(_url);
    const QString userAgent = KProtocolManager::userAgentForHost((url.isLocalFile() ? QL1S("localhost") : url.host()));

    if (userAgent == KProtocolManager::defaultUserAgent())
        return QWebPage::userAgentForUrl(_url);

    return userAgent;
}

bool KWebPage::acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type)
{
    kDebug(800) << "url: " << request.url() << ", type: " << type << ", frame: " << frame;

    if (frame && d->wallet && type == QWebPage::NavigationTypeFormSubmitted) {
        d->wallet->saveFormData(frame);
    }

    /*
        If the navigation request is from the main frame, set the cross-domain
        meta-data value to the current url for proper integration with KCookieJar...
      */
    if (frame == mainFrame() && type != QWebPage::NavigationTypeReload) {
        setSessionMetaData(QL1S("cross-domain"), request.url().toString());
    }

    return QWebPage::acceptNavigationRequest(frame, request, type);
}

#include "kwebpage.moc"
