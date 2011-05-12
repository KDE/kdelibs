/*
 * This file is part of the KDE project.
 *
 * Copyright (C) 2008 - 2009 Urs Wolfer <uwolfer @ kde.org>
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

#ifndef KIO_ACCESSMANAGER_H
#define KIO_ACCESSMANAGER_H

#include <kio/global.h>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkCookieJar>

class QWidget;

namespace KIO {

/**
 * @short A KDE implementation of QNetworkAccessManager.
 *
 * Use this class instead of QNetworkAccessManager if you want to integrate
 * with KDE's KIO and KCookieJar modules for network operations and cookie
 * handling respectively.
 *
 * Here is a simple example that shows how to set the QtWebKit module to use KDE's
 * KIO for its network operations:
 * @code
 *   QWebView *view = new QWebView(this);
 *   KIO::Integration::AccessManager *manager = new KIO::Integration::AccessManager(view);
 *   view->page()->setNetworkAccessManager(manager);
 * @endcode
 *
 * To access member functions in the cookiejar class at a later point in your
 * code simply downcast the pointer returned by QWebPage::networkAccessManager
 * as follows:
 * @code
 *   KIO::Integration::AccessManager *manager = qobject_cast<KIO::Integration::AccessManager*>(view->page()->accessManager());
 * @endcode
 *
 * Please note that this class is in the KIO namespace for backward compatablity.
 * You should use KIO::Integration::AccessManager to access this class in your
 * code.
 *
 * <b>IMPORTANT</b>This class is not a replacement for the standard KDE API.
 * It should ONLY be used to provide KDE integration in applications that
 * cannot use the standard KDE API directly.
 *
 * @author Urs Wolfer \<uwolfer @ kde.org\>
 * @author Dawit Alemayehu \<adawit @ kde.org\>
 *
 * @deprecated Use the KIO::Integration::AccessManager typedef to access this class instead.
 * @since 4.3
 */
class KIO_EXPORT AccessManager : public QNetworkAccessManager
{
    Q_OBJECT
public:
    /*!
     * Extensions to QNetworkRequest::Attribute enums.
     * @since 4.3.2
     */
    enum Attribute {
        MetaData = QNetworkRequest::User, /** < Used to send KIO MetaData back and forth. type: QVariant::Map. */
        KioError /**< Used to send KIO error codes that cannot be mapped into QNetworkReply::NetworkError. type: QVariant::Int */
    };

    /**
     * Constructor
     */
    AccessManager(QObject *parent);

    /**
     * Destructor
     */
    virtual ~AccessManager();

    /**
     * Set @p allowed to false if you don't want any external content to be fetched.
     * By default external content is fetched.
     */
    void setExternalContentAllowed(bool allowed);

    /**
     * Returns true if external content is going to be fetched.
     *
     * @see setExternalContentAllowed
     */
    bool isExternalContentAllowed() const;

    /**
     * Sets the cookiejar's window id to @p id.
     *
     * This is a convenience function that allows you to set the cookiejar's
     * window id. Note that this function does nothing unless the cookiejar in
     * use is of type KIO::Integration::CookieJar.
     *
     * By default the cookiejar's window id is set to false. Make sure you call
     * this function and set the window id to its proper value when create an
     * instance of this object. Otherwise, the KDE cookiejar will not be able
     * to properly manage session based cookies.
     *
     * @see KIO::Integration::CookieJar::setWindowId.
     * @since 4.4
     * @deprecated Use setWindow
     */
#ifndef KDE_NO_DEPRECATED
    KDE_DEPRECATED void setCookieJarWindowId(WId id);
#endif

    /**
     * Sets the window associated with this network access manager.
     *
     * Note that @p widget will be used as a parent for dialogs in KIO as well
     * as the cookie jar. If @p widget is not a window, this function will
     * invoke @ref QWidget::window() to obtain the window for the given widget.
     *
     * @see KIO::Integration::CookieJar::setWindow.
     * @since 4.7
     */
    void setWindow(QWidget* widget);

    /**
     * Returns the cookiejar's window id.
     *
     * This is a convenience function that returns the window id associated
     * with the cookiejar. Note that this function will return a 0 if the
     * cookiejar is not of type KIO::Integration::CookieJar or a window id
     * has not yet been set.
     *
     * @see KIO::Integration::CookieJar::windowId.
     * @since 4.4
     * @deprecated Use KIO::Integration::CookieJar::windowId
     */
#ifndef KDE_NO_DEPRECATED
    KDE_DEPRECATED WId cookieJarWindowid() const;
#endif

    /**
     * Returns the window associated with this network access manager.
     *
     * @see setWindow
     * @since 4.7
     */
    QWidget* window() const;

    /**
     * Returns a reference to the temporary meta data container.
     *
     * See kdelibs/kio/DESIGN.metadata for list of supported KIO meta data.
     *
     * Use this function when you want to set per request KIO meta data that
     * will be removed after it has been sent once.
     *
     * @since 4.4
     */
    KIO::MetaData& requestMetaData();

