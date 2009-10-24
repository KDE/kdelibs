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

namespace KIO {

/**
 * @short A KDE implementation of QNetworkAccessManager.
 *
 * Use this class instead of QNetworkAccessManager if you want to integrate
 * with KDE's KIO module for network operations.
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
 * Please note that this class is in the KIO namespace for backward compatiblity.
 * You should use KIO::Integration::AccessManager to access this class in your
 * code.
 *
 * <b>IMPORTANT</b>This class is not a replacement for the standard KDE API.
 * It should ONLY be used to to provide KDE integration in applications that
 * cannot use the standard KDE API directly.
 *
 * @author Urs Wolfer \<uwolfer @ kde.org\>
 *
 * @deprecated Use the KIO::Integration::AccessManager typedef to access this class instead.
 * @since 4.3
 */
class KIO_EXPORT AccessManager : public QNetworkAccessManager
{
    Q_OBJECT
public:
    /*!
      Extensions to QNetworkRequest::Attribute enums.
      @since 4.3.2
    */
    enum Attribute {
        MetaData = QNetworkRequest::User, /** < Used to send KIO MetaData back and forth. type: QVariant::Map. */
        KioError /**< Used to send KIO error codes that cannot be mapped into QNetworkReply::NetworkError. type: QVariant::Int */
    };

    AccessManager(QObject *parent);

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
 * @author Dawit Alemayehu <adawit @ kde.org>
 * @since 4.3.3
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
    qlonglong windowId() const;

    /**
     * Sets the window id of the application.
     *
     * This value is used by KDE's cookiejar to manage session cookies, namely
     * to delete them when the last application refering to such cookies is
     * closed by the end user.
     *
     * @see QWidget::window()
     * @see QWidget::winId()
     *
     * @param id the value of @ref QWidget::winId() from the window that contains your widget.
     */
    void setWindowId(qlonglong id);

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

private:
    class CookieJarPrivate;
    CookieJarPrivate* const d;
};

}

}

#endif // KIO_ACCESSMANAGER_H
