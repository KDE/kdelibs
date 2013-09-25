/*
 * This file is part of the KDE project.
 *
 * Copyright (C) 2008 Dirk Mueller <mueller@kde.org>
 * Copyright (C) 2008 Urs Wolfer <uwolfer @ kde.org>
 * Copyright (C) 2008 Michael Howell <mhowell123@gmail.com>
 * Copyright (C) 2009,2010 Dawit Alemayehu <adawit @ kde.org>
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
#ifndef KWEBPAGE_H
#define KWEBPAGE_H

#include <kdewebkit_export.h>

#include <QWebPage>

class KWebWallet;
class KJob;

namespace KIO {
    class MetaData;
    class Job;
}

/**
 * @short An enhanced QWebPage that provides integration into the KDE environment.
 *
 * This is a convenience class that provides full integration with KDE
 * technologies such as KIO for network request handling, KCookiejar for cookie
 * handling, KParts for embedding non-html content and KWallet for storing
 * form data.  It also sets standard icons for many of the actions provided by
 * QWebPage.
 *
 * Most of this integration happens behind the scenes.  If you want KWallet
 * integration, however, you will have to provide a mechanism for deciding
 * whether to allow form data to be stored.  To do this, you will need to
 * connect to the KWebWallet::saveFormDataRequested signal and call either
 * KWebWallet::acceptSaveFormDataRequest or
 * KWebWallet::rejectSaveFormDataRequest, typically after asking the user
 * whether they want to save the form data.  If you do not do this, no form
 * data will be saved.
 *
 * KWebPage will also not automatically load form data for you.  You should
 * connect to QWebPage::loadFinished and, if the page was loaded successfully,
 * call
 * @code
 * page->wallet()->fillFormData(page->mainFrame());
 * @endcode
 *
 * @see KIO::Integration
 * @see KWebWallet
 *
 * @author Urs Wolfer <uwolfer @ kde.org>
 * @author Dawit Alemayehu <adawit @ kde.org>
 *
 * @since 4.4
 */

class KDEWEBKIT_EXPORT KWebPage : public QWebPage
{
    Q_OBJECT
    Q_FLAGS (Integration)

public:
    /**
     * Flags for setting the desired level of integration.
     */
    enum IntegrationFlags
    {
        /**
         * Provide only very basic integration such as using KDE icons for the
         * actions provided by QWebPage.
         */
        NoIntegration = 0x01,
        /**
         * Use KIO to handle network requests.
         *
         * @see KIO::Integration::AccessManager
         */
        KIOIntegration = 0x02,
        /**
         * Use KPart componenets, if available, to display content in
         * &lt;embed&gt; and &lt;object&gt; tags.
         */
        KPartsIntegration = 0x04,
        /**
         * Use KWallet to store login credentials and other form data from web
         * sites.
         *
         * @see wallet() and setWallet()
         */
        KWalletIntegration = 0x08
    };
    Q_DECLARE_FLAGS(Integration, IntegrationFlags)

    /**
     * Constructs a KWebPage with parent @p parent.
     *
     * Note that if no integration flags are set (the default), all integration
     * options are activated.  If you inherit from this class you can use the
     * flags in @ref IntegrationFlags to control how much integration should
     * be used.
     *
     * @see KIO::Integration::CookieJar
     * @see KIO::Integration::AccessManager
     * @see wallet() and setWallet()
     */
    explicit KWebPage(QObject *parent = 0, Integration flags = Integration());

    /**
     * Destroys the KWebPage.
     */
    ~KWebPage();

    /**
     * Whether access to remote content is permitted.
     *
     * If this is @c false, only resources on the local system can be accessed
     * through this web page.  By default access to remote content is allowed.
     *
     * If KIO integration is disabled, this will always return @c true.
     *
     * @see setAllowExternalContent()
     * @see KIO::Integration::AccessManager::isExternalContentAllowed()
     *
     * @return @c true if access to remote content is permitted, @c false otherwise
     */
    bool isExternalContentAllowed() const;

    /**
     * The wallet integration manager.
     *
     * If you wish to use KDE wallet integration, you will have to connect to
     * signals emitted by this object and react accordingly.  See KWebWallet
     * for more information.
     *
     * @return the wallet integration manager, or 0 if KDE wallet integration
     *         is disabled
     */
    KWebWallet *wallet() const;

    /**
     * Set whether to allow remote content.
     *
     * If KIO integration is not enabled, this method will have no effect.
     *
     * @see isExternalContentAllowed()
     * @see KIO::Integration::AccessManager::setAllowExternalContent(bool)
     *
     * @param allow  @c true if access to remote content should be allowed,
     *               @c false if only local content should be accessible
     */
    void setAllowExternalContent(bool allow);

    /**
     * Set the @ref KWebWallet that is used to store form data.
     *
     * This KWebPage will take ownership of @p wallet, so that the wallet
     * is deleted when the KWebPage is deleted.  If you do not want that
     * to happen, you should call setParent() on @p wallet after calling
     * this function.
     *
     * @see KWebWallet
     *
     * @param wallet  the KWebWallet to be used for storing form data, or
     *                0 to disable KWallet integration
     */
    void setWallet(KWebWallet* wallet);

public Q_SLOTS:
    /**
     * Download @p request using KIO.
     *
     * This slot first prompts the user where to save the requested
     * resource and then downloads it using KIO.
     */
    virtual void downloadRequest(const QNetworkRequest &request);

    /**
     * Download @p url using KIO.
     *
     * This slot first prompts the user where to save the requested
     * resource and then downloads it using KIO.
     */
    virtual void downloadUrl(const QUrl &url);