    /**
     * Returns a reference to the persistent meta data container.
     *
     * See kdelibs/kio/DESIGN.metadata for list of supported KIO meta data.
     *
     * Use this function when you want to set per session KIO meta data that
     * will be sent with every request.
     *
     * Unlike @p requestMetaData, the meta data values set using the reference
     * returned by this function will not be deleted and will be sent with every
     * request.
     *
     * @since 4.4
     */
    KIO::MetaData& sessionMetaData();

    /**
     * Puts the ioslave associated with the given @p reply on hold.
     *
     * This function is intended to make possible the implementation of
     * the special case mentioned in KIO::get's documentation within the
     * KIO-QNAM integration.
     *
     * @see KIO::get.
     * @since 4.6
     */
    static void putReplyOnHold(QNetworkReply* reply);

    /**
     * Sets the network reply object to emit readyRead when it receives meta data.
     *
     * Meta data is any information that is not the actual content itself, e.g.
     * HTTP response headers of the HTTP protocol. You should only call this
     * function, when an application does not connect to a reply object's
     * metaDataChanged signal.
     *
     * @see QNetworkReply::metaDataChanged
     * @since 4.7
     */
    void setEmitReadyReadOnMetaDataChange(bool);

protected:
    /**
     * Reimplemented for internal reasons, the API is not affected.
     *
     * @see QNetworkAccessManager::createRequest
     * @internal
     */
    virtual QNetworkReply *createRequest(Operation op, const QNetworkRequest &req, QIODevice *outgoingData = 0);

private:
    class AccessManagerPrivate;
    AccessManagerPrivate* const d;
};

namespace Integration {
// KDE5: Move AccessManager into the KIO::Integration namespace.
typedef KIO::AccessManager AccessManager;

/**
 * Maps KIO SSL meta data into the given QSslConfiguration object.
 *
 * @since 4.5
 * @return true if @p metadata contains ssl information and the mapping succeeded.
 */
KIO_EXPORT bool sslConfigFromMetaData(const KIO::MetaData& metadata, QSslConfiguration& sslconfig);

/**
 * @short A KDE implementation of QNetworkCookieJar.
 *
 * Use this class in place of QNetworkCookieJar if you want to integrate with
 * KDE's cookiejar instead of the one that comes with Qt.
 *
 * Here is a simple example that shows how to set the QtWebKit module to use KDE's
 * cookiejar:
 * @code
 *   QWebView *view = new QWebView(this);
 *   KIO::Integration::CookieJar *cookieJar = new KIO::Integration::CookieJar;
 *   cookieJar->setWindowId(view->window()->winId());
 *   view->page()->networkAccessManager()->setCookieJar(cookieJar);
 * @endcode
 *
 * To access member functions in the cookiejar class at a later point in your
 * code simply downcast the pointer returned by QNetworkAccessManager::cookieJar
 * as follows:
 * @code
 *   KIO::Integration::CookieJar *cookieJar = qobject_cast<KIO::Integration::CookieJar*>(view->page()->accessManager()->cookieJar());
 * @endcode
 *
 * <b>IMPORTANT</b>This class is not a replacement for the standard KDE API.
 * It should ONLY be used to to provide KDE integration in applications that
 * cannot use the standard KDE API directly.
 *
 * @see QNetworkAccessManager::setCookieJar for details.
 *
 * @author Dawit Alemayehu \<adawit @ kde.org\>
 * @since 4.4
 */
class KIO_EXPORT CookieJar : public QNetworkCookieJar
{
    Q_OBJECT
public:
    /**
     * Constructs a KNetworkCookieJar with parent @p parent.
     */
    explicit CookieJar(QObject *parent = 0);

    /**
     * Destroys the KNetworkCookieJar.
     */
    ~CookieJar();

   /**
    * Returns the currently set window id. The default value is -1.
    */
    WId windowId() const;

    /**
     * Sets the window id of the application.
     *
     * This value is used by KDE's cookiejar to manage session cookies, namely
     * to delete them when the last application referring to such cookies is
     * closed by the end user.
     *
     * @see QWidget::window()
     * @see QWidget::winId()
     *
     * @param id the value of @ref QWidget::winId() from the window that contains your widget.
     */
    void setWindowId(WId id);

    /**
     * Reparse the KDE cookiejar configuration file.
     */
    void reparseConfiguration();

    /**
     * Reimplemented for internal reasons, the API is not affected.
     *
     * @see QNetworkCookieJar::cookiesForUrl
     * @internal
     */
    QList<QNetworkCookie> cookiesForUrl(const QUrl &url) const;

    /**
     * Reimplemented for internal reasons, the API is not affected.
     *
     * @see QNetworkCookieJar::setCookiesFromUrl
     * @internal
     */
    bool setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url);

    /**
     * Returns true if persistent caching of cookies is disabled.
     * 
     * @see setDisableCookieStorage
     * @since 4.6
     */
    bool isCookieStorageDisabled() const;

    /**
     * Prevent persistent storage of cookies.
     * 
     * Call this function if you do not want cookies to be stored locally for
     * later access without disabling the cookiejar. All cookies will be discarded
     * once the sessions that are using the cookie are done.
     * 
     * @since 4.6
     */
    void setDisableCookieStorage (bool disable);

private:
    class CookieJarPrivate;
    CookieJarPrivate* const d;
};

}

}

#endif // KIO_ACCESSMANAGER_H