    /**
     * Download the resource specified by @p reply using KIO.
     *
     * This slot first prompts the user where to save the requested resource
     * and then downloads it using KIO.
     *
     * In KDE 4.8 and higher, if @p reply contains a QObject property called
     * "DownloadManagerExe", then an attempt will be made to the command
     * specified by that property to download the specified resource.
     *
     * If the "DownloadManagerExe" property is not defined or the command
     * specified by it could not be successfully executed, then the user will
     * be prompted for the action to take.
     *
     * @since 4.5
     * @see handleReply
     */
    void downloadResponse(QNetworkReply *reply);

protected:
    /**
     * Get an item of session metadata.
     *
     * Retrieves the value of the permanent (per-session) metadata for @p key.
     *
     * If KIO integration is disabled, this will always return an empty string.
     *
     * @see KIO::Integration::AccessManager::sessionMetaData
     * @see setSessionMetaData
     *
     * @param key  the key of the metadata to retrieve
     * @return     the value of the metadata associated with @p key, or an
     *             empty string if there is no such metadata
     */
    QString sessionMetaData(const QString &key) const;

    /**
     * Get an item of request metadata.
     *
     * Retrieves the value of the temporary (per-request) metadata for @p key.
     *
     * If KIO integration is disabled, this will always return an empty string.
     *
     * @see KIO::Integration::AccessManager::requestMetaData
     * @see setRequestMetaData
     *
     * @param key  the key of the metadata to retrieve
     * @return     the value of the metadata associated with @p key, or an
     *             empty string if there is no such metadata
     */
    QString requestMetaData(const QString &key) const;

    /**
     * Set an item of metadata to be sent to the KIO slave with every request.
     *
     * If KIO integration is disabled, this method will have no effect.
     *
     * Metadata set using this method will be sent with every request.
     *
     * @see KIO::Integration::AccessManager::sessionMetaData
     *
     * @param key    the key for the metadata; any existing metadata associated
     *               with this key will be overwritten
     * @param value  the value to associate with @p key
     */
    void setSessionMetaData(const QString &key, const QString &value);

    /**
     * Set an item of metadata to be sent to the KIO slave with the next request.
     *
     * If KIO integration is disabled, this method will have no effect.
     *
     * Metadata set using this method will be deleted after it has been sent
     * once.
     *
     * @see KIO::Integration::AccessManager::requestMetaData
     *
     * @param key    the key for the metadata; any existing metadata associated
     *               with this key will be overwritten
     * @param value  the value to associate with @p key
     */
    void setRequestMetaData(const QString &key, const QString &value);

    /**
     * Remove an item of session metadata.
     *
     * Removes the permanent (per-session) metadata associated with @p key.
     *
     * @see KIO::Integration::AccessManager::sessionMetaData
     * @see setSessionMetaData
     *
     * @param key  the key for the metadata to remove
     */
    void removeSessionMetaData(const QString &key);

    /**
     * Remove an item of request metadata.
     *
     * Removes the temporary (per-request) metadata associated with @p key.
     *
     * @see KIO::Integration::AccessManager::requestMetaData
     * @see setRequestMetaData
     *
     * @param key  the key for the metadata to remove
     */
    void removeRequestMetaData(const QString &key);

    /**
     * @reimp
     *
     * This function is re-implemented to provide KDE user-agent management
     * integration through KProtocolManager.
     *
     * If a special user-agent has been configured for the host indicated by
     * @p url, that user-agent will be returned.  Otherwise, QWebPage's
     * default user agent is returned.
     *
     * @see KProtocolManager::userAgentForHost.
     * @see QWebPage::userAgentForUrl.
     */
    virtual QString userAgentForUrl(const QUrl& url) const;

    /**
     * @reimp
     *
     * This performs various integration-related actions when navigation is
     * requested.  If you override this method, make sure you call the parent's
     * implementation unless you want to block the request outright.
     *
     * If you do override acceptNavigationRequest and call this method,
     * however, be aware of the effect of the page's linkDelegationPolicy on
     * how * QWebPage::acceptNavigationRequest behaves.
     *
     * @see QWebPage::acceptNavigationRequest
     */
    virtual bool acceptNavigationRequest(QWebFrame * frame, const QNetworkRequest & request, NavigationType type);

    /**
     * Attempts to handle @p reply and returns true on success, false otherwise.
     *
     * In KDE 4.8 and higher, if @p reply contains a QObject property called
     * "DownloadManagerExe", then an attempt will be made to let the command
     * specified by that property to download the requested resource.
     *
     * If the "DownloadManagerExe" property is not defined or the command
     * specified by it could not be successfully executed, then the user will
     * be prompted for the action to take.
     *
     * @param reply        the QNetworkReply object to be handled.
     * @param contentType  if not null, it will be set to the content-type specified in @p reply, if any.
     * @param metaData     if not null, it will be set to the KIO meta-data specified in @p reply, if any.
     * @since 4.6.3
     */
    bool handleReply (QNetworkReply* reply, QString* contentType = 0, KIO::MetaData* metaData = 0);

private:
    class KWebPagePrivate;
    KWebPagePrivate* const d;
    Q_PRIVATE_SLOT(d, void _k_copyResultToTempFile(KJob*))
    Q_PRIVATE_SLOT(d, void _k_receivedContentType(KIO::Job*, const QString&))
    Q_PRIVATE_SLOT(d, void _k_contentTypeCheckFailed(KJob*))
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KWebPage::Integration)

#endif // KWEBPAGE_H
